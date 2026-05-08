# SDK Extraction — Analysis

Scope: scan of the Nextor codebase to identify constants and code that are good
candidates for an extracted SDK (public + private, assembler + C). Only v3
drivers (those carrying the `NEXTORv3_DRIVER` signature) were considered:
`StandaloneASCII8`, `SunriseIDE`, `MegaFlashRomSD`, plus `RAMDisk` and
`TurboRFDD` (the latter two were matched by the signature grep but were not
inventoried in detail in this pass — see Question 1).

---

## 1. Already centralized (do not re-extract)

| File | Contents | Likely SDK home |
|---|---|---|
| `source/kernel/codes.mac` | BDOS function codes (00h–7Fh), BDOS/disk error codes | Public, asm |
| `source/tools/SHARED.MAC` | Tool-side helpers: `EXTPAR`, `BYTE2ASC`, etc. | Public, asm |
| `source/kernel/kvar.mac` | UD/DPB field offsets, kernel workspace fields | Private, asm |
| `source/kernel/data.mac` | Page-3 kernel variables, JUMPBASE/HOOKBEG | Private, asm |
| `source/kernel/print_call_help.mac` | CALL-help formatted printer | Stays in kernel — not SDK material |

---

## 2. Duplicated across v3 drivers

These are copy-pasted into every v3 driver. Each driver carries roughly
40 lines of identical boilerplate for the page-1 kernel entry block, plus
its own redefinitions of query codes and a subset of disk error codes.

### 2.1 Page-1 kernel entry points (CALBNK helpers)

Identical in `StandaloneASCII8/driver.mac`, `SunriseIDE/driver.mac`,
`MegaFlashRomSD/mfrsd.asm`:

| Symbol | Value | Purpose |
|---|---|---|
| `GSLOT1` | 402Dh | Get current slot for page 1 |
| `RDBANK` | 403Ch | Read byte from another bank |
| `CALBAS` | 403Fh | Jump into BIOS preserving kernel bank |
| `CALBNK` | 4042h | Call routine in another bank |
| `GWORK`  | 4045h | Get slot work area address |

### 2.2 Driver query codes

Identical values across drivers; symbol set varies (see inconsistencies §6):

| Symbol | Value |
|---|---|
| `QUERY_OK` | 0 |
| `QUERY_TRUNCATED_STRING` | 1 |
| `QUERY_INVALID_DEVICE` | 2 |
| `QUERY_NOT_IMPLEMENTED` | 0FFh |
| `DRVQ_GET_VERSION` | 1 |
| `DRVQ_GET_STRING` | 2 |
| `DRVQ_GET_INIT_PARAMS` | 3 |
| `DRVQ_INIT` | 4 |
| `DRVQ_GET_MAX_DEVICE` | 5 |
| `DRVQ_INIT_RAM` | 6 |
| `DEVQ_GET_STRING` | 1 |
| `DEVQ_GET_PARAMS` | 2 |
| `DEVQ_GET_STATUS` | 3 |
| `DEVQ_GET_AVAILABILITY` | 4 |
| `DEVQ_GET_FORMAT_CHOICES` | 5 |
| `DEVQ_DO_FORMAT` | 6 |
| `DEVQ_STOP_MOTOR` | 7 |

Kernel-side authoritative definitions live in `source/kernel/data.mac`
(lines ~538–561, inside the `DRIVER` module).

### 2.3 Disk error codes (subset redefined locally)

`.NCOMP`, `.WRERR`, `.DISK`, `.NRDY`, `.DATA`, `.RNF`, `.WPROT`, `.UFORM`,
`.SEEK`, `.IFORM` — all match the values in `codes.mac`, just redeclared in
each driver.

### 2.4 MSX BIOS routine addresses (partial sets)

| Symbol | Value | Where redeclared |
|---|---|---|
| `RDSLT` | 000Ch | `data.mac`, all v3 drivers |
| `WRSLT` | 0014h | `data.mac` |
| `CALSLT` | 001Ch | `data.mac`, `print_call_help.mac`, `drv.mac` |
| `ENASLT` | 0024h | `data.mac`, all v3 drivers |
| `CHGET` | 009Fh | `print_call_help.mac`, `Flashjacks`, `MegaFlashRomSD`, `misc.mac` |
| `CHPUT` | 00A2h | `Flashjacks`, `misc.mac` (others call by literal) |

