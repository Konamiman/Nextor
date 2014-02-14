This folder contains the utilities needed to compile the sources in Windows, using the COMPILE.BAT scripts. Note also that M80.CPM, L80.CPM and LIB80.CPM files must be copied to each directory having sources.

This folder should be included in the PATH environment variable. Or alternatively, the files in this folder can be copied to the windows\system32 folder.


*** Important note about VirtualFloppyDisk ***

There are two files in the VirtualFloppyDisk folder named VFD.SYS_32 and VFD.SYS_64. These are the 32 bit and 64 bit versions of the virtual floppy driver. Before using the application, you must rename the apporpriate file for your type of operating system to VFD.SYS.


*** Note about MKDOSROM ***

MKDOSROM.EXE is a Windows application. However, the source code, MKDOSROM.C, is a standard C application that uses only standard libraries, therefore it can be compiled for any platform.
