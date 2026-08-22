# Nextor

Nextor is a disk operating system for MSX computers. It is built on top of the source code of MSX-DOS 2.31, released in 1991.

If you are already familiar with Nextor 2 you may want to take a look at [what's new in Nextor 3](docs/Nextor%203.0%20What's%20New.md). If you are new to Nextor [the getting started guide](docs/Nextor%203.0%20Getting%20Started%20Guide.md) will be useful for you.

The source code of Nextor is published with permission from the MSX Licensing Corporation under certain terms. **Please take a moment to read [the license terms](LICENSE.md) for details**.

Please visit [the releases section](https://github.com/Konamiman/Nextor/releases) for binaries.

## Looking for the drivers?

As of Nextor 3.0 this repository no longer contains Nextor drivers for specific hardware. Drivers that were part of the repository in Nextor 2 now live in their own separate repositories; other drivers are distributed by their developers in whatever way they choose. See [the known drivers document](docs/Nextor%203.0%20Known%20Drivers.md) for the list of available drivers and where to get each of them.

## Repository structure

Note that additionally to the `master` branch there are `v2.0` and `v2.1` branches that hold the code and documentation for the legacy versions of Nextor (and the Nextor 2 version of the hardware-specific drivers).

* [**source**](/source): The source code of Nextor itself.

    * [**kernel**](source/kernel): The kernel ROM, includes the FDISK tool.

    * [**nextor_sys**](source/nextor_sys): The `NEXTOR.SYS` file.

    * [**tools**](source/tools): The command line tools: the ones created for Nextor and the classic MSX-DOS tools (CHKDSK, UNDEL, DISKCOPY, FIXDISK, KMODE, XCOPY, XDIR), rewritten from the originals. The tools written in C live in [tools/C](source/tools/C).

    * [**drivers**](source/drivers): The standalone ROM driver and an example RAM driver.

    * [**commandcom**](source/commandcom): `COMMAND3.COM`, the command interpreter.

* [**sdk**](/sdk): Z80 assembler and C include files, helper routines and driver templates, intended to be used when developing Nextor drivers and Nextor-aware tools.

* [**buildtools**](/buildtools): The source code of `mknexrom`, a tool that generates a full Nextor ROM from the kernel base and a driver.

* [**docs**](/docs): Documentation for both users and developers.

* [**docker**](/docker): The infrastructure to create the Nextor development [Docker](https://www.docker.com/) image.

## How to build Nextor

_For instructions on how to build Nextor using the Nextor development Docker image, see [README.md in the Docker directory](docker/README.md)_.

Nextor requires Linux to be built. It should work on macOS too, but that hasn't been tested. If you are on Windows 10 or 11 you can use [WSL](https://learn.microsoft.com/en-us/windows/wsl/install).

To build Nextor you'll need:

* `make`, GNU make version 4.3 or newer (the kernel makefile uses grouped targets, introduced in that version). On Debian/Ubuntu-ish systems you can just `apt-get install make`.
* [The Nestor80 tools](https://github.com/Konamiman/Nestor80). Go to [the releases section](https://github.com/Konamiman/Nestor80/releases) and download the appropriate variant of the latest version for the assembler (N80), the linker (LK80) and the library manager (LB80).
* [SDCC](http://sdcc.sourceforge.net/) **v4.2 or newer**, for FDISK and the command line tools written in C. On Debian/Ubuntu-ish systems you can just `apt-get install sdcc`.
* `objcopy` from [the binutils package](https://www.gnu.org/software/binutils/). On Debian/Ubuntu-ish systems you can just `apt-get install binutils`.
* `mknexrom` to generate the ROM files with the drivers. You have it in [the releases section](https://github.com/Konamiman/Nextor/releases), but you can also build it from the source in the `buildtools/sources` directory.
* `mformat`, `mmd` and `mcopy` from [the mtools package](https://www.gnu.org/software/mtools/), only if you want to build the tools disk image. On Debian/Ubuntu-ish systems you can just `apt-get install mtools`.
* `zip`, only if you want to build the tools zip archive. On Debian/Ubuntu-ish systems you can just `apt-get install zip`.

Except for those obtained via `apt`, you'll need to place these tools at a suitable location to be able to use them, e.g. `/usr/bin`.

There are a number of makefiles that will take care of building the different components of Nextor. Once the tools are in place, `cd` to the appropriate directory and run `make`, adding an explicit target where the table says so - anything with a named target in the "Target" column is built _only_ when that target is requested, it's never included in a plain `make`:

| What do you want to build? | Makefile directory | Target | Result |
| --- | --- | --- | --- |
| Kernel base file (the input for `mknexrom` to produce complete kernel ROMs) | `source/kernel` | none | `bin/kernel-base/Nextor-<version>.base.dat` |
| Kernel base file, all twelve build variants¹ | `source/kernel` | `everything` | `bin/kernel-base/Nextor-<version>.base[.<variant>].dat`, twelve files |
| Standalone ROMs² (ASCII8 and ASCII16) | `source/drivers` | none | `bin/drivers/Nextor-<version>.StandaloneASCII8.ROM` and `...ASCII16.ROM` |
| Standalone ROMs², all twelve build variants¹ | `source/drivers` | `everything` | `bin/drivers/Nextor-<version>.StandaloneASCII{8,16}[.<variant>].ROM`, twenty-four files |
| Example RAM-loadable driver | `source/drivers` | `ram-example` | `bin/drivers/ram-driver-example.drv` |
| `NEXTOR.SYS`, plus its Japanese-messages variant | `source/nextor_sys` | none | `bin/tools/NEXTOR.SYS` and `bin/tools/NEXTOR.SYS.japanese` |
| `COMMAND3.COM`, the command interpreter | `source/commandcom` | none | `bin/tools/COMMAND3.COM` |
| Command line tools written in assembler | `source/tools` | none | `bin/tools`, one `.COM` file per tool |
| Command line tools written in C | `source/tools/C` | none | `bin/tools`, one `.COM` file per tool |
| Tools disk image³ | `source/tools` | `tools-disk` | `bin/tools/nextor.dsk` |
| Tools zip archive⁴ | `source/tools` | `tools-zip` | `bin/tools/tools.zip` |

¹ The twelve variants are: default, `INVERT_SHIFT` and `INVERT_CTRL`, each with and without `NO_UNDOC_CPU_INSTRUCTIONS`, and each of these six with and without `INVERT_KANJI` (the "6" boot key inverted, so the Kanji driver is installed at boot time unless the key is pressed). See the comments at the beginning of the kernel makefile for the details.

² A standalone ROM is a full usable ROM containing the Nextor kernel and a dummy driver that doesn't handle any hardware. The `source/drivers` makefile builds the kernel base file too (by recursing into `source/kernel`), unless the `NEXTOR_BASE` variable points it to a pre-built one.

³ The disk image is a 720K FAT12 image containing `NEXTOR.SYS` (plus the Japanese-messages variant, renamed to `NEXTORJ.SYS`), every `.COM` file present in `bin/tools` (which includes `COMMAND3.COM` and all the command line tools), a `README.TXT` file, and the `COMMAND3.COM` help files in a `HELP` directory. It boots straight to the DOS prompt on a computer with a Nextor kernel ROM. Files not built by this repository can be added through the `EXTRA_FILES` variable (e.g. use `EXTRA_FILES=MSXDOS.SYS,COMMAND.COM` for a disk that also boots to the DOS prompt in MSX-DOS 1 mode); relative paths are resolved against the current directory, the one you run `make` from. This target requires the mtools package, and expects all the files to be already built: use the `tools-disk` umbrella target (below) to build them and create the image in one go.

⁴ The zip archive contains just the command line tools (no `NEXTOR.SYS`, no `COMMAND3.COM`). This target requires the `zip` tool and, like `tools-disk`, expects the tools to be already built: the `tools-zip` umbrella target (below) builds them and creates the archive in one go.

Additionally, an "umbrella" makefile in `source` invokes the other makefiles in the right order, so the common combinations are a single `make` command run from the `source` directory. The same rule applies: plain `make` builds the first row only, every other combination needs its target spelled out.

| What do you want to build? | Target | Result |
| --- | --- | --- |
| Everything with a default target above: kernel base file, standalone ROMs, `NEXTOR.SYS`, `COMMAND3.COM` and all the command line tools | none | `bin/kernel-base`, `bin/drivers`, `bin/tools` |
| The same, but with the kernel base file and the standalone ROMs in all twelve variants¹ | `everything` | `bin/kernel-base`, `bin/drivers`, `bin/tools` |
| `NEXTOR.SYS`, `COMMAND3.COM` and all the command line tools, then the disk image³ | `tools-disk` | `bin/tools`, including `nextor.dsk` |
| All the command line tools, then the zip archive⁴ | `tools-zip` | `bin/tools`, including `tools.zip` |
| `NEXTOR.SYS`, `COMMAND3.COM` and all the command line tools, then both the disk image³ and the zip archive⁴ | `tools-all` | `bin/tools`, including `nextor.dsk` and `tools.zip` |

Note that the example RAM driver is the one thing no umbrella target builds: it is always an explicit `make ram-example` in `source/drivers`.

Every makefile (the umbrella one included) also supports a `clean` target that removes the intermediate files from the source directories, and the umbrella makefile adds `distclean`, which additionally deletes the generated `bin/kernel-base`, `bin/drivers` and `bin/tools` directories.


