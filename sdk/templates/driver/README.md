# Nextor driver template

A starter [Nextor](https://github.com/Konamiman/Nextor) disk driver. It builds
into a bootable kernel ROM that you can run on a real MSX (on a suitable flash
cartridge) or in an emulator. Copy this directory, follow the `TODO` comments,
run `make`.

The template builds as-is: it starts as the *standalone dummy driver*, a no-op
driver that boots with no hardware attached.

It can also build a **RAM-loadable driver** (a `.drv` file, loaded at runtime
with `CALL IDRIVER`): set `RAM_DRIVER` to 1 in `driver.asm` and assemble that
file directly — `N80 driver.asm mydriver.drv --include-directory <path to the
SDK>`. The `Makefile`, `chgbnk.asm` and the kernel base file only apply to ROM
drivers.

## What's here

| File | Purpose |
|------|---------|
| `driver.asm` | The driver itself. Replace the handler bodies (see the `TODO`s) with your device logic. |
| `chgbnk.asm` | Bank-switching module for the **ASCII8** mapper. Swap for the SDK's `asm/chgbnk/ascii16.asm` if your cartridge uses ASCII16. |
| `Makefile` | Builds the ROM from the two sources plus a kernel base file. |
| `.devcontainer/` | VS Code Dev Container config pointing at the Nextor dev image (optional, harmless otherwise). |

## Build

### With the Nextor dev Docker image (no toolchain on your host)

The image carries the toolchain, the SDK and the kernel base files, all
preconfigured — and a copy of this template at `$NEXTOR_SDK/templates/driver`:

```sh
mkdir my-driver && cd my-driver
docker run --rm -v "$PWD":/work ghcr.io/konamiman/nextor-dev \
    sh -c 'cp -R "$NEXTOR_SDK"/templates/driver/. .'
docker run --rm -v "$PWD":/work ghcr.io/konamiman/nextor-dev make
```

See `docker/README.md` in the Nextor repository for the full story (shell
alias, interactive sessions, file ownership, ...).

### Without Docker

You need [Nestor80](https://github.com/Konamiman/Nestor80)'s `N80` and
Nextor's `mknexrom` on your PATH, plus two things the Makefile must find
(see the comments at its top):

- **The SDK** (`NEXTOR_SDK`): defaults to `../..`, which works while this
  directory sits inside a Nextor checkout; point it at your SDK location if
  you copied the template elsewhere.
- **A kernel base file** (`NEXTOR_BASE`): build one (`make` in `source/kernel`
  of a Nextor checkout leaves them in `bin/kernel-base/`) or take the
  `Nextor-X.Y.Z.base.dat` file from a Nextor release.

```sh
make NEXTOR_SDK=path/to/Nextor/sdk NEXTOR_BASE=path/to/Nextor-X.Y.Z.base.dat
```

## Writing the driver

The jump table near the top of `driver.asm` is the driver's ABI: the kernel
calls these entry points. Of note:

- `DRIVER_QUERY` / `DEVICE_QUERY` — describe the driver and its devices.
- `READ_WRITE` — the heart of a storage driver: transfer sectors.
- `DIRECT_0..4` — driver-specific routines you can call from your own tools.

The constants and helper routines you'll need are in the SDK:
`asm/constants/driver_*.inc`, `asm/constants/dos_*.inc`, and ready-made
snippets under `asm/code/`. The full contract is in the **Nextor Driver
Development Guide** (`docs/` in the Nextor repository).

## Kernel variants

`NEXTOR_BASE` selects which kernel your driver is fused with. There are six
base-file variants (in the dev image they live in `/opt/nextor/kernel_base/`;
the repository build names them `Nextor-X.Y.Z.base[<suffix>].dat`):

```text
kernel_base.dat                    (default)
kernel_base.NO_UNDOC.dat           (Z180-safe, no undocumented opcodes)
kernel_base.SHIFT_INV.dat          kernel_base.CTRL_INV.dat
kernel_base.NO_UNDOC.SHIFT_INV.dat kernel_base.NO_UNDOC.CTRL_INV.dat
```

For a `.NO_UNDOC.` base, build with `make NO_UNDOC=1 NEXTOR_BASE=...` so the
driver stays undoc-free too. Use the `ld_`, `cp_`, `or_`, … macros from
`undoc.inc` (already included) anywhere you'd otherwise touch `ixh/ixl/iyh/iyl`.
