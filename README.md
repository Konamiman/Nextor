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

    * [**tools**](source/tools): The new command line tools created for Nextor.

    * [**drivers**](source/drivers): The standalone ROM driver and an example RAM driver.

    * [**command**](source/command): `COMMAND2.COM` and the command line tools that were originally supplied with MSX-DOS. These aren't currently included in the build pipeline.

* [**sdk**](/sdk): Z80 assembler and C include files, helper routines and driver templates, intended to be used when developing Nextor drivers and Nextor-aware tools.

* [**buildtools**](/buildtools): The source code of `mknexrom`, a tool that generates a full Nextor ROM from the kernel base and a driver.

* [**docs**](/docs): Documentation for both users and developers.

* [**docker**](/docker): The infrastructure to create the Nextor development [Docker](https://www.docker.com/) image.

## How to build Nextor

_For instructions on how to build Nextor using the Nextor development Docker image, see [README.md in the Docker directory](docker/README.md)_.

Nextor requires Linux to be built. It should work on macOS too, but that hasn't been tested. If you are on Windows 10 or 11 you can use [WSL](https://learn.microsoft.com/en-us/windows/wsl/install).

To build Nextor you'll need:

* `make`. On Debian/Ubuntu-ish systems you can just `apt-get install make`.
* [The Nestor80 tools](https://github.com/Konamiman/Nestor80). Go to [the releases section](https://github.com/Konamiman/Nestor80/releases) and download the appropriate variant of the latest version for the assembler (N80), the linker (LK80) and the library manager (LB80).
* [SDCC](http://sdcc.sourceforge.net/) **v4.2 or newer**, for FDISK and the command line tools written in C. On Debian/Ubuntu-ish systems you can just `apt-get install sdcc`.
* `objcopy` from [the binutils package](https://www.gnu.org/software/binutils/). On Debian/Ubuntu-ish systems you can just `apt-get install binutils`.
* `mknexrom` to generate the ROM files with the drivers. You have it in [the releases section](https://github.com/Konamiman/Nextor/releases), but you can also build it from the source in the `buildtools/sources` directory.

Except for those obtained via `apt`, you'll need to place these tools at a suitable location to be able to use them, e.g. `/usr/bin`.

There are a number of makefiles that will take care of building the different components of Nextor. Once the tools are in place you can just `cd` to the appropriate directory and run `make`:

* `source/kernel`: builds the kernel base file (the input for `mknexrom` to produce complete kernel ROMs) and copies it to the `bin/kernel-base` directory. Running `make everything` builds all six variant combinations (default, `INVERT_SHIFT` and `INVERT_CTRL`, each with and without `NO_UNDOC_CPU_INSTRUCTIONS`); see the comments at the beginning of the makefile for the details.
* `source/nextor_sys`: builds `NEXTOR.SYS` and copies it to the `bin/tools` directory.
* `source/tools`: builds the command line tools written in assembler and copies them to the `bin/tools` directory.
* `source/tools/C`: builds the command line tools written in C and copies them to the `bin/tools` directory.
* `source/drivers`: builds the standalone Nextor ROM (a full usable ROM containing the Nextor kernel and a dummy driver that doesn't handle any hardware) in their ASCII8 and ASCII16 variants. It also allows building an example RAM-loadable driver.

There's also an "umbrella" makefile in `source` that just invokes all the others in sequence, so it builds pretty much everything. It supports `make clean` too.

You may want to take a look at [this now closed pull request from Dean Netherton](https://github.com/Konamiman/Nextor/pull/79) that contains a different attempt at writing makefiles for building Nextor. It even has some nice extra features like building FDD and HDD images with Nextor, and building the `mknexrom` tool itself. Note however that that pull request was created targeting Nextor 2 and thus many of the ideas it uses may no longer be relevant.

