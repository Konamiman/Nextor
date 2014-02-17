## Nextor ##

Nextor is a disk operating system for MSX computers. It is built on top of the source code of MSX-DOS 2.31.

### How to build Nextor ###

You need:

1. A Windows machine (if you succeed in building Nextor from a Linux machine, please let me know!)
2. SDCC ([http://sdcc.sourceforge.net](http://sdcc.sourceforge.net)), targetting the Z80 processor (see not below!)
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

-----

** NOTE ABOUT SDCC:** (Quoted from [http://www.konamiman.com#sdcc](http://www.konamiman.com#sdcc))

SDCC comes with a Z80 version of the standard C library, it is at `(SDCC folder)\lib\z80\z80.lib`. The versions of the console related functions getchar, putchar and printf that are bundled in this library are not suited for developing MSX software. The logical fix for this would be to replace these functions with MSX compatible versions inside the library itself.

However there is a problem with the current version of SDCC, at least the Windows version: it is not possible to manage the file z80.lib with the SDCC library manager utility supplied, sdcclib.exe. If attempted, the error message "File was not created with sdcclib" is displayed.

The workaround I have come up with consists of using the msxchar library that you can download here, plus modifying the original file z80.lib by hand (this is necessary so that the compiler does not complaint about having duplicate function names). That's how this can be done:

1. Open the `z80.lib` file in a text editor. You will see garbage (binary data) mixed with text data.
2. Search for all the occurences of the names `printf`, `getchar` and `putchar` within the file, either as whole words or as part of other words.
3. Modify these names while maintaining their lengths. I have simply changed the first character into a 'x', so that they become 'xrintf', 'xetchar' and 'xutchar'.

This is not a perfect solution but it seems to work. If anyone knows a better way for dealing with this apparently defective z80.lib, please let me know.
