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

* The native MACRO80 tools provided by [the M80dotNet project](https://github.com/Konamiman/M80dotNet). Go to [the releases section](https://github.com/Konamiman/M80dotNet/releases) and download the appropriate variant of the latest version.
* [SDCC](http://sdcc.sourceforge.net/), for FDISK and the command line tools written in C. On Debian/Ubuntu-ish systems you can just `apt-get install sdcc`.
* `objcopy` from [the binutils package](https://www.gnu.org/software/binutils/). On Debian/Ubuntu-ish systems you can just `apt-get install binutils`.
* `sjasm` v0.39 to assemble some of the drivers. You have it in the `buildtools/Linux` folder, but you can also build it from [the sources](https://github.com/Konamiman/Sjasm/tree/v0.39) (remember to switch to the `v0.39` branch).
* `mknexrom` to generate the ROM files with the drivers. You have it in the `buildtools/Linux` folder, but you can also build it from the source in the `buildtools/sources` directory.

Except for those obtained via `apt`, you'll need to place these tools at a suitable location to be able to use them, e.g. `/usr/bin`.

Once the tools are in place you can use the following scripts to build the various components of Nextor:

  * `source/kernel/compile.sh`: builds the kernel ROM files and copies them to the `bin/kernels` directory. Can be executed as `compile.sh drivers` to only compile the drivers.
  * `source/kernel/bank5/compile.sh`: builds only the part of the kernel corresponding to the buildt-in FDISK tool and patches the ROM files in the `bin/kernels` directory with the result.
    * `source/kernel/bank5/compile_fdisk.sh`: this one is NOT intended to be used directly, it's called by the previous two and it does the actual compilation from the FDISK source files. If `fdisk.dat` already exists and is newer than `fdisk.c` then it will not be compiled (and the same for `fdisk2.dat` and `fdisk2.c`). 
  * `source/command/msxdos.sh`: builds `NEXTOR.SYS` and copies it to the `bin/tools` directory.
  * `source/tools/compile.sh`: builds the command line tools written in assembler and copies them to the `bin/tools` directory. To build only one of the tools pass its name as an argument (without extension).
  * `source/tools/C/compile.sh`: builds the command line tools written in C and copies them to the `bin/tools` directory. To build only one of the tools pass its name as an argument (without extension).

## Windows

If you use Windows 10 the recommended approach is to use the Linux tools and scripts with [WSL](https://docs.microsoft.com/en-us/windows/wsl/install-win10). If you use an older Windows the recommended approach is to upgrade to Windows 10 (or to install Linux in a separate partition or disk, or in a virtual machine).

However, if for some reason you are still using a non-WSL capable Windows, support for building Nextor is available as well; but note that it will probably be removed at some point in the future, as it's a maintenance burden (seriously, give Windows 10 and WSL a try, it's really worth it).

To build Nextor on Windows you need:

* The tools in the `buildtools/Windows` folder. These must be placed in some folder included in the `PATH` environment variable.
* [SDCC](http://sdcc.sourceforge.net/), for FDISK and the command line tools written in C.
* .NET Framework 2.0 or higher, for the `SymToEqus` tool.

You'll find a number of `.bat` files available at the same locations of the Linux `.sh` scripts (see "Linux" section above) that serve the same purpose.
