# Nextor 3.0 Driver Development Guide

## Index

[1. Introduction](#1-introduction)

[2. The Nextor kernel architecture](#2-the-nextor-kernel-architecture)

[2.1. The MSX-DOS 1 kernel](#21-the-msx-dos-1-kernel)

[2.2. The MSX-DOS 2 kernel](#22-the-msx-dos-2-kernel)

[2.3. The Nextor kernel](#23-the-nextor-kernel)

[3. Creating a Nextor kernel ROM with embedded driver](#3-creating-a-nextor-kernel-rom-with-embedded-driver)

[3.1. Manual creation](#31-manual-creation)

[3.2. Using the mknexrom utility](#32-using-the-mknexrom-utility)

[3.3. Rules for the bank switching code](#33-rules-for-the-bank-switching-code)

[4. Nextor driver structure](#4-nextor-driver-structure)

[4.1. One single driver model](#41-one-single-driver-model)

[4.2. Page 0 routines and data](#42-page-0-routines-and-data)

[4.2.1. GSLOT1 (402Dh)](#421-gslot1-402dh)

[4.2.2. RDBANK (403Ch)](#422-rdbank-403ch)

[4.2.3. CALLB0 (403Fh)](#423-callb0-403fh)

[4.2.4. CALBNK (4042h)](#424-calbnk-4042h)

[4.2.5. GWORK (4045h)](#425-gwork-4045h)

[4.2.6. CALDRV (4048h)](#426-caldrv-4048h)

[4.2.7. CALLB0_IX_IY (404Bh)](#427-callb0_ix_iy-404bh)

[4.2.8. K_SIZE (40FEh)](#428-k_size-40feh)

[4.2.9. CUR_BANK (40FFh)](#429-cur_bank-40ffh)

[4.2.10. CHGBNK (7FD0h)](#4210-chgbnk-7fd0h)

[4.3. The driver header](#43-the-driver-header)

[4.4. Driver routines](#44-driver-routines)

[4.4.1. TIMER_INT (4110h)](#441-timer_int-4110h)

[4.4.2. OEMSTAT (4113h)](#442-oemstat-4113h)

[4.4.3. BASDEV (4116h)](#443-basdev-4116h)

[4.4.4. EXTBIO (4119h)](#444-extbio-4119h)

[4.4.5. DRIVER_QUERY (411Ch)](#445-driver_query-411ch)

[4.4.6. DEVICE_QUERY (411Fh)](#446-device_query-411fh)

[4.4.7. CUSTOM_DRIVER_QUERY (4122h)](#447-custom_driver_query-4122h)

[4.4.8. CUSTOM_DEVICE_QUERY (4125h)](#448-custom_device_query-4125h)

[4.4.9. READ_WRITE (4128h)](#449-read_write-4128h)

[4.4.10. RESERVED_0/1/2 (412Bh/412Eh/4131h)](#4410-reserved_012-412bh412eh4131h)

[4.4.11. DIRECT_0...4 (4134h...4140h)](#4411-direct_04-4134h4140h)

[4.5. Driver queries](#45-driver-queries)

[4.5.1. Driver query 1: Get driver version number](#451-driver-query-1-get-driver-version-number)

[4.5.2. Driver query 2: Get driver information string](#452-driver-query-2-get-driver-information-string)

[4.5.3. Driver query 3: Get driver initialization parameters](#453-driver-query-3-get-driver-initialization-parameters)

[4.5.4. Driver query 4: Initialize driver](#454-driver-query-4-initialize-driver)

[4.5.5. Driver query 5: Get maximum supported device number](#455-driver-query-5-get-maximum-supported-device-number)

[4.5.6. Driver query 6: Initialize RAM driver](#456-driver-query-6-initialize-ram-driver)

[4.5.7. Driver query 7: Shut down RAM driver](#457-driver-query-7-shut-down-ram-driver)

[4.6. Device queries](#46-device-queries)

[4.6.1. Device query 1: Get device information string](#461-device-query-1-get-device-information-string)

[4.6.2. Device query 2: Get device parameters](#462-device-query-2-get-device-parameters)

[4.6.3. Device query 3: Get device status](#463-device-query-3-get-device-status)

[4.6.4. Device query 4: Get device availability](#464-device-query-4-get-device-availability)

[4.6.5. Device query 5: Get format choices for a floppy disk device](#465-device-query-5-get-format-choices-for-a-floppy-disk-device)

[4.6.6. Device query 6: Format a floppy disk device](#466-device-query-6-format-a-floppy-disk-device)

[4.6.7. Device query 7: Stop the motor of a floppy disk drive](#467-device-query-7-stop-the-motor-of-a-floppy-disk-drive)

[4.7. Other](#47-other)

[4.7.1. The free space at kernel main bank](#471-the-free-space-at-kernel-main-bank)

[5. Testing drivers with DRVTEST.COM](#5-testing-drivers-with-drvtestcom)


## 1. Introduction

Nextor is an enhanced version of MSX-DOS 2, the disk operating system for MSX computers. It is based on MSX-DOS 2.31, with which it is 100% compatible.

This document provides a complete guide for programmers who want to develop device drivers for Nextor. It is a good idea to get acquainted with Nextor by reading _[Nextor 3.0 User Manual](Nextor_3.0_User_Manual.md)_ prior to this document. Also, although not strictly necessary, it is recommended to take a look at the _[Nextor 3.0 Programmers Reference](Nextor_3.0_Programmers_Reference.md)_ document, which is a reference of the new features that Nextor adds to MSX-DOS 2 from a developer point of view other than driver development.

Nextor 3 uses a driver structure that's similar, but not identical, to the one used by Nextor 2: drivers developed for Nextor 2 will need to be adapted to the new structure before they can be used in Nextor 3. This process is detailed in the _[Nextor 3.0 Driver Migration Guide](Nextor_3.0_Driver_Migration_Guide.md)_.

## 2. The Nextor kernel architecture

This section explains some basic concepts about the Nextor kernel architecture, including a short explanation on how the MSX-DOS kernel worked and how things have changed in Nextor. The information provided will help driver developers to understand the context in which the driver code is executed.

### 2.1. The MSX-DOS 1 kernel

The original MSX-DOS kernel (labeled as version 1) was present as a ROM embedded in the external MSX floppy disk controllers first, and later as an internal ROM in the MSX computers with built-in floppy disk drive as well. It is a 16K ROM that uses the page 1 address space (addresses 4000h to 7FFFh) of its slot.

The MSX-DOS 1 kernel is divided in two main parts:

* The kernel common code. It contains the hardware-independent code, such as the code for all the function calls or the FAT filesystem management code. Most of the code in the ROM accounts for this part.

* The disk driver. This is the code that physically accesses the massive storage devices, mainly to read and write disk sectors, as requested by the kernel code when necessary. It consists of a series of routines with standardized input and output parameters.

The kernel common code part is not 100% driver independent. It contains a couple of points that must be patched depending on the disk driver used: one that specifies how many driver units will be controlled by the driver, and another one that specifies how much page 3 work area is needed by the driver.

Figure 1 shows a diagram with the structure of an MSX-DOS 1 kernel.

```
4000h +---------------------+
      |                     |
      |                     |
      |                     |
      |        Kernel       |
      |     common code     |
      |                     |
      |                     |
      +---------------------+
      |                     |
      |     Disk driver     |
      |                     |
7FFFh +---------------------+
```

_Figure 1 - MSX-DOS 1 kernel structure_

An MSX computer can have up to four MSX-DOS kernel ROMs active. If more than one is present, then the one with the smallest slot number becomes the "master" (the one whose kernel common code is actually executed), and the others are the "slaves" (only their driver code is executed).

MSX-DOS views the storage devices as drive letters, while the disk driver presents one or more driver units. The mapping between both entities is fixed and one-to-one, so for example drive A: is mapped to driver unit 0 of the first kernel, drive B: is mapped to driver unit 1, and so on.

### 2.2. The MSX-DOS 2 kernel

The MSX-DOS 2 kernel first appeared as a cartridge with no associated storage hardware, and it was intended to be used together with existing storage controllers associated to an MSX-DOS 1 kernel. Later it was included internally in MSX Turbo-R computers.

The MSX-DOS 2 kernel uses the page 1 address space of its slot, as the MSX-DOS 1 kernel does. However the MSX-DOS 2 kernel has a size of 64K. This space is divided in four 16K banks and a bank mapping mechanism is used so that only one of the banks is visible at the same time. The contents of the banks are as follows:

* Bank 0 contains kernel common code and the disk driver code.

* Banks 1 and 2 contain kernel common code.

* Bank 3 contains a copy of the MSX-DOS 1 kernel, with a copy of the disk driver code (only in MSX Turbo-R machines).

Figure 2 shows a diagram with the structure of an MSX-DOS 2 kernel.

```
               Bank 0            Banks 1 and 2             Bank 3
4000h +---------------------+---------------------+---------------------+
      |     Page 0 code     |     Page 0 code     |                     |
40FFh +---------------------+---------------------+                     |
      |     0 (bank ID)     |    1/2 (bank ID)    |                     |
4100h +---------------------+---------------------+                     |
      |                     |                     |                     |
      |       Bank 0        |      Banks 1/2      |       Bank 3        |
      |     kernel code     |     kernel code     |     kernel code     |
      |                     |                     |  (MSX-DOS 1 kernel) |
      |                     |                     |                     |
      +---------------------+                     |---------------------+
      |                     |                     |                     |
      |     Disk driver     |                     |     Disk driver     |
      |                     |                     |                     |
7FD0h +---------------------+---------------------+---------------------+
      | Bank switching code | Bank switching code | Bank switching code |
7FFFh +---------------------+---------------------+---------------------+
```

_Figure 2 - MSX-DOS 2 kernel structure_

There are three parts that are common to all banks (bank 3 contains the bank switching code only):

* The page 0 code is 255 bytes long and contains an entry point for the timer interrupt routine, a routine for calling code on another bank, and other useful utility code.

* The bank ID is just one byte with the bank number, it is needed for doing inter-bank calls.

* The bank switching code is needed for changing the visible bank. The exact code placed here depends on the ROM mapper type used (the original DOS 2 cartridge mapping is ASCII16).

When booting in DOS 2 mode, bank 0 is permanently switched, and other banks are only temporarily switched when bank 0 code needs to call a routine or access data on one of these banks. When booting in DOS 1 mode, bank 3 is switched at boot time, and it remains switched forever.

As was the case with the MSX-DOS 1 kernel, up to four MSX-DOS kernel ROMs can be active at the same time, one of them being the "master" and the others being the "slaves". However, this time the master will not be the kernel with the smallest slot number, but the kernel with the highest version number (the kernel with the smallest slot number is still selected as the master in case of two or more kernels having the same version number).

### 2.3. The Nextor kernel

The Nextor kernel has an architecture that is based on the one of the MSX-DOS 2 kernel, but introduces significant changes:

* The number of banks has grown. In the current version there are two extra banks for the code that implements the new features, including partition management; and one extra bank for the built-in partitioning tool.

* The disk driver ("device driver" in Nextor terminology) code is no longer embedded at the end of the kernel banks 0 and 3. Instead, now the driver has a whole bank for itself, which is located immediately after the last bank of the kernel common code. If necessary, the driver can span across more than one bank.

* The device driver structure is completely new. It of course contains routines to access storage devices, but it also contains extensibility points so that it is easy to add BASIC extended commands ("CALL" commands), extended BIOS commands, and a timer interrupt service routine.

* The page 0 code (the block of 255 bytes at the beginning of each ROM bank) has been modified to contain extra utility routines. These routines can be used by the driver code.

* A new information byte is added at address 40FEh of all banks, which contains the size of the kernel common code in 16K banks (alternatively, this value can be seen as the bank number of the driver).

* The MSX-DOS 1 kernel at bank 3 has been modified (by adding the page 0 code and the bank Id, amongst other things) so that it can perform calls to the device driver.

* There is a 256 byte unused space at the end of banks 0 and 3 (visible at addresses 7ED0h to 7FCFh, right before the bank switching code). This space does not contain any kernel code and can be used to put any code or data that is required by the driver to be here. See _[4.7.1. The free space at kernel main bank](#471-the-free-space-at-kernel-main-bank)_ for more details.

* There are five entry points at kernel banks 0 and 3 (starting at address 7850h) that will be redirected to another five entry points in the driver bank. This way, the driver can provide code that will be accessible via direct inter-slot call to the kernel slot. See _[4.4.11. DIRECT_0...4 (4134h...4140h)](#4411-direct_04-4134h4140h)_ for more details.

Figure 3 shows a diagram with the structure of a Nextor kernel.

```
           Banks 0-(K-1)            Bank K        Banks (K+1)-... (optional)
4000h +---------------------+---------------------+---------------------+
      |     Page 0 code     |     Page 0 code     |     Page 0 code     |
      +---------------------+---------------------+---------------------+
40FEh |          K          |          K          |          K          |
      +---------------------+---------------------+---------------------+
40FFh |       Bank ID       |     K (bank ID)     |       Bank ID       |
4100h +---------------------+---------------------+---------------------+
      |                     |                     |                     |
      |        Bank         |                     |     Additional      |
      |     kernel code     |     Driver code     |     driver code     |
      |                     |                     |                     |
      |                     |                     |                     |
      |                     |                     |                     |
7ED0h +---------------------+                     |                     |
      | Available 256 bytes |                     |                     |
      |  (on banks 0 and 3) |                     |                     |
7FD0h +---------------------+---------------------+---------------------+
      | Bank switching code | Bank switching code | Bank switching code |
7FFFh +---------------------+---------------------+---------------------+
```

_Figure 3 - Nextor kernel structure ("K" is the kernel common code bank count)_

Nextor will use the same rule as MSX-DOS 2 to decide which kernel will be the master if more than one kernel is found (the kernel with the highest version number will win). However this applies to other Nextor kernels only; Nextor will always override other MSX-DOS 1 or 2 kernels present in the system, regardless of their version number.

In Nextor 2 the only way to use a device driver was to append it to a Nextor kernel ROM as explained below. Nextor 3 adds the ability to dynamically load drivers in RAM too. Except where otherwise noted, the information provided in this document applies to both drivers embedded in ROM and drivers loaded in RAM. See [the source of the example RAM driver](../source/drivers/ram-driver-example.asm) for a complete working example.

## 3. Creating a Nextor kernel ROM with embedded driver

In order to create a complete Nextor kernel ROM that can be used in an MSX computer, up to four components are needed:

* The Nextor kernel base file. This file contains the kernel common code, that is, the "Banks 0-(K-1)" portion shown in Figure 3. Its bank switching code is for the ASCII16 mapper (the original mapper used by the MSX-DOS 2 kernel).

* The device driver file. It must be created conforming to the rules and structure detailed in _[4. Nextor driver structure](#4-nextor-driver-structure)_. Its size must be exactly 16080 bytes (16K minus the size of the page 0 code minus the size of the bank switching code). If the driver spans across more than one bank, this applies to each bank.

* The bank switching code file (only if the mapper to be used by the target hardware is not ASCII16). This code depends on the mapping type supported by the ROM cartridge where the complete kernel will be burned. Compiled bank switching code files are provided for the ASCII8 and ASCII16 mappers; for other type of mappers, custom code files must be made, following the rules detailed in _[3.3. Rules for the bank switching code](#33-rules-for-the-bank-switching-code)_.

**Note:** ROM mappers that work with 8K banks instead of 16K banks are supported only if it is possible to select the bank visible at the first half of page 1 (4000h-5FFFh) by writing a single byte in a memory mapped port with a `LD(xxxx),A` instruction. This is the case of ASCII8, for example.

* Optionally, the code that will be placed in the 256 byte unused space at the end of banks 0 and 3 (see _[4.7.1. The free space at kernel main bank](#471-the-free-space-at-kernel-main-bank)_ for more details).

The procedure for creating the complete Nextor kernel ROM file consists basically of appending the driver code to the kernel base file, and then patching the resulting file with the appropriate bank switching code. This can be done manually, or by using the `mknexrom` utility. Both options are explained below.

### 3.1. Manual creation

In order to manually create a complete Nextor ROM file, the following recipe must be followed. The file positions mentioned are zero based.

1.  Create a copy of the kernel base file, `Nextor-3.x.x-beta1.base.dat` or any of its variants (e.g. `Nextor-3.x.x-beta1.base.CTRL_INV.dat`).

2.  Append the page 0 code at the end of the file. This code can be simply copied from the first 255 bytes of the kernel base file itself.

3.  Append one byte with value K (the kernel base file bank count) at the end of the resulting file (this will be the bank ID of the driver bank). The value of K can be read from position 254 of the kernel base file itself.

4.  Append the driver file (which must be exactly 16080 bytes long, otherwise padding is required) at the end of the file obtained in step 3.

5.  Append the bank switching code at the end of the resulting file. If a file suitable for hardware supporting the ASCII16 mapper is desired, then this code can be simply copied from the last 48 bytes of the kernel base file itself. Otherwise, custom mapping code must be provided.

6.  If the driver code does not fit in one single bank, repeat steps 2-5 to append extra banks, increasing the bank ID for each bank as appropriate.

7.  If necessary, patch the resulting file to add custom code or data at the 256 byte free space on banks 0 and 3. Put the contents of the file (up to 256 bytes long) twice, at positions 3ED0h and FED0h in the file. Make sure that the area is empty (all zeros) in the kernel base file first: if it isn't, the kernel code has grown into it and the base file isn't compatible with this version of the guide.

8.  If the mapper type of the target hardware is not ASCII16, patch the bank switching code of the kernel common code banks (the last 48 bytes of the first "K" 16K blocks of the resulting file, where "K" is the value obtained in step 3) with custom bank switching code.

9.  If the mapper type of the target hardware is not ASCII16, put the same custom bank switching code used in steps 5 and 8 in the file position 2012.

10.  **Only** if the ROM mapper uses 8K banks: 

  a. Write a `LD(xxxxh),A` instruction at position 00F7h of the generated file, where xxxx is the memory mapped port that selects the 8K bank visible in the first half of page 1 (4000h-5FFFh). This is a 32h byte followed by xxxxh itself in little-endian format.

  b. Repeat the previous step for all the 16K portions of the file. That is, you must write the `LD(xxxxh),A` instruction at file positions (4000h*n)+F7h, where n goes from zero to the number of 16K banks in the file minus one.

The result of this procedure is a ready to use complete Nextor ROM file with your device driver properly embedded. There is no need to further patch or otherwise modify the generated ROM file.

### 3.2. Using the mknexrom utility

Instead of manually performing all the steps needed to build a complete Nextor kernel ROM, it is usually more convenient to use the supplied `mknexrom` utility. This tool can be used to create a new Nextor kernel ROM file, but it also allows modifying an existing file by changing the mapper code and/or adding extra content in the free 256 byte areas present at the end of banks 0 and 3.

`mknexrom` is supplied as a command-line executable file for Linux only, but the source code in standard C is provided as well, so it should be easy to port it to other platforms. The tool is also included in [the Nextor development Docker image](../docker/README.md).

The `mknexrom` tool usage syntax is as follows:

```
MKNEXROM <basefile> <newfile> [/d:<driverfile>] [/m:<mapperfile>]
         [/e:<extrafile>] [/8:<8K bank selection port address>]
```

_`<basefile>`_ can be one of the following:

* The Nextor kernel base file, that is, the file that contains the kernel common code only.

* A complete Nextor kernel ROM file with the driver bank(s) already appended.

_`<driverfile>`_ is the file containing the driver code. It must be a valid driver according to the rules and structure explained in section 4. The contents of this file are expected to be as follows:

1.  256 dummy bytes.
2.  The driver signature
3.  The driver jump table
4.  The driver code itself

And optionally, if the driver spans across more than one 16K bank, for each additional 16K block:

5. 256 dummy bytes.
6. The additional driver code or data.
7. Dummy space up to 16K (not needed for the last bank).

Specifying a driver file is mandatory if a kernel base file without driver is specified in _`<basefile>`_, and prohibited if a complete kernel ROM file is specified.

_`<mapperfile>`_ is the file containing the bank switching code. If no mapper file is specified, the mapper code from the base file itself is appended to the driver code.

_`<extrafile>`_ is the file containing the extra code or data for the resulting ROM file. This extra data can be up to 256 bytes long and will be placed at position 0x3ED0 of banks 0 and 3; this means that this code or data will be visible to applications via standard inter-slot calls (such as RDSLT or CALSLT) to the kernel slot, at address 0x7ED0. `mknexrom` refuses the file if the area is not empty in the kernel base file (this check is skipped when an existing full ROM file is being updated). See _[4.7.1. The free space at kernel main bank](#471-the-free-space-at-kernel-main-bank)_ for more details.

`/8` must be used only if the ROM mapper uses 8K banks. _`<8K bank selection port address>`_ is the memory mapped port address that selects the 8K bank visible in the first half of page 1 (4000h-5FFFh); for example 6000h for the ASCII8 mapper. This will appropriately patch the generated ROM boot code to support this kind of mapper.

As an alternative to using the `/8` parameter when using ROM mappers with 8K banks, `mknexrom` can be instructed to appropriately patch the generated ROM by adding a header to the mapper file itself. This header consists of a FFh byte followed by the bank selection port address in little-endian format. See below for an example.


### 3.3. Rules for the bank switching code

If the mapper type of the target hardware where the resulting ROM will be burned is not ASCII16, then a file containing compiled custom mapping code must be supplied. This code must follow these rules:

1.  It must be at most 48 bytes long.
2.  It must switch in page 1 the 16K ROM bank whose number is passed in register A (banks are numbered starting at zero). The ROM slot is assumed to be already switched on page 1.
3.  It can corrupt register pair AF only. All other registers must be preserved.
4.  It must be prepared to run at any address (so it can't contain absolute jumps or references to itself).

For illustration purposes, this is the source code of valid bank switching code for the ASCII8 mapper:

```
rlca
ld (6000h),a
inc a
ld (6800h),a
ret
```

If a file with the previous code is passed to `mknexrom` as the mapper file to be used, it is necessary to add a `/8:6000` parameter to the command line so that the generated ROM file includes the appropriate patch for 8K bank based ROM mappers. The same file with a header that renders the `/8` parameter unnecessary would be as follows:

```
db 0FFh
dw 6000h
rlca
ld (6000h),a
inc a
ld (6800h),a
ret
```

## 4. Nextor driver structure

This section contains all the details needed in order to develop a device driver for Nextor. The necessary elements, their locations, and the required routine input and output parameters are explained.

Note that [the source code of a dummy driver](../sdk/templates/driver/driver.asm) is supplied as part of [the Nextor development SDK](../sdk/README.md). You can use that file together with the supplied supporting files (the makefile and the bank switching code file) as the skeleton for developing your own driver.

### 4.1. One single driver model

Nextor 2 allowed two styles of drivers: "drive-based" and "device-based", each having a different structure. In Nextor 3 there's only one possible structure, which is equivalent to what was earlier called "device-based".

Another important difference is that a Nextor 3 driver can tell apart devices that don't exist (reported as `RESULT_INVALID_DEVICE`) from devices that exist but are not available at the moment, for example a card slot with no card inserted (see _[4.6.3. Device query 3: Get device status](#463-device-query-3-get-device-status)_ and _[4.6.4. Device query 4: Get device availability](#464-device-query-4-get-device-availability)_). Nextor 2 drivers couldn't express this difference, and the `DRV_CONFIG` routine existed as a workaround: drivers explicitly announced how many drives they wanted assigned at boot time. In Nextor 3 mapping drives is exclusively the kernel's business, so that routine is gone without a replacement.

### 4.2. Page 0 routines and data

This section explains the routines and data that are available at page 0 (addresses 4000h-40FFh) of all the Nextor banks, including the driver bank(s). These routines may be useful helpers for the driver code. They are directly available only for drivers embedded in ROM; drivers loaded in RAM can still invoke them by performing an inter-slot call (with the BIOS routine `CALSLT`) to the Nextor kernel slot, but since `CALSLT` itself uses IX and IY (to hold the address of the routine to call and the target slot), input values can't be passed to the called routine in these registers. The _[4.2.7. CALLB0_IX_IY (404Bh)](#427-callb0_ix_iy-404bh)_ routine exists precisely to work around this limitation; see how [the example RAM driver](../source/drivers/ram-driver-example.asm) uses it to invoke the `CALBAS` routine in BIOS.

Remember that as explained in _[3. Creating a Nextor kernel ROM with embedded driver](#3-creating-a-nextor-kernel-rom-with-embedded-driver)_, the page 0 code becomes part of all the driver banks when the complete Nextor kernel ROM is generated.

For drivers loaded in RAM the contents of this area are undefined as far as the kernel is concerned, but it can be used to pass initialization data when the driver is installed. See [the `_DRVRO` function call](Nextor_3.0_Programmers_Reference.md#315-driver-operations-_drvro-7fh) and _[4.5.6. Driver query 6: Initialize RAM driver](#456-driver-query-6-initialize-ram-driver)_ for details.

#### 4.2.1. GSLOT1 (402Dh)

Obtains in register A the slot currently switched on page 1 (that is, the slot of the current driver code). Preserves all other registers except F.

Note: This routine can't be called directly. It must be called via an inter-bank call to bank 0, in this way:

```
    xor a
    ld ix,GSLOT1
    call CALBNK
```

#### 4.2.2. RDBANK (403Ch)

This routine reads a byte from another bank. It must be called via an inter-bank call to the bank to be read, passing the address to be read in HL:

```
    ld a,<bank number>
    ld hl,<byte address> (must be a page 1 address)
    ld ix,RDBANK
    call CALBNK
```

It returns the read byte in A and preserves all other registers except F.

#### 4.2.3. CALLB0 (403Fh)

This routine temporarily switches the kernel main bank (usually bank 0, but will be 3 when running in MSX-DOS 1 mode), then invokes the routine whose address is at (BK4_ADD). It is necessary to use this routine to invoke CALBAS (so that kernel bank is correct in case of BASIC error) and to invoke DOS functions via F37Dh hook.

```
Input:  Address of code to invoke in (BK4_ADD).
        AF, BC, DE, HL, IX, IY passed to the called routine.
Output: AF, BC, DE, HL, IX, IY returned from the called routine.
```

Note: the address of `BK4_ADD` (called `CODE_ADD` in Nextor 2) is F1D0h.

See also: _[4.2.7. CALLB0_IX_IY (404Bh)](#427-callb0_ix_iy-404bh)_

#### 4.2.4. CALBNK (4042h)

Calls a routine in another bank. This is useful if the driver spans across two or more banks, and is needed for using the `GSLOT1`, `GWORK` and `RDBANK` routines.

```
Input:   A  = Bank number
         IX = Routine address (must be a page 1 address)
         AF'= Input parameter for the called routine
              (will be passed as AF to the called routine)
         BC, DE, HL, IY = Input parameters for the called routine
Output:  AF, BC, DE, HL, IX, IY = Output parameters from the called routine
```

#### 4.2.5. GWORK (4045h)

Gets the address of the 8 byte SLTWRK entry for the passed slot, or for the current slot in page 1. The first two bytes of this area will contain a pointer to the allocated page 3 work area for this driver (as requested in _[4.5.3. Driver query 3: Get driver initialization parameters](#453-driver-query-3-get-driver-initialization-parameters)_), or zero if no work area has been allocated.

```
Input:    A  =  Slot number
                (0, for the current slot in page 1)
Output:   A  = Current slot switched on page 1 (if 0 at input)
               Unchanged (if not 0 at input)
          IX =  Address of the 8 byte SLTWRK entry for the specified slot
Corrupts: F
```

Please see _[4.5.3. Driver query 3: Get driver initialization parameters](#453-driver-query-3-get-driver-initialization-parameters)_ for an explanation about how to use this routine and the SLTWRK area.

Note: This routine can't be called directly. It must be called via an inter-bank call to bank 0, in this way:

```
    ld a,<slot number or 0>
    ex af,af'
    xor a
    ld ix,GWORK
    call CALBNK
```

#### 4.2.6. CALDRV (4048h)

This routine calls a routine in the driver bank (bank 7 for drivers embedded in ROM, that is, the value of [`K_SIZE`](#428-k_size-40feh)). It works like [`CALBNK`](#424-calbnk-4042h), except that the bank number is fixed and the address of the routine to call is taken from (BK4_ADD) instead of IX.

```
Input:  Address of the routine to call in (BK4_ADD).
        AF, BC, DE, HL, IY passed to the called routine
        (IX can't be passed: it's used internally to hold
        the address of the routine to call).
Output: AF, BC, DE, HL, IY returned from the called routine.
```

Note: the address of `BK4_ADD` is F1D0h.

This routine is useful for code placed in [the free space at kernel main bank](#471-the-free-space-at-kernel-main-bank) that needs to call code in the driver bank.

#### 4.2.7. CALLB0_IX_IY (404Bh)

This routine does the same as [`CALLB0`](#423-callb0-403fh), but it reads the contents of registers IX and IY from `TMP_IX` (F1D2h) and `TMP_IY` (F1D4h) before invoking the routine at (BK4_ADD). Drivers loaded in RAM need to use this routine instead of `CALLB0` when invoking routines in the kernel ROM that make use of IX or IY; see for example how [the example RAM driver](../source/drivers/ram-driver-example.asm) uses it to invoke the `CALBAS` routine in BIOS.


#### 4.2.8. K_SIZE (40FEh)

This address contains one byte that tells how many banks form the Nextor kernel (or alternatively, the first bank number of the driver).

When a driver spans across more than one bank and needs to read data or call a routine in another driver bank (by using `RDBANK` and `CALBNK`), it should calculate the bank number by adding the appropriate offset to `K_SIZE` (or alternatively, to the value of `CUR_BANK`) instead of assuming a fixed bank number. When done this way, compiled drivers can still be used with future versions of the Nextor kernel even if they have more banks for the kernel common code.

#### 4.2.9. CUR_BANK (40FFh)

This address contains one byte with the current bank number. For the first driver bank this value is the same of `K_SIZE`, and it increases by one for each additional driver bank (if any).

#### 4.2.10. CHGBNK (7FD0h)

This is not strictly a page 0 routine, but is available on all banks as well. It will simply make the specified bank visible on Z80 page 1. Usually, driver code will not need to use this routine, but will use `CALBNK` instead.

```
Input:     A = Bank number
Output:    -
Corrupts:  AF
```

### 4.3. The driver header

The real driver content (after the first 256 bytes provided by the kernel itself) starts at address 4100h and consists of a header that has two parts:

1. A fixed driver signature: the verbatim ASCII string `NEXTORv3_DRIVER`, uppercased, and zero-terminated.
2. A jump table for a set of routines to be implemented by the driver.

This is the complete code for the driver header. The label names referenced in the jump table are for the routines to be implemented by the driver, which are detailed in the next section:

```
    org 4100h

    db "NEXTORv3_DRIVER",0

    jp TIMER_INT
    jp OEMSTAT
    jp BASDEV
    jp EXTBIO
    jp DRIVER_QUERY
    jp DEVICE_QUERY
    jp CUSTOM_DRIVER_QUERY
    jp CUSTOM_DEVICE_QUERY
    jp READ_WRITE
    jp RESERVED_0
    jp RESERVED_1
    jp RESERVED_2
    jp DIRECT_0
    jp DIRECT_1
    jp DIRECT_2
    jp DIRECT_3
    jp DIRECT_4
```

### 4.4. Driver routines

This section describes the routines that a driver must implement. The routine name presented is the label jumped to in the jump table above, and the provided address is the one of the corresponding entry in the jump table. How the routines are actually arranged in the driver memory space (in ROM or RAM) is up to the driver developer, as long as they are past the driver header.

Some of these routines return error codes that are referred to by name. For the corresponding numeric values see [the driver result codes file in the SDK](../sdk/asm/constants/driver_result_codes.inc).

None of these routines need to preserve any of the registers not used to return data.

#### 4.4.1. TIMER_INT (4110h)

This is the entry point for the timer interrupt routine of the driver, it will be called 50 or 60 times per second depending on the VDP frequency selected. If the driver does not need to handle the timer interrupt, it should fill this entry with `RET` instructions.

Note that this entry will only be called if _[4.5.3. Driver query 3: Get driver initialization parameters](#453-driver-query-3-get-driver-initialization-parameters)_ (for drivers in ROM) or _[4.5.6. Driver query 6: Initialize RAM driver](#456-driver-query-6-initialize-ram-driver)_ (for drivers in RAM) flags that the driver should be hooked to the timer interrupt.

#### 4.4.2. OEMSTAT (4113h)

This is the entry for the BASIC extended statements ("CALLs") handler. It works the same way as the standard handlers (see [MSX2 Technical Handbook, chapter 2, "Expansion of CMD command"](https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Chapter2.md), and [MSX2 Technical Handbook, chapter 5, "Developing Cartridge Software"](https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Chapter5.md) for details), except that if the handled statements have parameters, the MSX BIOS routine `CALBAS` (needed to invoke the MSX BASIC interpreter helper routines) can't be used directly; instead, it must be invoked via [the CALLB0 routine](#423-callb0-403fh) in kernel page 0:

```
CALBAS: equ 0159h
BK4_ADD: equ 0F1D0h
CALLB0: equ 403Fh

    ld ix,<address of the BASIC routine to execute>
    ld hl,CALBAS
    ld (BK4_ADD),hl
    call CALLB0
```

For drivers loaded in RAM the process is a bit more convoluted and [the CALLB0_IX_IY routine](#427-callb0_ix_iy-404bh) must be used instead. See [the code for the example RAM driver](../source/drivers/ram-driver-example.asm) for a working example.

If the driver does not handle BASIC extended statements, this routine must simply set the carry flag and return.

#### 4.4.3. BASDEV (4116h)

This is the entry for the BASIC devices (`OPEN "xyz:"`) handler. If the BASIC interpreter helper routines are needed, the same restrictions explained for `OEMSTAT` apply here.

#### 4.4.4. EXTBIO (4119h)

This is the extended BIOS handler. It works the same way as the standard handlers, except that it must return a value in IYl (the lower byte of the IY register) that tells the kernel what to do after the driver handler has finished:

```
IYl=0: Return immediately.
IYl=1: Execute the kernel and/or the system extended BIOS handler.
```

This routine will only be invoked if _[4.5.3. Driver query 3: Get driver initialization parameters](#453-driver-query-3-get-driver-initialization-parameters)_ (for drivers in ROM) or _[4.5.6. Driver query 6: Initialize RAM driver](#456-driver-query-6-initialize-ram-driver)_ (for drivers in RAM) flags that the driver handles extended BIOS calls.

#### 4.4.5. DRIVER_QUERY (411Ch)

This routine groups a series of queries (routines that don't have a dedicated entry in the main jump table) that target the driver itself. The signature of this routine is as follows:

```
Input:  A = Query index
        F, BC, DE, HL = Depends on the query
Output: A = Error code:
            RESULT_OK: success
            RESULT_NOT_IMPLEMENTED: query not implemented
            Others: depends on the query
        F, BC, DE, HL = Depends on the query
```

Each query is described separately in _[4.5. Driver queries](#45-driver-queries)_.

#### 4.4.6. DEVICE_QUERY (411Fh)

This routine groups a series of queries (routines that don't have a dedicated entry in the main jump table) that target a specific device controlled by the driver. The signature of this routine is as follows:

```
Input:  A = Query index
        C = Device number
        F, B, DE, HL = Depends on the query
Output: A = Error code:
            RESULT_OK: success
            RESULT_INVALID_DEVICE: invalid device number
            RESULT_NOT_IMPLEMENTED: query not implemented
            Others: depends on the query
        F, BC, DE, HL = Depends on the query
```

Each query is described separately in _[4.6. Device queries](#46-device-queries)_.

Note that this routine should first check if the supplied device number is valid, and if not, return `RESULT_INVALID_DEVICE`; only after the device number has been verified should the query index be verified and `RESULT_NOT_IMPLEMENTED` be returned for unsupported queries. A query index not described in this specification should always be treated as an unsupported query.

#### 4.4.7. CUSTOM_DRIVER_QUERY (4122h)

This is an extensibility point that allows driver developers to offer additional, non-standard functionality related to the driver itself in a clean way. This routine works the same way as [`DRIVER_QUERY`](#445-driver_query-411ch) (same signature), the difference being that it's the driver developer who decides what the available queries are and what are their input and output parameters. 

If the driver doesn't offer any custom driver query then it should just set A to `RESULT_NOT_IMPLEMENTED` and return.

#### 4.4.8. CUSTOM_DEVICE_QUERY (4125h)

This is an extensibility point that allows driver developers to offer additional, non-standard functionality related to a specific device in a clean way. This routine works the same way as [`DEVICE_QUERY`](#446-device_query-411fh) (same signature), the difference being that it's the driver developer who decides what the available queries are and what are their input and output parameters.

If the driver doesn't offer any custom device query then it should just set A to `RESULT_NOT_IMPLEMENTED` and return. Otherwise, it should proceed as `DEVICE_QUERY` does: first check the device number, and then the query index.

#### 4.4.9. READ_WRITE (4128h)

Reads or writes absolute sectors from/to a device. This is the only device query that doesn't go through the `DEVICE_QUERY` routine. The signature of this routine is as follows:

```
Input:  Cy  =  0 to read sectors
               1 to write sectors
        A  = Device number, 1 to 255
        B  = Number of sectors to read or write
        C  = Media descriptor byte if the device is a floppy
             disk drive, zero otherwise
        HL = Source or destination non-page 1 memory address for the transfer
        DE = Non-page 1 memory address where the 4 byte sector number is stored
Output:  A = Error code:
             0: Ok
             .IDEVN: Invalid device number
             .NRDY: Not ready
             .DISK: General unknown disk error
             .DATA: CRC error when reading
             .RNF: Sector not found
             .UFORM: Unformatted disk
             .WPROT: Write protected media, or read-only device
             .WRERR: Write error
             .NCOMP: Incompatible disk
             .SEEK: Seek error
         B = Number of sectors actually read or written
             (the kernel only uses this value when an error is
             returned, but the driver should always return an
             accurate value)
```

Note that what this routine must access is the raw physical device sectors, not partition sectors. The driver does not need to know anything about device partitioning.

The number of the first sector to read or write is a 32 bit number which is supplied in a memory area whose address is passed in DE. This address will never be on page 1, therefore the driver does not need to worry about paging and can access this data directly. The same applies to the sectors data source or destination address.

The available sector numbers must range from zero to the number of available sectors (as reported by _[4.6.2. Device query 2: Get device parameters](#462-device-query-2-get-device-parameters)_) minus one. If zero available sectors are reported, then the range of available sectors is undefined unless the driver developer explicitly documents it.

This routine must work for all block devices. If a non-block device supports reading and/or writing sectors, this routine may optionally work with that device as well.

The `.IDEVN` error must be returned only for device numbers that don't exist in the driver. For a device that exists but is currently unavailable (for example a removable device with no medium inserted, or an empty card slot) the routine must return `.NRDY` instead; otherwise, accessing a drive mapped to an offline device will report the wrong error. This distinction didn't exist in Nextor 2, so it deserves special attention when porting old driver code (see [the driver migration guide](Nextor_3.0_Driver_Migration_Guide.md)).

If the device is a floppy disk drive (as reported by the driver via _[4.6.2. Device query 2: Get device parameters](#462-device-query-2-get-device-parameters)_) then the routine should use the media descriptor byte passed in C in order to determine the correct disk geometry. This byte is obtained from the disk's boot sector itself, so before it's available this routine will be called with C=0; the driver should assume a sensible default disk geometry in this case. For any other kind of device the value passed in C will be zero and should be ignored.

The error codes returned are the same used by the Nextor function calls, see [the list in the Programmers Reference](Nextor_3.0_Programmers_Reference.md#4-new-error-codes) and [the DOS errors SDK file](../sdk/asm/constants/dos_errors.inc).

#### 4.4.10. RESERVED_0/1/2 (412Bh/412Eh/4131h)

These three entries are reserved for future expansion and drivers should simply implement these as `RET` instructions. In fact, it's not even necessary to provide proper jump instructions for these entries: a simple `ds 3*3,0C9h` is enough.

#### 4.4.11. DIRECT_0...4 (4134h...4140h)

These are the entries for direct calls to the driver. Calls to any of the five entry points available at addresses 7850h to 785Ch in the kernel ROM (bank 0 or bank 3) will be mapped to a call to the corresponding DIRECT_n entry point as follows:

| Main bank address | Driver address |
|-------------------|----------------|
| 7850h             | 4134h          |
| 7853h             | 4137h          |
| 7856h             | 413Ah          |
| 7859h             | 413Dh          |
| 785Ch             | 4140h          |

The preferred extensibility mechanism for drivers is the [`CUSTOM_DRIVER_QUERY`](#447-custom_driver_query-4122h) routine. These entries are provided for cases when the driver needs to expose entry points in the main bank, typically routines that are the targets of hooks other than the timer interrupt and the extended BIOS (which can then be set as inter-slot calls).

When these routines are entered, paging state will be the same as when the bank 0/3 entry was invoked, except of course that the driver bank will be switched on page 1 instead of the kernel bank. All registers except IX and AF' are passed unmodified from the caller.

If the driver does not implement any direct call code, it can simply fill these entry points with `RET` instructions, i.e. `ds 5*3,0C9h`.

These entries are only really useful for ROM drivers. Drivers loaded in RAM should always use `CUSTOM_DRIVER_QUERY` to implement custom extensibility; in fact, drivers loaded in RAM may omit these entries (together with the `RESERVED_0/1/2` entries) from the jump table entirely, as [the example RAM driver](../source/drivers/ram-driver-example.asm) does.


### 4.5. Driver queries

This section explains the queries defined for the [`DRIVER_QUERY`](#445-driver_query-411ch) routine. All of them are optional: a driver can return `RESULT_NOT_IMPLEMENTED` for any of them and then the kernel will use a sensible default (documented for each query). Other callers invoking these routines should assume the same defaults when the query is not implemented.

#### 4.5.1. Driver query 1: Get driver version number

```
Input:  A = 1
Output: A = RESULT_OK or RESULT_NOT_IMPLEMENTED
        Version in B.C.D (if A=RESULT_OK)
```

Returning `RESULT_NOT_IMPLEMENTED` is equivalent to returning `RESULT_OK` and version 1.0.0.

#### 4.5.2. Driver query 2: Get driver information string

```
Input: A  = 2
       B  = String index:
             1: Driver name
             2: Driver author name
             3: Hardware name
             4: Hardware author name
             5: Serial number
       D  = Buffer size
       HL = Buffer address
Output: A = RESULT_OK: ok, full string provided
            RESULT_TRUNCATED_STRING: string was truncated due to insufficient buffer size
            RESULT_NOT_IMPLEMENTED: the requested string is not available
```

This query allows the driver to provide some textual information about itself. If the driver doesn't provide the requested string, or if a string index not documented above is requested, `RESULT_NOT_IMPLEMENTED` should be returned; the caller must then assume that the string is not available and if needed, use a placeholder like "(unknown)" or similar instead. It's recommended to at least provide a driver name.

The returned string must be in ASCII and zero-terminated. The routine must return at most D bytes, this includes the terminating zero so actually D-1 characters will be returned. If the buffer is too small for the full string, `RESULT_TRUNCATED_STRING` must be returned. If D=0 is passed, nothing is copied to the buffer and `RESULT_TRUNCATED_STRING` is returned (callers can use this to check if a given string exists without actually retrieving it).

Driver developers can use [the `OUTPUT_STRING` routine from the SDK](../sdk/asm/code/output_string.asm) to easily implement this query.

#### 4.5.3. Driver query 3: Get driver initialization parameters

```
Input:  A  = 3
        HL = Amount of work area available to allocate
        B  = Number of available drives in the system
        C  = Flags:
             5: set if user is requesting reduced drive count (by pressing the 5 key)
             Others: 0
        DE = Address of a routine for printing a character	 
Output: A  = RESULT_OK, RESULT_INIT_ERROR or RESULT_NOT_IMPLEMENTED
        B  = Flags
             0: TIMER_INT should be hooked
             1: EXTBIO should be hooked
             2-7: Must be zero
        HL = Space required in page 3
```

This query is intended **only** for drivers in ROM. Drivers loaded in RAM must do nothing and return `RESULT_NOT_IMPLEMENTED` if they receive this query.

The kernel will invoke this query at boot time, giving the driver an opportunity to provide information about its own memory and hooking requirements. If this routine returns `RESULT_OK` or `RESULT_NOT_IMPLEMENTED`, the kernel will later invoke _[4.5.4. Driver query 4: Initialize driver](#454-driver-query-4-initialize-driver)_; if something would prevent the driver from functioning normally then it should return `RESULT_INIT_ERROR` so that the kernel skips the driver initialization.

Returning `RESULT_NOT_IMPLEMENTED` is equivalent to returning `RESULT_OK` plus B=0 and HL=0.

The "User is requesting reduced drive count" flag will be set if the user wants one single drive to be allocated per driver.
This happens when the user keeps the 5 key pressed at boot time, when the one-time boot keys mechanism is used (for example via the `NEXBOOT.COM` tool), or when the 5 key is marked as active in [the boot menu](Nextor_3.0_User_Manual.md#210-boot-keys-and-the-boot-menu). The same flag is passed to _[4.5.4. Driver query 4: Initialize driver](#454-driver-query-4-initialize-driver)_ as well.

After this query returns the driver is free to use [`GWORK`](#425-gwork-4045h) at any time to obtain the address of the space reserved for the current slot at SLTWRK. The driver should act as follows regarding the page 3 work area:

* If 8 bytes or less are required, this routine should return HL=0 on its first execution, and the 8 byte space reserved by the system for this slot at SLTWRK should be used as work area:

```
    xor a
    ex af,af'
    xor a
    ld ix,GWORK
    call CALBNK
    ;Use the 8 byte space pointed by IX as work area
```

* If more than 8 bytes are required, this routine should return the required space in HL, and should obtain the pointer to the allocated space from the first two bytes of the space reserved by the system for this slot at `SLTWRK`:

```
    xor a
    ex af,af'
    xor a
    ld ix,GWORK
    call CALBNK
    ld l,(ix)
    ld h,(ix+1)
    ;Use the space pointed by HL as work area
```

Please note that if this query requests more page 3 work area than is available, Nextor will skip this driver and won't further interact with it.

This query **must not** initialize the screen or print any text directly. If the driver wants to show an initialization text (although it's recommended to do that in the "Initialize driver" query, and only show an error message here if `RESULT_INIT_ERROR` is returned), it must use the callback provided in register DE, as explained in _[4.5.4. Driver query 4: Initialize driver](#454-driver-query-4-initialize-driver)_.

#### 4.5.4. Driver query 4: Initialize driver

```
Input:  A  = 4
        HL = Size of work area allocated for the driver in page 3
        C  = Flags:
             5: set if user is requesting reduced drive count (by pressing the 5 key)
             Others: 0
        DE = Address of a routine for printing a character
Output: A  = RESULT_OK, RESULT_INIT_ERROR or RESULT_NOT_IMPLEMENTED
```

This query is intended **only** for drivers in ROM. Drivers loaded in RAM must do nothing and return `RESULT_NOT_IMPLEMENTED` if they receive this query.

The kernel will invoke this query at boot time after invoking _[4.5.3. Driver query 3: Get driver initialization parameters](#453-driver-query-3-get-driver-initialization-parameters)_ and as long as that one didn't return an error. If something would prevent the driver from functioning normally then it should return `RESULT_INIT_ERROR` so that the kernel skips the driver registration. Returning `RESULT_NOT_IMPLEMENTED` is equivalent to returning `RESULT_OK`.

This query **must not** initialize the screen or print any text directly. If the driver wants to show an initialization text, it must use the callback provided in register DE. This callback will have the same semantics as the BIOS routine `CHPUT`: it will print the character passed in A and can modify AF only. Here's an example of how to display a message using that callback:

```
    push de      ;Print character callback address
    ld de,0C300h ;JP instruction
    push de
    ld ix,1
    add ix,sp    ;IX points now to "JP callback" in the stack
    
    ld hl,INIT_MSG ;Pointer to zero-terminated initialization message
    call PRINT_HL_USING_IX
    
    pop de ;Restore stack before returning
    pop de
    xor a  ;RESULT_OK
    ret

PRINT_HL_USING_IX:
    ld a,(hl)
    or a
    ret z
    call JPIX
    inc hl
    jr 	PRINT_HL_USING_IX

JPIX: jp (ix)
```

#### 4.5.5. Driver query 5: Get maximum supported device number

```
Input:  A = 5
Output: A = RESULT_OK or RESULT_NOT_IMPLEMENTED
        B = Maximum supported device number
```

Drivers can use this routine to inform the kernel about the highest device number they support. This query exists purely as a performance improvement: there are times (for example, when automatically mapping drives to devices/partitions at boot time) when the kernel scans the devices of a driver by asking for information about every possible device number starting with 1; the value returned by this query caps that scan, which otherwise would have to go through all the possible device numbers up to 255.

Note that the returned value is just an upper bound for the scan, not a device count: it isn't required that every device number up to the maximum corresponds to an existing device. For example, a driver could report a maximum device number of 10 while only devices 8, 9 and 10 actually exist; not recommended, but perfectly legal (the driver must return `RESULT_INVALID_DEVICE` for the device numbers that don't exist, as usual). This query has no effect on how drives are mapped to the devices at boot time.

There is one real limit on device numbers, though: the automatic partition search (the procedure that maps drives to partitions at boot time, and again on the first access to a drive that is attached to a device with no partition assigned) only supports device numbers 1 to 63 (internally, the two high bits of the device number are used as temporary flags during the search). Devices with higher numbers work normally in every other way, including having drives explicitly mapped to their partitions, but they can't take part in the automatic search; therefore drivers are advised to simply number their devices sequentially starting at 1.

Returning `RESULT_NOT_IMPLEMENTED` is equivalent to returning `RESULT_OK` and B=4.

#### 4.5.6. Driver query 6: Initialize RAM driver

```
Input:  A  = 6
        B  = RAM slot number where the driver is loaded
        C  = RAM segment number where the driver is loaded
        DE = Address of a routine for printing a character
Output: A  = RESULT_OK, RESULT_INIT_ERROR or RESULT_NOT_IMPLEMENTED
        B  = Flags
             0: TIMER_INT should be hooked
             1: EXTBIO should be hooked
             2-7: Must be zero
```

This query is intended **only** for drivers loaded in RAM. Drivers in ROM must do nothing and return `RESULT_NOT_IMPLEMENTED` if they receive this query.

This query is the equivalent of _[4.5.4. Driver query 4: Initialize driver](#454-driver-query-4-initialize-driver)_ for drivers loaded in RAM. These drivers are initialized after the system is fully operational and thus there's no way to request page 3 work area, therefore there's no previous "get initialization parameters" step. The returned flags have the same meaning.

Returning `RESULT_NOT_IMPLEMENTED` is equivalent to returning `RESULT_OK` plus B=0. Returning `RESULT_INIT_ERROR` will cause the kernel to skip the registration of this driver. 

Drivers are loaded in RAM and initialized typically using either [the `CALL IDRIVER` command](Nextor_3.0_User_Manual.md#3612-the-call-idriver-command) or [the `DRVROP.COM` tool](Nextor_3.0_User_Manual.md#3413-drvrop-the-driver-operations-tool), but custom loaders could be used too. When using these standard tools the first 256 bytes of the corresponding RAM segment (addresses 4000h-40FFh) may contain user-provided initialization data for the driver; by convention the byte at address 4000h holds the data length and the data itself starts at address 4001h. It's the responsibility of the driver developer to document which initialization data is supported or required by the driver, if any; if initialization data is required but not supplied, the driver should return `RESULT_INIT_ERROR`. See [the `_DRVRO` function call](Nextor_3.0_Programmers_Reference.md#315-driver-operations-_drvro-7fh) for details on the full process to load and initialize a driver in RAM, including the initialization data convention.

This query **must not** initialize the screen or print any text directly. If the driver wants to show an initialization text, it must use the callback provided in register DE, as explained in _[4.5.4. Driver query 4: Initialize driver](#454-driver-query-4-initialize-driver)_.

#### 4.5.7. Driver query 7: Shut down RAM driver

```
Input:  A  = 7
        DE = Address of a routine for printing a character
Output: A  = RESULT_OK or RESULT_NOT_IMPLEMENTED
```

This query is intended **only** for drivers loaded in RAM. Drivers in ROM must do nothing and return `RESULT_NOT_IMPLEMENTED` if they receive this query.

This query will be invoked when the user requests to uninstall a driver installed in RAM, typically by using [the `CALL UDRIVER` command](Nextor_3.0_User_Manual.md#3613-the-call-udriver-command) or [the `DRVROP.COM` tool](Nextor_3.0_User_Manual.md#3413-drvrop-the-driver-operations-tool), but a custom loader could be used too.

The kernel will invoke this routine as a "courtesy", but the driver can't return an error from this query and thus it can't abort the uninstall process: regardless of what this routine returns, the kernel will unregister the driver and then the tool doing the uninstall (if it follows the rules) will free the corresponding RAM segment. The driver should always return either `RESULT_OK` or `RESULT_NOT_IMPLEMENTED` (which are equivalent), though, for compatibility with possible changes to these mechanics in future versions of Nextor. See [the `_DRVRO` function call](Nextor_3.0_Programmers_Reference.md#315-driver-operations-_drvro-7fh) for details on the full uninstall process.

This query **must not** initialize the screen or print any text directly. If the driver wants to show any informative text, it must use the callback provided in register DE, as explained in _[4.5.4. Driver query 4: Initialize driver](#454-driver-query-4-initialize-driver)_.


### 4.6. Device queries

This section explains the queries defined for the [`DEVICE_QUERY`](#446-device_query-411fh) routine. All of them are optional: a driver can return `RESULT_NOT_IMPLEMENTED` for any of them and then the kernel will use a sensible default (documented for each query). Other callers invoking these routines should assume the same defaults when the query is not implemented. Note however that the driver must first check the device number supplied, and return `RESULT_INVALID_DEVICE` if the device doesn't exist. 

Note that a query for a device that exists but is not available for access (e.g. an SD card slot where no card is inserted) is **not** considered a non-existing device and thus no `RESULT_INVALID_DEVICE` should be returned for these; instead, either success or a dedicated error code should be returned, depending on the query.


#### 4.6.1. Device query 1: Get device information string

```
Input:  A  = 1
        C  = Device number
        B  = String index:
             1: Manufacturer name
             2: Medium name
             3: Serial number
             4: Device name
        D  = Buffer size
        HL = Buffer address
Output: A = RESULT_OK: ok, full string provided
            RESULT_TRUNCATED_STRING: string was truncated due to insufficient buffer size
            RESULT_INVALID_DEVICE: the device does not exist
            RESULT_NOT_IMPLEMENTED: the requested string is not available
```

This query allows the driver to provide some textual information about a device. It can be fixed information provided by the driver itself or information obtained from the device hardware itself. If the driver can't provide the requested string, or if a string index not documented above is requested, `RESULT_NOT_IMPLEMENTED` should be returned; the caller must then assume that the string is not available and if needed, use a placeholder like "(unknown)" or similar instead.

The returned string must be in ASCII and zero-terminated. The routine must return at most D bytes, this includes the terminating zero so actually D-1 characters will be returned. If the buffer is too small for the full string, `RESULT_TRUNCATED_STRING` must be returned. If D=0 is passed, and as long as the device and the string both actually exist, nothing is copied to the buffer and `RESULT_TRUNCATED_STRING` is returned (callers can use this to check if a given string exists without actually retrieving it).

"Device name" and "Medium name" differ in that the former is a "conceptual" name provided by the driver itself, while the latter is effectively retrieved from the device, when that's possible. For example, assume a driver that controls an SD card slot. Then the device name would always be the fixed string "SD card slot", and the medium name would be extracted from the inserted SD card (or if none is available, `RESULT_NOT_IMPLEMENTED` would be returned for the medium name query - but the device name query would still succeed in this case).

Driver developers can use [the `OUTPUT_STRING` routine from the SDK](../sdk/asm/code/output_string.asm) to easily implement this query, at least for fixed strings.


#### 4.6.2. Device query 2: Get device parameters

```
Input:  A  = 2
        C  = Device number
        HL = Buffer address, 0 for not returning information
             (only return error code)
Output: A =  RESULT_OK: ok, device information provided
             RESULT_INVALID_DEVICE: the device does not exist
             RESULT_NOT_IMPLEMENTED: query not implemented

On success, buffer filled with the following information:

+0 (1): Device type:
        0: Block device
        1: CD or DVD reader or recorder
        2-254: Unused. Additional codes may be defined in the future.
        255: Other
+1 (2): Sector size, 0 if this information does not apply or is
        not available.
+3 (4): Total number of available sectors.
        0 if this information does not apply or is not available.
+7 (1): Flags:
        bit 0: 1 if the device is removable.
        bit 1: 1 if the device is read only. A device that can dynamically
                 be write protected or write enabled is not considered
                 to be read-only.
        bit 2: 1 if the device is a floppy disk drive.
        bit 3: 1 if this device shouldn't be used for partition automapping.
        bits 4-7: must be zero.
+8 (2): Number of cylinders
+10 (1): Number of heads
+11 (1): Number of sectors per track
```

This query returns detailed invariant information about a given device. Returning `RESULT_NOT_IMPLEMENTED` is equivalent to returning `RESULT_OK` and filling the information buffer with a value of 512 for the sector size field and all zeros for the rest of the fields.

"Block devices" are all devices that can be read and written via access to logical sectors. This includes floppy disks, hard disks, pendrives, multimedia cards, etc. Block devices must be readable and optionally writable via [the `READ_WRITE` routine](#449-read_write-4128h).

In the current version Nextor will refuse to work with a device that is reported as a non-block device or having a sector size different from 512 bytes.

The information about cylinders, heads and sectors per track applies only to floppy disks and hard disks; for other device types, or when this information is not available for whatever reason, these fields should be returned with value zero. This information is not used by the Nextor kernel, but can be used by device partitioning tools in order to properly align partitions on the disk (in the current version of Nextor this information is not used by the built-in partitioning tool).

The "read only" flag should be set only for devices that are only readable by design (for example a CD-ROM). A device that can be dynamically write protected and write enabled should not be reported as a read-only device.

If the "floppy disk drive" flag is set Nextor will treat the device differently in some aspects, see ["Support for floppy disks" in the user manual](Nextor_3.0_User_Manual.md#25-support-for-floppy-disks). If a driver reports a device as being a floppy disk it should implement the _[4.6.5. Device query 5: Get format choices for a floppy disk device](#465-device-query-5-get-format-choices-for-a-floppy-disk-device)_ and _[4.6.6. Device query 6: Format a floppy disk device](#466-device-query-6-format-a-floppy-disk-device)_ queries too.

#### 4.6.3. Device query 3: Get device status

```
Input:  A  = 3
        C  = Device number
Output: A = RESULT_OK: ok, device status provided
            RESULT_INVALID_DEVICE: the device does not exist
            RESULT_NOT_IMPLEMENTED: query not implemented or device isn't removable
        B = Status for the specified device:
            0: The device exists but is not available at the moment
               (typically this means: removable device with no medium inserted)
            1: The device is available and has not
               changed since the last status request.
            2: The device is available and has changed
               since the last status request
            3: The device is available, but it is not
               possible to determine whether it has been changed
               or not since the last status request.
```

This query returns information about the current status of a given device. Returning `RESULT_NOT_IMPLEMENTED` is equivalent to returning `RESULT_OK` plus B=1.

This query is intended for removable devices. For fixed devices it should always return either `RESULT_NOT_IMPLEMENTED`, or `RESULT_OK` and B=1. For a removable device that exists but has no medium inserted `RESULT_OK` and B=0 should be returned; `RESULT_INVALID_DEVICE` **must not** be returned in this case.

The driver must keep track of calls to this query in order to accurately reflect the status compared to the last invocation. For example, after a removable medium has been changed the first invocation should return "device has changed" and subsequent invocations (assuming there are no further changes) should return "not changed". If the driver is unable to do that then it should either always return "it's not possible to determine change status", or not flag the device as removable.

#### 4.6.4. Device query 4: Get device availability

```
Input:  A  = 4
        C  = Device number
Output: A = RESULT_OK: ok, device availability provided
            RESULT_INVALID_DEVICE: the device does not exist
            RESULT_NOT_IMPLEMENTED: query not implemented or device isn't removable
        B = Status for the specified device:
            0: The device exists but is not available at the moment
               (typically this means: removable device with no medium inserted)
            1: The device is available
```

This routine is a simplified version of _[4.6.3. Device query 3: Get device status](#463-device-query-3-get-device-status)_. The difference is that it only returns either "not available" or "available", without any medium change information (and thus the driver doesn't need to keep track of calls to the routine). As in the "Get device status" query, returning `RESULT_NOT_IMPLEMENTED` is equivalent to returning `RESULT_OK` plus B=1, and one of these two should always be returned for fixed devices.

The invocation of this routine **must not** interfere with the status tracking for the "Get device status" query. For example, if a removable device is changed, and this query is invoked one or multiple times, the first invocation of "Get device status" after that must still report that the medium has changed.

#### 4.6.5. Device query 5: Get format choices for a floppy disk device

```
Input:  A  = 5
        C  = Device number
        DE = Buffer size (used if B=255 is returned)
        HL = Buffer address (used if B=255 is returned)
Output: A = RESULT_OK: ok, format information provided
            RESULT_INVALID_DEVICE: the device does not exist
            RESULT_NOT_IMPLEMENTED: not a floppy disk, 
                                    or formatting not supported
            RESULT_TRUNCATED_STRING: string was truncated due to the buffer size 
                                     being too short
        B = Choices:
            0: Only one format choice available
            1: Single side / double side, double density
            2: Single side / double side DD / double side HD
            255: Driver has written a custom null-terminated choice
                 string to the buffer at HL
```

This query is intended for floppy disk devices only. For any other device type (and for floppy disks if the driver doesn't support formatting) it should always return `RESULT_NOT_IMPLEMENTED`. If this query is implemented, _[4.6.6. Device query 6: Format a floppy disk device](#466-device-query-6-format-a-floppy-disk-device)_ must be implemented too.

If there's only one way of formatting the disk, B=0 should be returned. If the choices are _single side/double side_ or _single side/double side double density/double side high density_, it should return B=1 or B=2 respectively (note that this is true regardless of the actual form factor or capacity of the disk). These are the most common options for formatting floppy disks so these return values should cover the majority of cases. [The `CALL FORMAT` command](Nextor_3.0_User_Manual.md#363-the-call-format-command) and [the `_FORMAT` function call](Nextor_3.0_Programmers_Reference.md#27-_format-67h) will use stock strings hardcoded in the Nextor kernel in these cases.

If none of the built-in choice sets works for a given device, or if the driver wants to provide a custom choice string, the driver can copy a custom string (ASCII, zero-terminated) in the buffer provided in HL, constrained to the buffer length passed in DE (Note: currently the Nextor kernel will copy up to 512 bytes even if the reported buffer size is bigger and the choice string is longer). Driver developers can use [the `OUTPUT_STRING` routine from the SDK](../sdk/asm/code/output_string.asm) to easily copy custom choice strings to the supplied buffer address.

#### 4.6.6. Device query 6: Format a floppy disk device

```
Input:  A  = 6
        C  = Device number
        B  = Choice number (1-9, as chosen by user from choice string)
Output: A = RESULT_OK: ok, disk has been formatted
            RESULT_INVALID_DEVICE: the device does not exist
            RESULT_NOT_IMPLEMENTED: the device is not a floppy disk,
                                   formatting is not supported,
                                   or the choice number is invalid.
```

This query is intended for floppy disk devices only. For any other device type (and for floppy disks if the driver doesn't support formatting) it should always return `RESULT_NOT_IMPLEMENTED`. If this query is implemented, _[4.6.5. Device query 5: Get format choices for a floppy disk device](#465-device-query-5-get-format-choices-for-a-floppy-disk-device)_ must be implemented too.

The driver should format the floppy disk according to the selected choice. Choice numbers correspond to the format choices returned by "Get format choices for a floppy disk device"; if an unknown choice is supplied, `RESULT_NOT_IMPLEMENTED` should be returned.

After the physical formatting completes, an MSX-DOS 1 compatible set of disk parameters (boot sector, empty FAT and empty root directory) appropriate for the disk geometry must be written to the disk. The source code of [the MSX Turbo-R FDD driver](https://github.com/Konamiman/Turbo-R-FDD-Nextor-driver) contains these parameters for 3.5" single side and double side disks.

There's no way to report progress on the formatting process back to the caller so this query must simply perform the formatting in a blocking fashion until the process completes.

#### 4.6.7. Device query 7: Stop the motor of a floppy disk drive

```
Input:  A  = 7
        C  = Device number
Output: A = RESULT_OK: ok, motor has been stopped
            RESULT_INVALID_DEVICE: the device does not exist
            RESULT_NOT_IMPLEMENTED: the device is not a floppy disk
                                    or stopping the drive motor is not supported
```

This query is intended for floppy disk devices only. For any other device type (and for floppy disks if the driver doesn't support stopping the motor) it should always return `RESULT_NOT_IMPLEMENTED`.

This query is currently never invoked by the Nextor kernel, but this could change in future versions so drivers should implement it whenever possible.

### 4.7. Other

This section contains other useful information about the Nextor device driver structure.

#### 4.7.1. The free space at kernel main bank

The Nextor kernel has a 256 byte unused space at the end of the two main banks (bank 0 when running in normal mode, bank 3 when running in MSX-DOS 1 mode) that can be filled with any kind of data or code useful for the driver. The main bank is permanently switched on the Kernel slot in normal circumstances (other banks are switched only for temporary code calls), therefore this area can be accessed via the standard slot accessing mechanisms (such as inter-slot call via `CALSLT`, inter-slot read via `RDSLT`, etc) even by software that is not aware of the Nextor bank paging mechanism. This space is visible at addresses 7ED0h to 7FCFh, right before the bank switching code; the `DRIVER_EXTRA_AREA` and `DRIVER_EXTRA_AREA_SIZE` constants in `sdk/asm/constants/rom_bank_header.inc` define its location and size, so that driver code (for example, a hook that must point to a stub in this area) doesn't need to hardcode the address.

Note that in Nextor 2 this area was 1K long and started at 7BD0h; the first 768 bytes are now used by the kernel. A driver that placed code in the old area must be adjusted to the new location (anything that points into the area, such as hooks, must be updated), and its contents must fit in 256 bytes.

There are two main cases in which it may be necessary to add custom contents to this area:

* When data that is to be read by user software by using `RDSLT` or an equivalent mechanism is needed (for example, an [UNAPI](https://github.com/Konamiman/MSX-UNAPI-specification) implementation identifier).

* When a hook other than the timer interrupt hook or the extended BIOS hook is to be patched. In this case, code that performs an inter-bank call to the driver code should be placed in this area, and the hook should be set to do an inter-slot call to this code in the kernel slot.

The code at this area should use [the `CALBNK` routine](#424-calbnk-4042h) if it needs to invoke code in the driver bank, whose number can be read from [the `K_SIZE` address](#428-k_size-40feh).

Note that whatever is placed in this area, it must be identical in both banks 0 and 3, so that everything will work correctly in both the normal Nextor mode and the MSX-DOS 1 mode. The `mknexrom` tool will appropriately patch both banks if a data file for this area is supplied.

## 5. Testing drivers with DRVTEST.COM

Nextor is distributed with `DRVTEST.COM`, a command line tool that exercises the driver queries and the device queries of a driver installed in the system (either embedded in ROM or loaded in RAM) directly from the DOS prompt. The tool invokes the `DRIVER_QUERY` and `DEVICE_QUERY` routines of the driver by using [the `_CDRVR` function call](Nextor_3.0_Programmers_Reference.md#311-call-a-routine-in-a-device-driver-_cdrvr-7bh) and prints the results, so there's no need to write a dedicated test program (or to reboot the system) in order to verify that a driver under development handles the queries as expected.

The usage syntax is as follows (run `DRVTEST ?` for a detailed explanation of all the options):

```
DRVTEST <slot>[-<subslot>][:<segment>] [-l <string length>]
        [-a <space>] [-i] [-u] [-n <drive number>]
        [-d <device number>] [-t]
```

`<slot>` (and `<subslot>` if the slot is expanded) identifies the slot of the driver to be tested; `<segment>` must be supplied for drivers loaded in RAM. If no device number is specified (or `-d 0` is used) the tool runs the driver queries: get the driver version number and the driver information strings, plus the initialization queries when `-i` is supplied ("Initialize RAM driver" for RAM drivers, "Get driver initialization parameters" and "Initialize driver" otherwise) and the "Shut down RAM driver" query when `-u` is supplied. If a device number is specified with `-d`, the tool runs the device queries for that device instead: get the device information strings, get the device parameters, get the device availability, plus "Get device status" when `-t` is supplied.

A driver developer will typically use this tool after implementing or modifying the `DRIVER_QUERY` or `DEVICE_QUERY` routines, to quickly verify that the driver returns correct and sensible values for each of the queries described in _[4.5. Driver queries](#45-driver-queries)_ and _[4.6. Device queries](#46-device-queries)_.
