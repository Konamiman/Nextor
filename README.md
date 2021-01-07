# Nextor

Nextor is a disk operating system for MSX computers. It is built on top of the source code of MSX-DOS 2.31, released in 1991.

The source code of Nextor is published with permission from the MSX Licensing Corporation under certain terms. **Please take a moment to read [the license terms](LICENSE.md) for details**.

Please visit [the Nextor section in Konamiman's MSX page](https://www.konamiman.com/msx/msx-e.html#nextor) for binaries.

## Repository structure

Note that there is no `master` branch, but branches for each major version of Nextor (v2.0 and v2.1 currently).

* [**source**](/source): The source code of Nextor itself.

    * [**kernel**](source/kernel): The kernel ROM, includes the FDISK tool.

    * [**command**](source/command): `NEXTOR.SYS`, `COMMAND2.COM` and the command line tools that were originally supplied with MSX-DOS.

    * [**tools**](source/tools): The new command line tools created for Nextor.

* [**buildtools**](/buildtools): Tools needed for building Nextor on Windows (deprecated) and Linux (recommended). Includes the source for two custom made tools: [`mknexrom`](/buildtools/sources/mknexrom.c) (C) and [`SymToEqus`](/buildtools/sources/SymToEqus.cs) (C#).

* [**docs**](/docs): Documentation for both users and developers.

## How to build Nextor

The "official" environment for building Nextor is Linux. Legacy support for Windows is still offered but it's deprecated. Read on for the ugly details.

### Linux

To build Nextor on Linux you'll need:

* `make`. On Debian/Ubuntu-ish systems you can just `apt-get install make`.
* The native MACRO80 tools provided by [the M80dotNet project](https://github.com/Konamiman/M80dotNet). Go to [the releases section](https://github.com/Konamiman/M80dotNet/releases) and download the appropriate variant of the latest version.
* [SDCC](http://sdcc.sourceforge.net/), for FDISK and the command line tools written in C. On Debian/Ubuntu-ish systems you can just `apt-get install sdcc`.
* `objcopy` from [the binutils package](https://www.gnu.org/software/binutils/). On Debian/Ubuntu-ish systems you can just `apt-get install binutils`.
* `sjasm` v0.39 to assemble some of the drivers. You have it in the `buildtools/Linux` folder, but you can also build it from [the sources](https://github.com/Konamiman/Sjasm/tree/v0.39) (remember to switch to the `v0.39` branch).
* `mknexrom` to generate the ROM files with the drivers. You have it in the `buildtools/Linux` folder, but you can also build it from the source in the `buildtools/sources` directory.

Except for those obtained via `apt`, you'll need to place these tools at a suitable location to be able to use them, e.g. `/usr/bin`.

There are five makefiles that will take care of building the different components of Nextor. Once the tools are in place you can just `cd` to the appropriate directory and run `make`:

* `source/kernel`: builds the kernel ROM files and copies them to the `bin/kernels` directory. There are handy aliases for the different ROM files, so you can run e.g. `make ide`; see the `kernels` rule at the beginning of the file for the full list.
* `source/command/msxdos`: builds `NEXTOR.SYS` and copies it to the `bin/tools` directory.
* `source/tools`: builds the command line tools written in assembler and copies them to the `bin/tools` directory.
* `source/tools/C`: builds the command line tools written in C and copies them to the `bin/tools` directory.
* `source`: this one just invokes the other four in sequence, so it builds pretty much everything. It supports `make clean` too.

You may want to take a look at [this now closed pull request from Dean Netherton](https://github.com/Konamiman/Nextor/pull/79) that contains a different attempt at writing makefiles for bulding Nextor. It even has some nice extra features like building FDD and HDD images with Nextor, and building the `mknexrom` tool itself.

## Windows

If you use Windows 10 the recommended approach is to use the Linux tools and scripts with [WSL](https://docs.microsoft.com/en-us/windows/wsl/install-win10). If you use an older Windows the recommended approach is to upgrade to Windows 10 (or to install Linux in a separate partition or disk, or in a virtual machine).

However, if for some reason you are still using a non-WSL capable Windows, support for building Nextor is available as well; but note that it will probably be removed at some point in the future, as it's a maintenance burden (seriously, give Windows 10 and WSL a try, it's really worth it).

To build Nextor on Windows you need:

* The tools in the `buildtools/Windows` folder. These must be placed in some folder included in the `PATH` environment variable.
* [SDCC](http://sdcc.sourceforge.net/), for FDISK and the command line tools written in C.
* .NET Framework 2.0 or higher, for the `SymToEqus` tool.

You'll find a number of `.bat` files available at the same locations of the Linux makefiles except for the one in `source` (see "Linux" section above). These are not "makefile-ish" and always build the whole set of kernels/tools.
