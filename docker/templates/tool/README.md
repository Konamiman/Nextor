# @@NAME@@

A Nextor-aware MSX-DOS command (`.COM`), scaffolded by `nextor-init tool`.
It starts as a minimal program that prints a banner and exits; extend it to do
real work with DOS and Nextor function calls.

## What's here

| File | Purpose |
|------|---------|
| `tool.asm` | The program. `org 0100h` makes the output a ready-to-run `.COM`. |
| `Makefile` | Builds `@@NAME@@.COM` with N80 against the SDK. |
| `.devcontainer/` | VS Code Dev Container config pointing at the Nextor dev image. |

## Build

Inside the Nextor dev container (this directory mounted at `/work`):

```sh
make
```

This produces **`@@NAME@@.COM`**, which you can copy to a Nextor/MSX-DOS volume
and run. From your host:

```sh
docker run --rm -v "$PWD":/work @@IMAGE@@ make
```

## Going further

The SDK on `$NEXTOR_SDK` has everything you need:

- `asm/constants/dos_calls.inc` — DOS/Nextor function numbers (`_STROUT`,
  `_FOPEN`, `_TERM`, …).
- `asm/constants/dos_errors.inc` — error codes.
- `asm/constants/msx_bios.inc`, `msx_workarea.inc` — BIOS entry points and
  system work-area addresses.
- `asm/code/` — drop-in routines: `chk_nextor.asm` (verify Nextor is running),
  `output_string.asm`, `extpar.asm` (parse command-line parameters), and more.

To make the tool refuse to run on a non-Nextor system, assemble in
`chk_nextor.asm` and `call CHK_NEXTOR` at the top of `START`.