---

## 3. Duplicated across the C tools (`source/command/`)

Found by comparing `xcopy.h` and `xdir.c`.

### 3.1 FIB layout

| Offset | Symbol(s) used | Value |
|---|---|---|
| 1  | `FIB_FILE_NAME`, `FIB_NAME` | 1 |
| 14 | `FIB_ATTRIBUTES` | 14 |
| 19 | `FIB_CLUSTER` | 19 |
| 21 | `FIB_FSIZE`, `FIB_SIZE` | 21 |
| 25 | `FIB_DRIVE` | 25 |
| 64 | `FIB_LENGTH` | 64 |

### 3.2 File attribute bitmasks

`MASK_R_ONLY` / `MASK_READ_ONLY` (0x01), `MASK_HIDDEN` (0x02),
`MASK_SYSTEM` (0x04), `MASK_SUB_DIR` (0x10), `MASK_ARCHIVE` (0x20),
`MASK_DEVICE` (0x80).

### 3.3 BDOS error C aliases

`FE_NO_FILE` (0xD7), `FE_DIR_EXISTS` (0xCC), `FE_FILE_EXISTS` (0xCB),
`FE_R_ONLY` (0xD1), `FE_IOPT` (0x88), `FE_IPARM` (0x8B), `FE_NORAM` (0xDE),
`FE_FOPEN` (0xCA), `FE_PLONG` (0xD8), `FE_IFNM` (0xDA), `FE_IDEV` (0xC1).

All values match `codes.mac`; this is just the C-cased mirror.

---

## 4. Reusable code snippets

| Snippet | Where | SDK home |
|---|---|---|
| `EXTPAR` (command-line param extraction) | `source/tools/SHARED.MAC` | Public, asm |
| `BYTE2ASC` (8-bit → decimal string) | `source/tools/SHARED.MAC` | Public, asm |
| `allocate` / `deallocate` (heap pointer wrappers) | `source/command/msxlib.c` | Public, C |
| `print_call_help` formatter | `source/kernel/print_call_help.mac` | Stays in kernel |
| `chgbnk.mac` | One per driver, hardware-specific bit shuffling | Public template only |
| C `printf` | Mentioned in PLAM.md but not located in `source/command/` — see Question 6 | TBD |

---

## 5. Kernel-internal items (private SDK candidates)

These aren't safe to expose to driver/tool authors but are worth
extracting so kernel banks share a single source of truth.

- **MSX system work area** (`EXPTBL` 0FCC1h, `SLTATR` 0FCC9h,
  `SLTWRK` 0FD09h, `INTFLG` 0FC9Bh, `ESCCNT` 0FCA7h, `BOTTOM` 0FC48h,
  `HIMEM` 0FC4Ah, `KBUF` 0F41Fh, `RAMAD0`–`RAMAD3` F341–F344) — currently
  defined in `data.mac`, with a few redeclared in drivers. Drivers should
  not be poking these directly, so they belong in the private SDK; the
  kernel includes them, drivers shouldn't.
- **DPB layout** — defined in `kvar.mac` / referenced by name in many
  banks.
- **Unit Descriptor (UD) layout** — same.
- **Page-3 kernel variables / JUMPBASE / HOOKBEG** — currently in
  `data.mac`.

---

## 6. Inconsistencies surfaced by the scan

1. **`QUERY_INIT_ERROR` (=3) vs `QUERY_NOT_IMPLEMENTED` (=0FFh)** —
   `StandaloneASCII8` defines both for similar purposes; other drivers
   only define `QUERY_NOT_IMPLEMENTED`. The SDK should pick one.
2. **DEVQ codes 4–7 missing in `SunriseIDE` / `MegaFlashRomSD`** — only
   `StandaloneASCII8` declares `DEVQ_GET_AVAILABILITY` through
   `DEVQ_STOP_MOTOR`. The codes themselves are kernel contract; missing
   declarations are just gaps.
3. **`CHPUT` / `CHGET`** — sometimes equated locally, sometimes called by
   literal address.
