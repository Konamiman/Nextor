# Better support for floppy disks

## The problem

Originally MSX-DOS was designed with floppy disks in mind, and as such it lacked the features needed to support more modern storage devices; for example it didn't handle partitions and had no concept of "devices", only "drives".

Nextor is built on top of MSX-DOS with the main goal of addressing these shortcomings, but it never got real support for floppy disks handled by Nextor drivers. For example there's no way to format disks, and Nextor will always try to manage partitions even when floppy disks never have these.

## The solution

This is a somewhat ambitious plan that aims at adding (or "restoring", if we compare it with MSX-DOS) proper floppy disk support in Nextor, that meaning supporting floppy disk drives controlled by Nextor kernels. The details come later, but in a nutshell:

- Drivers should be able to tell Nextor that a given device is a floppy disk drive.
- Nextor should not attempt to manage partitions in a floppy disk.
- Nextor should allow to format floppy disks.
- The single sided/double sided disks shenanigans should be supported.
- MSX-DOS drivers included a "stop motor" feature that should be made available too.
- The "ghost drive" mechanism, which allows to transfer files between two disks when one single drive is available, should be present too.

Details on each of these points follow.

## Identifying a floppy disk drive device

The "Get device parameters" device query currently returns:

```
; +7 (1): Flags:
;         bit 0: 1 if the device is removable.
;         bit 1: 1 if the device is read only. A device that can dinamically
;                  be write protected or write enabled is not considered
;                  to be read-only.
;         bit 2: 1 if the device is a floppy disk drive.
;         bit 3: 1 if this device shouldn't be used for automapping.
;         bits 4-7: must be zero.
```

so this part is already covered by the flag in bit 3.

Whenever a device is mapped to a drive Nextor should include a flag indicating that the device is a floppy disk drive.

- In DOS 2 mode, a new flag can be defined in `UD_DFLAGS`. 
- In DOS 1 mode, there's a `UD1_RELATIVE_DRIVE` field in the `KERNEX_DOS1` table. Bit 7 could be used as the "floppy disk drive" flag.

## No partitions

When a device is identified as a floppy disk drive:

