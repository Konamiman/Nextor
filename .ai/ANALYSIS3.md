# SDK Extraction — Concrete Plan

## Confirmed during scan

- **`asm.h` and `asm.lib`** — zero references in any source file (the only hit
  is a stale comment in `bank5/fdisk.c` line 8). Both will be dropped.
- **Driver workarea candidates** — only one v3 driver (RAMDisk) actually pokes
  page-3 kernel vars directly. The minimum set drivers need today is
  `BK4_ADD`, `TMP_IX`, `TMP_IY`, `MASTER_SLOT`. Reasonable additions
  (probably useful to driver authors even if no current driver uses them):
  `DOSFLG`, `$NUMDRV`, `DOS_VER`, `NXT_VER`, `DRVTBL`, `MAIN_BANK`. (User to
  trim/extend before freeze.)

## Final concrete tree

```
sdk/                                  # public SDK (git submodule target)
├── asm/
│   ├── bdos_calls.inc                # _TERM0 ... _DRVRO   (from codes.mac §1)
│   ├── bdos_errors.inc               # .NCOMP, .NRDY, ... (from codes.mac §2)
│   ├── bdos_file_handles.inc         # STDIN, STDOUT, STDERR, STDAUX, STDLST
│   ├── rom_bank_header.inc           # GSLOT1, RDBANK, CALBAS, CALBNK, GWORK, CALDRV
│   ├── driver_routines.inc           # NEXTORv3_DRIVER signature, jump-table layout
│   ├── driver_driver_queries.inc     # DRVQ_*
│   ├── driver_device_queries.inc     # DEVQ_*
│   ├── driver_error_codes.inc        # QUERY_OK, QUERY_TRUNCATED_STRING,
│   │                                 #   QUERY_INVALID_DEVICE, QUERY_INIT_ERROR,
│   │                                 #   QUERY_NOT_IMPLEMENTED
│   ├── driver_workarea.inc           # BK4_ADD, TMP_IX, TMP_IY, MASTER_SLOT,
│   │                                 #   DOSFLG, $NUMDRV, DOS_VER, NXT_VER,
│   │                                 #   DRVTBL, MAIN_BANK   (review this list)
│   ├── msx_bios.inc                  # driver-relevant subset: RDSLT, CALSLT,
│   │                                 #   ENASLT, CHGET, CHPUT, INTRPT
│   ├── msx_workarea.inc              # driver-relevant subset: EXPTBL, SLTATR,
│   │                                 #   SLTWRK, RAMAD0..3
│   ├── chgbnk_template.inc           # documented skeleton (label + doc block)
│   └── shared.inc                    # EXTPAR, BYTE2ASC, ... (from SHARED.MAC)
└── c/
    ├── crt0_msxdos_advanced.asm        # moved from source/tools/C/
    ├── asmcall.c                     # moved
    ├── asmcall.h                     # moved
    ├── printf.c                      # moved
    ├── print_msxdos.c                # moved
    ├── strcmpi.c                     # moved
    ├── strcmpi.h                     # moved
    ├── dos.h                         # moved
    ├── system.h                      # moved
    ├── types.h                       # moved
    ├── partit.h                      # moved
    ├── bdos_functions.h / bdos_errors.h                 # NEW — function codes + error codes
    └── nextor_fib.h                  # NEW — FIB_* offsets + MASK_* attribute masks

private_sdk/
└── asm/
    ├── msx_bios.inc                  # kernel-only additions: WRSLT, ID_BYTE,
    │                                 #   CALLF, SSLOT/SSLOTL/SSLOTE, KINIT,
    │                                 #   KBDOS, $ALL_SEG, $FRE_SEG, KCONIN,
    │                                 #   KCONOUT, KCONST, KLIST, KLISTST,
    │                                 #   KPUNCH, KREADER
    ├── msx_workarea.inc              # kernel-only: BOTTOM, HIMEM, INTFLG,
    │                                 #   ESCCNT, SLTTBL, KBUF, BUF
    ├── kernel_workarea.inc           # kernel-private page-3 vars (the var3
    │                                 #   routine vectors, HOOKBEG, MFLAGS bits,
    │                                 #   etc. — all the kernel-internal
    │                                 #   contents of data.mac that aren't a
    │                                 #   layout construct)
    ├── unit_descriptor.inc           # UD_* offsets (extracted from kvar.mac)
    └── dpb.inc                       # DPB offsets (extracted from kvar.mac)
```

## Decisions

1. **Driver workarea list above is a proposal** — confirmed yes, use as-is
   (subject to user trim/extend later).
2. **`data.mac` cannot be fully replaced.** Most of `data.mac` builds the
   kernel's page-3 memory layout via the `var0`/`var1`/`var2`/`var3` macros —
   that's a layout construct, not just a constants file. The private-SDK files
   extract the *static address constants* (things like
   `MASTER_SLOT equ 0F348h`); the layout-building part of `data.mac` stays in
   the kernel. Confirmed.
3. **`const` macro change.** `macros.inc` will be modified so `const NAME, VAL`
   only emits `public NAME` when `GENERATE_PUBLIC_SYMBOLS` is defined. Kernel
   Makefile gets `--define-symbols GENERATE_PUBLIC_SYMBOLS` added. Confirmed.
4. **`bdos_functions.h / bdos_errors.h` scope and out-of-scope tools.** `xcopy`, `xdir`, and
   anything else in `source/command/` *except* `msxdos` are inherited from the
   MSX-DOS project and stay untouched — their local `FE_*` aliases and `FIB_*`
   / `MASK_*` definitions remain duplicated, no migration. The new
   `bdos_functions.h / bdos_errors.h` and `nextor_fib.h` are aimed at Nextor's own C code
   (`bank5/fdisk.c`, the tools under `source/tools/C/`, `source/command/msxdos`,
   and any new external tools). Confirmed.
5. **`QUERY_INIT_ERROR` and `QUERY_NOT_IMPLEMENTED`** both live in
   `driver_error_codes.inc` (they mean different things). Confirmed.
6. **C naming for BDOS function codes in `bdos_functions.h / bdos_errors.h`** — keep the `_NAME`
   convention verbatim from asm (e.g. `_TERM0`, `_FOPEN`). Confirmed.
