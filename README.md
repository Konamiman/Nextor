# Nextor

Nextor is a disk operating system for MSX computers. It is built on top of the source code of MSX-DOS 2.31, released in 1991.

Please visit [the Nextor section in Konamiman's MSX page](https://www.konamiman.com/msx/msx-e.html#nextor) for binaries.

## Repository structure

Note that there is no `master` branch, but branches for each major version of Nextor (v2.0 and v2.1 currently).

* [**source**](/source): The source code of Nextor itself.

    * [**msxdos25**](source/msxdos25): The kernel ROM, includes the FDISK tool.

    * [**command25**](source/command25): `NEXTOR.SYS`, `COMMAND2.COM` and the command line tools that were originally supplied with MSX-DOS.

    * [**tools**](source/tools): The new command line tools created for Nextor.

* [**wintools**](/wintools): Windows tools needed for building Nextor. Includes the source for two custom made tools: [`mknexrom`](/wintools/mknexrom.c) (C) and [`SymToEqus`](/wintools/SymToEqus.cs) (C#).

* [**docs**](/docs): Documentation for both users and developers.

## How to build Nextor

You need:

1. A Windows machine (if you succeed in building Nextor from a Linux machine, please let me know!)
2. SDCC ([http://sdcc.sourceforge.net](http://sdcc.sourceforge.net)), targetting the Z80 processor, to build FDISK.
3. .NET Framework 2.0 or higher (for the `SymToEqus` tool in the `wintools` folder)
4. The `wintools` folder must be added to the `PATH` environment variable

### To build the Nextor kernel

Run the `compile.bat` script located in the `source\msxdos25` folder. If the FDISK tool has not been compiled already (the `fdisk.dat` and `fdisk2.dat` files do not exist in the `bank5` folder), they will be compiled on the fly.

The generated kernel base file and the complete ROM files will be generated in the `bin\kernels` folder. One ROM file will be generated for each folder existing in the `source\msxdos25\drivers` folder.

### To build the FDISK tool only

If you make a change in the FDISK tool, you can compile it without having to compile the full kernel again. Just run the `compile.bat` script in the `source\msxdos25\bank5` folder (do NOT run `compfdsk.bat`). The ROM files in `bin\kernels` will be appropriately updated.

### To build the command line tools

Run the `compile.bat` script in the `source\tools` folder. The tools will be generated in the `bin\tools` folder.

### To build `NEXTOR.SYS`

Run the `compile.bat` script in the `source\command25\msxdos` folder. The file will be generated in the `bin\tools` folder.

### To build `COMMAND2.COM`

Run the `compile.bat` script in the `source\command25\command` folder. The file will be generated in the `bin\tools` folder.

At this time there's no specific script (other than the original makefile) for building the original MSX-DOS command line tools.
