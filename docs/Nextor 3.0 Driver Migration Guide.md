# Nextor 3.0 Driver Migration Guide

## Index

[1. Introduction](#1-introduction)

[2. What changed in a nutshell](#2-what-changed-in-a-nutshell)

[3. The migration, step by step](#3-the-migration-step-by-step)

[3.1. Re-origin the driver code](#31-re-origin-the-driver-code)

[3.2. Include the SDK files](#32-include-the-sdk-files)

[3.3. Replace the driver header](#33-replace-the-driver-header)

[3.4. Rename the Nextor 2 routines](#34-rename-the-nextor-2-routines)

[3.5. Add the query dispatchers and adapters](#35-add-the-query-dispatchers-and-adapters)

[3.6. Replace the direct BIOS CHPUT calls](#36-replace-the-direct-bios-chput-calls)

[3.7. Delete DEV_FORMAT, DEV_CMD and DRV_CONFIG](#37-delete-dev_format-dev_cmd-and-drv_config)

[3.8. Adapt space-padded device information strings](#38-adapt-space-padded-device-information-strings)

[3.9. Adapt the extended BIOS handler](#39-adapt-the-extended-bios-handler)

[3.10. Set up the new build](#310-set-up-the-new-build)

[4. Building the migrated driver](#4-building-the-migrated-driver)

[4.1. The optional RAM driver target](#41-the-optional-ram-driver-target)

[5. Testing the migrated driver](#5-testing-the-migrated-driver)

[6. Beyond the compatibility layer](#6-beyond-the-compatibility-layer)


## 1. Introduction

Nextor 3 introduces a new device driver structure that is not compatible with the one used by Nextor 2: the driver signature changed, the jump table entries moved, and most of the per-routine entries of the old header were replaced by a query based API. A Nextor 2 driver ROM will not work in a Nextor 3 system; moreover, when Nextor 3 boots it detects and deactivates any Nextor 2 kernel present in the machine (both versions can't coexist), so a Nextor 2 driver ROM left in place simply stops working.

However, the incompatibility is limited to the driver interface: the driver mechanics (the code that actually talks to the hardware and handles "extras" like the timer interrupt hook or `EXTBIO` hook) remain largely unchanged, so most of your existing code can be reused by adding a compatibility layer. This is exactly how [the Sunrise IDE driver](https://github.com/Konamiman/SunriseIDE-Nextor-driver) and [the MegaFlashROM SCC+ SD driver](https://github.com/Konamiman/MegaFlashROM-SCC-SD-Nextor-driver) were migrated: the old header was swapped for the new one and about 300 lines of adapter code were added, while all the hardware access code was left untouched. This guide walks you through that same recipe, step by step, quoting that real adapter code for reference where that's useful for illustration purposes.

This guide assumes that you have a working **device-based** driver for Nextor 2. Note for authors of **drive-based** drivers: there is no direct migration path for you, since the drive-based model was removed in Nextor 3 (see _[4.1. One single driver model](Nextor%203.0%20Driver%20Development%20Guide.md#41-one-single-driver-model)_ in the Driver Development Guide); such drivers must first be restructured around the device model, and only then do the steps in this guide apply.

You will want to keep two references at hand while migrating:

* The _[Nextor 3.0 Driver Development Guide](Nextor%203.0%20Driver%20Development%20Guide.md)_, which is the full specification of the new driver structure. This guide links to it liberally instead of repeating its contents.

* [The dummy driver template](../sdk/templates/driver/driver.asm) supplied with [the Nextor SDK](../sdk/README.md), which contains per-routine comments of the form "this is the same as Nextor 2 X, except..." - a good companion when in doubt about a specific routine.

## 2. What changed in a nutshell

This table maps each element of the Nextor 2 device-based driver structure to its Nextor 3 equivalent:

| Nextor 2 | Nextor 3 | What changes |
|----------|----------|--------------|
| `"NEXTOR_DRIVER"` signature at 4100h | `"NEXTORv3_DRIVER"` signature at 4100h | New verbatim string, still zero-terminated |
| Flags byte at 410Eh | Gone | Device-based is the only model; for the hot-plug flag see the note below the table; the `DRV_CONFIG` presence flag is useless (`DRV_CONFIG` is gone) |
| `DRV_NAME`, 32 space-padded characters at 4110h | _[4.5.2. Driver query 2: Get driver information string](Nextor%203.0%20Driver%20Development%20Guide.md#452-driver-query-2-get-driver-information-string)_ | Zero-terminated string served on demand from anywhere in the bank |
| `DRV_TIMI` (4130h) | `TIMER_INT` (4110h) | Same contract; hooking is now requested via the output flags of _[4.5.3. Driver query 3: Get driver initialization parameters](Nextor%203.0%20Driver%20Development%20Guide.md#453-driver-query-3-get-driver-initialization-parameters)_ (was Cy from the first `DRV_INIT` call) |
| `DRV_VERSION` (4133h) | _[4.5.1. Driver query 1: Get driver version number](Nextor%203.0%20Driver%20Development%20Guide.md#451-driver-query-1-get-driver-version-number)_ | Version moves from A.B.C to B.C.D (A now holds a result code) |
| `DRV_INIT` with A=0 | _[4.5.3. Driver query 3: Get driver initialization parameters](Nextor%203.0%20Driver%20Development%20Guide.md#453-driver-query-3-get-driver-initialization-parameters)_ | Print callback arrives in DE; hook requests returned as flags in B; initialization can fail |
| `DRV_INIT` with A=1 | _[4.5.4. Driver query 4: Initialize driver](Nextor%203.0%20Driver%20Development%20Guide.md#454-driver-query-4-initialize-driver)_ | Can fail with `RESULT_INIT_ERROR` (driver is then ignored); the "allocated drives" input in B is gone |
| `DRV_BASSTAT` (4139h) | `OEMSTAT` (4113h) | Identical contract, new address |
| `DRV_BASDEV` (413Ch) | `BASDEV` (4116h) | Identical contract, new address |
| `DRV_EXTBIO` (413Fh) | `EXTBIO` (4119h) | Chain flag moves from D' to IYl (with no preset value); the hook is now opt-in via driver query 3 |
| `DRV_DIRECT0-4` (4142h-414Eh) | `DIRECT_0-4` (4134h-4140h) | Same feature, new addresses in the driver header; the kernel-side entry points stay at 7850h-785Ch, as in Nextor 2 |
| `DRV_CONFIG` (4151h) | Gone, no replacement | It was a workaround for Nextor 2 drivers not being able to tell apart nonexistent and offline devices, which Nextor 3 drivers can express directly (see step 3.7) |
| `DEV_RW` (4160h) | `READ_WRITE` (4128h) | C changes from logical unit number to media descriptor byte (floppies) or zero; same MSX-DOS error codes (`.IDEVL` was renamed to `.IDEVN`, same value 0B5h) |
| `DEV_INFO` (4163h) | _[4.6.1. Device query 1: Get device information string](Nextor%203.0%20Driver%20Development%20Guide.md#461-device-query-1-get-device-information-string)_ | Device number in C (was A); string indexes renumbered; buffer size passed in D; strings are zero-terminated, not padded |
| `DEV_STATUS` (4166h) | _[4.6.3. Device query 3: Get device status](Nextor%203.0%20Driver%20Development%20Guide.md#463-device-query-3-get-device-status)_ and _[4.6.4. Device query 4: Get device availability](Nextor%203.0%20Driver%20Development%20Guide.md#464-device-query-4-get-device-availability)_ | Status returned in B, result code in A; a nonexistent device returns `RESULT_INVALID_DEVICE` (was status 0) |
| `LUN_INFO` (4169h) | _[4.6.2. Device query 2: Get device parameters](Nextor%203.0%20Driver%20Development%20Guide.md#462-device-query-2-get-device-parameters)_ | Same 12 byte buffer layout; no logical unit index; HL=0 (validate the device only) must be supported; new "don't use for automapping" flag (bit 3) |
| `DEV_FORMAT` (416Ch) | _[4.6.5. Device query 5: Get format choices for a floppy disk device](Nextor%203.0%20Driver%20Development%20Guide.md#465-device-query-5-get-format-choices-for-a-floppy-disk-device)_ and _[4.6.6. Device query 6: Format a floppy disk device](Nextor%203.0%20Driver%20Development%20Guide.md#466-device-query-6-format-a-floppy-disk-device)_ | New code in practice: formatting was never available for device-based drivers in Nextor 2 (the entry existed in the header, but the kernel never invoked it), and the choice model is new too: built-in choice sets, or a custom string copied to a caller-supplied buffer |
| `DEV_CMD` (416Fh) | `CUSTOM_DEVICE_QUERY` (4125h), plus `CUSTOM_DRIVER_QUERY` (4122h) | Free-form extensibility, now with a well defined signature |
| — | _[4.6.7. Device query 7: Stop the motor of a floppy disk drive](Nextor%203.0%20Driver%20Development%20Guide.md#467-device-query-7-stop-the-motor-of-a-floppy-disk-drive)_ | New (floppy disk drivers only) |
| — | _[4.5.6. Driver query 6: Initialize RAM driver](Nextor%203.0%20Driver%20Development%20Guide.md#456-driver-query-6-initialize-ram-driver)_ and _[4.5.7. Driver query 7: Shut down RAM driver](Nextor%203.0%20Driver%20Development%20Guide.md#457-driver-query-7-shut-down-ram-driver)_ | New; only for drivers loaded in RAM (see _[4.1. The optional RAM driver target](#41-the-optional-ram-driver-target)_) |
| `DSKIO`, `DSKCHG`, `GETDPB`, `CHOICE`, `DSKFMT`, `MTOFF` | Gone | The drive-based model was removed; such drivers must be restructured around the device model |

Additionally, some general conventions changed:

* **Everything is a query with a result code.** All the driver and device queries return a result code in A: `RESULT_OK` (0), `RESULT_TRUNCATED_STRING` (1), `RESULT_INVALID_DEVICE` (2), `RESULT_INIT_ERROR` (3) or `RESULT_NOT_IMPLEMENTED` (FFh). Returning `RESULT_NOT_IMPLEMENTED` is legal for every query: the kernel then uses a sensible default.

* **The device number travels in C** for all the device queries (it was in A for `DEV_INFO`, `DEV_STATUS` and `LUN_INFO`); register A carries the query index instead. `READ_WRITE` still takes the device number in A, as `DEV_RW` did.

* **Logical units are gone.** A device is now the addressable unit; drives are mapped to a device and partition.

* **Initialization can fail** (`RESULT_INIT_ERROR`), in which case the kernel ignores the driver entirely, and **initialization must not print directly**: a print callback with `CHPUT` semantics is passed in DE.

* **The "reduced drive count" boot request** (the 5 key) arrives as bit 5 of register C in both initialization queries, instead of being inferred from the drive count passed to the second `DRV_INIT` call.

* **The extended BIOS chaining flag moves from D' to IYl** (1 = chain to the next handler, 0 = stop; the handler is entered with an undefined value in IYl), and the hook is only installed if the driver requests it.

The hot-plug header flag has no direct replacement: report hot-pluggable devices as removable in the device parameters and implement the "get device status" and "get device availability" queries (returning B=0 when no medium/device is present); this provides the equivalent behavior.

## 3. The migration, step by step

The steps below are, in order, exactly the changes that were applied to convert the Nextor 2 Sunrise IDE driver into [the Nextor 3 one](https://github.com/Konamiman/SunriseIDE-Nextor-driver). All the code snippets are taken from the migrated `driver.asm` of that project (the conditional assembly blocks for its "master only" build variant have been removed here for clarity).

### 3.1. Re-origin the driver code

`mknexrom` expects the driver file to start with 256 dummy bytes, which it overwrites with the kernel's common bank header code; this was already the case in Nextor 2, but drivers that lived inside the kernel repository were assembled with `org 4100h` and got that block prepended by the kernel's own Makefile. Now that drivers are standalone projects the simplest approach is to assemble at 4000h and emit the dummy block from the source itself:

```
    org 4000h
    ds 4100h-$,0    ;Driver code starts at 4100h
DRV_START:
```

Alternatively you can keep `org 4100h` and prepend 256 zero bytes to the assembled binary at build time; [the MSX Turbo-R FDD driver](https://github.com/Konamiman/Turbo-R-FDD-Nextor-driver) does it that way in its Makefile (a `dd`-generated zero block concatenated with `cat`).

### 3.2. Include the SDK files

Add these includes at the top of the file (the paths are relative to the root of [the Nextor SDK](../sdk/README.md); you will pass the SDK location to the assembler in step _[3.10. Set up the new build](#310-set-up-the-new-build)_):

```
    INCLUDE asm/macros/undoc.inc

    INCLUDE asm/constants/driver_result_codes.inc

    module DRIVER_QUERY
    INCLUDE asm/constants/driver_driver_queries.inc
    endmod

    module DEVICE_QUERY
    INCLUDE asm/constants/driver_device_queries.inc
    endmod
```

Notes:

* `driver_result_codes.inc` defines the `RESULT_*` codes used by all the adapter code in this guide.

* The two query index files are wrapped in modules because both define constants with the same names (e.g. `GET_STRING`); they become available as `DRIVER_QUERY.GET_STRING`, `DEVICE_QUERY.GET_PARAMS`, etc.

* `undoc.inc` is optional: it provides macros that replace the undocumented Z80 instructions (those operating on IXh/IXl/IYh/IYl) with documented equivalents when the `NO_UNDOC_CPU_INSTRUCTIONS` symbol is defined, for Z180 compatibility.

Additionally, replace the block of hand-copied kernel entry point `EQU`s that every Nextor 2 driver carried (`GSLOT1 equ 402Dh`, `CALBNK equ 4042h`, `GWORK equ 4045h`, etc.) with:

```
    INCLUDE asm/constants/rom_bank_header.inc
```

If your driver uses kernel variables in page 3 (`BK4_ADD`, `TMP_IX`, `TMP_IY`), take them from `asm/constants/driver_workarea.inc` too. Always use the SDK constants instead of addresses hardcoded from Nextor 2 sources, as some names and addresses changed between versions.

### 3.3. Replace the driver header

Delete the entire Nextor 2 header (signature, flags byte, 32 character name field, and the two jump tables at 4130h and 4160h) and replace it with the new one. As an example, this is the actual header used for the Sunrise IDE driver; note how the jump table simply points to the old routine names where the contract is unchanged, and to the new adapter routines everywhere else:

```
    ;Driver signature

    db "NEXTORv3_DRIVER",0

    ;Jump table

    jp DRV_TIMI ;TIMER_INT
    jp DRV_BASSTAT ;OEMSTAT
    jp DRV_BASDEV ;BASDEV
    jp DRV_EXTBIO ;EXTBIO
    jp DRIVER_QUERY
    jp DEVICE_QUERY
    jp CUSTOM_DRIVER_QUERY
    jp CUSTOM_DEVICE_QUERY
    jp READ_WRITE

    ; 3 reserved entries + 5 direct call entries
    rept 8*3
    ret
    endm

DRV_NAME:
    db "Sunrise IDE",0
```

Notes:

* The driver name is now just a zero-terminated string placed anywhere in the bank; it will be served by the "get driver information string" adapter in step _[3.5. Add the query dispatchers and adapters](#35-add-the-query-dispatchers-and-adapters)_.

* This header assumes that direct calls aren't implemented, so the three reserved entries and the five direct call entries are filled with `ret` instructions in one go. If your Nextor 2 driver implemented `DRV_DIRECT0-4`, fill the three reserved entries with `ds 3*3,0C9h` and keep proper `jp` instructions for the five direct entries (see _[4.4.11. DIRECT_0...4 (4134h...4140h)](Nextor%203.0%20Driver%20Development%20Guide.md#4411-direct_04-4134h4140h)_). The kernel-side entry points for direct calls remain at 7850h-785Ch, unchanged from Nextor 2, so external tools of yours that invoke them will keep working without changes.

* The full header layout is specified in _[4.3. The driver header](Nextor%203.0%20Driver%20Development%20Guide.md#43-the-driver-header)_.

### 3.4. Rename the Nextor 2 routines

Rename the routines whose contract changed, leaving their bodies completely untouched. The `NEXTOR2_` prefix makes it obvious which code is legacy:

| Old label | New label |
|-----------|-----------|
| `DRV_VERSION` | `NEXTOR2_DRV_VERSION` |
| `DRV_INIT` | `NEXTOR2_DRV_INIT` |
| `DEV_RW` | `NEXTOR2_DEV_RW` |
| `DEV_INFO` | `NEXTOR2_DEV_INFO` |
| `DEV_STATUS` | `NEXTOR2_DEV_STATUS` |
| `LUN_INFO` | `NEXTOR2_LUN_INFO` |

The routines whose contract is (almost) unchanged (`DRV_TIMI`, `DRV_BASSTAT`, `DRV_BASDEV`, `DRV_EXTBIO`) keep their names, since the new jump table references them directly. Auxiliary routines like the Sunrise IDE driver's `MY_GWORK` (which wraps the kernel's `GWORK` call) keep working as-is too.

### 3.5. Add the query dispatchers and adapters

This is the heart of the compatibility layer: two dispatcher routines that map each query index to a small adapter, and the adapters themselves, which adapt registers and then call the renamed Nextor 2 routines. Also include [the SDK's `OUTPUT_STRING` helper](../sdk/asm/code/output_string.asm), which all the string-serving adapters rely on:

```
    INCLUDE asm/code/output_string.asm
```

Here's the driver query dispatcher (see _[4.4.5. DRIVER_QUERY (411Ch)](Nextor%203.0%20Driver%20Development%20Guide.md#445-driver_query-411ch)_ for the signature) and its adapters:

```
DRIVER_QUERY:
    dec a
    jr z,DO_DRVQ_GET_VERSION
    dec a
    jr z,DO_DRVQ_GET_STRING
    dec a
    jr z,DO_DRVQ_GET_INIT_PARAMS
    dec a
    jr z,DO_DRVQ_INIT
    dec a
    jr z,DO_DRVQ_GET_MAX_DEVICE
    ld a,RESULT_NOT_IMPLEMENTED
    ret

DO_DRVQ_GET_VERSION:
    call NEXTOR2_DRV_VERSION    ;Returns the version in A.B.C
    ld d,c
    ld c,b
    ld b,a                      ;Nextor 3 wants it in B.C.D
    xor a                       ;RESULT_OK
    ret

DO_DRVQ_GET_STRING:
    ld a,b  ;String index
    ld b,d  ;Buffer size
    ex de,hl
    dec a
    ld hl,DRV_NAME
    jp z,OUTPUT_STRING
    ld a,RESULT_NOT_IMPLEMENTED
    ret

DO_DRVQ_GET_INIT_PARAMS:
    push de                     ;Print routine address --> IY, see step 3.6
    pop iy
    xor a                       ;The old routine expects A=0 on its first call
    call NEXTOR2_DRV_INIT
    ld b,0
    rl b                        ;Old Cy output ("hook the timer interrupt") --> B bit 0
    xor a                       ;RESULT_OK; HL (required work area size) passes through
    ret

DO_DRVQ_INIT:
    push de                     ;Print routine address --> IY, see step 3.6
    pop iy
    ld a,1                      ;The old routine expects A=1 on its second call
    call NEXTOR2_DRV_INIT
    xor a                       ;RESULT_OK
    ret

DO_DRVQ_GET_MAX_DEVICE:
    ld b,2                      ;Two devices: master and slave
    xor a
    ret
```

Notes on the initialization adapters:

* Initialization can now fail: if your hardware detection code can report failure, return `RESULT_INIT_ERROR` instead of the final `xor a` and the kernel will skip the driver entirely (it won't be counted as an existing Nextor kernel). The old `DRV_INIT` had no way to report failure, so a plain `xor a` faithfully reproduces the Nextor 2 behavior.

* If your old driver examined the drive count passed in B on the second `DRV_INIT` call to detect the "reduced drive count" boot request, check bit 5 of register C instead (it arrives untouched at both adapters).

* `DO_DRVQ_GET_MAX_DEVICE` returns the highest device number the driver can possibly handle, or `RESULT_NOT_IMPLEMENTED` to get the default of 4 (see _[4.5.5. Driver query 5: Get maximum supported device number](Nextor%203.0%20Driver%20Development%20Guide.md#455-driver-query-5-get-maximum-supported-device-number)_). This query exists purely as a performance improvement: it caps the range of device numbers the kernel probes when scanning the driver's devices, which otherwise would have to cover all 255 possibilities. It's an upper bound, not a device count (not every device number up to the maximum has to exist), and it has nothing to do with how many drive letters the driver gets at boot time: don't confuse it with the old `DRV_CONFIG`, which is covered in _[3.7. Delete DEV_FORMAT, DEV_CMD and DRV_CONFIG](#37-delete-dev_format-dev_cmd-and-drv_config)_.

* Unrelated to the migration, but worth repeating: don't use the memory at C000h-C400h as a temporary work area during initialization; a few Panasonic MSX machines use that area at boot time (this was already true in Nextor 2; the Sunrise IDE driver uses C400h).

This is the device query dispatcher (see _[4.4.6. DEVICE_QUERY (411Fh)](Nextor%203.0%20Driver%20Development%20Guide.md#446-device_query-411fh)_): it validates the device number in C **before** looking at the query index (this is a requirement of the new API) and then dispatches. Note that it covers the `READ_WRITE` routine too; `DO_DEVQ_GET_STRING`, on the other hand, is covered separately in step _[3.8. Adapt space-padded device information strings](#38-adapt-space-padded-device-information-strings)_.

```
DEVICE_QUERY:
    push af
    ld a,c
    or a
    jr z,INVALID_DEVICE
    cp 3                        ;This driver handles two devices (master and slave)
    jr nc,INVALID_DEVICE

    pop af
    dec a
    jr z,DO_DEVQ_GET_STRING
    dec a
    jp z,DO_DEVQ_GET_PARAMS
    dec a
    jp z,DO_DEVQ_GET_STATUS
    dec a
    jp z,DO_DEVQ_GET_AVAILABILITY
    ld a,RESULT_NOT_IMPLEMENTED
    ret

INVALID_DEVICE:
    pop af
    ld a,RESULT_INVALID_DEVICE
    ret

DO_DEVQ_GET_PARAMS:
    ld a,h
    or l
    ret z   ;No buffer: just return no error (device id is ok)

    ld a,c
    ld b,1
    call NEXTOR2_LUN_INFO
    or a
    ret z

    ;Assume error is "device not available" (we checked the device id first),
    ;then return default parameters but with removable bit set
    push hl
    pop ix
    xor a
    ld (ix),a
    ld (ix+1),a
    ld (ix+2),2 ;Sector size, high byte
    ld (ix+3),a
    ld (ix+4),a
    ld (ix+5),a
    ld (ix+6),a
    ld (ix+7),1 ;Removable flag
    ld (ix+8),a
    ld (ix+9),a
    ld (ix+10),a
    ld (ix+11),a
    xor a
    ret

DO_DEVQ_GET_STATUS:
DO_DEVQ_GET_AVAILABILITY:
    ld a,c
    ld b,1
    call NEXTOR2_DEV_STATUS
    ld b,a
    ;Assume A=0 means "device not available" and not "invalid device id"
    ;(we checked the device id first)
    xor a
    ret

CUSTOM_DRIVER_QUERY:
CUSTOM_DEVICE_QUERY:
    ld a,RESULT_NOT_IMPLEMENTED
    ret

READ_WRITE:
    push af                     ;Save Cy (0 = read, 1 = write) and device number
    or a                        ;Device number 0 never exists
    jr z,RW_BADDEV
    cp 3                        ;Only devices 1 and 2 exist
    jr nc,RW_BADDEV
    pop af
    ld c,1
    call NEXTOR2_DEV_RW
    cp _IDEVL                   ;The device number is valid, so an "invalid device"
    ret nz                      ;error from the old driver code actually means
    ld a,_NRDY                  ;"device currently absent": return "not ready",
    ret                         ;as the Nextor 3 driver interface requires
RW_BADDEV:
    pop af
    ld a,_IDEVL
    ld b,0
    ret

RETURN_NOT_IMP:
    ld a,RESULT_NOT_IMPLEMENTED
    ret
```

Notes:

* The old routines took the device number in A and a logical unit number in B (`LUN_INFO`, `DEV_STATUS`) or C (`DEV_RW`). Since virtually every Nextor 2 device-based driver implemented exactly one logical unit per device, the adapters simply move C to A and hardcode a logical unit of 1.

* `DO_DEVQ_GET_PARAMS` must support HL=0, meaning "just validate the device number"; the `ld a,h / or l / ret z` sequence handles that (conveniently returning A=0, i.e. `RESULT_OK`). The old `LUN_INFO` didn't accept a zero buffer address.

* In `READ_WRITE`, register C changed meaning: it was the logical unit number, now it's the media descriptor byte for floppy disk drives and zero otherwise. For a non-floppy driver like this one, replacing it with the fake logical unit 1 is all the old `DEV_RW` needs; floppy disk drivers should instead pass C through and use it as described in _[4.4.9. READ_WRITE (4128h)](Nextor%203.0%20Driver%20Development%20Guide.md#449-read_write-4128h)_. The MSX-DOS error codes returned are unchanged, although the code formerly named `.IDEVL` ("invalid device or logical unit") is now `.IDEVN` ("invalid device number"), with the same value 0B5h.

* **Watch out for the `.IDEVL` error in your old `DEV_RW`**: since Nextor 2 didn't distinguish nonexistent devices from devices that exist but are currently absent, old drivers commonly return `.IDEVL` for both; for example, for a card slot that was empty when the driver initialized. In Nextor 3 these are different results: `READ_WRITE` must return `.NRDY` ("not ready") when the device exists but is currently unavailable, reserving `.IDEVN` for device numbers that your driver never provides. The kernel is somewhat forgiving about this particular mistake (in MSX-DOS 1 mode it converts an `.IDEVN` result from `READ_WRITE` into the same "disk offline" error that `.NRDY` produces, and in MSX-DOS 2 mode the media change check reports an unavailable removable device as "not ready" before `READ_WRITE` is ever called), but don't rely on that: in MSX-DOS 2 mode, a device that isn't reported as removable, or a driver that doesn't implement the "get device status" query, will still surface `.IDEVN` ("Invalid device number") to the application instead of "Not ready". An easy way to get this right in the glue routine: validate the device number range yourself before calling the old `DEV_RW`, and afterwards translate an `.IDEVN` result to `.NRDY` (at that point the device number is known to be valid, so the old code can only mean "device absent"). That's exactly what the `READ_WRITE` routine quoted above does.

* `CUSTOM_DRIVER_QUERY` and `CUSTOM_DEVICE_QUERY` are mandatory entries but a two-line stub satisfies them if you have nothing custom to offer.

### 3.6. Replace the direct BIOS CHPUT calls

Nextor 2 drivers printed their initialization messages by calling the BIOS `CHPUT` routine directly. In Nextor 3 the initialization queries **must not** print directly; instead, they receive in DE the address of a print callback with `CHPUT` semantics (prints the character in A, modifies only AF), as described in _[4.5.4. Driver query 4: Initialize driver](Nextor%203.0%20Driver%20Development%20Guide.md#454-driver-query-4-initialize-driver)_.

The Sunrise IDE driver converts its entire legacy printing code with a two-line trick. The initialization adapters in step 3.5 already copied the callback address to IY (`push de / pop iy`); now delete the old BIOS equate and replace it with a trampoline of the same name:

```
    ;Delete this:
    ;CHPUT equ 00A2h

    ;Add this instead:
CHPUT: jp (iy)
```

Every existing `call CHPUT` in the initialization path now lands on the trampoline and jumps to the kernel-provided callback. The only requirement is that the legacy initialization code doesn't use IY for anything else (if it does, store the callback address in a variable and make the `CHPUT` label jump through that instead).

### 3.7. Delete DEV_FORMAT, DEV_CMD and DRV_CONFIG

These three routines have no entry in the new header, so delete them:

* `DEV_FORMAT`: for non-floppy drivers (which returned "not implemented" anyway) nothing replaces it: the device query dispatcher from _[3.5. Add the query dispatchers and adapters](#35-add-the-query-dispatchers-and-adapters)_ already returns `RESULT_NOT_IMPLEMENTED` for the format-related queries. Floppy disk drivers need real work here, and it's genuinely new code: Nextor 2 never invoked this entry (formatting simply wasn't available for drives mapped to device-based drivers), so there's no old behavior to adapt. The driver now reports one of the built-in choice sets (B=0, 1 or 2) or copies a custom choice string to a caller-supplied buffer (B=255), and performs the format itself. Implement device queries 5 and 6 as described in _[4.6.5. Device query 5: Get format choices for a floppy disk device](Nextor%203.0%20Driver%20Development%20Guide.md#465-device-query-5-get-format-choices-for-a-floppy-disk-device)_ and _[4.6.6. Device query 6: Format a floppy disk device](Nextor%203.0%20Driver%20Development%20Guide.md#466-device-query-6-format-a-floppy-disk-device)_, see [the MSX Turbo-R FDD driver](https://github.com/Konamiman/Turbo-R-FDD-Nextor-driver) for a working example.

* `DEV_CMD`: if your driver used it (or abused the direct call entries) for tool-facing custom functionality, reimplement those commands as custom queries behind `CUSTOM_DEVICE_QUERY`/`CUSTOM_DRIVER_QUERY`, which have the same signatures as the standard query routines (see _[4.4.7. CUSTOM_DRIVER_QUERY (4122h)](Nextor%203.0%20Driver%20Development%20Guide.md#447-custom_driver_query-4122h)_ and _[4.4.8. CUSTOM_DEVICE_QUERY (4125h)](Nextor%203.0%20Driver%20Development%20Guide.md#448-custom_device_query-4125h)_).

* `DRV_CONFIG`: gone without a replacement, so simply delete it. This routine (with its two configuration indexes: number of drives at boot, and default device/logical unit per drive) existed as a workaround for a Nextor 2 limitation: drivers had no way to tell apart devices that don't exist from devices that exist but happen to have no medium inserted, so a driver with e.g. two SD card slots had to explicitly announce "assign me two drives at boot" to get correct mappings. It was also a layering violation: how the kernel maps drives shouldn't be the driver's business. In Nextor 3 drivers report the existence and availability of each device directly (`RESULT_INVALID_DEVICE` vs the "Get device status"/"Get device availability" queries), and the kernel handles all the drive mapping on its own. If you need to keep a device out of the automatic boot mapping, use the per-device "don't use for automapping" flag in the device parameters (bit 3 of the flags byte, see _[4.6.2. Device query 2: Get device parameters](Nextor%203.0%20Driver%20Development%20Guide.md#462-device-query-2-get-device-parameters)_).

### 3.8. Adapt space-padded device information strings

Nextor 2's `DEV_INFO` wrote fixed-size, space-padded strings to a caller buffer of assumed size; Nextor 3's "get device information string" query wants zero-terminated strings honoring an explicit buffer size, and the string indexes were renumbered:

| Nextor 2 `DEV_INFO` (B) | Nextor 3 device query 1 (B) |
|--------------------------|------------------------------|
| 0: Basic information | - (no equivalent) |
| 1: Manufacturer name | 1: Manufacturer name |
| 2: Device name (read from the hardware) | 2: Medium name |
| 3: Serial number | 3: Serial number |
| - | 4: Device name (fixed, provided by the driver) |

Note how the indexes 1-3 conveniently line up: what the old IDE driver reported as "device name" read from the hardware (the disk model from the ATA IDENTIFY data) is, in Nextor 3 terms, the medium name; and the new index 4 (a "conceptual" device name) is best served by new static strings.

The Sunrise IDE wrapper works by pointing the old `NEXTOR2_DEV_INFO` at a scratch buffer, zero-terminating the real content, and letting `OUTPUT_STRING` do the size-limited copy. First, a 65 byte scratch field is appended to the driver's work area (since the work area size returned by `NEXTOR2_DRV_INIT` is derived from the structure definition, the extra field is allocated automatically):

```
field STRBUFF,65    ; Scratch buffer used by the DEVICE_QUERY GET_STRING
                    ; compatibility wrapper (64 bytes of string content
                    ; plus a trailing terminator).
```

Then the adapter itself:

```
DO_DEVQ_GET_STRING:
    ld a,b
    or a
    jp z,RETURN_NOT_IMP

    cp 4
    jp z,DO_DEVQ_GET_DEV_NAME

    ld a,d
    or a
    ret z      ;Buffer size=0: do nothing, no error
    dec a
    jr nz,DO_DEVQ_GET_STRING_2
    ld (hl),0  ;Buffer size=1: just output terminating 0, no error
    ret

DO_DEVQ_GET_STRING_2:
    ;HL=user buf, D=user size, B=substring code, C=device number
    ;
    ;Sunrise IDE NEXTOR2_DEV_INFO ignores the user buffer size and always
    ;writes a fixed-layout 64-byte image to its HL argument:
    ;
    ;   substring 2 (device name): 20 chars at offsets  0..19, then spaces
    ;   substring 3 (serial)     : spaces, 10 chars at offsets 44..53, then spaces
    ;
    ;The wrapper therefore points it at a scratch buffer in WRKAREA, then
    ;writes a zero terminator at the end of the actual content and copies
    ;the substring into the user buffer with OUTPUT_STRING (which handles
    ;the truncation / RESULT_TRUNCATED_STRING accounting against D).

    push hl                     ;[SP+4] user buffer
    push de                     ;[SP+2] user size (D=size)
    push bc                     ;[SP+0] B=substring, C=device

    xor a
    call MY_GWORK               ;IX = WRKAREA base, preserves BC/DE/HL

    push ix
    pop hl
    ld bc,WRKAREA.STRBUFF
    add hl,bc                   ;HL = STRBUFF base

    pop bc                      ;Restore B=substring, C=device
    push bc                     ;Re-save for after NEXTOR2_DEV_INFO
    push hl                     ;Save STRBUFF for after NEXTOR2_DEV_INFO

    ld a,c                      ;A = device number for NEXTOR2_DEV_INFO
    call NEXTOR2_DEV_INFO       ;A = result code

    pop hl                      ;HL = STRBUFF
    pop bc                      ;B = substring, C = device

    or a
    jr nz,DO_DEVQ_GET_STRING_FAIL

    ;Success. Place a 0 at the end of the actual content so OUTPUT_STRING
    ;can detect the real string length:
    ;  substring 2 (device name): terminator at STRBUFF+20
    ;  substring 3 (serial)     : terminator at STRBUFF+54
    push hl
    ld de,20
    ld a,b
    cp 3
    jr nz,DO_DEVQ_GET_STRING_TERM
    ld de,54
DO_DEVQ_GET_STRING_TERM:
    add hl,de
    ld (hl),0
    pop hl                      ;HL = STRBUFF

    ;For the serial number, point the source past the 44 bytes of left
    ;padding sunride.asm writes before the content.
    ld a,b
    cp 3
    jr nz,DO_DEVQ_GET_STRING_SRC
    ld de,44
    add hl,de                   ;HL = STRBUFF + 44
DO_DEVQ_GET_STRING_SRC:

    ;Stack: [user_size, user_buf]. HL is the zero-terminated source.
    pop de                      ;D = user size
    ld b,d                      ;B = max length for OUTPUT_STRING
    pop de                      ;DE = user buffer (destination)
    jp OUTPUT_STRING

DO_DEVQ_GET_STRING_FAIL:
    ;Stack: [user_size, user_buf]
    pop de                      ;Discard saved user size
    pop hl                      ;Discard saved user buffer
    jp RETURN_NOT_IMP
```

Finally, the new device name strings (index 4), which are just static zero-terminated strings served with `OUTPUT_STRING`:

```
DO_DEVQ_GET_DEV_NAME:
    ex de,hl
    ld b,h                      ;B = user buffer size (was D)
    ld a,c
    dec a
    ld hl,MASTER_DEV_S
    jp z,OUTPUT_STRING
    ld hl,SLAVE_DEV_S
    jp OUTPUT_STRING

MASTER_DEV_S:
    db "IDE master device",0
SLAVE_DEV_S:
    db "IDE slave device",0
```

The exact offsets (20, 44, 54) are of course specific to the Sunrise IDE driver's `DEV_INFO` layout; adjust them to whatever fixed layout your own routine produces. If your `DEV_INFO` already wrote strings of known length, the wrapper gets simpler; the pattern to keep is: **scratch buffer, zero-terminate, `jp OUTPUT_STRING`**.

### 3.9. Adapt the extended BIOS handler

Skip this step if your driver doesn't hook the extended BIOS (the Sunrise IDE driver doesn't: its `DRV_EXTBIO` is an empty routine referenced as `EXTBIO` in the jump table, and no flag is requested).

If it does, two changes are needed:

1. The hook is now opt-in: set bit 1 of B in the "get driver initialization parameters" adapter (`DO_DRVQ_GET_INIT_PARAMS` from step 3.5), e.g. with a `set 1,b` after the `rl b` line.

2. The chaining convention changed: in Nextor 2 the handler used the alternate register D' as the chain flag (it was entered with D'=1, and returned D'=0 to prevent the kernel and system handlers from running); in Nextor 3 the flag lives in IYl instead, as described in _[4.4.4. EXTBIO (4119h)](Nextor%203.0%20Driver%20Development%20Guide.md#444-extbio-4119h)_, and the handler doesn't get any preset value for the flag (as the IY register is used for the inter-slot call that invokes the handler). A small wrapper reproduces the old entry state and converts the result, so the legacy handler runs unmodified. This is the wrapper used by [the example RAM driver](../source/drivers/ram-driver-example.asm), where `_DO_EXTBIO` is the old-style handler:

```
DO_EXTBIO:
    exx
    ld d,1
    exx
    call _DO_EXTBIO
    exx
    ld e,d
    push de
    pop iy
    exx
    ret
```

Rename your old `DRV_EXTBIO` to `_DO_EXTBIO` (or similar), add the wrapper, and point the `EXTBIO` entry of the jump table at the wrapper.

### 3.10. Set up the new build

The last step is building the migrated driver with the Nextor 3 toolchain, which is different enough from the Nextor 2 days to deserve its own section: see _[4. Building the migrated driver](#4-building-the-migrated-driver)_ below.

## 4. Building the migrated driver

In the Nextor 2 era, drivers typically lived inside a fork of the Nextor kernel repository and were built by the kernel's own Makefile. In Nextor 3 drivers are standalone projects, organized and distributed however their developers prefer (the drivers that were part of Nextor 2 live in their own git repositories, but a dedicated web site or plain downloadable binaries are equally valid options), and build against two artifacts:

* **The Nextor SDK** ([`sdk/` in the Nextor repository](../sdk/README.md), see also _[8.1. The Nextor SDK](Nextor%203.0%20Programmers%20Reference.md#81-the-nextor-sdk)_): the include files and helper code used in the steps above. The migrated drivers pull it in as a git submodule; both the Sunrise IDE and the Turbo-R FDD repositories have a `make setup` target that initializes the submodule as a blobless, sparse checkout of the `sdk/` directory only, so the full Nextor repository is never fetched.

* **The Nextor kernel base file** (`Nextor-<version>.base[.variant].dat`), distributed with the Nextor releases, pointed at by a `NEXTOR_BASE` variable in the Makefile.

The assembler is [Nestor80](https://github.com/Konamiman/Nestor80) (N80), and the ROM is put together with the mknexrom tool, exactly as described in _[3. Creating a Nextor kernel ROM with embedded driver](Nextor%203.0%20Driver%20Development%20Guide.md#3-creating-a-nextor-kernel-rom-with-embedded-driver)_. The minimal recipe is:

```
N80 driver.asm driver.bin --build-type abs --include-directory $NEXTOR_SDK
N80 chgbnk.asm chgbnk.bin --build-type abs
mknexrom $NEXTOR_BASE Nextor-3.0.MyDriver.ROM /d:driver.bin /m:chgbnk.bin
```

Notes:

* `--include-directory` must point to the SDK root so the `INCLUDE asm/...` lines from step 3.2 resolve correctly.

* The bank switching code (`chgbnk`) is unchanged from Nextor 2, so you can keep using your existing file; the SDK also ships ready-made sources for the ASCII8 and ASCII16 mappers ([`asm/chgbnk/ascii8.asm`](../sdk/asm/chgbnk/ascii8.asm) and [`asm/chgbnk/ascii16.asm`](../sdk/asm/chgbnk/ascii16.asm)). The `/m:` parameter can be omitted for ASCII16, see _[3.2. Using the mknexrom utility](Nextor%203.0%20Driver%20Development%20Guide.md#32-using-the-mknexrom-utility)_.

* If you chose to keep `org 4100h` in step 3.1, prepend 256 zero bytes to `driver.bin` before invoking `mknexrom`.

* The space available for the driver in each bank is the same as in Nextor 2: 4100h to 7FD0h, i.e. 3ED0h (16080) bytes. Pad the binary to that size with a `ds` directive (e.g. the SDK driver template ends with `ds 7FD0h-$,0FFh`), which has the nice side effect of making the assembly fail if the driver outgrows the bank.

* To support Z180-based MSX machines, assemble with `--define-symbols NO_UNDOC_CPU_INSTRUCTIONS` (this activates the documented-instructions variants of the `undoc.inc` macros) and pair the result with a `.NO_UNDOC.` variant of the kernel base file.

* Instead of installing a local toolchain you can build inside [the Nextor development Docker image](../docker/README.md) (see _[8.2. The Docker development image](Nextor%203.0%20Programmers%20Reference.md#82-the-docker-development-image)_), which provides N80, `mknexrom`, the SDK and the kernel base files. The migrated driver repositories include a `docker-build.sh` wrapper that does exactly this.

The Makefiles of [the Sunrise IDE driver](https://github.com/Konamiman/SunriseIDE-Nextor-driver) and [the MSX Turbo-R FDD driver](https://github.com/Konamiman/Turbo-R-FDD-Nextor-driver) are complete, commented, real-world examples of this setup.

### 4.1. The optional RAM driver target

Nextor 3 drivers can also be loaded into a mapped RAM segment at runtime, with no ROM flashing involved; see _[4.5.6. Driver query 6: Initialize RAM driver](Nextor%203.0%20Driver%20Development%20Guide.md#456-driver-query-6-initialize-ram-driver)_. The Turbo-R FDD driver builds both flavors from the same source file, selected by a `RAM_DRIVER` symbol (`make ram` runs a single N80 invocation with `--define-symbols RAM_DRIVER`; no kernel base file, bank switching code or mknexrom involved, the output is a plain `.drv` file). Under `if RAM_DRIVER`, the source:

* skips the 256 byte dummy block: the source assembles at 4100h in both flavors, but only the ROM build gets the block prepended at build time (RAM drivers are loaded at 4100h directly). A RAM-only driver may also omit the `RESERVED_*` and `DIRECT_*` jump table entries, as [the example RAM driver](../source/drivers/ram-driver-example.asm) does;

* returns `RESULT_NOT_IMPLEMENTED` for driver queries 3 and 4, and implements driver queries 6 ("initialize RAM driver", which receives the slot and segment in B and C) and 7 ("shut down RAM driver") instead.

The resulting file is loaded with [the `CALL IDRIVER` command](Nextor%203.0%20User%20Manual.md#3612-the-call-idriver-command) or [the `DRVROP.COM` tool](Nextor%203.0%20User%20Manual.md#3413-drvrop-the-driver-operations-tool). [The SDK driver template](../sdk/templates/driver/driver.asm) has this dual-target conditional structure already in place.

## 5. Testing the migrated driver

Nextor 3 ships a dedicated tool for exactly this situation: `DRVTEST.COM` invokes the driver queries and the device queries of an installed driver directly from the DOS prompt (via [the `_CDRVR` function call](Nextor%203.0%20Programmers%20Reference.md#311-call-a-routine-in-a-device-driver-_cdrvr-7bh)) and prints the results, so you can verify every adapter from section 3 without writing a test program or rebooting. See _[5. Testing drivers with DRVTEST.COM](Nextor%203.0%20Driver%20Development%20Guide.md#5-testing-drivers-with-drvtestcom)_ for the full syntax; in short, run it with the driver's slot to exercise the driver queries, and add `-d <device number>` to exercise the device queries of each device.

Additionally, [the `DRIVERS.COM` tool](Nextor%203.0%20User%20Manual.md#342-drivers-the-driver-information-tool) (also available in BASIC as [the `CALL DRIVERS` command](Nextor%203.0%20User%20Manual.md#367-the-call-drivers-command)) and [the `DEVINFO.COM` tool](Nextor%203.0%20User%20Manual.md#343-devinfo-the-device-information-tool) display the driver and device information as the kernel sees it; a quick end-to-end check that the version, name and string adapters behave.

If you build the RAM driver variant described in _[4.1. The optional RAM driver target](#41-the-optional-ram-driver-target)_, the whole edit-build-test cycle can happen without touching the ROM: load the new build with `DRVROP.COM`, test, unload, repeat.

## 6. Beyond the compatibility layer

The steps above give you a fully functional Nextor 3 driver with minimal changes. Once it works, consider adopting these Nextor 3 features, all of them small additions on top of the migrated code:

* **More driver information strings.** Driver query 2 also accepts indexes for the driver author name, the hardware name, the hardware author name and a serial number (see _[4.5.2. Driver query 2: Get driver information string](Nextor%203.0%20Driver%20Development%20Guide.md#452-driver-query-2-get-driver-information-string)_). Each one is a static string plus three lines in the `DO_DRVQ_GET_STRING` adapter.

* **A real medium name.** If your hardware can identify the inserted medium, serving device string 2 makes tools like `DEVINFO.COM` much more informative (the Sunrise IDE wrapper in _[3.8. Adapt space-padded device information strings](#38-adapt-space-padded-device-information-strings)_ gets this for free from the old hardware-querying code).

* **An accurate maximum device number.** Returning the true value from driver query 5 saves the kernel from probing devices that don't exist, speeding up boot-time automapping.

* **The "don't use for automapping" flag** (bit 3 of the device parameters flags byte), for devices that exist but shouldn't get drive letters automatically at boot.

* **Custom functionality.** Anything you used to bolt onto `DEV_CMD` or the direct call entries can become proper custom queries behind `CUSTOM_DRIVER_QUERY`/`CUSTOM_DEVICE_QUERY`, callable from user programs via [the `_CDRVR` function call](Nextor%203.0%20Programmers%20Reference.md#311-call-a-routine-in-a-device-driver-_cdrvr-7bh).

* **Full floppy disk support.** If your devices are floppy disk drives, implement the format queries (device queries 5 and 6), the "stop motor" query (device query 7), and report the "floppy disk drive" flag in the device parameters so the kernel applies its floppy-specific behaviors (see ["Support for floppy disks" in the User Manual](Nextor%203.0%20User%20Manual.md#25-support-for-floppy-disks)).

* **A RAM-loadable variant** of the same source, as described in _[4.1. The optional RAM driver target](#41-the-optional-ram-driver-target)_. Besides speeding up your own testing, it lets users run your driver without flashing anything. RAM drivers can even receive install-time configuration: when loaded with the standard tools, addresses 4000h-40FFh of the segment may contain user-supplied initialization data (length at 4000h, data from 4001h on), see [the `_DRVRO` function call](Nextor%203.0%20Programmers%20Reference.md#315-driver-operations-_drvro-7fh) for the details.
