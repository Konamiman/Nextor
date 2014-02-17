## Nextor ##

Nextor is a disk operating system for MSX computers. It is built on top of the source code of MSX-DOS 2.31.

### How to build Nextor ###

You need:

1. A Windows machine (if you succeed in building Nextor from a Linux machine, please let me know!)
2. SDCC (http://sdcc.sourceforge.net), targetting the Z80 processor   
3. .NET Framework 2.0 or higher (for the `SymToEqus` tool in the `wintools` folder)
4. The `wintools` folder must be added to the `PATH` environment variable

** To build the Nextor kernel: **

Run the `compile.bat` script located in the `source\msxdos25` folder. If the FDISK tool has not been compiled already (the `fdisk.dat` and `fdisk2.dat` files do not exist in the `bank5` folder), they will be compiled on the fly.

The generated kernel base file and the complete ROM files will be generated in the `bin\kernels` folder. One ROM file will be generated for each folder existing in the `source\msxdos25\drivers` folder.

** To build the FDISK tool: **

If you make a change in the FDISK tool, you can compile it without having to compile the full kernel again. Just run the `compile.bat` script in the `source\msxdos25\bank5` folder (do NOT run `compfdsk.bat`). The ROM files in `bin\kernels` will be appropriately updated.

** To build the command line tools: **

Run the `compile.bat` script in the `source\tools` folder. The tools will be generated in the `bin\tools` folder.

** To build NEXTOR.SYS: **

Run the `compile.bat` script in the `source\command25\msxdos` folder. The file will be generated in the `bin\tools` folder.
