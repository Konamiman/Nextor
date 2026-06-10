# @@NAME@@

A [Nextor](https://github.com/Konamiman/Nextor) disk driver, scaffolded by
`nextor-init driver`. It builds into a bootable kernel ROM that you can run on
a real MSX (on a suitable flash cartridge) or in an emulator.

## What's here

| File | Purpose |
|------|---------|
| `driver.asm` | The driver itself. Starts as the *standalone dummy driver* (boots with no hardware); replace the handler bodies with your device logic. |
| `chgbnk.asm` | Bank-switching module for the **ASCII8** mapper. Swap for `ascii16` (see `$NEXTOR_SDK/asm/chgbnk/`) if your cartridge uses it. |
| `Makefile` | Builds `@@NAME@@.ROM` from the two sources plus the kernel base file. |
| `.devcontainer/` | VS Code Dev Container config pointing at the Nextor dev image. |

## Build

Inside the Nextor dev container (this directory mounted at `/work`):

```sh
make
```

This produces **`@@NAME@@.ROM`**. From your host, that's typically:

```sh
docker run --rm -v "$PWD":/work @@IMAGE@@ make
```

## Writing the driver

The jump table near the top of `driver.asm` is the driver's ABI: the kernel
calls these entry points. Of note:

- `DRIVER_QUERY` / `DEVICE_QUERY` — describe the driver and its devices.
- `READ_WRITE` — the heart of a storage driver: transfer sectors.
- `DIRECT_0..4` — driver-specific routines you can call from your own tools.

The constants and helper routines you'll need are in the SDK on `$NEXTOR_SDK`:
`asm/constants/driver_*.inc`, `asm/constants/dos_*.inc`, and ready-made snippets
under `asm/code/`. The full contract is in the **Nextor Driver Development
Guide** (`docs/` in the Nextor repository).

## Kernel variants

`NEXTOR_BASE` selects which kernel your driver is fused with. The image ships
six base files in `/opt/nextor/kernel_base/`:

```
kernel_base.dat                    (default)
kernel_base.NO_UNDOC.dat           (Z180-safe, no undocumented opcodes)
kernel_base.SHIFT_INV.dat          kernel_base.CTRL_INV.dat
kernel_base.NO_UNDOC.SHIFT_INV.dat kernel_base.NO_UNDOC.CTRL_INV.dat
```

For a `.NO_UNDOC.` base, build with `make NO_UNDOC=1 NEXTOR_BASE=...` so the
driver stays undoc-free too. Use the `ld_`, `cp_`, `or_`, … macros from
`undoc.inc` (already included) anywhere you'd otherwise touch `ixh/ixl/iyh/iyl`.
