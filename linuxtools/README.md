
This document details the build process for linux.

## Makefiles

The makefiles for linux are:
```
./source/Makefile
./source/Makefile-bank0.mk
./source/Makefile-bank1.mk
./source/Makefile-bank2.mk
./source/Makefile-bank3.mk
./source/Makefile-bank4.mk
./source/Makefile-bank5.mk
./source/Makefile-bank6.mk
./source/Makefile-chkdsk.mk
./source/Makefile-command.mk
./source/Makefile-driver-sunrise-ide.mk
./source/Makefile-kernel.mk
./source/Makefile-msxdos.mk
./source/Makefile-tools.mk
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

All outputs are places in the `bin` directory.

The targets of interest are:

```
                      Primary Targets

sunrise:              Build the sunrise rom image into bin/drivers/sunriseide
drivers:              Build all drivers
hdddsk:               Build a FAT12 hard disk image containing nextor.sys, command2.com and all other tools
msxdos:               Build the nextor.sys and nextork.sys files (bin/cli/)
command:              Build the bin/cli/command2.com unit
chkdsk:               Build the bin/cli/chkdsk.com unit
tools:                Build all the tools binaries into bin/cli

                      Other targets

clean:                Remove the bin directory
install-prereq:       Install required tooling (sdcc, hex2bin, cpm) into (linuxtools/prereq)
help:                 Display this help message
```

### Make process

The process for building of the various units (eg: Bank0, tools, msxdos, etc) is a 2 phase process.

The first step is to create a working directory, symlinking in the relevant files, including a specific Makefile.
Then, the next step, is to invoke the sub-make.

In more details:

1. Create a mirror directory under `bin/working/...` eg (`bin/working/kernel/bank0`)
2. symlink the source files (eg: link the mac & inc files from `source/kernel/bank0` to `bin/working/kernel/bank0`)
3. symlink in any of the dependencies from other directory structures (eg link in related rel and inc files)
4. symlink in the unit's specific makefile (eg: `./source/Makefile-bank0.mk` -> `bin/working/kernel/bank0/Makefile`)
5. Invoke the sub-make
