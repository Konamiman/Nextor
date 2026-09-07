# Nextor 3.0 What's New

## Index

[1. Introduction](#1-introduction)

[2. General information](#2-general-information)

[2.1. The new driver system](#21-the-new-driver-system)

[2.2. Drivers loadable in RAM](#22-drivers-loadable-in-ram)

[2.3. Support for floppy disks](#23-support-for-floppy-disks)

[2.4. The boot menu](#24-the-boot-menu)

[2.5. The Kanji driver can be installed at boot time](#25-the-kanji-driver-can-be-installed-at-boot-time)

[2.6. One drive letter per active partition/offline device at boot](#26-one-drive-letter-per-active-partitionoffline-device-at-boot)

[2.7. Drivers are now distributed separately](#27-drivers-are-now-distributed-separately)

[2.8. Z180-compatible builds](#28-z180-compatible-builds)

[2.9. Better boot error messages](#29-better-boot-error-messages)

[2.10. MSX-DOS 1 mode can boot from any drive](#210-msx-dos-1-mode-can-boot-from-any-drive)

[2.11. New and changed tools and commands](#211-new-and-changed-tools-and-commands)

[2.12. The new COMMAND3.COM command interpreter](#212-the-new-command3com-command-interpreter)

[3. Information for application developers](#3-information-for-application-developers)

[3.1. New function call: driver operations (_DRVRO, 7Fh)](#31-new-function-call-driver-operations-_drvro-7fh)

[3.2. Changed function calls](#32-changed-function-calls)

[3.3. New error codes](#33-new-error-codes)

[3.4. Nextor in MSX-DOS 1 mode](#34-nextor-in-msx-dos-1-mode)

[3.5. The Nextor SDK and the Docker development image](#35-the-nextor-sdk-and-the-docker-development-image)

[4. Information for driver developers](#4-information-for-driver-developers)


## 1. Introduction

Nextor 3.0 is the successor of Nextor 2.1. This document summarizes what has changed, aimed at people who are already familiar with Nextor 2 but new to Nextor 3. It doesn't go into full detail: each item links to the section of the appropriate document where the change is completely explained.

The most important change in Nextor 3 is the completely new, and incompatible at the driver API level, device driver system: **Nextor 2 drivers do not work on Nextor 3**, and vice versa, so running a Nextor 3 kernel requires a Nextor 3 version of the driver for your storage hardware (see _[Nextor 3.0 Known Drivers](Nextor_3.0_Known_Drivers.md)_ for the list of available drivers). Consistent with this, a Nextor 3 kernel deactivates any Nextor 2 kernels it finds at boot time; having both kernel versions in the same machine is expected to be a temporary situation only, e.g. for flashing purposes. See _[3.2. Booting Nextor](Nextor_3.0_User_Manual.md#32-booting-nextor)_ in the user manual.

Beyond that, the highlights of Nextor 3 are: proper support for floppy disk drives (including ghost drives and `CALL FORMAT`), drivers that can be loaded in RAM at any time without flashing anything, a boot menu, one drive letter assigned per active partition (instead of one per device) at boot, and a new command interpreter, `COMMAND3.COM`. The rest of this document covers these and the other changes: first the ones everybody should know about, then the ones relevant to application developers, and finally a short note for driver developers. Note that the "Change history" section that used to exist in the user manual is gone; this document supersedes it for the 3.0 release.


## 2. General information

### 2.1. The new driver system

Nextor 3 device drivers follow a completely new structure, built around a query-based API. The full details are in the _[Nextor 3.0 Driver Development Guide](Nextor_3.0_Driver_Development_Guide.md)_; from the user's point of view the changes are:

* There are no more logical units (LUNs): a driver now directly exposes up to 255 devices, and a device is identified everywhere by a single device number.

* There are no more "drive-based" drivers: all Nextor 3 drivers are what was called a "device-based" driver in Nextor 2 (now simply "Nextor drivers").

* Drivers can now tell the difference between a device that is offline (e.g. a removable device, like an SD card reader, with no medium inserted) and a device that doesn't exist at all.

* Drivers can be loaded in RAM (see _[2.2. Drivers loadable in RAM](#22-drivers-loadable-in-ram)_) and can expose floppy disk drives (see _[2.3. Support for floppy disks](#23-support-for-floppy-disks)_).

### 2.2. Drivers loadable in RAM

Drivers no longer need to be embedded in a kernel ROM: a driver can be distributed as a file and installed into a mapped RAM segment at any time, then uninstalled when no longer needed. This is done with the new [`CALL IDRIVER`](Nextor_3.0_User_Manual.md#3612-the-call-idriver-command) and [`CALL UDRIVER`](Nextor_3.0_User_Manual.md#3613-the-call-udriver-command) commands in BASIC, or with the new [`DRVROP.COM`](Nextor_3.0_User_Manual.md#3413-drvrop-the-driver-operations-tool) tool in the command line. Drivers loaded in RAM have full feature parity with ROM drivers: drive mapping, `CALL` commands, partitioning with FDISK, listing with `DRIVERS`, etc. They are not available in MSX-DOS 1 mode.

### 2.3. Support for floppy disks

Nextor 3 has first-class support for floppy disk drives, which in Nextor 2 could only be handled by legacy MSX-DOS drivers. See _[2.5. Support for floppy disks](Nextor_3.0_User_Manual.md#25-support-for-floppy-disks)_ in the user manual for the whole story; in short:

* A device is considered to be a floppy disk drive when the driver flags it as such; see _[4.6.2. Device query 2: Get device parameters](Nextor_3.0_Driver_Development_Guide.md#462-device-query-2-get-device-parameters)_ in the driver development guide.

* Floppy disks are assumed not to have partitions: drives are mapped directly to sector 0 of the device.

* The kernel manages ghost drives: a single physical floppy disk drive can serve an additional drive letter, with the classic prompt asking to insert the disk for the other drive when needed. See _[2.5.1. Ghost drives](Nextor_3.0_User_Manual.md#251-ghost-drives)_.

* Formatting works: see _[3.6.3. The CALL FORMAT command](Nextor_3.0_User_Manual.md#363-the-call-format-command)_.

A brand new driver for the floppy disk controller built into the MSX Turbo-R computers is available, in both ROM and RAM-loadable flavors; see _[Nextor 3.0 Known Drivers](Nextor_3.0_Known_Drivers.md)_.

### 2.4. The boot menu

Pressing the N key while the machine boots opens the new boot menu, which lets you enable or disable each of the Nextor 3 kernels present in the machine, and turn the numeric boot keys on or off, without having to keep the corresponding keys pressed while booting. See _[2.10. Boot keys and the boot menu](Nextor_3.0_User_Manual.md#210-boot-keys-and-the-boot-menu)_.

### 2.5. The Kanji driver can be installed at boot time

A new boot key, 6, makes Nextor install the Kanji driver (the equivalent of `CALL KANJI` followed by `CALL ANK` in BASIC, so the driver is installed but the screen is left in ANK mode) before the DOS environment is loaded, in both MSX-DOS 2 and MSX-DOS 1 modes; this is what disks patched with `KMODE.COM` did. Like the other boot keys, it can be selected in the boot menu, set via the one-time boot keys mechanism, and inverted in the ROM (so that the driver is installed unless the key is pressed), either with `mknexrom /k:0040` or by using one of the new "KANJI_INV" kernel variants. See _[2.10. Boot keys and the boot menu](Nextor_3.0_User_Manual.md#210-boot-keys-and-the-boot-menu)_.

### 2.6. One drive letter per active partition/offline device at boot

At boot time Nextor 2 assigned one drive letter per storage device found, mapped to its first suitable partition. Nextor 3 instead assigns one drive letter to every FAT12 or FAT16 partition flagged as active on every device (in MSX-DOS 1 mode, only to MSX-DOS 1 compatible partitions: FAT12 with three or less sectors per FAT). A device that has suitable partitions but none of them flagged as active still gets one drive letter, mapped to its first suitable partition, as in Nextor 2.

The handling of offline devices at boot has been revised too: removable devices get a drive letter even if no medium is inserted, while fixed devices that are offline don't get any. Devices that are online but don't hold any mappable partition also get one drive letter. Drives assigned in these last two ways have no partition attached initially: accessing them returns an error, and a partition is searched automatically on each access until one is found (e.g. after the device is partitioned, or after a medium is inserted). See _[3.2. Booting Nextor](Nextor_3.0_User_Manual.md#32-booting-nextor)_.

### 2.7. Drivers are now distributed separately

The Nextor repository no longer contains the drivers for specific hardware, and no longer builds ready-to-use kernel ROMs for them: it builds only the Nextor kernel base file. Drivers are now developed and distributed independently, combining the driver code with the kernel base file to produce the final ROM; a RAM-loadable driver file, when applicable, is instead obtained by assembling the driver code on its own.

The drivers that were part of Nextor 2 now live in their own git repositories, but driver developers are free to distribute their work in any other way (a dedicated web site, plain downloadable binaries, etc.). The list of known drivers and where to get each of them is maintained in _[Nextor 3.0 Known Drivers](Nextor_3.0_Known_Drivers.md)_.

### 2.8. Z180-compatible builds

The kernel can now be built with the `NO_UNDOC_CPU_INSTRUCTIONS` option, which avoids all the undocumented Z80 instructions so that the resulting kernel also works on machines with a Z180 processor. See [the main README file](https://github.com/Konamiman/Nextor/blob/HEAD/README.md) for how to build the kernel.

### 2.9. Better boot error messages

When the DOS environment fails to load at boot time, NEXTOR.SYS no longer keeps asking the user to insert the proper disk or drops silently into BASIC: a proper error message, such as "Command interpreter not found" or "Incompatible DOS version", is printed as part of the initial BASIC prompt. Application programs can take advantage of the underlying mechanism too; see _[7.3. DOS environment load errors](Nextor_3.0_Programmers_Reference.md#73-dos-environment-load-errors)_ in the programmers reference.

### 2.10. MSX-DOS 1 mode can boot from any drive

The original MSX-DOS 1 kernel only ever tried to boot from drive A:, so if the disk in A: wasn't bootable the system dropped to Disk BASIC, and `CALL SYSTEM` behaved the same way. When booting in MSX-DOS 1 mode, Nextor 3 instead scans all the drives in order and boots from the first one holding a disk with a valid boot sector; that drive becomes the default drive, so `MSXDOS.SYS` and `COMMAND.COM` are loaded from it. This matters in Nextor because the internal floppy disk drive isn't necessarily drive A:, e.g. when other storage devices take the first drive letters. See _[3.2.1. Booting in DOS 1 mode](Nextor_3.0_User_Manual.md#321-booting-in-dos-1-mode)_ in the user manual.

### 2.11. New and changed tools and commands

* New [`DRVROP.COM`](Nextor_3.0_User_Manual.md#3413-drvrop-the-driver-operations-tool) ("driver operations") command line tool, and new [`CALL IDRIVER`](Nextor_3.0_User_Manual.md#3612-the-call-idriver-command) and [`CALL UDRIVER`](Nextor_3.0_User_Manual.md#3613-the-call-udriver-command) BASIC commands: they install and uninstall drivers loaded in RAM (see _[2.2. Drivers loadable in RAM](#22-drivers-loadable-in-ram)_).

* The `MAPDRV`, `DEVINFO` and `DRIVERS` command line tools, as well as the related `CALL` commands, now understand drivers loaded in RAM: wherever a driver is specified or displayed, a RAM segment number can accompany the driver slot number. See _[3.4. The command line tools](Nextor_3.0_User_Manual.md#34-the-command-line-tools)_ and _[3.6.10. The CALL MAPDRV command](Nextor_3.0_User_Manual.md#3610-the-call-mapdrv-command)_.

* The `MAPDRV` tool and the `CALL MAPDRV` command can map a drive to a device while skipping the partition assignment (new "s" and -3 partition parameter values, respectively): the drive is attached to the device with no partition, and the first suitable partition is searched automatically on each access to the drive. This makes it possible to map a drive to an offline removable device, or to a device that hasn't been partitioned yet. See _[3.4.1. MAPDRV: the drive mapping tool](Nextor_3.0_User_Manual.md#341-mapdrv-the-drive-mapping-tool)_ and _[3.6.10. The CALL MAPDRV command](Nextor_3.0_User_Manual.md#3610-the-call-mapdrv-command)_.

* Since logical units don't exist anymore, the tools and commands that used to take or display a logical unit number no longer do.

* The classic transient tools of MSX-DOS 2 (`CHKDSK`, `UNDEL`, `DISKCOPY`, `FIXDISK`, `KMODE`, `XCOPY` and `XDIR`) are now part of Nextor: rewritten from the MSX-DOS 2.20 versions, built with the rest of the tools and included in the tools disk together with their help files. The highlights: `CHKDSK` and `UNDEL` now handle FAT16 volumes besides FAT12 (both require Nextor); `DISKCOPY` can copy a disk using a single drive (swapping the disks in several passes) and by default preserves the boot sector of the target disk; `FIXDISK` gains a `/B` switch that writes a standard boot sector; `KMODE /S` now safely refuses disks with a standard boot sector instead of corrupting them; `XCOPY` gains the `/Dx` switch family to control what happens when the destination file already exists (overwrite, skip, keep the newer/older/smaller/bigger, or ask per file); and `XDIR` displays sizes and totals of any magnitude correctly on FAT16 volumes, displays sizes of 10K and over in kilobytes following the same rules as the `DIR` command of `COMMAND3.COM` (see _[2.12. The new COMMAND3.COM command interpreter](#212-the-new-command3com-command-interpreter)_) and gains a `/B` switch that displays all the sizes in bytes. As a general behavior change, all of these tools except `XDIR` now require their main argument and display a usage summary when run without arguments (`DISKCOPY` no longer prompts for the drives, and `XCOPY` no longer copies the current directory onto itself; a bare `XDIR` still lists the current directory). See _[3.4.15. The classic MSX-DOS tools](Nextor_3.0_User_Manual.md#3415-the-classic-msx-dos-tools)_ in the user manual.

### 2.12. The new COMMAND3.COM command interpreter

Nextor 3 introduces its own command interpreter: `COMMAND3.COM`. It is based on COMMAND 2.44 (so everything you know from it applies: internal commands, aliases, command line editing and history, batch file enhancements, the HELP command, etc.) but adds new features specific to Nextor 3:

* New internal commands `MAPDRV`, `DRIVERS`, `DRVINFO`, `DEVINFO`, `LOCK`, `RALLOC` and `Z80MODE`: the Nextor command line tools of the same names are now built into the interpreter, no `.COM` file needed (the tools are still supplied, since they also work with older interpreter and Nextor versions).

* New internal command `MEM`: displays a compact memory mapper listing that fits in 32 columns.

* New internal command `SHELLRAM`: allows freeing the RAM segment that the interpreter normally allocates for the command history and the aliases, for users who need every RAM segment they can get.

* New internal command `YENSLASH`: on Japanese (and Korean) computers, whose character set has the yen (won) sign in place of the backslash, redefines that character as a regular backslash in the video RAM, so that paths are displayed as `A:\DIR\FILE` instead of `A:¥DIR¥FILE` (in SCREEN 0 and SCREEN 1; it has no effect while the kanji driver is installed, since the driver draws the characters itself). The state is recorded in an environment item of the same name, read when the interpreter starts, so it survives visits to BASIC (a `/T` option, also added to `SHELLRAM`, changes the state without recording it), and the redefinition is repeated before every prompt, since any screen initialization restores the original character set.

* The `DIR` command displays the file sizes, the total size of the listed files and the free space figure in kilobytes (rounded to the nearest, with a `K` suffix) when they are 10K or over, and in bytes below that. The threshold can be changed with the new `DIRK` environment item (a number of kilobytes from 1 to 65535, optionally followed by one or two letters to be used as the suffix of those figures instead of `K`; `0` or `OFF` keeps the file sizes in bytes, with the totals in kilobytes from 1K up, `0` accepting the suffix letters too), and the new internal command `DIRB` is the same as `DIR` but displays all the figures in bytes. The `XDIR` tool follows the same rules, with a new `/B` switch as the equivalent of `DIRB`.

* The `FORMAT` command now works for drives mapped to floppy disk devices handled by Nextor drivers (`COMMAND2.COM` can only format drives controlled by legacy MSX-DOS drivers), and gains a new `/Q` switch that performs a quick format: only the allocation table and the root directory are cleared.

`COMMAND3.COM` contains all its messages in both English and Japanese, like the `COMMAND2.COM` of the Japanese MSX-DOS 2 did: the Japanese messages are used while the kanji mode is active, unless the `ERRLANG` environment item is set to `EN` (the same rules that the kernel and `NEXTOR.SYS` apply to the error messages). To make room for the second language, the list of subjects printed by `HELP` with no parameters is no longer built into the interpreter: it is read from the new `INDEX.HLP` file (`JINDEX.HLP`, in Japanese, while the Japanese messages are active, falling back to `INDEX.HLP` when that file does not exist) in the help directory, and "File for HELP not found" (followed by a hint about where the index file is expected to be) is reported when the file is not present. See _[3.10.5. Japanese messages](Nextor_3.0_User_Manual.md#3105-japanese-messages)_ in the user manual for the details.

`COMMAND3.COM` requires a Nextor 3 kernel and version 3 of `NEXTOR.SYS`, which loads it when present and falls back to loading `COMMAND2.COM` otherwise; any `COMMAND2.COM` from version 2.20 still works with Nextor 3, but without the new features. See _[3.10. The COMMAND3.COM command interpreter](Nextor_3.0_User_Manual.md#310-the-command3com-command-interpreter)_ in the user manual for the details.

## 3. Information for application developers

### 3.1. New function call: driver operations (_DRVRO, 7Fh)

The new [`_DRVRO`](Nextor_3.0_Programmers_Reference.md#315-driver-operations-_drvro-7fh) function performs operations on device drivers; currently these are: initialize and register a driver loaded in RAM, and shut down and unregister it. It is the function behind `DRVROP.COM` and `CALL IDRIVER`/`CALL UDRIVER`, and what you would use to write a custom driver installer tool. It is not available in MSX-DOS 1 mode.

### 3.2. Changed function calls

* [`_FORMAT`](Nextor_3.0_Programmers_Reference.md#27-_format-67h): formatting now works for drives mapped to Nextor drivers (that support it), not only for MSX-DOS drivers. A new choice number, 80h, gets the format choice string into a RAM buffer and works for both driver types; the old choice 00h works for MSX-DOS drivers only and is deprecated. The documentation now also covers the special boot sector related choices dating back to the MSX-DOS 2 era (FEh, FFh) and to Nextor 2 (FBh-FDh), all of which work for both driver types as well.

* [`_GDRVR`](Nextor_3.0_Programmers_Reference.md#38-get-information-about-a-device-driver-_gdrvr-78h): driver names can now be up to 255 characters long, and a new input flag requests the extended name (otherwise names are truncated as before). The returned driver flags always have the legacy "device-based driver" flag set for compatibility with Nextor 2-aware tools.

* [`_GDLI`](Nextor_3.0_Programmers_Reference.md#39-get-information-about-a-drive-letter-_gdli-79h): a new drive status value identifies drives assigned to a ghost floppy disk drive, with an extra indication of which drive letter is the main one.

* [`_GPART`](Nextor_3.0_Programmers_Reference.md#310-get-information-about-a-device-partition-_gpart-7ah): the logical unit number input parameter is gone, and the driver segment number (which was always FFh in Nextor 2) is now meaningful (for drivers loaded in RAM).

* [`_CDRVR`](Nextor_3.0_Programmers_Reference.md#311-call-a-routine-in-a-device-driver-_cdrvr-7bh): the driver slot byte must now have bits 6-4 set to 001. This is a deliberate incompatibility: the set of routines exposed by drivers has changed, and this safeguard prevents old Nextor 2 programs from unknowingly calling routines of a Nextor 3 driver.

* [`_MAPDRV`](Nextor_3.0_Programmers_Reference.md#312-map-a-drive-letter-to-a-driver-and-device-_mapdrv-7ch): the logical unit number byte of the mapping data buffer is now unused, new rules govern the automatic mapping of ghost drives, and a starting sector of FFFFFFFFh in the mapping data attaches the drive to the device with no partition assigned (one is searched automatically on each access to the drive).

### 3.3. New error codes

The error codes are listed in _[4. New error codes](Nextor_3.0_Programmers_Reference.md#4-new-error-codes)_ in the programmers reference. Compared to Nextor 2.1:

* `.IDEVN` ("Invalid device number") replaces `.IDEVL` ("Invalid device or logical unit number") with the same numeric error code, since logical units don't exist anymore; the matching BASIC error is renamed accordingly.

* `.INITE` ("Initialization error") is returned by `_DRVRO` when the initialization routine of a driver loaded in RAM fails; it has a matching new BASIC error too. See _[3.7. New BASIC error codes](Nextor_3.0_User_Manual.md#37-new-basic-error-codes)_ in the user manual.

* `.NOCMD` ("Command interpreter not found") and `.IDOSV` ("Incompatible DOS version") are used by NEXTOR.SYS to explain why a jump to the BASIC environment was forced. They work through the new `ERR_TO_BASIC` mechanism (a DOS error code stored at F245h is printed at the next entry to BASIC), which application programs can use as well; see _[7.3. DOS environment load errors](Nextor_3.0_Programmers_Reference.md#73-dos-environment-load-errors)_.

### 3.4. Nextor in MSX-DOS 1 mode

The Nextor-specific function calls available in MSX-DOS 1 mode are the same as in Nextor 2 (`_DOSVER`, `_GDRVR`, `_GPART`, `_CDRVR`, `_GDLI`, and `_MAPDRV` with restrictions), plus `_FORMAT`, which is newly available in this mode. `_DRVRO` is not available: drivers loaded in RAM don't work in MSX-DOS 1 mode, and neither do the new `CALL IDRIVER` and `CALL UDRIVER` commands. The usual restrictions for function calls in this mode still apply; see the introduction of _[3. New function calls](Nextor_3.0_Programmers_Reference.md#3-new-function-calls)_ in the programmers reference, and _[3.2.1. Booting in DOS 1 mode](Nextor_3.0_User_Manual.md#321-booting-in-dos-1-mode)_ in the user manual.

### 3.5. The Nextor SDK and the Docker development image

Nextor 3 ships with an SDK (Software Development Kit): a collection of assembler include files and C headers with the function call and error code constants, the driver structure definitions, helper macros and ready-to-use code snippets, plus project templates for drivers and for command line tools. It lives in the [`sdk`](https://github.com/Konamiman/Nextor/tree/HEAD/sdk) directory of the Nextor repository, and it is designed to be pulled into your own projects (e.g. as a git submodule); see [the SDK README file](https://github.com/Konamiman/Nextor/blob/HEAD/sdk/README.md) for the details.

Additionally, a Docker image for Nextor development, with the required assembler and C compiler preinstalled, is published as `ghcr.io/konamiman/nextor-dev`. See _[8. Development helpers](Nextor_3.0_Programmers_Reference.md#8-development-helpers)_ in the programmers reference for an overview of both.


## 4. Information for driver developers

The driver structure of Nextor 3 is completely new and incompatible with Nextor 2: drivers have a new signature (`NEXTORv3_DRIVER`), the old fixed table of routines has been replaced by a query-based API (driver queries and device queries), logical units and drive-based drivers are gone, drivers can be loaded in RAM, and drivers can expose floppy disk drives, including support for formatting them. There are also new kernel entry points available to drivers, such as [`CALLB0_IX_IY`](Nextor_3.0_Driver_Development_Guide.md#427-callb0_ix_iy-404bh).

The good news is that the changes are mostly at the driver API level rather than at the driver functionality level, so most of the code of an existing Nextor 2 driver can be reused: it is mostly a matter of adding a compatibility layer that adapts it to the new structure. The _[Nextor 3.0 Driver Migration Guide](Nextor_3.0_Driver_Migration_Guide.md)_ explains how to do exactly that. The complete reference for the new driver system is _[4. Nextor driver structure](Nextor_3.0_Driver_Development_Guide.md#4-nextor-driver-structure)_ in the _[Nextor 3.0 Driver Development Guide](Nextor_3.0_Driver_Development_Guide.md)_; and once your driver is written, the new `DRVTEST.COM` tool will help you exercise it: see _[5. Testing drivers with DRVTEST.COM](Nextor_3.0_Driver_Development_Guide.md#5-testing-drivers-with-drvtestcom)_.

The free area for driver code or data at the end of the kernel main banks (the `/e:` option of `mknexrom`) shrinks from 1K at 7BD0h to 256 bytes at 7ED0h-7FCFh: the kernel now uses the rest. Drivers that used that area must move their contents to the new location (and update anything that points into it, such as hooks) and fit them in 256 bytes; `mknexrom` refuses the extra file if the area isn't empty in the kernel base file. The new `DRIVER_EXTRA_AREA` and `DRIVER_EXTRA_AREA_SIZE` SDK constants give the location and size. See _[4.7.1. The free space at kernel main bank](Nextor_3.0_Driver_Development_Guide.md#471-the-free-space-at-kernel-main-bank)_.
