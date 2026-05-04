# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Nextor is a disk operating system for MSX computers, evolved from MSX-DOS 2.31. The kernel is a 16KB banked ROM written in Z80 assembly, assembled with the Nestor80 toolchain (N80 assembler, LK80 linker). Bank 5 contains FDISK written in C (compiled with SDCC).

## Build Commands

All builds run from `source/kernel/`:

```bash
make nextor_base.dat    # Build the kernel base ROM (all banks)
make clean              # Remove all build artifacts
make bank4/B4.BIN       # Build a single bank (useful for iteration)
```

To build a complete ROM with a driver:
```bash
mknexrom nextor_base.dat output.ROM /d:drivers/SunriseIDE/sunride.bin /m:drivers/SunriseIDE/chgbnk.bin
```

The assembler has no label name length restrictions.

## Architecture

### Bank Structure

The kernel uses 7 ROM banks (16KB each at 4000h-7FFFh):

- **Bank 0**: BDOS entry, DOS bootstrap, initialization, BASIC CALL command dispatcher (`dskbasic.mac`), disk driver entry points (`doshead.mac`)
- **Bank 1**: DOS initialization (`dosinit.mac`), mapper initialization (`mapinit.mac`)
- **Bank 2**: Filesystem operations — character I/O, path handling, directory ops, file handles, FAT management, sector buffer management, disk I/O dispatch (`val.mac`)
- **Bank 3**: DOS 1 backward-compatibility kernel (`dos1ker.mac`)
- **Bank 4**: Partition management (`partit.mac`), environment variables, CALL command implementations (MAPDRV, DRIVERS, DRVINFO, etc.), driver registration
- **Bank 5**: FDISK tool (C via SDCC)
- **Bank 6**: FORMAT tool, ghost drive setup, boot messages, CALL IDRIVER/UDRIVER implementations (`idrvauto.mac`)

### Fixed Header (doshead.mac, 4000h-40FFh)

Identical in all banks. Contains critical entry points:
- **CALBNK (4042h)**: Cross-bank call — A=target bank, IX=routine address, AF'=AF for routine, BC/DE/HL/IY passed through
- **CALLB0 (403Fh)**: Call routine in bank 0 (MAIN_BANK) via BK4_ADD
- **C4PBK (4092h)**: Call bank 4 or bank 6 — reads address from BK4_ADD, bit 15 of BK4_ADD selects bank 6

### Memory Pages

- **Page 0 (0000h-3FFFh)**: BIOS ROM normally; kernel RAM during BDOS calls
- **Page 1 (4000h-7FFFh)**: Bank-switched kernel ROM
- **Page 2 (8000h-BFFFh)**: Kernel data segment (DATA_SEG) or BASIC RAM (P2_64K)
- **Page 3 (C000h-FFFFh)**: Always accessible — system variables, BUF (F55Eh), PROCNM (FD89h)

### CALL Command Dispatch

The COMMAND table in `dskbasic.mac` maps command names to addresses. The dispatch checks the high byte of the address:
- **D=41h**: Bank 4 jump table entry → `CALBNK` with A=4
- **D=C1h**: Bank 6 jump table entry (bit 15 set) → `CALBNK` with A=6
- **Other**: Direct call in bank 0

This requires bank 4 and 6 jump tables to stay within 4100h-41FFh (max 85 entries each).

### Jump Tables

Each bank has a jump table at 4100h defined in `bankN/jtable.inc` (shared with `data.mac` for page 3 variable generation). Adding an entry: add `jentry NAME` to the jtable, declare `public ?NAME` and `?NAME:` label in the implementing `.mac` file.

### Shared Code Across Banks

`print_call_help.mac` is assembled once and linked into both bank 4 and bank 6. Referenced as `PRINT_CALL_HELP##` (external). This pattern can be used for other shared routines.

## Key Files

- `macros.inc` — Assembly macros (`pcall`, `printchar`, `const`, `var`, `field`)
- `const.inc` — Constants (MAX_UNITS=8, error codes)
- `data.mac` — Page 3 variables (`var3` for always-accessible, `var2` for page 2)
- `kvar.mac` — Kernel workspace variables, unit descriptor (UD) field definitions
- `codes.mac` — DOS function codes and error codes
- `bank0/doshead.mac` — Bank switching infrastructure (CALBNK, CALLB0, C4PBK)

## Critical Pitfalls

### Bank 4 doshead JP entries
Never call doshead JP entries (GSLOT1 at 402Dh, etc.) from bank 4. They contain `jp XXXX##` targeting bank 0 addresses, but with bank 4 paged, the jump lands in bank 4 code. Only BIOS page 0 calls (RST, RDSLT, CALSLT) and CALLB0/C4PBK are safe.

### Page 0 during BDOS calls
During BDOS processing, page 0 = kernel RAM, not BIOS ROM. CALROM and direct BIOS calls through page 0 addresses fail silently. Use `P0_CALL##` (page 3 vector) for BIOS I/O during BDOS calls.

### pcall from BASIC context
`pcall` calls bank 2 routines via page 0 kernel RAM. This only works when page 0 has kernel RAM (during BDOS processing). From BASIC CALL command context, page 0 has BIOS ROM — pcall crashes.

### CALL_MAP from bank 6
`CALL_MAP##` (page 3 routine for calling RAM driver code) does not properly restore the ROM bank when called from bank 6. Driver queries from bank 6 code must dispatch to bank 4 via CALBNK, where CALL_MAP works correctly.

### BUF (F55Eh) overlap
BUF is the BASIC line input buffer. `F_DRVRO` and automap code write parameters to BUF+0 through BUF+13. Ensure the BASIC text pointer (HL) points past any overwritten area before returning to BASIC.

### UD_DFLAGS bit names
Use `UF_FDD` (not `UF_FD`) for the floppy bit, `UFM_FD` for the mask. Wrong names cause "different values in pass 1/2" assembler errors (treated as undefined forward references).

### Page 3 safe areas for new variables
- **Avoid**: F1C9-F236 (DOS 1 LDIR overwrites), F300-F30D (DOS 1 reuses)
- **Safe**: F237-F2B7, F327+ (after AUXBODY)

## Code Conventions

- `##` suffix on symbols = external reference (resolved by linker)
- `::` suffix on labels = public (exported to other modules)
- `db`/`defw` for data in bank 0; `defb`/`defw` with single quotes in bank 6
- PRINT_CALL_HELP special chars in help strings: 1=space/newline, 2=no-space/newline, 3=pause(40-col only)
- `rst 18h` for character output (BIOS OUTDO, works from any bank)
- CHPUT in bank 4 adds Ctrl-STOP check for LF; `rst 18h` is simpler and bank-independent

## String Rules

Do not shorten, rephrase, or otherwise modify user-facing strings (help text, error messages, status messages) without explicit approval.
