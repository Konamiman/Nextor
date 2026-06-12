# Nextor tool template

A starter Nextor-aware MSX-DOS command (`.COM`). It begins as a minimal program that prints a banner and exits; extend it to do real work with DOS and Nextor function calls. Copy this directory, follow the `TODO` comments, run `make`.

## What's here

| File | Purpose |
|------|---------|
| `tool.asm` | The program. `org 0100h` makes the output a ready-to-run `.COM`. |
| `Makefile` | Builds the `.COM` with N80 against the SDK. |
| `.devcontainer/` | VS Code Dev Container config pointing at the Nextor dev image (optional, harmless otherwise). |

## Build

### With the Nextor dev Docker image (no toolchain on your host)

The image carries N80 and the SDK, preconfigured - and a copy of this template at `$NEXTOR_SDK/templates/tool`:

```sh
mkdir my-tool && cd my-tool
docker run --rm -v "$PWD":/work ghcr.io/konamiman/nextor-dev \
    sh -c 'cp -R "$NEXTOR_SDK"/templates/tool/. .'
docker run --rm -v "$PWD":/work ghcr.io/konamiman/nextor-dev make
```

See `docker/README.md` in the Nextor repository for the full story.

### Without Docker

You need [Nestor80](https://github.com/Konamiman/Nestor80)'s `N80` on your PATH, and the Makefile must find the SDK: `NEXTOR_SDK` defaults to `../..`, which works while this directory sits inside a Nextor checkout; point it at your SDK location if you copied the template elsewhere:

```sh
make NEXTOR_SDK=path/to/Nextor/sdk
```

The resulting `.COM` runs on Nextor or MSX-DOS: copy it to a volume and type its name.

## Going further

The SDK has everything you need:

- `asm/constants/dos_calls.inc`: DOS/Nextor function numbers (`_STROUT`, `_FOPEN`, `_TERM`, …).
- `asm/constants/dos_errors.inc`: error codes.
- `asm/constants/msx_bios.inc`, `msx_workarea.inc`: BIOS entry points and system work-area addresses.
- `asm/code/`: drop-in routines: `chk_nextor.asm` (verify Nextor is running), `output_string.asm`, `extpar.asm` (parse command-line parameters), and more.

To make the tool refuse to run on a non-Nextor system, assemble in `chk_nextor.asm` and `call CHK_NEXTOR` at the top of `START`.