4. **`EXPTBL` / `SLTWRK`** — redeclared in drivers despite being already
   in `data.mac`.
5. **FIB symbol naming** — `FIB_FILE_NAME` vs `FIB_NAME`, `FIB_FSIZE` vs
   `FIB_SIZE`. The SDK should pick canonical names.

---

## 7. Proposed directory tree (draft for discussion)

```
sdk/
├── public/
│   ├── asm/
│   │   ├── bdos_calls.inc      # _TERM0 ... _DRVRO   (from codes.mac)
│   │   ├── bdos_errors.inc     # .NCOMP, .NRDY, ...  (from codes.mac)
│   │   ├── driver_queries.inc  # DRVQ_*, DEVQ_*, QUERY_*
│   │   ├── driver_calls.inc    # GSLOT1, RDBANK, CALBAS, CALBNK, GWORK + doc
│   │   ├── msx_bios.inc        # RDSLT, CALSLT, ENASLT, CHGET, CHPUT, ...
│   │   ├── chgbnk_template.inc # commented skeleton, hardware-specific body TBD
│   │   └── tool_macros.inc     # EXTPAR, BYTE2ASC, ...   (from SHARED.MAC)
│   └── c/
│       ├── bdos_functions.h / bdos_errors.h       # function codes + FE_* error codes
│       ├── nextor_fib.h        # FIB offsets + attribute masks
│       └── nextor_lib.{c,h}    # allocate / deallocate ; printf if shared
└── private/
    └── asm/
        ├── msx_workarea.inc    # EXPTBL, SLTWRK, RAMAD0-3, ...
        ├── kernel_workarea.inc # page-3 kernel vars (subset of data.mac)
        ├── unit_descriptor.inc # UD_* offsets
        └── dpb.inc             # DPB offsets
```

Notes on this draft:

- `chgbnk.mac` is **not** unified — each driver's logic is genuinely
  hardware-specific. The SDK only ships a documented skeleton.
- Per-driver bank-switch register addresses (e.g. `IDE_BANK = 4104h`) are
  not SDK material; they belong with the driver.
- The split assumes the public SDK becomes a git submodule consumed by
  external driver/tool repos. Submodule include-path conventions affect
  how the tree should ultimately be flattened (Question 10).

---

## 8. Open questions

1. **Driver coverage** — should the SDK fully cover `RAMDisk` and
   `TurboRFDD` as well? Quick grep says they carry the v3 signature; the
   detailed inventory pass focused on the other three.
2. **MSX BIOS scope** — only the entry points Nextor itself uses, or the
   full MSX BIOS spec (hundreds of routines) so external developers get
   a complete reference?
3. **MSX work-area scope** — same question for system-variable
   addresses.
4. **`codes.mac` migration** — move the file bodily into
   `sdk/public/asm/` and have the kernel `include` it from there (single
   source of truth, but kernel now depends on `sdk/`), or duplicate (no
   cross-tree includes, but two copies to keep in sync)?
5. **Kernel migration timing** — refactor the kernel to consume the new
   SDK as part of this change, or land the SDK first and migrate the
   kernel later?
6. **C `printf`** — the scan did not find a shared `printf` in
   `source/command/`. Is it in `source/lib/konamiman-sdcc/`? If that
   directory is already an external library, the C SDK should probably
   reference it rather than absorb it.
7. **`chgbnk.mac`** — confirm the SDK ships a *commented template* only
   (no working routine).
8. **Canonical "not implemented" return** — adopt `QUERY_NOT_IMPLEMENTED`
   (=0FFh) and drop `QUERY_INIT_ERROR` (=3)?
9. **Include file extension** — `.inc` (matches existing kernel style)
   or something else?
10. **Submodule layout** — when an external repo pulls the public SDK as
    a submodule, should the include path be `sdk/public/asm/...`, or
    should the submodule's root *be* the public area so includes look
    like `asm/...`?
11. **Tool macros (`SHARED.MAC`)** — public SDK (useful to any MSX-DOS
    tool author) or Nextor-specific? They aren't Nextor-coupled.
12. **Symbol naming style** — keep current mix verbatim (`.NCOMP`,
    `?BDOS`, `_TERM0`) for compatibility, or normalize?
