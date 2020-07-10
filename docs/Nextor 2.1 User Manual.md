# Nextor 2.1 User Manual

## Index

[1. Introduction](#1-introduction)

[1.1. Background](#11-background)

[1.2. Goals](#12-goals)

[1.3. System requirements](#13-system-requirements)

[2. Features](#2-features)

[2.1. FAT16 filesystem support](#21-fat16-filesystem-support)

[2.2. Standarized and documented driver development system](#22-standarized-and-documented-driver-development-system)

[2.3. Drive to device/partition mapping management](#23-drive-to-devicepartition-mapping-management)

[2.4. Drive lock](#24-drive-lock)

[2.5. Reduced and zero allocation information mode](#25-reduced-and-zero-allocation-information-mode)

[2.6. Z80 access mode](#26-z80-access-mode)

[2.7. Fast STROUT mode](#27-fast-strout-mode)

[2.8. Extended mapper support routines](#28-extended-mapper-support-routines)

[2.9. Boot keys](#29-boot-keys)

[2.9.1. Boot key inverters](#291-boot-key-inverters)

[2.9.2. One-time boot keys](#292-one-time-boot-keys)

[2.10. Built-in partitioning tool](#210-built-in-partitioning-tool)

[2.11. Embedded MSX-DOS 1](#211-embedded-msx-dos-1)

[2.12. Enhanced Disk BASIC](#212-enhanced-disk-basic)

[2.13. File mounting and disk emulation mode](#213-file-mounting-and-disk-emulation-mode)

[3. Using Nextor](#3-using-nextor)

[3.1. Installing Nextor](#31-installing-nextor)

[3.1.1. Note for Sunrise IDE/CF users](#311-note-for-sunrise-idecf-users)

[3.2. Booting Nextor](#32-booting-nextor)

[3.2.1. Booting in DOS 1 mode](#321-booting-in-dos-1-mode)

[3.3. Managing media changes](#33-managing-media-changes)

[3.3.1. Media changes in MSX-DOS 1 mode](#331-media-changes-in-msx-dos-1-mode)

[3.4. The command line tools](#34-the-command-line-tools)

[3.4.1. MAPDRV: the drive mapping tool](#341-mapdrv-the-drive-mapping-tool)

[3.4.2. DRIVERS: the driver information tool](#342-drivers-the-driver-information-tool)

[3.4.3. DEVINFO: the device information tool](#343-devinfo-the-device-information-tool)

[3.4.4. DRVINFO: the drive information tool](#344-drvinfo-the-drive-information-tool)

[3.4.5. LOCK: the drive lock and unlock tool](#345-lock-the-drive-lock-and-unlock-tool)

[3.4.6. RALLOC: the reduced/zero allocation information mode tool](#346-ralloc-the-reducedzero-allocation-information-mode-tool)

[3.4.7. Z80MODE: the Z80 access mode tool](#347-z80mode-the-z80-access-mode-tool)

[3.4.8. FASTOUT: the fast STROUT mode tool](#348-fastout-the-fast-strout-mode-tool)

[3.4.9. DELALL: the partition quick format tool](#349-delall-the-partition-quick-format-tool)

[3.4.10. NSYSVER: the NEXTOR.SYS version changer](#3410-nsysver-the-nextorsys-version-changer)

[3.4.11. NEXBOOT: the one-time boot keys configuration tool](#3411-nexboot-the-one-time-boot-keys-configuration-tool)

[3.4.12. EMUFILE: the disk emulation mode tool](#3412-emufile-the-disk-emulation-mode-tool)

[3.5. The built-in partitioning tool](#35-the-built-in-partitioning-tool)

[3.6. Extensions to Disk BASIC](#36-extensions-to-disk-basic)

[3.6.1. The DSKF command](#361-the-dskf-command)

[3.6.2. The DSKI$ and DSKO$ commands](#362-the-dski-and-dsko-commands)

[3.6.3 The CALL NEXTOR command](#363-the-call-nextor-command)

[3.6.4 The CALL CHDRV command](#364-the-call-chdrv-command)

[3.6.5 The CALL CURDRV command](#365-the-call-curdrv-command)

[3.6.6. The CALL DRIVERS command](#366-the-call-drivers-command)

[3.6.7. The CALL DRVINFO command](#367-the-call-drvinfo-command)

[3.6.8. The CALL LOCKDRV command](#368-the-call-lockdrv-command)

[3.6.9. The CALL MAPDRV command](#369-the-call-mapdrv-command)

[3.6.10. The CALL MAPDRVL command](#3610-the-call-mapdrvl-command)

[3.6.11. The CALL USR command](#3611-the-call-usr-command)

[3.7. New BASIC error codes](#37-new-basic-error-codes)

[3.8. Mounting files](#38-mounting-files)

[3.9. Disk emulation mode](#39-disk-emulation-mode)

[3.9.1. Entering and exiting the disk emulation mode](#391-entering-and-exiting-the-disk-emulation-mode)

[3.9.2. Changing the image file](#392-changing-the-image-file)

[3.9.3. Rules and restrictions](#393-rules-and-restrictions)

[3.9.4 How to free some memory](#394-how-to-free-some-memory)

[3.9.5. Known bugs](#395-known-bugs)

[4. Other improvements](#4-other-improvements)

[4.1. load" in F7](#41-load-in-f7)

[4.2. English error messages in kanji mode](#42-english-error-messages-in-kanji-mode)

[4.3. Reduced NEXTOR.SYS without Japanese error messages](#43-reduced-nextorsys-without-japanese-error-messages)

[5. Change history](#5-change-history)

[5.1. v2.1.0 beta 2](#51-v210-beta-2)

[5.2. v2.1.0 beta 1](#52-v210-beta-1)


## 1. Introduction

Nextor is an enhanced version of MSX-DOS 2, the disk operating system for MSX computers. It is based on MSX-DOS 2.31, with which it is 100% compatible.

This document provides a description of the features that Nextor adds to MSX-DOS 2 and is intended primarily for end users, but it explains basic concepts that will be useful for developers as well. There are however two other documents aimed specifically at developers: _[Nextor 2.1 Programmers Reference](Nextor%202.1%20Programmers%20Reference.md)_ and _[Nextor 2.1 Driver Development Guide](Nextor%202.1%20Driver%20Development%20Guide.md)_. The reader of this document is assumed to have experience with MSX-DOS 2 at least at the user level.

### 1.1. Background

MSX-DOS is the only official disk operating system for MSX computers. The last version, labeled 2.31, appeared in 1990 accompanying MSX Turbo-R computers.

MSX-DOS was developed in a time in which the only option for massive storage in MSX computers was the floppy disk, and when used as a "floppy disk only operating system" MSX-DOS works indeed just fine. Over the years, however, more modern massive storage options have appeared in the form of amateur-made hardware -- from the early 90's SCSI and IDE hard disk controllers to today's multimedia card readers. MSX-DOS has been used to manage these devices, but not without some problems:

*  MSX-DOS handles sector numbers as 16 bit entities, and the only filesystem it supports is FAT12. This limits the size of a single filesystem volume to 32MB. Unofficial patches have been developed to add support for the FAT16 filesystem.

*  The actual device driver (the code that interacts with the massive storage controller hardware) is embedded within the operating system kernel ROM, present in computers with built-in floppy disk drives and in external floppy disk controllers. There is no officially documented way to embed a custom device driver within the kernel ROM; developers of custom storage controller hardware have to reverse-engineer the kernel code in order to embed a custom driver.

*  There is a fixed direct, one-to-one correspondence between the drive letters as seen by the user and the device units exposed by the device driver API. For example, in order to access drive A:, MSX-DOS asks the driver to access its first device; while the second device is queried when accessing drive B:. This is OK for floppy disks, but when using more complex devices that have one or more partitions, it is up to the driver (and usually also to external tools made by the driver developer) to manage the drive to device and partition assignment.

*  Managing non-block devices (such as CD-ROMs) is extremely difficult, as it implies a hard work of reverse-engineering on the kernel code.


### 1.2. Goals

The primary goal of Nextor is to solve the aforementioned problems, by using MSX-DOS 2 as the basis for implementing the features that are needed for a MSX computer equipped with 21th century storage devices. More specifically, the main goals that Nextor development efforts aim to are:

*  Provide native support for the FAT16 filesystem.

*  Provide a standardized, well documented system for developing custom storage device drivers and either embedding them within the OS kernel ROM or loading them dynamically from RAM.

*  Provide a device-based driver system (in contrast with the MSX-DOS drive-based system), so that the driver developer must only worry about enumerating and accessing storage devices and it is the operating system who manages the device- and partition-to-drive assignment.

*  Provide support for non-block devices, and for block devices with filesystems other than FAT12/16.

Aside from the main goals, Nextor offers other secondary but also useful new features not present in MSX-DOS. Keep on reading for more details.

### 1.3. System requirements

Nextor will run on any MSX computer (from MSX1 onwards) having at least 128K of mapped memory. In computers with no mapped memory or having less than 128K in the largest mapper, Nextor will boot in MSX-DOS 1 mode (the DOS prompt is available only if the computer has 64K of RAM).

You can simply burn a standalone version of Nextor (with a dummy device driver) and use it together with storage controllers associated to a MSX-DOS kernel. You will then benefit from features such as the FAT16 filesystem support or the Z80 access mode; note however that the drive to device/partition mapping management feature requires a device driver specifically made for Nextor.

## 2. Features

This section overviews the features that actually make Nextor an enhanced version of MSX-DOS 2. Operational details are provided in further sections.

### 2.1. FAT16 filesystem support

Nextor provides built-in support for the FAT16 filesystem. There is no need to install any patch, and it is perfectly possible to boot the system from a FAT16 volume. Volumes up to 4GB in size can be used.

Additionally, standard boot sectors (those present in factory-formatted or partitioned devices, or in devices formatted or partitioned by PC computers) are fully supported as well. In contrast, MSX-DOS 2 treated all disks not formatted by itself as MSX-DOS 1 disks.

### 2.2. Standarized and documented driver development system

Developers of custom storage controller hardware now have a standardized and well-documented system for developing custom drivers. The driver structure, the details about the routines to be implemented and the "recipe" for embedding the driver within the Nextor kernel are provided so that no more reverse-engineering is needed.

The driver main purpose is to enumerate and access storage devices, but it also contains some extensibility points to add custom BASIC statements (via CALL command), extended BIOS commands, or a timer interrupt service routine.

The following resources are available for Nextor device driver developers:

*  The _[Nextor 2.1 Driver Development Guide](Nextor%202.1%20Driver%20Development%20Guide.md)_ document.

*  A template driver file, DRIVER.ASM, that can be used as the skeleton for developing custom drivers.

*  A command line utility, MKNXEROM, that will do all the work of embedding a device driver within the Nextor kernel ROM. It is provided as a Windows executable (MKNEXROM.EXE) and as a standard C source file (MKNEXROM.C).

Note: at this time it is not possible to invoke the FORMAT command on a drive mapped to a device controlled by a Nextor device-based driver. This will change in a future version of Nextor.

### 2.3. Drive to device/partition mapping management

Driver developers can choose between two driver styles when developing a Nextor device driver: the _drive-based driver_ and the _device-based driver_. The former mimics the MSX-DOS drivers by providing a one-to-one mapping between OS drive letters and driver units; it is still the responsibility of the driver to manage the drive to device and partition mapping. The latter is way more interesting.

Device-based drivers do not work in terms of driver units but directly in terms of devices. This means that the driver has no routines like "Read sector X of unit N" but rather "Return information of device X" and "Read raw data from device X". A device-based driver can handle up to seven devices, and each device can have from one to seven logical units.

The best part is that when using device-based drivers, Nextor will handle the assignment of devices and partitions to drive letters, both automatically (at boot time, see _[3.2. Booting Nextor](#32-booting-nextor)_) and manually (by using a mapping utility that in turn invokes a new function call, see _[3.4.1. MAPDRV: the drive mapping tool](#341-mapdrv-the-drive-mapping-tool)_, and _[3.6.9. The CALL MAPDRV command](#369-the-call-mapdrv-command)_). The driver developer only needs to implement raw access to the device.

### 2.4. Drive lock

Nextor allows marking drives as locked. When a drive is locked, the kernel code will not ask the driver if the media in the drive has changed; instead, it will assume that the user will never change the media. This is useful when a removable device such as a multimedia card is used as the main storage device, as it prevents the kernel to waste time executing media verification code. 
Drives can be locked by using the supplied tool LOCK.COM or by invoking the CALL LOCKDRV command from within the BASIC prompt. All drives can be locked, even those belonging to MSX-DOS drivers (including floppy disk drives). See _[3.4.5. LOCK: the drive lock and unlock tool](#345-lock-the-drive-lock-and-unlock-tool)_, and _[3.6.8. The CALL LOCKDRV command](#368-the-call-lockdrv-command)_.

### 2.5. Reduced and zero allocation information mode

Nextor allows setting drives in reduced allocation information mode. When in this mode, the ALLOC function, which returns information about the total and free space available in a drive, will return fake information if necessary, so that the calculated total or free sector count will always fit in 16 bits. In other words, on drives with the reduced allocation information mode active, when the total or free space is greater than 32MB (which is possible in FAT16 volumes), ALLOC will return 32MB. See _[3.4.6. RALLOC: the reduced/zero allocation information mode tool](#346-ralloc-the-reducedzero-allocation-information-mode-tool)_.

This feature is intended to avoid compatibility issues with applications that assume the underlying filesystem to be always FAT12 and therefore expect a total or free space information of up to 32MB. 

If an environment item named ZALLOC is created with a value –case insensitive- of ON (command `SET ZALLOC=ON` in the command interpreter prompt), the reduced allocation information mode becomes the zero allocation information mode. In this case, ALLOC will return a free space of zero for the drives that have this mode active. This is useful because calculating the free space on a device (at the end of a DIR command, for example) may take a somewhat long time on large devices (about 4 seconds in Z80 mode for a SD card, for example); when the zero allocation information mode is active, this time is reduced to zero.

The zero allocation information mode is available since Nextor 2.0.3.

### 2.6. Z80 access mode

In MSX Turbo-R computers MSX-DOS 2 always switches to the Z80 CPU when accessing a disk driver. Nextor will never change the CPU when accessing drivers attached to a Nextor kernel, but when accessing drivers attached to a MSX-DOS kernel it is possible to have the Z80 access mode active or not. When active, Nextor will switch to Z80 before accessing the driver, as MSX-DOS does. See _[3.4.7. Z80MODE: the Z80 access mode tool](#347-z80mode-the-z80-access-mode-tool)_.

The Z80 access mode is active by default for all MSX-DOS drivers. It is possible to switch it on or off on a per driver basis (it is not possible to change it for specific drive letters).

### 2.7. Fast STROUT mode

The MSX-DOS function STROUT prints a string terminated with a "$" character. What this function actually does is to perform one separate call to the CONOUT function (which prints one single character) for every character of the string.

Nextor introduces the _fast STROUT_ mode. When this mode is active, the string will be copied to a 512 byte buffer in page 3 and then it will be printed in one single call to the kernel code, which increases the speed of the printing process. The drawback is that the string length is limited to 511 bytes when this mode is active; longer strings will be truncated before being displayed. See _[3.4.8. FASTOUT: the fast STROUT mode tool](#348-fastout-the-fast-strout-mode-tool)_.

### 2.8. Extended mapper support routines

MSX-DOS 2 provides a set mapper support routines, which allow applications to allocate 16K RAM segments. Nextor maintains the original routines, but provides two new ones that allow allocating a contiguous block of memory (from 1 byte to 16K) inside a given segment. See the _[Nextor 2.1 Programmers Reference](Nextor%202.1%20Programmers%20Reference.md)_ for details.

### 2.9. Boot keys

The boot time configuration of Nextor can be modified by keeping pressed some special keys while the system is booting. These keys and their behavior are:

*  **0**: Disable permanent disk emulation mode by deleting the emulation data file pointer from the partition table. See _[3.9. Disk emulation mode](#39-disk-emulation-mode)_.

*  **1**: Force boot in MSX-DOS 1 mode. If the computer is a MSX Turbo-R, switches CPU to Z80 mode.

*  **2**: Force boot in MSX-DOS 1 mode. If the computer is a MSX Turbo-R, switches CPU to R800-ROM mode. Note that in MSX-DOS 1 mode, the active CPU is never changed when accessing disk drives; this may cause some storage devices to not work properly, especially those mapped to MSX-DOS drivers such as floppy disk drives.

*  **3**: Force boot to the BASIC prompt, ignoring any existing boot code (that is, do not try to load and run NEXTOR.SYS, AUTOEXEC.BAS or the code in the boot sector).

*  **4**: (for MSX Turbo-R only) Boot in R800-ROM mode, assign the largest mapper found as the primary mapper (instead of the internal mapper), and free the 64K allocated for the R800-DRAM mode. This is useful for using software that requires a huge amount of mapped RAM and can work only with the primary mapper; note however that there is a big penalty in the system speed.

*  **5**: Assign only one drive to each Nextor kernel with a device-based driver regardless of the number of devices controlled by the driver. This overrides the normal behavior, in which Nextor assigns one drive per device found (see _[3.2. Booting Nextor](#32-booting-nextor)_). This is just the default behavior, though - drivers can override it they implement [the DRV_CONFIG routine](Nextor%202.1%20Driver%20Development%20Guide.md#448-drv_config-4151h)).

* **CTRL**: The state of this key is passed to MSX-DOS kernels on initialization. Typically this will cause the internal floppy disk drive to disable it second "ghost" drive, allowing to free some extra memory, especially in MSX-DOS 1 mode. Note that this key is inverted by default so you'll get the opposite behavior unless you customize the Nextor ROM (see _[2.9.1. Boot key inverters](#291-boot-key-inverters)_).

*  **SHIFT**: Prevent MSX-DOS kernels from booting, but allow Nextor kernels to boot normally. This is useful to disable the internal floppy disk drive in order to get some extra TPA memory, especially in MSX-DOS 1 mode.

*  **slot key**: Prevent the Nextor kernel associated to the specified slot key from booting. This is useful when the kernel ROM must be updated so you need to disable it. The associated keys for each slot are:

    * Q for primary slot 1
    * A for primary slot 2
    * QWER for slots 1-0 to 1-3, respectively
    * ASDF for slots 2-0 to 2-3, respectively
    * ZXCV for slots 3-0 to 3-3, respectively
    * In the rare event that you have a Nextor kernel at slot 0, use the keys UIOP for 0-0 to 0-3, respectively.
    
Example: if your Nextor kernel is in primary slot 1, press Q to prevent it from booting. If you have it in slot 2-3, press F.

#### 2.9.1. Boot key inverters

The Nextor kernel has two bytes, at offsets 512 and 513 in the ROM that act as _boot key inverters_. There's one bit assigned to each of the keys that affect the booting process (not including the slot keys), and when that bit it set, then the meaning of the key is inverted. For example, if the bit for the SHIFT key is pressed, then MSX-DOS drivers will be disabled unless SHIFT is pressed while booting.

Being hardcoded values, the only way to customize them is to modify the Nextor ROM file before flashing it into your device. The MKNEXROM.EXE tool can be used for that, or you can do it manually using a hexadecimal editor.

Here's how bits are assigned to each key:

* First byte (offset 512):

  * Bits 1 to 5: keys 1 to 5

* Second byte (offset 513):

  * Bit 5: CTRL key
  * Bit 4: SHIFT key

All other bits are currently unused and should always be 0 to ensure compatibility with possible future extensions.

If you use MKNEXROM.EXE you need to supply a 16 bit hexadecimal value with the `/k` parameter. You should build that number by adding the values for each key as follows:

  * 1: 0002
  * 2: 0004
  * 3: 0008
  * 4: 0010
  * 5: 0020
  * CTRL: 2000
  * SHIFT: 1000

e.g. `/k:3002` to invert the 1, CTRL and SHIFT keys.

By default (when using the "official" ROM files) only the CTRL key is inverted, so that by default internal floppy disk drives will boot with only one drive letter assigned.

#### 2.9.2. One-time boot keys

There's an alternative way to modify the Nextor booting procedure: the _one-time boot keys_. If at boot time Nextor finds a certain signature at a certain position in RAM, it will read a handful of bytes following that signature and use them as the values for the boot keys (including the slot keys), ignoring the keyboard. The RAM area used is at page 2, therefore this only works on computers with at least 32K RAM.

Being a RAM based mechanism, it's "one-time" in the sense that it won't work again on the next computer reset unless the signature and the key data is put on memory again. The signature is explicitly erased by Nextor after being read to make this behavior consistent.

The [NEXBOOT.COM tool](#3411-nexboot-the-one-time-boot-keys-configuration-tool) can be used to easily set this data and reset the computer, but all the tool does is writing to RAM, and thus any other tool could be used instead. The details on the location and format of the data used by this mechanism are in the _[Nextor 2.1 Programmers Reference](Nextor%202.1%20Programmers%20Reference.md)_ document.

### 2.10. Built-in partitioning tool

The Nextor kernel has a built-in device partitioning tool that can be started by just executing CALL FDISK in the BASIC prompt. It can be used to create partitions of any size between 100KB and 4GB on devices controlled by Nextor device based drivers. See _[3.5. The built-in partitioning tool](#35-the-built-in-partitioning-tool)_.

### 2.11. Embedded MSX-DOS 1

The Nextor kernel contains the MSX-DOS 1 kernel, so that it is possible to boot in this environment when necessary. The Nextor version of MSX-DOS 1 does not provide any additional functionality to users or developers relative to the original version, however it has been modified internally so that it can access devices attached to Nextor drivers. See _[3.2.1. Booting in DOS 1 mode](#321-booting-in-dos-1-mode)_.

### 2.12. Enhanced Disk BASIC

Disk BASIC has been extended with new commands. Also, some of the existing commands have been improved. See _[3.6. Extensions to Disk BASIC](#36-extensions-to-disk-basic)_.

### 2.13. File mounting and disk emulation mode

Since version 2.1 Nextor allows to mount disk image files in two ways:

* Botting normally and mounting a disk image file in a drive. See [3.8. Mounting files](#38-mounting-files).

* Booting in disk emulation mode, so the system boots in MSX-DOS 1 mode and uses a set disk image files (one at a time) as the boot device. See [3.9. Disk emulation mode](#39-disk-emulation-mode).


## 3. Using Nextor

This section explains the operational details of Nextor and the associated utilities.

### 3.1. Installing Nextor

Nextor consists of the following components:

* The Nextor kernel ROM. It must contain a device driver, although a "standalone" version is provided which contains a dummy driver exposing no devices.

* The NEXTOR.SYS file, which is necessary in order to boot in the DOS prompt. This file has the role that MSXDOS2.SYS had in MSX-DOS 2 (in fact, NEXTOR.SYS is just an extended version of MSXDOS2.SYS).

* The COMMAND2.COM file. There is no special command interpreter for Nextor; instead, the same command interpreter of MSX-DOS 2 is used (any version of COMMAND2.COM from 2.20 will do), and new features are handled by using external commands.

**Note:** two variants of the NEXTOR.SYS file exist. See _[4.3. Reduced NEXTOR.SYS without Japanese error messages](#43-reduced-nextorsys-without-japanese-error-messages)_.

**Note:** starting with Nextor 2.1.0 beta 2, the kernel will try to load MSXDOS2.SYS if NEXTOR.SYS is not found. However in this case the Nextor command line tools won't work.

In order to boot in the MSX-DOS 1 prompt, you need the usual MSXDOS.SYS and COMMAND.COM files. Also, if you have just the kernel and no NEXTOR.SYS or MSXDOS.SYS files, Nextor will boot in the BASIC prompt (running AUTOEXEC.BAS if present).

Therefore, in order to "install" Nextor, you have two options:

1.  Burn a ROM with the appropriate Nextor driver directly in the storage device controller.

2.  Burn a standalone version in a flash ROM cartridge, and use it together with your storage device controller in another slot.

Also, you need to copy at least NEXTOR.SYS and COMMAND2.COM to your boot device (it is recommended to have the associated utilities available as well) unless you are happy in the BASIC prompt. More details about the boot procedure follow.

#### 3.1.1. Note for Sunrise IDE/CF users

If you want to burn the Nextor kernel ROM in a Sunrise IDE cartridge or in a Sunrise CF reader cartridge, you can't use the IDEFLOAD.COM program, because it assumes that the file to be burned has a size of 64K but the Nextor kernel is bigger (128K including the driver in current version). Instead, you should use IDEFL128.COM, a modified version of the program that burns 128K files. This program is available at [Konamiman's web site](https://www.konamiman.com/msx/msx-e.html#ide) as well.

Also, note that the Sunrise IDE driver supplied with Nextor 2.0 is an experimental driver that has some limitations, more precisely:

*  Only LBA mode block devices are supported, there is no support for CHS or ATAPI devices.

*  Devices are reported as fixed by the driver. If you are using a CF card reader and you want to change the card(s), you must switch off your computer; hot swapping of cards is not supported.

### 3.2. Booting Nextor

The Nextor booting procedure is similar to the one performed by MSX-DOS 2. However, if Nextor device-based drivers are present, things are a little different since it is necessary to perform a drive to device and partition mapping for all the drives attached to Nextor drivers (if you are not using any Nextor kernel with a device-based driver attached, then the booting procedure is identical to MSX-DOS 2).

At boot time, Nextor will perform a query to all the available device-based drivers to find out how many devices are being controlled by these drivers (actually, how many logical units, but devices will usually have one single logical unit), and will assign to each driver as many drives as devices are controlled by the driver. If 5 is pressed at boot time, only one drive is assigned to each driver instead (see _[2.9. Boot keys](#29-boot-keys)_).

For example, assume that you have two Nextor kernels with a device-based driver attached. The kernel in slot 1 controls one device, while the kernel in slot 2 controls three devices. Then the initial drive assignment would be as follows:

```
A: for driver on slot 1
B:, C:, D: for driver on slot 2
E:, F: for the internal disk drive
```

If you boot while pressing 5, the assignment will be:

```
A: for driver on slot 1
B: for driver on slot 2
C:, D: for the internal disk drive
```

The internal disk drive would not have any drives attached if you pressed SHIFT while booting (see _[2.9. Boot keys](#29-boot-keys)_).

After all drives have been assigned to drivers, a device and partition to drive automatic mapping procedure will be run for each of these drives. Each drive is mapped to the first partition found that meets the following conditions:

1. The device doesn't have the "don't use for automapping" flag set (this flag is set by the driver)
2. Is a valid FAT12 or FAT16 partition (only FAT12 when booting in MSX-DOS 1 mode)
3. Has the "active" flag set in the partition table (this can be set using [FDISK](#35-the-built-in-partitioning-tool))
4. No drives have been already mapped to partitions in the same device

If no partitions are found that meet all three conditions, then the search is started over, but this time skipping the "active" flag check. If this fails again, absolute sector 0 of the device is checked (to see if the device doesn't have partitions but holds a valid FAT filesystem) as a last resort before leaving the drive unmapped.

Note that in order to speed up the botting procedure, only the first 9 partitions of each device are scanned during this procedure; consequently, [FDISK](#35-the-built-in-partitioning-tool) allows to change the "active" flag on these first 9 partitions only.

Starting with Nextor 2.0.5, device-based drivers can tell Nextor how many drives they want at boot time and which devices should be mapped to these drivers, bypassing part of this automatic procedure (partitions are still selected automatically). This feature is optional, and must be implemented by the driver developer.

After the automatic mapping is finished, the boot procedure will continue with the following steps:

1.  If the "3" key is being pressed, the system displays the BASIC prompt.

2.  Otherwise, if the NEXTOR.SYS (or MSXDOS2.SYS) and COMMAND2.COM files are present in the boot drive (the first drive that is not unmapped), the DOS prompt is shown after AUTOEXEC.BAT is executed (if present).

3.  Otherwise, if the boot drive has a MSX-DOS 1 or MSX-DOS 2 boot sector, its boot code is executed as in the case of MSX-DOS: first in the BASIC environment with the carry flag reset, then in the DOS environment with the carry flag set. This will usually cause MSXDOS.SYS and COMMAND.COM to be loaded if present.

4.  If the previous step returns, then the BASIC environment is activated, and AUTOEXEC.BAS is executed if present.

Note that step 3 will not be done if the disk has a standard boot sector (not created by MSX-DOS 1 or MSX-DOS 2). The built-in disk partitioning tool will create MSX-DOS 2 boot sectors for all partitions of 32MB or less, and standard boot sectors for larger partitions.

Starting with Nextor 2.1.0 beta 2, the Nextor kernel will load MSXDOS2.SYS if present when NEXTOR.SYS is not found, thus allowing booting from old MSX-DOS 2 disks. Note however that in this case the Nextor command line tools won't work.

#### 3.2.1. Booting in DOS 1 mode

Nextor kernel can boot in MSX-DOS 1 mode. This will happen if anything of the following conditions is met:

* The computer has no mapped memory, or the largest mapper has less than 128K.

* The boot drive has a MSX-DOS 1 boot sector (boot sectors not having standard format or MSX-DOS 2 format will be considered MSX-DOS 1 boot sectors).

* The "1" key or the "2" key is kept pressed while booting.

The boot procedure for MSX-DOS 1 mode is the same as for the normal (MSX-DOS 2 compatible) mode, with the following differences:

* During the automatic mapping procedure, only the MSX-DOS 1 compatible partitions will be examined. These are FAT12 partitions with three or less sectors per FAT.

* After the automatic mapping procedure, the NEXTOR.SYS and COMMAND2.COM search step is omitted.

Partitions of 16MB or less created with the built-in disk partitioning tool will have three sectors per FAT or less, so these can be used in MSX-DOS 1 mode.

Remember that MSX-DOS 1 can boot the DOS environment (MSXDOS.SYS and COMMAND.COM) if the computer has 64K of RAM. Otherwise, only Disk BASIC can be used.

On MSX Turbo-R computers, the CPU mode will be switched to Z80 when booting in MSX-DOS 1 mode, unless the 2 key is pressed during boot (see _[2.9. Boot keys](#29-boot-keys)_).

Note: when booting directly in the BASIC prompt in MSX-DOS 1 mode, it is no longer necessary to execute "POKE &HF346,1" prior to CALL SYSTEM.

### 3.3. Managing media changes

Before trying to read or write data from a device, MSX-DOS asks the device driver if the media has changed, in order to update its internal information about the accessed filesystem. Nextor does the same, but if the drive being accessed is mapped to a device-based driver, things get a little trickier because disk partitioning is involved.

When Nextor detects a media change in a drive mapped to a removable device-based driver, the following procedure is performed:

* The drive is mapped to the first available valid primary partition found on the device. Valid partitions are FAT12 and FAT16 partitions. If the device has no partition table, the drive is mapped to its absolute sector zero.

* All the other drives mapped to other partitions of the same device will be left unmapped.

The device driver may also reply "not sure" when asked for device change status. In that case, the procedure is as follows:

* When Nextor first reads the boot sector of a drive mapped to a device on a device-based driver, it calculates a 16 bit checksum of the boot sector contents and stores it together with the rest of the disk parameters.

* When Nextor asks the driver for device change status and the reply is "not sure", it re-reads the boot sector of the drive and calculates the checksum again. If it matches the previously stored checksum, Nextor assumes that the device has not been changed. Otherwise, it assumes that the device has changed and it performs the same mapping procedure as when the driver reports a device change.

It is recommended to lock drives mapped to removable devices in order to avoid unnecessary media checks (and unnecessary boot sector reads and checksum calculations).

#### 3.3.1. Media changes in MSX-DOS 1 mode

When Nextor is running in MSX-DOS 1 mode, media changes are not managed for drives mapped to device-based drivers. For these drives, Nextor will assume that the medium does never change, and therefore will never ask for change status information to the driver; if the medium is changed, it is necessary to manually inform Nextor about the change by issuing a CALL MAPDRV command from the BASIC prompt.

### 3.4. The command line tools

Nextor is supplied with a set of tools that allow managing the new capabilities available. All of these tools are .COM files intended to be executed from within the DOS prompt.

This section explains how to use these tools. Note however that you can also get a summary of the parameters accepted by each tool by invoking it without parameters; more detailed help is available as well by displaying the desired file directly with the TYPE command (for example: TYPE MAPDRV.COM).

All the tools rely on the new function calls provided by Nextor for its behavior. If you are a developer and want to know more details, please refer to the _[Nextor 2.1 Programmers Reference](Nextor%202.1%20Programmers%20Reference.md)_ document.

Please note that none of these tools work in MSX-DOS 1 mode. However there are equivalent BASIC CALL commands that provide equivalent functionality for most of the tools.

Some of the tools admit a `<driver slot>` parameter. In all of these, number 0 may be specified instead of a slot number, with the meaning of "the primary controller".

#### 3.4.1. MAPDRV: the drive mapping tool

MAPDRV.COM is a tool that allows mapping a drive letter to a partition on a device controlled by a Nextor device-based driver. It is possible to map any drive, even those initially unmapped or associated to a MSX-DOS driver or a Nextor drive-based driver.

The usage syntax for MAPDRV is:

```
MAPDRV [/L] <drive>: <partition>|d|u [<device index>-[<LUN index>]
       [<driver slot>[-<driver subslot>]]]
```

Partition number 1 refers to the first primary partition on the device. Partitions 2 to 4 refer to extended partitions 2-1 to 2-4 if partition 2 of the device is extended, otherwise they refer to primary partitions 2 to 4. Partitions 5 onwards always refer to the extended partition 2-(P-1).

If partition number 0 is specified, then the drive is mapped to the absolute sector zero of the device.

There are three options for specifying the device where the partition is located:

* Do not supply any parameter after the partition number. In this case, the partition is assumed to be in the same device already mapped to the drive (this works only if the drive is currently mapped to a device-based driver). 

* Supply a device index, but not a slot number. In this case, the partition is assumed to be in the specified device, and the device is assumed to be controlled by the kernel on the same slot of the currently mapped device (this works only if the drive is currently mapped to a device-based driver). 

* Supply a device index and a slot number. In this case, the slot corresponds to the Nextor kernel that contains the driver that handles the device.

When a device number is supplied, a logical unit number can be supplied too; default value for the logical unit number is 1.

If "d" is specified instead of a partition number, then the drive will be mapped to its default state, which can be one of the following:

* If the drive was unmapped at boot time, then it is left unmapped.

* If at boot time the drive was assigned to a MSX-DOS driver unit, or to a Nextor drive-based unit, then it is mapped to the same unit.

* If at boot time the drive was assigned to a Nextor device-based driver, then an automatic mapping procedure (equal to the one performed at boot time, except that the NEXTOR.DAT file is not searched) will be performed. This may or may not result in the drive having the same mapping it had at boot time, depending on the mapping state of the other drives.

If "u" is specified instead of a partition number, then the drive will be left unmapped

The optional parameter "/L" locks the drive immediately after doing the mapping (recommended for removable devices that will not be changed).

Since Nextor 2.1 the MAPDRV tool can be used to mount a disk image file in a drive as well. The syntax in this case is:

```
MAPDRV <drive> <file> [/ro]
```

The `/ro` parameter will cause the file to be mounted in read-only mode. However, if the file has the read-only attribute set, it will always be mounted in read-only mode, even if no `/ro` parameter is supplied.

There are some restrictions in place when mounting files to drives. See [3.8. Mounting files](#38-mounting-files) for details.


#### 3.4.2. DRIVERS: the driver information tool

The DRIVERS.COM utility, which is ran without parameters, displays information about the available MSX-DOS and Nextor drivers. It will display the name and version (for Nextor drivers only), the slot number, and the assigned drives at boot time. MSX-DOS drivers will be identified as "Legacy driver".

This tool is useful mainly to get the slot numbers of the drivers, in order to supply them as parameters to the other tools.

 #### 3.4.3. DEVINFO: the device information tool

The DEVINFO.COM utility displays information about the devices controlled by a given Nextor device-based driver. The information displayed includes the device name and manufacturer (when available), the device index, and the associated logical units types and sizes.

The usage syntax for DEVINFO is:

```
DEVINFO <driver slot>[-<driver subslot>]
```

This tool is useful mainly to get the device and logical unit indexes, in order to supply them as parameters to the MAPDRV tool.

#### 3.4.4. DRVINFO: the drive information tool

The DRVINFO.COM utility, which is ran without parameters, displays information about all the available drive letters (those that are not unmapped). The displayed information includes the associated driver slot and other information that depends on the associated driver type (driver name and version for Nextor drivers; device and logical unit numbers for Nextor device-based drivers; relative unit for MSX-DOS and Nextor drive-based drivers). MSX-DOS drivers are identified as "Legacy driver".

#### 3.4.5. LOCK: the drive lock and unlock tool

The LOCK.COM utility allows locking and unlocking drive letters. The usage syntax for LOCK is:

```
LOCK [<drive letter>: [ON|OFF]]
```

When ran without parameters, a list of the drive letters currently locked is shown. If only a drive letters is specified, the current lock status for the drive is shown.

When a drive is marked as locked, Nextor will never check the media change status for the drive; instead, the inserted media is assumed to never change. This speeds up media access, but be careful since data corruption may happen if the media is changed while it is locked.

Any disk error which is aborted will automatically unlock the involved drive; other than that, drives will be unlocked only when the LOCK utility is ran with the OFF parameter. Nextor will never automatically lock a drive.

#### 3.4.6. RALLOC: the reduced/zero allocation information mode tool

The RALLOC.COM utility allows activating or deactivating the reduced allocation information mode for a drive. The usage syntax for RALLOC is:

```
RALLOC [<drive letter>: ON|OFF]
```

If no parameters are specified, a list of drives currently in reduced allocation information mode will be shown.

When a drive is in this mode, the ALLOC function, which returns information about the total and free space available in a drive, will return fake information if necessary, so that the calculated total or free sector count will always fit in 16 bits. In other words, on drives with the reduced allocation information mode active, when the total or free space is greater than 32MB (which is possible in FAT16 volumes), ALLOC will return 32MB.

If an environment item named ZALLOC exists whose value –case insensitive- is ON (command `SET ZALLOC=ON` in the command interpreter), then the reduced allocation information becomes the zero allocation information mode (available since Nextor 2.0.3): the ALLOC function will return a free space of zero for the drives having this mode active. This makes the function to return immediately, which may be useful on very large or very slow devices.

Nextor will never modify the reduced allocation information mode status for a drive automatically, it is the user who always controls this behavior. Disk errors or media changes do not modify the reduced allocation information mode status either.

#### 3.4.7. Z80MODE: the Z80 access mode tool

The Z80MODE.COM utility, which works on MSX Turbo-R computers only, allows activating or deactivating the Z80 access mode for a MSX-DOS driver. The usage syntax for Z80MODE is:

```
Z80MODE <driver slot>[-<driver subslot>]] [ON|OFF]
```

If only a driver slot is specified, the current Z80 access mode state for the driver will be shown. The Z80 access mode is set or unset on a per driver basis (it is not possible to change it for specific drive letters).

The Z80 access mode can be set or unset on MSX-DOS drivers only (Nextor will never switch the current CPU when accessing a Nextor driver). When set, Nextor will switch the current CPU to Z80 prior to performing any operation with the driver. When not set, Nextor will not change the current CPU when accessing the driver.

Whether a given MSX-DOS driver needs the Z80 access mode to be set or not depends on each driver; when in doubt, look at the driver documentation or ask the driver developer if at all possible. Floppy disk drives are likely to need the Z80 access mode to be active.

At boot time Nextor will activate the Z80 access mode for all MSX-DOS drivers. Other than that, Nextor will never automatically change the Z80 access mode for any driver, it is the user who always controls this behavior.


#### 3.4.8. FASTOUT: the fast STROUT mode tool

The FASTOUT.COM utility allows to switch on an off the fast STROUT mode. The usage syntax for FASTOUT is:

```
FASTOUT [ON|OFF]
```

When invoked without parameters, it will show the current status of the FASTOUT mode.

The MSX-DOS function STROUT prints a string terminated with a "$" character. What this function actually does is to perform one separate call to the CONOUT function (which prints one single character) for every character of the string.

When the fast STROUT mode is active, the string will be copied to a 512 byte buffer in page 3 and then it will be printed in one single call to the kernel code, which increases the speed of the printing process. The drawback is that the string length is limited to 511 bytes when this mode is active; longer strings will be truncated (only the first 511 characters will be displayed).

#### 3.4.9. DELALL: the partition quick format tool

The DELALL.COM utility will perform a quick format on the filesystem visible on a given drive letter. The usage syntax for DELALL is:

```
DELALL <drive letter>:
```

What this tool does is to clean the FAT and root directory areas of the filesystem, thus effectively deleting all the information on the filesystem. There is no way to undo the operation; the files will be permanently lost so please use with care.

This tool can be used on any drive, even those attached to MSX-DOS drivers. Note that the drive must be mapped to a valid FAT12 or FAT16 filesystem, otherwise this tool will not work.

#### 3.4.10. NSYSVER: the NEXTOR.SYS version changer

Some MSX-DOS command line applications are known to check the version number of MSXDOS2.SYS (NEXTOR.SYS in the case of Nextor) and refuse to work if this number is smaller than a certain value, typically 2.20. This is a problem since the current NEXTOR.SYS version number is 2.1.

As a workaround for this issue, starting at version 2.0 beta 2 the NEXTOR.SYS version number returned by the DOSVER function call is stored in RAM and can be changed easily (see the _[Nextor 2.1 Programmers Reference](Nextor%202.1%20Programmers%20Reference.md)_ document for more details). A command line tool that allows to easily do this change has been created as well, its name is NSYSVER.COM and can be used as follows:

```
NSYSVER <major version number>.<secondary version number>
```

For example: `NSYSVER 2.20`. Note that this will change only the value of the NEXTOR.SYS version number returned by the DOSVER function call; the VER command will still display the real file version number.

Note: the version number change performed by this tool is temporary and it will cease to have effect (that is, the NEXTOR.SYS version number will revert to its real value) when NEXTOR.SYS is reloaded, either because the BASIC prompt is entered and exited via CALL SYSTEM, or because the computer is rebooted.

Note: do not use this tool with NEXTOR.SYS versions older than 2.0 beta 2.

#### 3.4.11. NEXBOOT: the one-time boot keys configuration tool

The NEXBOOT.COM tool allows to easily configure the keys to be used as [one-time boot keys](#292-one-time-boot-keys) in the next reset. The syntax is:

```
NEXBOOT <boot keys>|. [<slot> [<slot>... ]]
```

where the boot keys are the numeric keys, C for CTRL and S for shift, and `<slot>` are the slot numbers of the Nextor kernels to be disabled. For example `NEXBOOT 1C` will invert CTRL and 1 keys, `NEXBOOT S 1 23` will invert the SHIFT keys and disable the Nextor kernels in slots 1 and 2-3, and `NEXBOOT . 2` will just disable the Nextor kernel in slot 2.

In all cases, the tool resets the computer immediately after apporpriately setting the keys information in RAM.

#### 3.4.12. EMUFILE: the disk emulation mode tool

The EMUFILE.COM tool allows to create disk emulation mode data files and to enter disk emulation mode. The syntax for creating an emulation data file is:

```
emufile [<options>] <output file> <files> [<files> ...]
```

`<output file>` is the name of the emulation data file that will be created (default extension is .EMU), and `<files>` are the disk image files that will be used for the emulation (these can contain wildcards). Numbers (for disk change) are assigned to the disk image files in the same order as they are specified; when using wildcards, in the order they are found in the storage device that contains them (the same order that you see when you do run the DIR command).

The `-b <number>` option allows you to specify the number of the disk image file that will be used to boot when the emulation session starts, default is 1.

The `-a <address>` option allows you to specify the page 3 address that Nextor will use as work area (about 16 bytes) during the emulation session, must be a hexadecimal number in page 3 (C000 or higher). If not specified, this area will be allocated by Nextor before starting the emulation session.

The `-p` option will print all the filenames and associated keys after creating the data file. Note however that you can see this same information afterwards if you `TYPE /B` the emulation data file.

The syntax for starting a disk emulation session is as follows:

```
emufile set <data file> [o|p[<device index>[<LUN index>]]]
```

`o` will start the emulation using the one-time variant (this is the default), and `p` will start the emulation using the persistent variant. For the later, by default the emulation file data pointer will be written to the device where `<data file>` is stored, but you can specify a different `<device index>` and also optionally a `<LUN index>`. The default LUN index is 1 (i.e. `p3` is the same as `p31`).

Note that in both variants the computer will reset immediately after `EMUFILE.COM` writes the emulation data file pointer to the appropriate place.


### 3.5. The built-in partitioning tool

The Nextor kernel has an embedded utility for partitioning storage devices attached to Nextor device-based drivers. To start it, just invoke CALL FDISK from the BASIC prompt. It works properly on both 40 columns and 80 columns mode. Please note that starting the FDISK tool will delete the current BASIC program from memory.

The tool has a user interface based on menus, so anyone should be able to use it by just following the indications provided in the screen (when in doubt, look for an indication on what to do next in the lower line of the screen). There are however some points of interest to consider that are not mentioned in the tool itself:

* The tool allows creating up to 256 FAT12 and FAT16 partitions on any block device attached to a Nextor device-based driver. MSX-DOS drivers and Nextor drive-based drivers are not supported.

* With this tool it is not possible to add new partitions to an already partitioned device. All existing partitions must be removed before defining new partitions.

* Partitions from 100KB (the minimum supported partition size) up to 32MB will be FAT12, partitions from 33MB to 4GB (the maximum supported partition size) will be FAT16.

* Partitions of 16MB or less will have three sectors per FAT or less, therefore they can be used in MSX-DOS 1 mode.

* Partitions up to 32MB will have a MSX-DOS 2 boot sector, partitions of 33MB and more will have a standard boot sector.

* To get an optimum cluster size, it is recommended to define the partition sizes as powers of two (that is: 1M, 2M, 4M, 8M, 16M or 32M for FAT12 partitions; 64M, 128M, 256M, 512M, 1G, 2G or 4G for FAT16 partitions). If this is not possible, it is better to select the partition size as slightly smaller than the closest power of two than slightly higher (that is, for example 31M is better than 33M).

Remember that Nextor can handle devices with FAT16 partitions and standard boot sectors; if you use a factory-partitioned device of 2GB or less you probably don't need to partition it, unless you want to create MSX-DOS 1 compatible partitions (4GB devices are usually shipped with a FAT32 partition, so you will need to partition it with FDISK anyway).

When creating new partitions you can choose which one(s) will have the "active" flag set, thus being eligible for automatic mapping at boot time (see _[3.2. Booting Nextor](#32-booting-nextor)_); it is also possible change the flag on already existing partitions.

The partitioning tool works in MSX-DOS 1 mode too. Note however that the tool will always allow you to create partitions larger than 16M, which are not compatible with MSX-DOS 1.

### 3.6. Extensions to Disk BASIC

Nextor adds some new commands to Disk BASIC, mainly to ease the management of devices and partitions from this environment. Also, some of the commands that already existed in MSX-DOS have been extended or improved.

Some of the new CALL commands take parameters. These commands can be run without parameters in order to get help on how to use them. 

Unless otherwise stated, the Nextor modifications of existing Disk BASIC are not available in MSX-DOS 1 mode but the new commands are.

#### 3.6.1. The DSKF command

The DSKF command, which tells the free space available on a drive, returns a free cluster count in MSX-DOS. In Nextor the behavior of this command has been changed: now returns a free KB count.

This behavior represents a breaking change relative to MSX-DOS. However, most of the existing programs that use this command do not actually calculate the free space count in KB, displaying the raw cluster count to the user instead. Also, for many years the most popular storage media for MSX computers has been the 2DD floppy disk, in which the cluster size is 1K, so many users were incorrectly assuming that the DSKF command was returning a KB count anyway.

This modification does not apply to MSX-DOS 1 mode, in this mode the free cluster count is still returned as a cluster count.

The DSKF command will always return the real free space even if the drive has the reduced allocation information mode active. However, if the drive has the zero allocation information mode active, then the value returned will be zero.

#### 3.6.2. The DSKI$ and DSKO$ commands

The DSKI$ function and the DSKO$ command, which allow to read and write one disk sector respectively, now accept 32 bit sector numbers, therefore allowing access to any drive sector, not only the first 65536 sectors.

In order to access sectors with numbers over 32767, the sector number must be specified as a single or double precision constant, expression or variable. If a single precision value is specified and the number is so big that one or more of the least significant digits of the number is lost due to truncation, these commands will fail with an "Overflow" error. This is designed this way to prevent inadvertent access to the wrong sector. For example:

```
10 DEFSNG S
20 S=12345678
30 PRINT S 'Prints "12345700"
40 PRINT DSKI$(0, S) 'Throws "Overflow"
```

The previous example will work (provided that the sector exists in the device) if line 10 is changed to DEFDBL S. Always use double precision variables if you are going to access arbitrary sector numbers in your BASIC code.

An "Overflow" error will be thrown too if the sector number specified does not fit in 32 bits, that is, if it is greater than 4294967295.

In order to maintain compatibility with the MSX-DOS equivalent command, negative sector numbers are accepted (to which 65536 is added to get the real sector number) but only if the sector number can be evaluated as an integer (16 bit) expression. Therefore the following commands are equivalent and will work if the sector exists in the device:

```
PRINT DSKI$(0, 65535)
PRINT DSKI$(0, &HFFFF)
PRINT DSKI$(0, -1)
DEFINT S: S=-1: PRINT DSKI$(0, S)
```

However, the following will throw a "Disk I/O error":

```
PRINT DSKI$(0, CDBL(-1))
DEFDBL S: S=-1: PRINT DSKI$(0, S)
```

None of this apply to MSX-DOS 1 mode, in this mode only integer (16 bit) sector numbers are accepted.

#### 3.6.3 The CALL NEXTOR command

This command will simply display a list of the new CALL commands that Nextor provides for the BASIC environment.

#### 3.6.4 The CALL CHDRV command

This command changes the current drive and it exists already in MSX-DOS 2 Disk BASIC. However Nextor expands it in two ways:

* The command is now available in MSX-DOS 1 mode as well.

* The drive number can be specified as a number instead of a drive letter (from 1 being A: to 8 being H:). So for example `_CHDRV(3)` is the same as `_CHDRV("C:")`.

#### 3.6.5 The CALL CURDRV command

This command will simply display the current drive.

#### 3.6.6. The CALL DRIVERS command

This command is equivalent to the DRIVERS.COM tool, which displays information about the available MSX-DOS and Nextor drivers. It will display the name and version (for Nextor drivers only), the slot number, and the assigned drives at boot time. MSX-DOS drivers will be identified as "Legacy driver".

#### 3.6.7. The CALL DRVINFO command

This command is equivalent to the DRVINFO.COM utility, which displays information about all the available drive letters (those that are not unmapped). The displayed information includes the associated driver slot and other information that depends on the associated driver type (driver name and version for Nextor drivers; device and logical unit numbers for Nextor device-based drivers; relative unit for MSX-DOS and Nextor drive-based drivers). MSX-DOS drivers are identified as "Legacy driver".

#### 3.6.8. The CALL LOCKDRV command

This command allows to lock and unlock drives (see _[2.4. Drive lock](#24-drive-lock)_ and _[3.4.5. LOCK: the drive lock and unlock tool](#345-lock-the-drive-lock-and-unlock-tool)_). It is used as follows:

```
CALL LOCKDRV(<drive>)
```

Displays the current lock status of the drive.

```
CALL LOCKDRV(<drive>, 0)
```

Unlocks the drive.

```
CALL LOCKDRV(<drive>, <any non-0 number>)
```

Locks the drive.

`<drive>` is a string with the drive letter followed by a colon (for example "A:") or a number, being 1 to 8 for drives A: to H:, or 0 for the current drive.

This command is not available in MSX-DOS 1 mode, in which the concept of "drive lock" does not exist.

#### 3.6.9. The CALL MAPDRV command

This command that allows changing the drive to device and partition mapping from the BASIC environment. It is equivalent to the MAPDRV.COM tool.

The CALL MAPDRV syntax is explained below. Some of the parameters are optional, therefore all the possible variations are explained, starting with the most complete (using all parameters) one. Details about the possible values for each parameter are explained later.

```
CALL MAPDRV(<drive>, <partition>, <device>, <slot>|0)
```

Maps the specified drive to the specified partition of the specified device, which is controlled by the driver on the specified slot. If 0 is specified instead of a slot number, the slot of the primary controller is used.

```
CALL MAPDRV(<drive>, <partition>, <device>)
```

Maps the specified drive to the specified partition of the specified device. The driver slot is assumed to be the same of the device which contains the partition already mapped to the drive; if the drive is not currently mapped to a device-based driver, an "Invalid device driver" error will be thrown.

```
CALL MAPDRV(<drive>, <partition>)
```

Maps the specified drive to the specified partition. The device is assumed to be the same one that contains the partition already mapped to the drive; if the drive is not currently mapped to a device-based driver, an "Invalid device driver" error will be thrown.

```
CALL MAPDRV(<drive>, -1)
```

Leaves the specified drive unmapped. Further attempts to access the drive will throw a "Bad drive name" error ("Disk I/O error" in MSX-DOS 1 mode).

```
CALL MAPDRV(<drive>, -2)
CALL MAPDRV(<drive>)
```

Maps the specified drive to its default value. If at boot time the drive was unmapped or was mapped to a MSX-DOS driver or to a Nextor drive-based driver, then the drive will be reverted to its original mapping state. Otherwise, and automatic mapping procedure will be performed (the procedure is equal to the one performed at boot time except that the NEXTOR.DAT file will not be searched; see _[3.2. Booting Nextor](#32-booting-nextor)_ for more details); this may result or not on the drive having the same mapping it had at boot time, depending on which devices are available and how the other drives are mapped.

The command parameters syntax is as follows:

* `<drive>` is a string with the drive letter followed by a colon (for example "A:") or a number, being 1 to 8 for drives A: to H:, or 0 for the current drive.

* `<partition>` is a number in the range 0-255, interpreted as follows:
    * 0: Assumes that the device has no partitions. The drive will be mapped to the absolute sector 0 of the device.
    * 1: First primary partition of the device.
    * 2, 3 or 4: If device partition 2 is extended, the number is interpreted as the first, second or third extended partition, respectively. Otherwise, the number is interpreted as the second, third or fourth primary partition of the device, respectively. 
    * 5 or greater: The number is interpreted as the (n-1)th extended partition of the device.
    * `<device>` is a device index in the range 1-7. If the device has multiple logical units, use the formula `<device>+16*<logical unit>`. The possible values for the logical unit are 1-7 too (0 is accepted as well and interpreted as 1).
    * `<slot>` is a slot number in the range 0-3. If the slot is expanded, use the formula `<main slot>+4*<subslot>`. As a special case, If 0 is specified as the slot number and no subslot number is specified, the slot of the primary controller is used.

In MSX-DOS 1 mode there are some additional restrictions imposed by the Nextor architecture:

* The specified drive must have been mapped to a device-base driver at boot time. It is not possible to change the mapping of a drive that was unmapped or mapped to a MSX-DOS driver or a Nextor drive-based driver at boot time.

* The new mapping information may specify a different partition and/or device, but the driver slot must be the same that was assigned to the drive at boot time. This is not an issue if there is only one Nextor kernel in the system.
Also, please note that in MSX-DOS 1 mode, if you map a drive to an unsupported partition type (a FAT16 partition or a FAT12 partition having more than 3 sectors per FAT) you will always get a "Disk I/O error" when accessing that drive. This does not mean that the device is actually faulty, only that Nextor refuses to access it.

Since Nextor 2.1 the CALL MAPDRV command can be used to mount a disk image file in a drive as well. The syntax in this case is:

```
CALL MAPDRV(<drive>, <file> [,0|1])
```

The `,1` parameter will cause the file to be mounted in read-only mode. However, if the file has the read-only attribute set, it will always be mounted in read-only mode, even if no `,1` parameter is supplied.

There are some restrictions in place when mounting files to drives. See [3.8. Mounting files](#38-mounting-files) for details.

#### 3.6.10. The CALL MAPDRVL command

The CALL MAPDRVL command is identical to the CALL MAPDRV command, except that it will perform a drive lock (see _[2.4. Drive lock](#24-drive-lock)_ and _[3.4.5. LOCK: the drive lock and unlock tool](#345-lock-the-drive-lock-and-unlock-tool)_) immediately after changing the drive mapping.

Note that this command is not available in MSX-DOS 1 mode, in which the concept of "drive lock" does not exist.

#### 3.6.11. The CALL USR command

The CALL USR command allows the execution of assembler code from BASIC code. It is equivalent to the standard MSX-BASIC DEF USR command and the USR function, but with an added feature: it allows to specify the input values of the Z80 registers for the code to execute, and to read the output values after the execution.

The syntax of the CALL USR command is as follows:

````
CALL USR(<code address> [,<registers address>])
````

`<code address>` is the address of the assembler code to be executed. Value -1 is treated as a special case: `_USR(-1)` will do nothing but will not throw an error. You can use this feature together with the ON ERROR GOTO command to detect the presence of Nextor from within a BASIC program.

`<registers address>` is the address of a 12 byte buffer for the Z80 registers values. If this parameter is specified, the registers will be loaded with the contents of this area before the code is invoked; after the code execution, the reverse process is performed: the buffer is updated with the values hold by the registers. The order of the registers in the buffer is: F, A, C, B, E, D, L, H, IXl, IXh, IYl, IYh.

Here is a simple BASIC program to test the CALL USR command. Change the registers assignment in lines 40-90 and the address of the code to be invoked in line 100 as appropriate to invoke different code (the MSX BIOS itself is a good source of routines to play around).

```
10 ON ERROR GOTO 20: _USR(-1): ON ERROR GOTO 0: GOTO 30
20 PRINT "Nextor not found!": END
30 DEFINT R: DIM R(12)
40 R(0)=&H2100 ‘AF
50 R(1)=&H3040 ‘BC
60 R(2)=&H5060 ‘DE
70 R(3)=&H7080 ‘HL
80 R(4)=&H90A0 ‘IX
90 R(5)=&HB0C0 ‘IY
100 CALL USR(&H00A2, VARPTR(R(0))) ‘Prints a "!" (passed in A as &H21) 
110 PRINT "AF=&H";HEX$(R(0))
120 PRINT "BC=&H";HEX$(R(1))
130 PRINT "DE=&H";HEX$(R(2))
140 PRINT "HL=&H";HEX$(R(3))
150 PRINT "IX=&H";HEX$(R(4))
160 PRINT "IY=&H";HEX$(R(5))
````

### 3.7. New BASIC error codes

The following new BASIC error codes are defined to handle the possible errors of the new BASIC commands. These errors are available in MSX-DOS 1 mode as well for the commands that work in this environment. The numbers in parenthesis are the error codes.

* Invalid device driver (76), thrown by the CALL MAPDRV command in any of these events:

    * The specified slot number does not contain a Nextor device-based driver.

    * No slot number is specified, but the drive is not currently mapped to a Nextor device-based driver.

    * In MSX-DOS 1 mode, the drive was not originally mapped to a Nextor device-based driver, or was mapped to a different driver.

* Invalid device or LUN (77), thrown by the CALL MAPDRV command in any of these events:

    * The device and/or LUN with the specified index is not available on the specified or implicit driver.

    * The device and/or LUN with the specified index exists on the specified or implicit driver, but it is not a block device.

* Invalid partition number (78)

This error will be thrown by the CALL MAPDRV command if the specified partition does not exist on the specified or implicit device. 

* Partition already in use (79)

This error will be thrown by the CALL MAPDRV command if you try to map a combination of partition, device and driver that is already mapped on another drive. You can however map the same combination to the same drive again.

* File is mounted (80)

An attempt to open or alter a mounted file, or to perform any other disallowed operation involving a mounted file, has been made.

* Bad file size (81)

Thrown by the CALL MAPDRV command when attempting to mount a file that is smaller than 512 bytes or larger than 32 MBytes.


### 3.8. Mounting files

Nextor 2.1 introduces the ability to mount disk image files on drive letters. When a disk image file is mounted, you can access its contained files and directories by using regular MSX-DOS/MSX BASIC commands and tools.

To mount a file, use [the MAPDRV tool](#341-mapdrv-the-drive-mapping-tool) with the `MAPDRV <drive> <file> [/ro]` syntax; or in BASIC environment, [the CALL MAPDRV command] (#369-the-call-mapdrv-command) with the `CALL MAPDRV(<drive>, <file> [,0|1])`. To unmount the file, change the mapping of the drive to anything else, or simply leave the drive unmapped (`MAPDRV <drive> U` or `CALL MAPDRV(<drive>, -1)`).

This feature has some restrictions:

* To be mountable a disk image file must have a size of at least 512 bytes and at most 32 MBytes.

* The file is expected to contain a proper FAT filesystem already, it is not possible to apply the FORMAT command on a mounted drive. 

* The file cannot contain partitions, the contained filesystem is expected to start right at the beginning of the file.

* It is not possible to mount a file on the drive where the file itself is located:

```
MAPDRV A: A:TOOLS.DSK --> Error
```

* It is not possible to mount the same file in two drives at the same time:

```
MAPDRV B: TOOLS.DSK
MAPDRV C: TOOLS.DSK --> Error
```

* It is not possible to do a recursive file mount (mounting a file that is itself inside a mounted disk image file):

```
MAPDRV B: TOOLS.DSK
MAPDRV C: B:FILE.DSK --> Error
```

* It is not possible to alter the mapping state of a drive if it contains one or more files that are currently mounted:

```
MAPDRV B: A:TOOLS.DSK
MAPDRV A: U --> Error
```

- It is not possible to open or to alter (rename, move, delete, overwrite, change attributes) a mounted file:

```
MAPDRV B: TOOLS.DSK
TYPE TOOLS.DSK --> Error
ECHO HELLO > TOOLS.DSK --> Error
REN TOOLS.DSK X.DSK --> Error
MOVE TOOLS.DSK SOMEDIR\ --> Error
DEL TOOLS.DSK --> Error
ATTRIB +R TOOLS.DSK --> Error
```

**Note:** Currently `ECHO HELLO > TOOLS.DSK` doesn't actually throw an error due to a bug.

**Warning:** After mounting a file do not extract or swap the medium where the file is contained. The behavior of Nextor if this is done is undefined and you could lose data.


### 3.9. Disk emulation mode

Since version 2.1 Nextor allows to boot in disk emulation mode. In this mode Nextor uses a disk image file (or a set of swappable files) as the boot device instead of a regular device. This is ideal for playing disks that were released in floppy disk and can't be run from a modern storage device, because they don't have a filesystem or because they need to run in MSX-DOS 1 mode.

The technical details about how the disk emulation mode works are in the _[Nextor 2.1 Programmers Reference](Nextor%202.1%20Programmers%20Reference.md)_ document in case you are interested in building your own tool instead of using `EMUFILE.COM`.


#### 3.9.1. Entering and exiting the disk emulation mode

First of all, the data needed during a disk emulation mode session (which disk image files will be used and where are they located) must exist in a file with a certain format, the _disk emulation data file_. You can create these files using [the `EMUFILE.COM` tool](#3412-emufile-the-disk-emulation-mode-tool). These files can have any name and will typically have the .EMU extension, but that's not mandatory.

Second, in order to tell Nextor to boot in disk emulation mode, a pointer to the appropriate disk emulation data file must exist at a special location while the computer boots. There are two variants of the emulation mode, each requiring a different location for the emulation data file pointer:

* **One-time:** Nextor will enter disk emulation mode only once, that is, after resetting the computer again Nextor will boot normally. In this mode the pointer to the emulation data file is set in RAM.

* **Persistent:** Nextor will enter disk emulation mode on every computer reset, until that mode is manually disabled by pressing 0 while booting. In this mode the pointer to the emulation data file is set in the partition table of one of the devices controlled by Nextor (usually the same device that contains the emulation data file and the disk image files, but that's not mandatory).

Both variants of disk emulation mode can be entered by using the `EMUFILE.COM` tool with the `set` parameter, being the one-time variant the default one.


#### 3.9.2. Changing the image file

Up to 32 disk image files can be specified for an emulation session, but only one of them is active at a given time. In order to switch to a different file, you must press the appropriate key while the computer is trying to read the file; this will emulate a disk change. The keys are 1-9 for the first nine image files, then A-W for the rest, in alphabetical order.

For example, assume that you are playing a two disks game. You boot with disk 1 and at some point the game asks you to insert disk 2 and press space key. Just press 2 (they key assigned to the second image file) and the space key at the same time and you're good to go.

Alternatively, you can also press the GRAPH key when the computer is trying to read the file. The CAPS led will lit and the computer will freeze until you release GRAPH and press the appropriate file key (or you can press GRAPH again if you change your mind and want to keep using the same disk). This is useful when having to directly press an alphanumeric key while disk access is performed is a problem (for example, you are in the BASIC prompt and you want to trigger a file change when executing a FILES command: the pressed key would be added to "FILES" causing a Syntax Error).


#### 3.9.3. Rules and restrictions

The following rules and restrictions apply to the disk emulation mode:

- The primary controller must be a Nextor kernel with a device-based driver.

- The emulation data file and all the disk image files must be placed in devices controlled by the primary controller (but they can be in different partitions and even in different devices).

- The emulation data file stores information about absolute device sectors, therefore it will be unusable if the disk image files are moved and file renames will have no effect. It is recommended to either generate the file immediately before using it, or have a partition reserved only for disk image files and their corresponding emulation data files (that is, a partition where you usually don' create or move files around).

- The disk image files must have a size of at least 512 bytes and at most 32 MBytes, must not contain partitions (the contained filesystem is expected to start right at the beginning of the file), and must contain a proper FAT12 filesystem (the FORMAT command will not work in disk emulation mode).

- The disk image files must not be fragmented, that is, their contents must be placed across consecutive sectors in the device.

- Disk emulation mode is always started in DOS 1 mode and in Z80 mode. If you want to start a game in R800 mode, do the following: keep pressed GRAPH and 2 while the computer boots, and when the caps led lits, release both keys and press 1.

- All Nextor controllers but the primary one will be disabled when disk emulation mode is entered. MSX-DOS kernels (such as the internal floppy disk drive) will not, but you can force them to disable themselves by pressing SHIFT while booting; this is useful to free some memory.


#### 3.9.4 How to free some memory

Some games will not work "out of the box" because they assume that only the floppy disk drive is present in the system, but now there are drives allocated for both Nextor and the floppy drive, and thus the amount of free memory is smaller. You can do the following in order to increase the amount of memory available for games:

- Press SHIFT while booting to disable the internal floppy disk drive (and any other MSX-DOS kernel, for that matter).

- Press 5 while booting to force Nextor to allocate only one drive for itself (useful only if you have more than one device connected to your Nextor controller). If your emulation session has five or more disk images, do the following instead: press GRAPH+5 until the caps led lits, then release both keys and press 1.


#### 3.9.5. Known bugs

* The current version of the `EMUFILE.COM` tool does not verify that the disk image files are not fragmented.

* If you have more than one device in the primary Nextor controller (for example, for the MegaFlashROM SCC+ SD this means two SD cards, or one or two cards plus the ROM disk), Nextor will allocate one dummy drive letter for each extra device. MSX-DOS devices (if any) will then have drive letters assigned after these. For example, if you have three devices, A: is where the emulated disk image file is mounted, B: and C: are dummy, and D: is the internal floppy disk drive. These dummy drives will NOT have memory allocated for FAT buffers.


## 4. Other improvements

### 4.1. load" in F7

Nextor will force the computer to boot with the `load"` string assigned to the F7 key, even on MSX1 and MSX2 computers, which have `cload"` assigned by default. Note however that any code that invokes the INIFNK BIOS routine will cause the key to be assigned to `cload"` again (you can try it yourself: `_USR(&H3E)` ).

### 4.2. English error messages in kanji mode

If an environment item named ERRLANG is created with a value –case insensitive- of EN (command `SET ERRLANG=EN` in the command interpreter prompt), error messages in the command interpreter will be displayed in English, instead of Japanese, when the kanji mode is active (`CALL KANJI` in the BASIC interpreter). This feature is available since Nextor 2.0.4.

### 4.3. Reduced NEXTOR.SYS without Japanese error messages

Two variants of the NEXTOR.SYS file are offered. The full variant contains Japanese equivalents for part of the error messages (such as the "reading/writing" part or the "Abort, Retry, Ignore" string), while the reduced variant contains only the English versions. The advantage of the reduced variant is that it is smaller and using it saves 256 bytes of TPA space compared to the full version.

These two variants are offered since NEXTOR.SYS version 2.01 (released together with kernel version 2.0.4). Note that version 2.00 was already reduced, but had a bug that caused garbage to be displayed instead of the proper error messages in kanji mode.

Note that error messages will be displayed in English regardless of the variant used if the ERRLANG environment item exists with value EN (see _[4.2. English error messages in kanji mode](#42-english-error-messages-in-kanji-mode)_).


## 5. Change history

This section contains the change history for the different versions of Nextor. Changes that affect application or driver development are not listed here; instead, you should look at the _[Nextor 2.1 Programmers Reference](Nextor%202.1%20Programmers%20Reference.md)_ and _[Nextor 2.1 Driver Development Guide](Nextor%202.1%20Driver%20Development%20Guide.md)_ documents for a list of changes of that type.

This list contains the changes for the 2.1 branch only. For the change history of the 2.0 branch see the _[Nextor 2.0 User Manual](../../../blob/v2.0/docs/Nextor%202.0%20User%20Manual.md#5-change-history)_ document.


### 5.1. v2.1.0 beta 2

- Nextor will now try to load `MSXDOS2.SYS` if `NEXTOR.SYS` is not found in the boot drive.

- The method for selecting partitions for automatic mapping has changed from requiring a `NEXTOR.DAT` file in the root directory to having the "active" flag set in the partition table.

- Now the first 9 partitions of a device will be scanned during the automatic mapping procedure, this includes extended partitions.

- FDISK allows to change the "active" flag of new and existing partitions.

- FDISK now always creates extended partitions, even if 4 or less partitions are defined.

- FDISK now creates FAT16 partitions with a partition type code of 14 (FAT16 LBA) instead of 6 (FAT16 CHS).

- The numeric keyboard can now be used both when booting and when changing disks in disk emulation mode.

- Russian keyboard is now properly recognized (numeric keys only).

- Introduced the [boot key inverters](#291-boot-key-inverters).

- Introduced the [one-time boot keys](#292-one-time-boot-keys).

- Introduced the [NEXBOOT.COM tool](#3411-nexboot-the-one-time-boot-keys-configuration-tool) to set the RAM based one-time boot keys.

- Introduced the RAM based one-time [disk emulation mode](#39-disk-emulation-mode).

- The method to enter the old disk emulation mode (now named "persistent") has changed from requiring a `NEXT_DSK.DAT` file in the root directory to storing a pointer to the emulation data file in the partition table of the device.

- Pressing the 0 key at boot time will delete the pointer to the emulation data file in the partition table, thus permanently disabling the emulation mode - no need to manually do anything else.

- When Nextor is waiting for a disk key press after having pressed GRAPH in disk emulation mode, now you can press GRAPH again to cancel the disk change.

- The first Nextor kernel to boot now clears the screen before invoking the driver initialization.

- ARG is no longer used as temporary work area by the Nextor kernel, this should improve the compatibility of games in disk emulation mode.

- Fix: drive was remapped in case of error (even if it was retried successfully).

- Fix: boot sector checksum calculation had a bug that caused "Wrong disk" errors.

- Fix: [#1 pressing CTRL+STOP while Nextor was trying to load NEXTOR.SYS hanged the computer](https://github.com/Konamiman/Nextor/issues/1).

- Fix: [#23 computer hanged when booting with one single drive letter (e.g. when using single-device controller in a computer without internal disk drive)](https://github.com/Konamiman/Nextor/issues/23).

- Fix: [#29 wrong stack management hangedd the computer when a file handle was read or written to a number of times](https://github.com/Konamiman/Nextor/issues/29).

- Fix: computer crashing when more than one Nextor kernel was present as soon as the extended BIOS hook was called (for example, when loading COMMAND2.COM).


### 5.2. v2.1.0 beta 1

- All the changes and fixes of [Nextor 2.0.5](../../../blob/v2.0/docs/Nextor%202.0%20User%20Manual.md#51-v205-beta-1).

- Introduced [file mounting and disk emulation mode](#213-file-mounting-and-disk-emulation-mode).

- Change in boot keys: now the key to request one single drive per driver is the 5 key, and CTRL is simply passed to MSX-DOS kernels as when booting with MSX-DOS (see [2.9. Boot keys](#29-boot-keys)).

- Fixed [#3 Can't access read-only files with _RDBLK](../../../issues/3)
