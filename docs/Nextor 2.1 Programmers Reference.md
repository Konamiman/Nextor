# Nextor 2.1 Programmers Reference

## Index

[1. Introduction](#1-introduction)

[2. Changes in existing function calls](#2-changes-in-existing-function-calls)

[2.1. _STROUT (09h)](#21-_strout-09h)

[2.2. _ALLOC (1Bh)](#22-_alloc-1bh)

[2.3. _RDABS (2Fh) and _WRABS (30h)](#23-_rdabs-2fh-and-_wrabs-30h)

[2.4. _DPARM (31h)](#24-_dparm-31h)

[2.5. _DEFER (64h)](#25-_defer-64h)

[2.6. _EXPLAIN (66h)](#26-_explain-66h)

[2.7. _FORMAT (67h)](#27-_format-67h)

[2.8. _DOSVER (6Fh)](#28-_dosver-6fh)

[2.8.1 Detecting Nextor in MSX-DOS 1 mode](#281-detecting-nextor-in-msx-dos-1-mode)

[3. New function calls](#3-new-function-calls)

[3.1. Get/set fast STROUT mode (_FOUT, 71h)](#31-getset-fast-strout-mode-_fout-71h)

[3.2. Print a zero-terminated string (_ZSTROUT, 72h)](#32-print-a-zero-terminated-string-_zstrout-72h)

[3.3. Read absolute sectors from drive (_RDDRV, 73h)](#33-read-absolute-sectors-from-drive-_rddrv-73h)

[3.4. Write absolute sectors to drive (_WRDRV, 74h)](#34-write-absolute-sectors-to-drive-_wrdrv-74h)

[3.5. Get/set reduced allocation information mode vector (_RALLOC, 75h)](#35-getset-reduced-allocation-information-mode-vector-_ralloc-75h)

[3.6. Get drive space information (_DSPACE, 76h)](#36-get-drive-space-information-_dspace-76h)

[3.7. Lock/unlock a drive, or get lock state for a drive (_LOCK, 77h)](#37-lockunlock-a-drive-or-get-lock-state-for-a-drive-_lock-77h)

[3.8. Get information about a device driver (_GDRVR, 78h)](#38-get-information-about-a-device-driver-_gdrvr-78h)

[3.9. Get information about a drive letter (_GDLI, 79h)](#39-get-information-about-a-drive-letter-_gdli-79h)

[3.10. Get information about a device partition (_GPART, 7Ah)](#310-get-information-about-a-device-partition-_gpart-7ah)

[3.11. Call a routine in a device driver (_CDRVR, 7Bh)](#311-call-a-routine-in-a-device-driver-_cdrvr-7bh)

[3.12. Map a drive letter to a driver and device (_MAPDRV, 7Ch)](#312-map-a-drive-letter-to-a-driver-and-device-_mapdrv-7ch)

[3.13. Enable or disable the Z80 access mode for a driver (_Z80MODE, 7Dh)](#313-enable-or-disable-the-z80-access-mode-for-a-driver-_z80mode-7dh)

[4. New error codes](#4-new-error-codes)

[5. Extended mapper support routines](#5-extended-mapper-support-routines)

[5.1: CALL_MAP: Call a routine in a mapped RAM segment](#51-call_map-call-a-routine-in-a-mapped-ram-segment)

[5.2: RD_MAP: Read a byte from a RAM segment](#52-rd_map-read-a-byte-from-a-ram-segment)

[5.3: CALL_MAPI: Call a routine in a mapped RAM segment, with inline routine identification](#53-call_mapi-call-a-routine-in-a-mapped-ram-segment-with-inline-routine-identification)

[5.4: WR_MAP: Write a byte to a RAM segment](#54-wr_map-write-a-byte-to-a-ram-segment)

[5.5. The UNAPI RAM Helper discovery procedure](#55-the-unapi-ram-helper-discovery-procedure)

[5.6. Breaking change notice](#56-breaking-change-notice)

[6. Other features](#6-other-features)

[6.1. Correction of the ESC-Y escape sequence bug in STROUT function](#61-correction-of-the-esc-y-escape-sequence-bug-in-strout-function)

[6.2. Changing the NEXTOR.SYS version number](#62-changing-the-nextorsys-version-number)

[7. Nextor internals](#7-nextor-internals)

[7.1. One-time boot keys](#71-one-time-boot-keys)

[7.2. Disk emulation mode](#72-disk-emulation-mode)

[7.2.1. Disk emulation data file format](#721-disk-emulation-data-file-format)

[7.2.2. Entering disk emulation mode](#722-entering-disk-emulation-mode)

[8. Change history](#8-change-history)

[8.1. v2.1.0 beta 2](#81-v210-beta-2)

[8.2. v2.1.0 beta 1](#82-v210-beta-1)


## 1. Introduction

Nextor is an enhanced version of MSX-DOS 2, the disk operating system for MSX computers. It is based on MSX-DOS 2.31, with which it is 100% compatible.

This document provides a reference of the new features that Nextor adds to MSX-DOS 2 from a developer point of view (basically the new function calls provided, but also some other useful information). The development of device drivers for Nextor is not covered in this document; this topic has a separate document devoted to itself, _[Nextor 2.1 Driver Development Guide](Nextor%202.1%20Driver%20Development%20Guide.md)_.

The reader of this document is assumed to have experience developing applications for MSX in general and for MSX-DOS 2 in particular (specifically, the information covered by chapter 3 of _MSX2 Technical Handbook_ and the _[MSX-DOS 2 Program Interface Specification](DOS2-PIS.TXT)_ and _[MSX-DOS 2 Function Codes Specification](DOS2-FCS.TXT)_ documents is assumed to be known). Also, it is a good idea to get acquainted with Nextor by reading _[Nextor 2.1 User Manual](Nextor%202.1%20User%20Manual.md)_ prior to this document.

## 2. Changes in existing function calls

This section details what Nextor has changed in the function calls already existing in MSX-DOS 2. All the changes are non-breaking except a minor issue with 16 bit sector numbers in function _DPARM.

Only the changes introduced in Nextor are explained, the complete description of the functions is not provided here. See the _[MSX-DOS 2 Function Codes Specification](DOS2-FCS.TXT)_ document for more details on these function calls.

### 2.1. _STROUT (09h)

When the fast STROUT mode is enabled, the maximum printable string length is 511 characters; if the string is longer, only the first 511 characters will be printed. The fast STROUT mode is disabled by default, it must be explicitly enabled by using the new _FOUT function.

### 2.2. _ALLOC (1Bh)

When the reduced allocation information mode is enabled for a drive, this function will return a false total and/or free cluster count when necessary so that when multiplied by the sectors per cluster amount, the result will give 32MB or less when called for that drive. The reduced allocation information mode is disabled by default for all drives, it must be explicitly enabled for each desired drive by using the new _RALLOC function.

Moreover, starting at Nextor 2.0.3 the reduced allocation information mode can be turned into a zero allocation information mode by simply creating an environment item named ZALLOC with value ON (case insensitive). When this item exists, _ALLOC returns zero clusters free for all the drives in reduced allocation information mode.

### 2.3. _RDABS (2Fh) and _WRABS (30h)

These functions will work only when the accessed drive contains a FAT12 filesystem. They will return a "Not a DOS disk" error when a drive containing a FAT16 filesystem or an unknown filesystem is accessed.

Strictly speaking, this is not a change from the behavior in MSX-DOS, since FAT12 was the only filesystem supported by that operating system. However, when a drive containing a FAT16 filesystem is read in MSX-DOS, its boot sector is mistook for a FAT12 boot sector, and the functions succeed. In Nextor FAT16 filesystems are explicitly rejected by these functions; this is done on purpose to prevent programs, such as CHKDSK or IMPROVE, that do low-level processing on drives assuming that they have a FAT12 filesystem, to cause data corruption when accidentally ran on these drives.

New applications should use instead the new functions _RDDRV and _WRDRV, which accept 32 bit sector numbers and allow access to any drive regardless of the contained filesystem.

### 2.4. _DPARM (31h)

This function now returns the total number of logical sectors as a 32 bit value at position +24..27 in the returned parameter block. Moreover, when this number is greater than 65535, the 16 bit sector count returned at position +9,10 will be zero.

Also, position +28 of the returned parameter block contains the filesystem type:

```
0: FAT12
1: FAT16
255: Other
```

Remember that Nextor can currently handle FAT12 and FAT16 filesystems only.

### 2.5. _DEFER (64h)

The parameters passed to the user routine in case of disk error are extended to support 32 bit sector numbers. In MSX-DOS 2, part of these parameters were as follows:

```
C:b3 - set if sector number is valid
DE = Sector number (if b3 of C is set)
```

In Nextor, these parameters are:

```
C:b3 - set if sector number is valid and fits in 16 bits (that is, b4 is set and HL=0)
C:b4 - set if sector number is valid
HL:DE = Sector number (if b4 of C is set)
```

### 2.6. _EXPLAIN (66h)

If an environment variable named ERRLANG exists with value EN (`SET ERRLANG=EN` from the command interpreter prompt), this function will return error messages in English even if the kanji mode is active (`CALL KANJI` from the BASIC interpreter). This feature is available since Nextor 2.0.4.

### 2.7. _FORMAT (67h)

In MSX-DOS 2 this function accepts two special choice parameters, FFh and FEh, that do not actually format the disk but generate a MSX-DOS 2 boot sector, including the disk parameters, based on the media ID of the disk. This feature is used by the FIXDISK program to convert old MSX-DOS 1 disks into MSX-DOS 2 disks.

Nextor adds three new choice parameters:

* FDh: Assuming that the disk has a FAT12 or FAT16 filesystem with valid disk parameters on the boot sector (otherwise a "Not a DOS disk" will be returned), a standard boot sector will be composed for the disk by (1) setting the manufacturer name as "NEXTOR", and (2) generating an extended block (byte 29h, plus volume ID, plus volume name, plus "FAT12" or "FAT16" mark). The disk parameters will not be modified. If the disk contains an extended block already, only the manufacturer name will be changed (thus maintaining the existing volume ID). This choice is useful for using the dirty disk flag feature on disks already formatted by another system.

* FCh: It is the same as FDh, but if the disk filesystem is FAT12, a MSX-DOS 2 boot sector (with the "VOL_ID" string) will be composed instead of a standard boot sector. If the disk filesystem is FAT16, this choice works the same way as FDh. This choice is useful if the disk is to be used on MSX-DOS 2 systems.

* FBh: Will perform a "quick format" on the disk, by simply clearing the FAT and root directory areas. As with the other two new choices, the disk must have a valid FAT12 or FAT16 boot sector, otherwise a "Not a DOS disk" error will be returned. 

When the disk is actually formatted (choice 1..9), a MSX-DOS 2 boot sector will always be generated.

### 2.8. _DOSVER (6Fh)

This function call has been expanded in order to allow applications to detect whether they are running MSX-DOS 2 or Nextor (both in Normal or MSX-DOS 1 mode), while at the same time still working for applications that expect the operating system to be MSX-DOS.

The procedure for detecting Nextor is as follows. The application must invoke the _DOSVER function with the following “magic numbers” as input parameters:

```
B = 5Ah
HL = 1234h
DE = ABCDh
IX = 0
```

When the operating system is MSX-DOS 2, then this function call will return IX = 0 (since MSX-DOS does not modify the IX and IY registers unless they return a result, and no result is returned in these registers by this function). When running Nextor and the magic numbers are NOT supplied, the function call will return IX and IY unmodified as well, to maintain compatibility with MSX-DOS 2.

When the operating is Nextor and the magic numbers are provided, then the following information is returned:

```
B = Emulated MSX-DOS kernel major version number (always 2)
C = Emulated MSX-DOS kernel minor version number (always 31h)
D = NEXTOR.SYS major version number in BCD format
E = NEXTOR.SYS minor version number in BCD format
HL = Address of an operating system descriptive string in kernel ROM
IXh = 1
IXl = Nextor major version number (2-15)
IYh = Nextor secondary version number (0-15)
IYl = Nextor revision number (0-255)
```

The procedure for detecting the operating system for Nextor aware applications is then as follows:

1.  Call the DOSVER function with the magic numbers set.
2.  If there is an error (A<>0) then the operating system is neither MSX-DOS nor Nextor.
3.  If B<2 then the operating system is MSX-DOS 1.
4.  If IX is 0 then the operating system is MSX-DOS 2. Look at registers B and C for the version number.
5.  If IXh is 1 then the operating system is Nextor. Look at registers IXh, IYh and IYl for the version number.
6.  If IXh is neither 0 nor 1 then the operating system is neither MSX-DOS nor Nextor.

The value returned in HL is a pointer to a zero-terminated printable string that describes the operating system running, for example "Nextor kernel version 2.0". The string resides in the kernel master slot (slot number is available at 0F348h) and can be readed via standard RDSLT calls.

#### 2.8.1 Detecting Nextor in MSX-DOS 1 mode

The DOSVER function has been made available in MSX-DOS 1 mode, so you can detect Nextor in this mode.

The results of the DOSVER function when invoked in MSX-DOS 1 mode are the same returned in normal mode, except that BC=0100h and no meaningful value is returned in HL. However, since the function can be called via de 0F37Dh hook only (see _[3. New function calls](#3-new-function-calls)_), index registers get corrupted, therefore the “check that IX=0” method can't be used to detect Nextor. Instead, DOSVER will return A=1 when ran in MSX-DOS 1 mode.

Therefore, if your application can work in MSX-DOS 1 mode but you still want to know if you are running in Nextor or in an old MSX-DOS 1 kernel, the detection procedure is as follows:

1.  Set the magic numbers as in the normal mode detection case.
2.  Call the DOSVER function via the F37Dh entry point. Do NOT use the 0005h entry point.
3.  If A=1, B=1 and IXh=1, then you are running Nextor in MSX-DOS 1 mode. Look at registers IXh, IYh and IYl for the Nextor version number.
4.  Otherwise, follow the procedure for the normal mode, as described in the previous section.

## 3. New function calls

This section details the new function calls introduced by Nextor. These are invoked the same way as the existing MSX-DOS calls, by setting the function number in register C and calling address 0005h or F37Dh. The specified short name for each function (for example “_FOUT”) is the suggested name for referring the function call in code, and is also the name used for function cross references in this manual.

Some of the new function calls can be invoked in MSX-DOS 1 mode as well; in the current version, these functions are GDRVR, GPART, CDRVR and GDLI. When invoked in MSX-DOS 1 mode, all the new Nextor function calls have the following restrictions:

* They must be called by using the F37Dh entry point. The 0005h entry point is not supported, since there is no special version of MSXDOS.SYS for Nextor.

* When data is exchanged with the invoking application program by using a buffer in RAM, the buffer cannot be in page 1 (addresses 4000h-7FFFh).
Additional restrictions apply for some of the functions. See the functions descriptions for more details.

### 3.1. Get/set fast STROUT mode (_FOUT, 71h)

```
Parameters:  C = 71H (_FOUT)
             A =  00H => get fast STROUT mode
                  01H => set fast STROUT mode
             B =  00H => disable (only if A=01H)
                  FFH => enable (only if A=01H)

Results:     A = Error
             B = Current fast STROUT mode
```

This function enables or disables the fast STROUT mode. When enabled, the _STROUT and _ZSTROUT functions will work faster, but the maximum printable string length will be 511 characters; if the string is longer, only the first 511 characters will be printed.

### 3.2. Print a zero-terminated string (_ZSTROUT, 72h)

```
Parameters:  C = 72H (_ZSTROUT)
             DE = Address of string

Results:     A = 0 (never returns an error)
```

Prints on the screen the string pointed by DE, the string must be terminated with a zero character. This function is affected by the fast STROUT mode.

### 3.3. Read absolute sectors from drive (_RDDRV, 73h)

```
Parameters:  C = 73H (_RDDRV)
             A = Drive number (0=A: etc.)
             B = Number of sectors to read
             HL:DE = Sector number

Results:     A = Error code (0=> no error)
```

This function reads sectors directly from a drive. Unlike _RDABS, this function is able to read sectors regardless of the filesystem viewed through the drive (FAT12, FAT16 or an unknown filesystem), and even when there is no filesystem at all.

The sectors will be read to the current disk transfer address. Any disk error will be reported by the system in the usual way.

### 3.4. Write absolute sectors to drive (_WRDRV, 74h)

```
Parameters:  C = 74H (_WRDRV)
             A = Drive number (0=A: etc.)
             B = Number of sectors to write
             HL:DE = Sector number

Results:     A = Error code (0=> no error)
```

This function writes sectors directly to a drive. Unlike _WRABS, this function is able to write sectors regardless of the filesystem viewed through the drive (FAT12, FAT16 or an unknown filesystem), and even when there is no filesystem at all.

The sectors will be written from the current disk transfer address. Any disk error will be reported by the system in the usual way.

### 3.5. Get/set reduced allocation information mode vector (_RALLOC, 75h)

```
Parameters:  C = 75H (_RALLOC)
             A =  00H => get current vector
                  01H => set vector
             HL = new vector (only if A=01H)

Results:     A = 0 (never returns an error)
             HL = Current vector
```

This function obtains or sets the reduced allocation information mode vector. The vector assigns one bit for each drive; bit 0 of L is for A:, bit 1 of L is for B:, etc. This bit is 1 if the reduced allocation mode is currently enabled (when getting vector) or to be enabled (when setting vector) for the drive, 0 when the mode is disabled or to be disabled.

### 3.6. Get drive space information (_DSPACE, 76h)

```
Parameters:  C = 76H (_DSPACE)
             E = drive number (0 = default, 1 = A:, etc)
             A =  00H => get free space
                  01H => get total space

Results:     A = error code
             HL:DE = space in kilobytes
             BC = extra space in bytes
```

This function returns the total or free space for a drive. The space information in HL:DE is always returned in Kilobytes, regardless of the type and the cluster size of the filesystem mapped to the drive.

The "extra free space in bytes" result will be different from zero only when the minimum allocation unit of the drive is not a whole number of kilobytes. In case of FAT drives, it will be non-zero (specifically, it will be 512) only when the drive uses one sector per cluster and the cluster count is odd. For example, for a drive having one sector per cluster and 15 free clusters, this function will return HL=0, DE=7 and BC=512 when called with A=0 for that drive.

The space information returned by this function is always real, it is not affected by the reduced/zero allocation information mode.

### 3.7. Lock/unlock a drive, or get lock state for a drive (_LOCK, 77h)

```
Parameters:  C = 77H (_LOCK)
             E = physical drive (0=A:, 1=B:, etc)
             A =  00H => get lock status
                  01H => set lock status
             B =  00H => unlock drive (only if A=01H)
                  FFH => lock drive (only if A=01H)

Results:     A = Error code
             B = Current lock status, same as input
```

This function locks or unlocks a drive, or gets the current lock state for a drive. When a drive is locked, Nextor will assume that the media on that drive will never be changed, and therefore will never ask the associated driver for media change status; thus resulting in an overall increase on media access speed. This is useful when using removable devices, such as multimedia cards, as the main storage device.

In order to be locked, the drive must be mapped to a valid filesystem (that is, the drive must be accessible); otherwise an error will be returned and the drive will not be locked.

Once a drive is locked, any disk error that is aborted will automatically unlock the drive.

Locking and unlocking operations cause all the buffers for the drive to be flushed and invalidated. Also, cached disk parameters for the media are deleted so the next access to the media will re-read them.

It is possible to lock non-removable devices associated to device-based drivers, however it makes no sense to do that, since Nextor will never ask for media change status for this kind of devices.

Locking feature must be used with care. Changing the device of a locked drive without first unlocking it may result in data corruption, both in the original (locked) media and in the newly inserted media.

### 3.8. Get information about a device driver (_GDRVR, 78h)

```
Parameters:  C = 78H (_GDRVR)
             A = Driver index, or 0 to specify slot and segment
             D = Driver slot number (only if A=0)
             E = Driver segment number, FFh for drivers in ROM
                 (only if A=0)
             HL = Pointer to 64 byte data buffer

Results:     A = Error code
             HL = Filled with data about driver
```

Returns information about a device driver present in the system. This function works in MSX-DOS 1 mode.

The device driver can be specified by index or by slot and segment number pair. To specify the driver by index, set the index number (starting at 1) in register A; the slot and segment number for the driver is returned in the data buffer together with other driver information. This is useful to discover which drivers are present in the system.

If you know already the slot and segment numbers of the driver you want to gather information about, set these parameters in registers D and E, and set A=0. The slot and segment numbers are returned anyway in the data buffer, as in the case of specifying a driver index.

An .IDRVR error will be returned if there is no driver associated to the specified index, or if there is no driver with the specified slot and segment pair. There is no way to know in advance how many drivers are present in the system, so to discover all the drivers this function must be invoked several times, starting with driver index 1 and increasing the index number until a .IDRVR error is obtained.

The information returned in the data buffer is as follows:

```
+0: Driver slot number
+1: Driver segment number, FFh if the driver is embedded within a Nextor
    or MSX-DOS kernel ROM (always FFh in current version)
+2: Number of drive letters assigned to this driver at boot time
+3: First drive letter assigned to this driver at boot time (A:=0, etc), 
    unused if no drives are assigned at boot time
+4: Driver flags:
    bit 7: 1 => the driver is a Nextor driver
           0 => the driver is a MSX-DOS driver
                (embedded within a MSX-DOS kernel ROM)
    bits 6-3: Unused, always zero
    bit 2: 1 if the driver implements the DRV_CONFIG routine
    bit 1: Unused, always zero
    bit 0: 1 => the driver is a device-based driver
           0 => the driver is a drive-based driver
+5: Driver main version number
+6: Driver secondary version number
+7: Driver revision number
+8: Driver name, left justified, padded with spaces (32 bytes)
+40..+63: Reserved (currently always zero)
```

In the case of MSX-DOS drivers, the driver flags byte is always zero, and no information about driver version number or driver name is returned.

Nextor uses the DRV_CONFIG routine starting at version 2.0.5. See  the _[Nextor 2.1 Driver Development Guide](Nextor%202.1%20Driver%20Development%20Guide.md)_ document for details.

### 3.9. Get information about a drive letter (_GDLI, 79h)

```
Parameters:  C = 79H (_GDLI)
             A = physical drive (0=A:, 1=B:, etc)
             HL = Pointer to 64 byte data buffer

Results:     A  = Error code
             HL = Filled with data about the drive
```

Returns information about a given drive letter. This function works in MSX-DOS 1 mode.

The information returned in the data buffer is as follows:

```
+0: Drive status
    0: Unassigned
    1: Assigned to a storage device attached to a Nextor or MSX-DOS driver
    2: Unused
    3: A file is mounted in the drive
    4: Assigned to the RAM disk (all other fields will be zero)
+1: Driver slot number
+2: Driver segment number, FFh if the driver is embedded within a Nextor 
    or MSX-DOS kernel ROM (always FFh in current version)
+3: Relative drive number within the driver 
    (for drive-based drivers only; FFh if device-based driver)
+4: Device index (for device-based drivers only; 
    0 for drive-based drivers and MSX-DOS drivers)
+5: Logical unit index (for device-based drivers only; 
    0 for drive-based drivers and MSX-DOS drivers)
+6..+9: First device sector number (for devices in device-based drivers only;
        always zero for drive-based drivers and MSX-DOS drivers)
+10..+63: Reserved (currently always zero)
```

If a file is mounted in the drive, the information returned in the data buffer is insetad as follows:

```
+1: Drive where the mounted file is located (0 = A:, etc)
+2: Flags:
    bit 0: mount mode, 0 = read and write, 1 = read-only
+3: Always 0
+4: Filename in printable format (up to 12 characters, plus a terminating zero)
```

If a drive larger than the maximum drive number supported by the system is specified, an .IDRV error will be returned. Note that if a drive number is specified which is legal in Nextor, but is currently not assigned to any driver, then no error will be returned, but an empty information block will be returned (the drive status byte should be checked).

The "first device sector number" is the absolute device sector number that is treated as the first logical sector for the drive; usually it is either the starting sector of a device partition, or the device absolute sector zero, if the device has no partitions. Note that you can't test this value against zero to check whether the drive is assigned to a block device on a device-based driver or not (use the “drive status” field for this purpose).

### 3.10. Get information about a device partition (_GPART, 7Ah)

```
Parameters:  C = 7AH (_GPART)
             A = Driver slot number
             B = Driver segment number, FFh for drivers in ROM
                 (must be always FFh in current version)
             D = Device index
             E = Logical unit index
             H = Primary partition number (1 to 4)
             H:7 = 0: Get information about the partition
                   1: Get the device sector number that holds
                      the partition table entry
             L = Extended partition number
                 (0 for an entry in the primary partition table)

Results:     A = Error code
             If partition information is requested:
                 B = Partition type code
                 C = Status byte of the partition
                 HL:DE = Starting device absolute sector number of the partition
                 IX:IY = Partition size in sectors
             If the sector number of the partition table entry is requested:
                 HL:DE = Device sector number that holds the partition table entry
```

Returns information about a device partition. This function works in MSX-DOS 1 mode.

This function only works on device-based drivers; if a non-existing driver, a drive-based driver, or a MSX-DOS driver is specified in A and B, then an .IDRVR error will be returned. If the specified device and/or logical unit do not exist in the driver, an .IDEVL error will be returned.

Storage devices are usually divided in partitions, each one being an independent logical volume residing in a contiguous block of sectors in the media. This function allows finding the starting sector of a given partition in the media, usually in order to map it to a drive letter by using the MAPDRV function, so that the contained filesystem can be accessed by Nextor.

The partition type code returns information about the filesystem that the partition holds. The code may be one of the following:

```
0: None (the partition with the specified number does not exist)
1: FAT12
4: FAT16, smaller than 32MB (obsolete)
5: Extended (see below)
6: FAT16 (CHS)
14: FAT16 (LBA)
```

There are many more partition type codes defined, but they refer to filesystems that can't be handled by Nextor so they are not listed here.

A device can have up to four primary partitions, numbered 1 to 4. In order to accommodate more than four partitions, partition number 2 may be of a special type named "Extended". An extended partition is actually a container for more partitions; there is no limit in the number of extra partitions that a partition of type "Extended" can contain. Primary partitions 3 and 4 do not exist when partition 2 is extended.

In order to enumerate all the partitions existing in a device , the following procedure should be followed, to take in account the possible presence of extended partitions:

1.  Search partition 1-0 (primary number 1, extended number 0).

2.  Search partition 2-0. If it exists and is of type "Extended", search partitions 2-1, 2-2, 2-3, etc, until a partition code 0 is returned.

3.  If partition 2-0 does not exist or is not of type "Extended", search partitions 3-0 and 4-0.

Note that it is possible that a device has no partitions at all. In this case, it is still possible that the device contains a valid filesystem, mapped to the absolute device sector zero; this is indeed the case of floppy disks and devices with very small capacity.

When a partition is mapped to a drive letter, the partition first sector will always be examined in order to determine the actual filesystem hold by the partition. Nextor will never rely in the partition type code to determine the filesystem type.

Nextor needs to read the device in order to search for partitions. If there is any error when accessing the device (for example, not ready), an error code will be returned. The standard system error handling routine (or the user error handling routine, if one is defined with _DEFER) will NOT be invoked.

When the specified partition does not exist in the device (for example, when a primary partition number larger than 4 is specified, or when an extended partition number is specified for a non-extended primary partition), then A=.IPART will be returned.

Starting with Nextor 2.1.0 beta 2, it is possible to request the device sector number that holds the partition table, instead of requesting information about the partition; this is useful for applications intended for modifying the partition table entries. The way to locate the partition table entry in the returned sector is as follows:

* If a primary partition was requested (extended number was 0), then the sector number returned is always 0, and the offset for the partition table entry in the sector is 1BEh, 1CEh, 1DEh and 1EEh for primary partition numbers 1, 2, 3 and 4 respectively.

* If an extended partition is requested (primary partition number is 2, extended partition number is not 0) then the partition table entry is the first one in the returned sector (at offset 1BEh).

For more information on the partition table structure see [Master Boot Record on Wikipedia](https://en.wikipedia.org/wiki/Master_boot_record).


### 3.11. Call a routine in a device driver (_CDRVR, 7Bh)

```
Parameters:  C = 7BH (_CDRVR)
             A = Driver slot number
             B = Driver segment number, FFh for drivers in ROM
                 (must be always FFh in current version)
             DE = Routine address
             HL = Address of a 8 byte buffer with the input register
                  values for the routine

Results:     A = Error code
             BC, DE, HL = Results from the routine
             IX = Value of AF returned by the routine
```

Allows direct invocation of a routine in a device driver. This function works in MSX-DOS 1 mode.

Routines for any driver type (MSX-DOS, device-based and drive-based) can be invoked with this function, however it is intended primarily for device-based drivers, in order to enumerate devices (DEV_INFO and LUN_INFO) and directly access the device absolute sectors (DEV_RW routine), for example to develop device partitioning tools. The available routines for device drivers are enumerated and described in detail in the _[Nextor 2.1 Driver Development Guide](Nextor%202.1%20Driver%20Development%20Guide.md)_ document.

The input value of registers AF, BC, DE and HL for the routine must be provided in an 8 byte buffer pointed by HL. The order of the register values in the buffer is as follows: F, A, C, B, E, D, L, H. The output values of these registers, on the other hand, are returned directly in the registers themselves; except the output value of AF which is returned in IX.

Some routines accept data from, or write data to, memory buffers supplied by the user. There are two limitations for exchanging data with the driver routines in this way: first, the buffer must be in the primary mapper slot; and second, the buffer may not be partially or totally in page-1. These limitations do not apply for the 8 byte register buffer (remember however that when invoking Nextor function calls via the F37Dh hook, no parameters can be passed in page-1). The register buffer is only used before effectively executing the driver routine, therefore there is no problem if it overlaps with any buffer used by the routine to return data.

An .IDRVR error will be returned by this function call if a non-existing driver is supplied in A and B. Use the Nextor function _GDRVR to discover the location of the existing drivers.

### 3.12. Map a drive letter to a driver and device (_MAPDRV, 7Ch)

```
Parameters:  C = 7CH (_MAPDRV)
             A = Physical drive (0=A:, 1=B:, etc)
             B = Action to perform
                 0: Unmap the drive
                 1: Map the drive to its default state
                 2: Map the drive by using specific mapping data
                 3: Mount a file in the drive
             HL = Address of a 8 byte buffer with mapping data (if B=2)
                  Address of the file name or FIB (if B=3)
             D = File mount type (if B=3)
                 0: Automatic (read-only if the file has that attribute set, read and write otherwise)
                 1: Read-only
Results:     A = Error code
```

Allows mapping a drive number to a specific combination of device number, logical unit number, and starting absolute device sector number, within a device-based driver. It also allows to revert back the drive mapping to its default state (the state at boot time), and to completely unmap the drive. This function works in MSX-DOS 1 mode with some restrictions (see below).

If B=0 at input, the drive will be unmapped. This means that the drive will be unavailable from that moment, and any attempt to access it will result in an "Invalid drive" error. If the drive is already unmapped, nothing will happen and no error will be returned.

If B=1 at input, the drive will be reverted to its default state. If at boot time the drive was unmapped (not assigned to any driver), or was mapped to a drive on a MSX-DOS driver or on a drive-based driver, then the drive will be reverted to the same state. If at boot time the drive was assigned to a device-based driver, then an auto-assign procedure will be performed for this drive, using the same rules as the automatic mapping procedure performed at boot time (the automatic mapping procedure is described in the _[Nextor 2.1 User Manual](Nextor%202.1%20User%20Manual.md)_ document) except that the NEXTOR.DAT file will not be searched. This may result or not on the drive having the same mapping as it had at boot time, depending on the presence of removable devices in the associated driver and the state of the other drives.

If the automatic mapping procedure resulting from invoking this function with B=1 fails (because there are no suitable devices or partitions), the drive will be unmapped, regardless of its previous mapping state. An .IDEVL error will be returned in this case.

If B=2 at input, the drive will be mapped according to the mapping data provided in the buffer pointer by HL. It is possible to map any system drive to any device-based driver by using this method, even drives that were unmapped or were mapped to a different driver at boot time. The contents of the mapping data buffer must be as follows:

```
+0: Driver slot number
+1: Driver segment number,
    FFh if the driver is embedded within a Nextor kernel ROM
    (must be always FFh in current version)
+2: Device number
+3: Logical unit number
+4..+7: Starting sector
```

An .IDRVR error will be returned if the specified driver does not exist or is not a device-based driver. An .IDEVL error will be returned if the device with the specified device and logical unit numbers does not exists in the driver. In these cases, the previous drive mapping will not be modified.

A .RAMDX error will be returned if the drive specified is H: and a RAM disk exists.

The "starting sector" parameter is the device absolute sector number that will be used as the sector zero for the drive. Usually this will be the starting sector of a device partition, obtained via a call to the _GPART function. Note however that no checking is done for the presence of an actual (and recognized by Nextor) filesystem starting in the specified sector; if no valid filesystem is found, the _MAPDRV function will succeed, but the next access to the drive will return a "Not a DOS disk" error.

Also, it is possible to map a drive to a removable device which has no media inserted. In this case, the _MAPDRV function will succeed, but the next access to the drive will return a "Disk offline" error.

It is not possible to map two drives to the same combination of driver, device, logical unit, and start sector; this is to prevent data corruption resulting from dealing with unsynchronized sector buffers. An .IDEVL error will be returned in this case. In order to change the drive letter for a given mapping, the old drive letter must be first unmapped.

Also, note that it is not possible to explicitly map a drive to a MSX-DOS driver or a drive-based driver.

Before changing the mapping state of a drive, any open file handles relative to that drive will be closed by this function. This is equivalent to invoking the CLOSE function call for each of these file handles, so disk errors may arise if there are dirty buffers for that drive and there are errors when flushing them to the device.

When invoked in MSX-DOS 1 mode, the following restrictions apply to this function:

* The specified drive must have been mapped to a device-base driver at boot time. It is not possible to change the mapping of a drive that was unmapped or mapped to a MSX-DOS driver or a Nextor drive-based driver at boot time.

* The new mapping information may specify a different partition and/or device, but the driver slot must be the same that was assigned to the drive at boot time. This is not an issue if there is only one Nextor kernel in the system.
These restrictions are imposed by the Nextor architecture.

If B=3 at input, the file whose name or FIB is passed in HL will be mounted in the drive; file mounting is available since Nextor 2.1.0. A .BFSZ error will be returned if the file is too small or too big.


### 3.13. Enable or disable the Z80 access mode for a driver (_Z80MODE, 7Dh)

```
Parameters:  C = 7DH (_Z80MODE)
             A = Driver slot number
             B =  00H => get current Z80 access mode
                  01H => set Z80 access mode
             D =  00H => disable Z80 access mode (only if B=01H)
                  FFH => enable Z80 access mode (only if B=01H)

Results:     A = Error code
             D = Current Z80 access mode for the specified driver,
                 same as input
```

This function works on MSX Turbo-R computers only. On MSX1/2/2+ it will always return an .IDRVR error.

This function allows enabling or disabling the Z80 access mode for a MSX-DOS driver (a driver embedded within a MSX-DOS kernel ROM). When the Z80 access mode for a driver is enabled, Nextor will switch to the Z80 CPU prior to accessing any drive associated to the driver. When the Z80 access mode for a driver is disabled, no CPU switching is performed, being therefore possible to access the drives on the driver in R800 mode.

When the computer boots, the Z80 access mode is enabled for all MSX-DOS drivers. This is necessary because some old device drivers do not work when accessed in R800 mode, and it would not be possible to boot from a device on one of these drivers. Once the system boot has finished, this function can be used to disable (and later re-enable if necessary) the Z80 access mode for any MSX-DOS driver.

Note that the Z80 access mode is enabled or disabled for a whole driver, affecting all the drives associated to the driver. It is not possible to selectively enable or disable the Z80 access mode for individual drives.

The Z80 access mode applies to MSX-DOS drivers only. Nextor will never change the CPU when accessing drive-based and device-based drivers.

## 4. New error codes

New error codes are defined to handle error conditions when managing the new features of Nextor. These errors are returned in MSX-DOS 1 mode as well by the new functions supported in this mode.

The error codes and descriptions are the following:

* Invalid device driver (.IDRVR, 0B6h)

An operation involving a device driver has been requested but the specified driver does not exist, or is not of the valid type (for example, the driver is a MSX-DOS or drive-based driver but a device-based driver is required).

* Invalid device or LUN (.IDEVL, 0B5h)

An operation involving a device on a device-based driver has been requested but the specified device does not exist in the driver, or the specified logical unit does not exist in the specified device.

* Invalid partition number (.IPART, 0B4h)

Information about a disk partition on a device has been requested, but the specified partition does not exist in the device.

* Partition is already in use (.PUSED, 0B3h)

An attempt has made to map a drive to a driver, device and starting sector number; but there is already another drive which is mapped to the same combination of driver, device, logical unit, and starting sector number.

* File is mounted (.FMNT, 0B2h)

An attempt to open or alter a mounted file, or to perform any other disallowed operation involving a mounted file, has been made.

* Bad file size (.BFSZ, 0B1h)

Attempt to mount a file that is smaller than 512 bytes or larger than 32 MBytes.


## 5. Extended mapper support routines

The original MSX-DOS 2 mapper support routines (see "5. Mapper support routines" in [MSX-DOS 2 Program Interface Specification](https://github.com/Konamiman/Nextor/blob/v2.1/docs/DOS2-PIS.TXT))
have been extended with four new routines that allow reading data, writing data and calling routines placed in another RAM segment; they work
much like the existing routines RD_SEG, WR_SEG, CAL_SEG and CALLS but they accept a pair of slot number + RAM segment number as input instead of only the RAM segment number.
These routines are compatible with the [UNAPI RAM helper specification](https://github.com/Konamiman/MSX-UNAPI-specification/blob/master/docs/MSX%20UNAPI%20specification%201.1.md#4-the-ram-helper),
including the extended BIOS based discovery mechanism; this means that any application program that relies on the presence of the UNAPI RAM helper will work out of the box
with Nextor, without needing to first install a standalone helper.

The names and locations of these routines in the mapper support routines jump table is as follows:

```
+30h: CALL_MAP
+33h: RD_MAP
+36h: CALL_MAPI
+39h: WR_MAP
```


### 5.1: CALL_MAP: Call a routine in a mapped RAM segment

* Input:
  * IYh = Slot number
  * IYl = Segment number
  * IX = Target routine address (must be a page 1 address)
  * AF, BC, DE, HL = Parameters for the target routine
* Output:
  * AF, BC, DE, HL, IX, IY = Parameters returned from the target routine

The routine will be called by switching the specified slot and segment in page 1, therefore the routine address must be in page 1 as well.


### 5.2: RD_MAP: Read a byte from a RAM segment

* Input:
  * A = Slot number
  * B = Segment number
  * HL = Address to be read from (higher two bits will be ignored)
* Output:
  * A = Data read from the specified address
  * F, BC, DE, HL, IX, IY preserved


### 5.3: CALL_MAPI: Call a routine in a mapped RAM segment, with inline routine identification

* Input:
  * AF, BC, DE, HL = Parameters for the target routine
* Output:
  * AF, BC, DE, HL, IX, IY = Parameters returned from the target routine

The routine is to be called as follows:

    CALL CALLSEG

    CALLSEG:
      CALL <address of CALL_MAPI>
      DB &Bmmeeeeee
      DB <segment number>
      ;no RET is needed here

 where

* `mm` is the mapper slot, as an index (0 to 3) in the mapper variables table provided by the standard mapper support routines
(see "5.2 Mapper variables and routines" in [MSX-DOS 2 Program Interface Specification](https://github.com/Konamiman/Nextor/blob/v2.1/docs/DOS2-PIS.TXT)).

* `eeeeee` is the routine to be called, as an index (0 to 63) of a jump table that starts at address 4000h of the segment. That is, 0 means 4000h, 1 means 4003h, 2 means 4006h, etc.

The way to specify the mapper slot and the segment number is weird, but it allows to pack the entire call in five bytes. This allows to use this routine with hooks in the same way
it's usually done with the BIOS routine CALLF.


### 5.4: WR_MAP: Write a byte to a RAM segment

* Input:
  * A = Slot number
  * B = Segment number
  * E = Byte to write
  * HL = Address to be written to (higher two bits will be ignored)
* Output:
  * A = Data readed from the specified address
  * F, BC, DE, HL, IX, IY preserved


### 5.5. The UNAPI RAM Helper discovery procedure

Nextor implements the UNAPI RAM Helper discovery procedure in order to make these new mapper support routines compatible with the already existing 
[UNAPI RAM Helper specification](https://github.com/Konamiman/MSX-UNAPI-specification/blob/master/docs/MSX%20UNAPI%20specification%201.1.md#4-the-ram-helper).
For reference, the discovery procedure is repeated here:

> To check for the presence of a RAM helper, and to obtain the address of its routines, EXTBIO (0FFCAh) must be called with DE=2222h, HL=0, and A=FFh. If the RAM helper is not installed, then HL=0
> at output; otherwise the following register values will be returned:
>
> * HL = Address of a jump table in page 3
> * BC = Address of the reduced mappers table in page 3 (zero if not provided)
> * A = Number of entries in the jump table

In the case of Nextor the following applies:

* HL will point to the location of CALL_MAP (offset +30h from the start of the mapper support routines jump table).
* BC will always be returned as zero (the reduced mappers table is mandatory for UNAPI RAM Helpers only when the mapper support routines are not present).
* A will always be 4 (in the MSX UNAPI specification it's 3 since the WR_MAP routines is not defined - this is a non-breaking change).


### 5.6. Breaking change notice

In versions of Nextor older than 2.1.0, including the alphas and betas of 2.1.0, the mapper support routines jump table area that is now used for the RD_MAP, WR_MAP, CALL_MAP and CALL_MAPI
entry points was used for two different routines that are not available anymore, BLK_ALLOC and BLK_FREE; this represents a breaking change and existing applications making use of them
will need changes.

Although not used by Nextor anymore, the source code of these removed routines is kept as part of the Nextor code base
(at [source/kernel/bank4/bkalloc.mac](https://github.com/Konamiman/Nextor/blob/v2.1/source/kernel/bank4/bkalloc.mac)).
This way, if you have an application that makes use of these routines you can simply incorporate the code from that file to your application and call the routines directly.

You can read the documentation for the removed routines in [the Programmers Reference for Nextor 2.0](https://github.com/Konamiman/Nextor/blob/v2.0/docs/Nextor%202.0%20Programmers%20Reference.md#5-extended-mapper-support-routines).


## 6. Other features

This section describes other miscellaneous new features offered by Nextor.

### 6.1. Correction of the ESC-Y escape sequence bug in STROUT function

The STROUT function prints a string finished with a "$" character. There is an escape sequence that allows positioning cursor at any location in the screen, the sequence is: ESC (27) "Y" (89) x+32 y+32, where (x,y) is the desired character position.

The bug appears when this escape sequence is used and either the x or the y coordinate are 4. In this case, the third or fourth byte of the sequence becomes 36, which is the ASCII code of "$"; then MSX-DOS incorrectly assumes that this is the end of string mark and the string is truncated.

This bug is corrected in Nextor, so the ESC-Y escape sequence can be safely used.

### 6.2. Changing the NEXTOR.SYS version number

Some MSX-DOS command line applications are known to check the version number of MSXDOS2.SYS (NEXTOR.SYS in the case of Nextor) and refuse to work if this number is smaller than a certain value, typically 2.20. This is a problem since the current NEXTOR.SYS version number is 2.1.

As a workaround for this issue, starting at version 2.0 beta 2 the NEXTOR.SYS version number returned by the DOSVER function call is stored in RAM and can be changed easily. There is a command line tool, NSYSVER.COM, that allows to easily do this change (see the _[Nextor 2.1 User Manual](Nextor%202.1%20User%20Manual.md)_ for more details) but if you want to do the change programmatically, here is the procedure:

1.  When in MSX-DOS mode (page 0 mapped to TPA RAM), read the 16 bit value stored at address 0001h.
2.  Add 32h to the obtained value.
3.  The resulting value is an address where a 16 bit number is stored. This is the number that the DOSVER function call returns in register DE, that is, the reported NEXTOR.SYS version.

For example, the code for changing the NEXTOR.SYS version to 2.31 would be as follows:

```
ld ix,(0001h)
ld bc,32h
add ix,bc
ld (ix),31h
ld (ix+1),2
```

Of course this change is temporary and it will cease to have effect (that is, the NEXTOR.SYS version number will revert to its real value) when NEXTOR.SYS is reloaded, either because the BASIC prompt is entered and exited via CALL SYSTEM, or because the computer is rebooted.

All of this applies only if the loaded version of NEXTOR.SYS is 2.0 beta 2 or newer.

## 7. Nextor internals

This section details how some of the Nextor features work internally. This may be useful to develop applications that make use of these features as an alternative to the supplied Nextor tools.

## 7.1. One-time boot keys

The [one-time boot keys mechanism](Nextor%202.1%20User%20Manual.md#292-one-time-boot-keys) kicks in at boot time when the zero-terminated signature string NEXTOR_BOOT_KEYS is found at address A100h. In that case, the status of the alphanumeric keys are taken from the bytes that follow the signature, instead of being read from the keyboard, as the following table shows; a bit set to 1 means that the key is considered to be pressed.

| Address | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|:-------:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| A111h   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
| A112h   | F | E | D | C | B | A | 9 | 8 |
| A113h   | N | M | L | K | J | I | H | G |
| A114h   | V | U | T | S | R | Q | P | O |
| A115h   |   |   |CTRL|SHFT| Z | Y | X | W |

Note that currently not all keys are actually used by Nextor at boot time (e.g. numbers 6 to 9); but if any future version of Nextor makes use of any of the currently unused keys in the table, the key status will be expected to be at the position defined in this table when using the one-time boot keys mechanism.

## 7.2. Disk emulation mode

This section explains some details about the [disk emulation mode](Nextor%202.1%20User%20Manual.md#39-disk-emulation-mode).

### 7.2.1. Disk emulation data file format

The disk emulation data file, used by Nextor to know which disk image files must be used during an emulation session, consists of a header followed by a table with information about each of the disk image files. The header has the following contents:

| Offset | Meaning |
|:------:|---------|
|   +0   | Signature string `NEXTOR_EMU_DATA`, zero terminated |
|   +16  | Number of entries in the disk image files table     |
|   +17  | 1-based index of the file to mount at boot time     |
|   +18  | Address to use as work area during the emulation session, or 0 if this area must be allocated (2 bytes, little endian) |
|   +20  | Reserved, must be zero (4 bytes)                    |

Each entry in the disk image files table is as follows:

| Offset | Meaning |
|:------:|---------|
|   +0   | Number of the device that contains the file (0 = same as emulation data file) |
|   +1   | Number of the logical unit that contains the file (if device number is not 0)  |
|   +2   | Absolute device sector number where the file starts (4 bytes, little endian) |
|   +6   | Size of the file in sectors (2 bytes, little endian) |

If the device number in a disk image files table entry is zero, then Nextor will assume that the disk image file is located at the same device and logical unit as the emulation data file (the logical unit number in the table entry is ignored in this case). The `EMUFILE.COM` tool sets the device number to 0 for all entries if all the disk image files are located in the same device and logical unit as the emulation data file that is being created.

Any contents in the emulation data file past the last entry in the disk images file table is ignored by the Nextor kernel. The `EMUFILE.COM` tool places here a printable list of the disk image filenames, it can be displayed by executing `TYPE /B datafile` in the command prompt.

### 7.2.2. Entering disk emulation mode

At boot time Nextor will enter disk emulation mode if it finds a pointer to the disk emulation data file. This pointer consists simply of a device+logical unit number and an absolute device sector number.

Note that although this documentation and [the user manual](Nextor%202.1%20User%20Manual.md) use the term "emulation data file", an actual file isn't required - the Nextor kernel is only concerned about the sector number where the emulation data is located, regardless of whether this sector is part of a file or not. Using a file is usually the most convenient way to store this information, but a tool could be developed to use e.g. a reserved sector right before the FAT for this purpose.

Nextor will enter the one-time disk emulation mode if it finds the following information in RAM at boot time:

| Address | Contents |
|:-------:|---------|
| A000h   | Signature string `NEXTOR_EMU_DATA`, zero terminated        |
| A010h   | Number of the device that contains the emulation data      |
| A011h   | Number of the logical unit that contains the emulation data |
| A012h   | Absolute device sector number that contains the emulation data (4 bytes, little endian) |

If the above information is not found, Nextor will enter the persistent disk emulation mode if it finds the following information in the first partition table entry of any of the available devices in the primary Nextor controller:

| Sector offset | Partition table entry offset | Partition table meaning | Nextor meaning |
|:-------------:|:----------------------------:|-------------------------|----------------|
| 1BEh          | +0                           | Status byte             | Bit 0 set = enter disk emulation mode                 |         
| 1BFh          | +1                           | Start CHS               | Number of the device that contains the emulation data |
| 1C0h          | +2                           | Start CHS               | Number of the logical unit that contains the emulation data |
| 1C1h          | +3                           | Start CHS               | MSB of the absolute device sector number that contains the emulation data |
| 1C2h          | +4                           | Partition type          | Must be non zero |
| 1C3h          | +5                           | End CHS                 | 3rd byte of the absolute device sector number that contains the emulation data |
| 1C4h          | +6                           | End CHS                 | 2nd byte of the absolute device sector number that contains the emulation data |
| 1C5h          | +7                           | End CHS                 | LSB of the absolute device sector number that contains the emulation data |

For example, if the emulation data is located at device 1, logical unit 2, sector 33445566h, and the partition is FAT16 (partition type 0Eh) and has the "active" flag set in the status byte, then the start of the partition table entry set for persistent emulation would look like this (in hexadecimal):

    81 01 02 33 0E 44 55 66

Note that the condition for Nextor entering emulation mode is that bit 0 of the status byte must be set **and** the partition type code must be non-zero (but can be any other value, not necessarily FAT). Also note that the device and logical unit information here refers to the emulation data file only - the disk image files themselves can be located at a different device.

Also worth noting: Nextor will check that the emulation data actually starts with the `NEXTOR_EMU_DATA` signature, and if that's not the case then it will boot normally without entering disk emulation mode.


## 8. Change history

This section contains the change history for the different versions of Nextor. Only the changes that are meaningful from the application developer point of view are listed. For information on changes in general, please look at the _[Nextor 2.1 User Manual](Nextor%202.1%20User%20Manual.md)_ document. For information on changes related to driver development, please look at the _[Nextor 2.1 Driver Development Guide](Nextor%202.1%20Driver%20Development%20Guide.md)_ document.

This list contains the changes for the 2.1 branch only. For the change history of the 2.0 branch see the _[Nextor 2.0 Programmers Reference](../../../blob/v2.0/docs/Nextor%202.0%20Programmers%20Reference.md#7-change-history)_ document.

### 8.1. v2.1.0 beta 2

* [_GPART](#310-get-information-about-a-device-partition-_gpart-7ah) now returns the status byte of the partition, and allows to retrieve the device sector number that holds the partition table entry instead of information about the partition.

### 8.2. v2.1.0 beta 1

* [_GDRVR](#38-get-information-about-a-device-driver-_gdrvr-78h) now returns an extra flag that tells if the driver implements the DRV_CONFIG routine.

* [_MAPDRV](#312-map-a-drive-letter-to-a-driver-and-device-_mapdrv-7ch) now allows to mount files.

* [_GDLI](#39-get-information-about-a-drive-letter-_gdli-79h) returns a new set of information for drives where a file is mounted.

* [Error codes](#4-new-error-codes) "File is mounted" (.FMNT) and "Bad file size" (.BFSZ) introduced.
