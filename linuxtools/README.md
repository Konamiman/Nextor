
This document details the build process for linux.

## Makefiles

The makefiles for linux are:
```
./source/Makefile
./source/Makefile-main.mk
```

## Key Requirements

* Bash 4.4
* make 4.1
* gcc

## Prerequisites

The linux build requires 3 external support tools:
* sdcc
* cpm
* hex2bin

The can be install with

`make install-prereq`

This will be wget/cloned the require code into `linuxbuild\prereq\...` and compile the tools.

During all make operation, the path is overriden to ensure these versions of the tools are used.

## Building.

All make targets are accessed thru the main `Makefile` in `source`.  So to make roms, binaries, etc:

```
cd source
make <target>
```

Where target is the desired target - eg `make msxdos` to make the nextor.sys binary.

All key outputs are places in the `bin` directory.

The targets of interest are:

```
tools:                Build the common tools
sunrise:              Build the sunrise kernel rom
chkdsk:               Build the chkdsk.com
command2:             Build the command2.com
nextor:               Build the nextor.sys
nextork:              Build the nextork.sys
hdddsk:               Build a FAT12 hard disk image containing nextor.sys, command2.com and all other tools
fdddsk:               Build a FAT12 hard disk image containing nextor.sys, command2.com and all other tools
mknexrom:             build the mknexrom utility

                      Other targets

clean:                Remove the bin directory
install-prereq:       Install required tooling (sdcc, hex2bin, cpm) into (linuxtools/prereq)
help:                 Display this help message
```

### Make process

The linux build process is as follows:
1. Symlink all source files from the various directories into `bin/working`.  (Any name clashes are resolved by renaming some files with a prefix).
2. Link in the `Makefile-main.mk` as `Makefile` into `bin/working`.
3. Invoke the desired make target from within the `bin/working` directory.
4. Specific targets will copy their output to bin.

See the `source/Makefile`'s *prep* target for more details of the symlinking structure and name conflict resolutions.
