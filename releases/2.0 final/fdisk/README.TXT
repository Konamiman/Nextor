This is the source code of the device partitioner embedded within the Nextor kernel (the one that appears when issuing a CALL FDISK command from the BASIC prompt). You can change it to correct any bug or extend its functionality (subject to code size constraints, see below), or you can use it as the basis for developing a custom tool.

In order to compile it you need:

- The SDCC compiler (http://sdcc.sourceforge.net) (See note below!)
- A tool to convert .HEX files to binary files, such as HEX2BIN.EXE for Windows (included).
- A file portions copying tool tool such as DD. Linux has this tool built-in, a Windows port (DD.EXE) exists and is included.

A script for Windows is included (compile.bat) that will compile the tool and embed it within the Nextor ROM file. You need to copy the Nextor ROM file to the folder where the rest of the files are located, and edit the script to specify the file name (in the line starting with "set nextorfile=").

Alternatively, you can compile and embed the tool manually. See the beginning of the source files (fdisk.c and fdisk2.c) for details.

In case that you want to add new code and embed the result within the Nextor kernel, note that the size of the compiled code cannot exceed 16000 bytes for fdisk.c, and 8000 bytes for fdisk2.c, due to ROM page size constraints. You cannot rely on the size of the generated files (fdisk.dat and fdisk2.dat) since these contain data sections that are stripped down in the embedding process. In order to know the size of the generated code, open the .lst files produced by the compilation process (you need to remove the "del *.lst" line in the compilation script) and look at the 4 digit hexadecimal number in the first column in the last line. This value should be no greater than 3E80 for fdisk.lst, or 1F40 for fdisk2.lst.

More about Nextor here: http://www.konamiman.com#nextor

-------------------------------------------------------

NOTE ABOUT SDCC: Quoted from http://www.konamiman.com#sdcc

SDCC comes with a Z80 version of the standard C library, it is at (SDCC folder)\lib\z80\z80.lib. The versions of the console related functions getchar, putchar and printf that are bundled in this library are not suited for developing MSX software. The logical fix for this would be to replace these functions with MSX compatible versions inside the library itself.

However there is a problem with the current version of SDCC, at least the Windows version: it is not possible to manage the file z80.lib with the SDCC library manager utility supplied, sdcclib.exe. If attempted, the error message "File was not created with sdcclib" is displayed.

The workaround I have come up with consists of using the msxchar library that you can download here, plus modifying the original file z80.lib by hand (this is necessary so that the compiler does not complaint about having duplicate function names). That's how this can be done:

1. Open the z80.lib file in a text editor. You will see garbage (binary data) mixed with text data.
2. Search for all the occurences of the names printf, getchar and putchar within the file, either as whole words or as part of other words.
3. Modify these names while maintaining their lengths. I have simply changed the first character into a 'x', so that they become 'xrintf', 'xetchar' and 'xutchar'.

This is not a perfect solution but it seems to work. If anyone knows a better way for dealing with this apparently defective z80.lib, please let me know.