- Nextor will always assign absolute sector 0 of the device to the drive at mapping time.
- FDISK will list the device but will not allow to operate on it, same as it happens now with e.g. offline devices.
- This implies that a boot time the device will get exactly one drive assigned (but later w'll discuss about ghost drives).
- F_GPART will always return the same result as for a regular block device that doesn't have any partition defined.

## Formatting support

In MSX-DOS formatting is a three step process:

1. The kernel retrieves the address of a string (fixed address in ROM) with the formatting options from the driver, and prints it verbatim for the user to choose.
2. The kernel invokes the driver's format routine, passing the chosen option.
3. The driver writes a MSX-DOS 1 compatible boot sector and FAT to the disk, then MSX-DOS 2 rewrites the boot sector updating some of the fields.

In MSX-DOS 2 there's a function call that handles this:

```
3.83   FORMAT A DISK (67H)

Parameters:    C = 67H (_FORMAT)
               B = Drive number (0=>current, 1=>A:)
               A =    00H       => return choice string
                      01H...09H => format this choice
                      0AH...FDH => illegal
                      FEH, FFH  => new boot sector
              HL = Pointer to buffer (if A=1...9)
              DE = Size of buffer (if A=1...9)
Results:      A = Error
              B = Slot of choice string (only if A=0 on
                  entry)
             HL = Address of choice string (only if A=0
                  on entry)
```

(more details in docs/DOS2-FCS.TXT)

Nextor added a few more choice options, see https://github.com/Konamiman/Nextor/blob/v2.1/docs/Nextor%202.1%20Programmers%20Reference.md#27-_format-67h

We can follow a similar approach but there are a couple of things that can be improved:

- The most popular floppy disk format for MSX is by far the 3.5" disk. We could add built-in support for these, while also allowing other formats.
- ROM encoded strings aren't nice, especially when we have the driver in a different ROM bank. We need a RAM based approach.
- No need to pass a RAM buffer, the driver can allocate RAM in the stack as needed.

And based on that we could implement the following:

### New driver queries

First a query to get the formatting choices:

```
Input:  B  = Device number
        HL = Buffer address (used only if B=255 at output)
        D  = Buffer length (used only if B=255 at output)
Output: A = Ok, or
            QUERY_INVALID_DEVICE, or
            QUERY_NOT_IMPLEMENTED if device is not a floppy disk drive or formatting is not supported
        B = Choice type:
            0: Single choice, no string to display
            1: Single side, double density; and double side, double density
            2: Single side, double density; double side, double density; and double side, high density
            255: Custom, up to D string bytes have been copied to buffer at HL
```

Then a query to do the actual formatting:

```
Input:  B  = Device number
        C  = Formatting choice
Output: A = Ok, or
            QUERY_INVALID_DEVICE, or
            QUERY_NOT_IMPLEMENTED if device is not a floppy disk drive, formatting is not supported, or an unknwon choice is passed, or
            any read-write error code (see source/kernel/drivers/StandaloneASCII8/driver.mac)
```

The driver should still be placing a standard MSX-DOS 1 boot sector, FAT and root directory in the disk once it's formatted (these can be hardcoded, see e.g. the "BOOT_*" labels in https://github.com/Konamiman/RookieDrive-FDD-ROM/blob/master/msx/bank1/choice_dskfmt.asm, these are for 3.5" disks).

### Changes in function calls

The "Format a disk" function call (67h, _FORMAT) can get the following changes depending on the input value at A:

- 0 (return choice string): this will work only on drives assigned to MSX-DOS kernels, since in Nextor the procedure is not "extract string from ROM" but "the drivers copies a string to a buffer". So it's the MSX-DOS 2 behavior unchanged.
- 1 to 9 (format this choice): works for MSX-DOS and Nextor drives. For the later, no buffer or buffer length is passed to the driver.
- 80h: New option. It's equivalent to "get choices", but a buffer address is passed in HL and a buffer length is passed in D, then the choices string is copied to that buffer. For Nextor drives: the driver itself copies the string. For MSX-DOS drives: Nextor extracts the string from ROM and copies it to the buffer.
- FBh to FFh: These can work for both MSX-DOS and Nextor drives.

"Work on Nextor drives" above means for drives that are assigned to devices that are floppy disk drives, for any other kind of drive it will return an error. Note that in Nextor 2 the FBh to FFh options work on any drive: this is dangerous and in Nextor 3 this should be restricted to only MSX-DOS drives and Nextor drives that are floppy disks.

The idea is that Nextor-aware applications willing to format floppy disks will use option 80h instead of option 0 to get the choice string. Old applications that support disk formatting won't be able to format disks on Nextor drives, but actually disk formatting support in user applications is not very common.

Nextor will continue adjusting the disk boot sector to MSX-DOS 2 format after the driver has added a MSX-DOS 1 format.

### Changes in CALL FORMAT

The CALL FORMAT command in BASIC can work for all kind of drives:

- For MSX-DOS drives: it gets the choices by invoking _FORMAT with A=0 and extracting them from ROM.
- For Nextor drives: it gets the choices by invoking _FORMAT with A=80h and having them copied to a temporary buffer in RAM.

Again, for Nextor drives, it should work (show the drives as choices) only for those that are floppy disk drives.

### New CALL QFORMAT command

A new "quick format" command could be provided to easily wipe any content from a floppy disk by just rewritting the boot sector, FAT and root directory as if the disk had been freshly formatted (again, only for MSX-DOS drives and Nextor drives that are floppy disks). The disk must have a boot sector with proper disk parameters so that Nextor knows how to initialize the disk (same as when the disk is initialized after physical formatting).

## Single side vs double sided

For floppy disk drives that support single sided and double sided disks, the drive needs to be provided with information about which type is accessing, since the procedure for the physical access to the disk changes. In MSX-DOS this is handled by keeping a copy of the media ID for the disk in the DPB for the drive, and passing it to the driver for each read/write operation.

In Nextor we can do the same: pass the media id to the READ_WRITE driver routine in register C (currently unused), but only when the device is a floppy disk.

## "Stop motor" feature

That one is quite straightforward: add a new "stop motor" driver query that just gets the device number, and call it whenever MSX-DOS would have called the equivalent MSX-DOS driver routine if the drive is a floppy disk.

## Ghost drives

Ok, this is a tricky one.

### What's this thing?

MSX-DOS has the concept of "ghost drives" to allow transferring files between disks when the computer has only one disk drive. It works as follows:

- The driver reports that it controls two drives (except if CTRL is pressed at boot, then it only reports one).
- The driver keeps an internal flag indicating which of the two drives was last accessed.
- When a read/write operation is requested for a drive different from the one that was last accessed, it invokes a routine (provided by the MSX-DOS kernel) that shows a "Insert disk for drive X:" message and waits for the user to press a key.

We need a similar mechanism for Nextor now that it support floppy disks.

### Ghost drive at boot, or not?

The first question is: when to provide a ghost drive for a drive that identifies as a floppy disk at boot time?

Idea: when Nextor detects that one of the scanned devices for a given driver is a floppy disk drive, query information for all othe other devices in the same driver. If none of them is a floppy disk drive, then assign a ghost drive for the device being originally queried (the drive having the following letter). Additionally, when a ghost drive has been assigned for a driver, don't assign any more for any other driver.

This more or less mimics the original behavior of MSX computers: when only one disk drive was present, a ghost drive was assigned; when two drives were present, no ghost drive was assigned.

There's still the possibility of a system having two disk drives, but controlled by different Nextor kernels: in this case, a ghost drive will be assigned but it shouldn't; however, this will be a rare case and trying to deal with it would be overkill.

### Controlling the ghost drive behavior

While in MSX-DOS it was the disk driver itself who handled all the procedure, Nextor itself should handle everything instead, freeing the driver from this hassle. This implies storing information about:

- Which drives are floppy disk drives.
- The drive that is a ghost drive, if any; and which drive it is "ghosting".
- The last drive of the normal/ghost pair that was accessed.

In MSX-DOS 2 mode this should be easy, we just add new flags to `UD_DFLAGS` and/or assign new internal variables. In MSX-DOS 1 this gets more tricky, as everything is stored in the `KERNEX_DOS1` structure which is quite packed already. Earlier I suggested using bit 7 of `UD1_RELATIVE_DRIVE` to signal that a drive is a floppy disk drive, we could complement this with bit 6 to indicate that the drive is a ghost drive, and bit 5 to signal that the drive was the last one accessed, or something like that. We still would have to figure out how to pair the ghost drive with its "normal" drive - we could assume that the first non-ghost floppy disk drive available is the one, but that could cease to be true if drive mappings are changed manually after boot. That's something to think about.

So on drive access, Nextor itself would take care of checking if the accessed drive is one of a normal/ghost pair, if the drive being accessed is not the same as the one accessed the last time, showing the message, and updating the "last drive of the pair accessed" status information.

### Customizng the message

MSX-DOS 2 offers a "Define disk error handling routine" to customize how disk errors are handled, see "3.80   DEFINE DISK ERROR HANDLER ROUTINE (64H)" in docs/DOS2-FCS.TXT. It would be good to have a similar mechanism to customize the message shown when a disk change is needed. A new dedicated function call could be defined for that, or the existing _DEFER routine could be reused with a dedicated new error code, but I'm not sure if this could break existing applications that make use of this routine already.

## Phew!

That's all I have in mind. Far from trivial, I know. As usual: read it, analyze it, ask questions, raise concerns, suggest improvements.
