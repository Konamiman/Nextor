# Drivers loadable in RAM - Full Conversation

## PLAN.md (Original Plan Document)

# Drivers loadable in RAM

A Nextor ROM consists of a base common kernel and a driver that's attached after the last ROM bank. So in order to have multiple drivers available in the system, each must be occupying a memory slot complete with its own copy of the kernel.

This projects aims at allowing to load (and unload) drivers in mapped RAM, where each driver will reside in one previously allocated 16K RAM segment and will have basically the same structure as drivers in ROM.

## DOS 2 mode only

This feature will only be available in the DOS 2 (normal) mode: loading drivers in RAM won't be possible in DOS 1 mode, at least initially.

## Pre-existing infrastructure

The idea of RAM loadable drivers was already present when Nextor was first designed, and therefore, the public API for developers of applications (function calls) already has built-in support for it at the "function signature" level:

- The `_GDRVR` function accepts a segment number in E; the returned information contains the segment number at offset +1 too.
- The information returned by `_GDLI` function contains the segment number at offset +2.
- The `_GPART` function accepts a segment number in B.
- The `_CDRVR` function accepts a segment number in B too.
- The `_MAPDRV` function accepts a segment number as part of the information block provided (at offset +1) when the mapping mode is "use specific mapping data".

In all cases using (or returning) the value FFh for the segment number means "the driver is in ROM". Since ROM drivers are currently the only type of driver supported, FFh is currently the only value accepted/returned by Nextor in these calls; after we implement the RAM drivers feature, this will no longer be the case.

## Driver structure

The structure for RAM drivers will be in principle the same as for ROM drivers: same `NEXTORv3_DRIVER` driver signature, same entry points, same behavior; drivers will always be accessed from page 1 (4000h-7FFFh addresses) in their corresponding RAM slot too. In particular, the TIMER_INT, OEMSTAT, BASDEV and EXTBIO routines should continue to work as expected (and the same for driver/device queries and READ/WRITE of course). However, there will be some differences:

- `DIRECT_0` to `DIRECT_4` entry points don't make much sense in a RAM driver, since any point of the driver can be called externally using the mapper support routines; but let's keep them in place as to not break the structure of the jump table (and implementing them is optional anyway).
- The "Get driver initialization parameters" driver query doesn't make much sense, since none of the parameters passed to the routine have meaning after the system has booted (no work area to allocate, no requested drive count to worry about). However, the "TIMER_INT should be hooked" flag that's returned is still significant.
- Similarly for the "Initialize driver" routine: HL and C passed as input don't make sense, but the driver does need a way to be initialized.
- So maybe the best alternative is: never call the above driver queries for RAM drivers, and instead define two new "initialize RAM driver" and "shut down RAM driver" routines, exclusive for RAM drivers; this also allows "dual" drivers (work in ROM and RAM with the same code) to tell apart where are they:

```
Driver query: initialize RAM driver

Input:  DE = Routine for printing a character
Output: A  = QUERY_OK or QUERY_NOT_IMPLEMENTED (equivalent)
             QUERY_INIT_ERROR
        B  = Flags
             0: TIMER_INT should be hooked
             1-7: Must be zero
```

```
Driver query: shut down RAM driver

Input:  DE = Routine for printing a character
Output: A  = QUERY_OK or QUERY_NOT_IMPLEMENTED (equivalent)
```

ROM drivers should always return QUERY_NOT_IMPLEMENTED (and do nothing else) on these queries. RAM drivers should do the same for the old "get driver initialization parameters" and "initialize driver" routines; the exception to this is drivers that can work on ROM and on RAM with the same codebase.

A second call to "initialize" for an already initialized driver should result in the driver doing noathing and returning no error, same for a call to "shut down" for a driver that's not currently initialized.

## New function call

We'll need a new function call for applications to initialize/shut down RAM drivers:

```
_DRVRO: Operate on a RAM driver

Input:  A  = Slot number
        B  = Segment number
        DE = Printing routine, or 0 for "none"
        H = Operation:
            1: Initialize and register a new driver
            2: Shut down an existing driver
Output: A = Error code
```

For the initialization part, this function call will:

1. Switch the slot and segment in page 1.
2. Check the driver signature ("NEXTORv3_DRIVER"). If not present, return an .IDRVR error.
3. Invoke the "initialize RAM driver" routine, passing to it either the passed DE or a pointer to a "ret" if it's 0.
4. If the routine returns anything but "ok" or "not implemented", return an .INITE error.
5. Register the driver as a known RAM driver in the system, including the "hook the timer interrupt routine" flag (more on that later).

For the shutdown part:

1. If there's no RAM driver registered in the passed slot+segment, return an .IDRVR error.
2. Unmap all the drives that are mapped to devices controlled by that driver.
3. Switch the slot and segment in page 1.
4. Check the driver signature ("NEXTORv3_DRIVER"). If not present, jump to step 5.
5. Invoke the "shut down RAM driver" routine, passing to it either the passed DE or a pointer to a "ret" if it's 0.
6. Unregister the driver as a known RAM driver from the system (more on that later).

Notes:

1. When initializing a driver, the function call will NOT check that the passed slot number is actually RAM, that the segment exists, or that the segment has been allocated: it's the responsibility of the caller to previously allocate the segment in system mode, load the driver code in it, and then invoke the function call. The lack of a proper driver signature should be enough to detect the case of a wrongly specified segment number; but we won't be able to detect if a segment gets driver code loaded without having been actually allocated. It's an acceptable tradeoff for simplicity.
2. Similarly, when shutting down a driver, the function call will NOT free the segment: the caller must do that. The function call will, however, verify the driver signature again as a very basic integrity check for the driver (since it's in RAM it could get corrupted by malformed driver code or by misbehaving external code) and skip the shut down routine call if the driver seems corrupted.
3. A driver can't prevent its own unregistration: no matter what error code the shut down routine returns, the driver always gets unregistered. The shut down request is more like a courtesy warning to allow freeing resources.
4. Another case that we won't cover is a misbehaving application freeing a RAM segment where a driver is currently installed. Application shoulnd't do that (and shouldn't free segments not allocated by themselves, to start with).
5. Step 3 should clear and invalidate all the disk buffers associated to the affected drives, but better to double check if that's actually the case.
6. If an unknown value is passed in H the function call will do nothing and return an .ISBFN error.
7. .INITE is a new error code that needs to be registered in codes.mac and get an equivalent error message for the _EXPLAIN function ("Initialization error"), a BASIC error code, and a BASIC error message.

## Changes in existing function calls

Function calls that accept or return driver slot names now must accept and return information about RAM drivers, as mentioned in "Pre-existing infrastructure". One detail: when `_GDRVR` is executed in driver enumeration mode, it should return the ROM drivers first, then the RAM drivers in roughly installation order. "Roughly" means that successive uninstalls and (re)installas of drivers might put drivers installed later "ahead" of drivers installed earlier (sepending of the data structure used to register installed drivers), and these might then be enumerated first; this is ok, no need to complicate things to account for that.

## Registering the drivers

In principle the only information that needs to be stored about a RAM driver is: slot number, segment number, and timer interrupt hooked flag (which can be one of the unused bits in the slot number). There could be a linked list of entries, with the pointer to the first entry being in a fixed variable in the data segment; this enables to load as many drivers as the availability of RAM segments allow.

The allocated RAM segments table maintained by the mapper support routines is probably some kind of linked list too, since it grows dynamically. So we could look into how that one works and use a similar approach.

## Invoking driver code

Right now, whenever the kernel needs to execute code in a driver (driver/device queries, EXTBIO hook, timer interrupt hook, etc) it uses the CALDRV function that's available in all the ROM segments at address 4048h.

Now the driver needs to check if a driver is in ROM or in RAM beforehand. If it's in ROM, use the existing mechanism; if it's in RAM, use the CALL_MAP routine that's available as part of the mapper support routines (see docs/Nextor 2.1 Programmers Reference.md, sections 5 and 5.1).

## The first 256 bytes

In ROM drivers the first 256 bytes of the ROM bank for the driver contains some utility routines to e.g. call another bank. None of these really apply ro RAM drivers, so for those the contents of this area will be undefined as far as the `_DRVRO` function is concerned; but we can reuse this area to pass some initialization parameters to the driver when it's installed (more on that follows).

## The CALL IDRIVER command

A new `CALL IDRIVER` ("install driver") BASIC command can be added with this syntax:

```
CALL IDRIVER[M](<driver file name>[,<init data>[,<init data>...]])`
CALL IDRIVER[M](<driver file name>,,<init data address>,<init data length>)`
```

This command will:

1. Parse the file name and the init data arguments, save them in memory. The `SECBUF` buffer should be long enough, as file paths can be at most 64 chars and we are taking up to 255 bytes of init data.
   - Init data values need to be evaluable as integers, otherwise an error will be thrown. Values where the high byte is 0 or FFh will be considered single-byte, otherwise double-byte and little-endian. Up to 255 bytes will be read.
   - If there's an empty value after the file name (two consecutive commas), then the address and length of a memory area with initialization data is assumed to be present after. Length is limited to 254, if it's larger we'll assume 254.
2. Allocate a RAM segment in system mode, with preference for the non-primary slots. Terminate with "Not enough memory" error if that's not possible.
3. Open the file and read it into the RAM segment at its address 0100h (so up to 3F00h bytes will be read). Terminate with "File not found" or the appropriate disk error if something goes wrong.
4. Set the first 256 bytes of the segment to zero.
5. Copy the initialization data (if any) to address 1 in the RAM segment. Set the length of the data in address 0.
6. Initialize the driver by invoking the `_DRVRO` function call, passing a suitable printing routine in DE. Terminate with "Invalid driver" or "Initialization error" if something fails.
7. Terminate without error, printing the slot and segment numbers of the allocated segment.

The `IDRIVERM` variant will automatically map the first free drive to the first existing device controlled by the newly installed driver (with preference for partitions with the active flag set, as usual) and print this information too ("Drive X: mapped to device N, Device name"). Of course this will fail if no free drives are available.

Note that an error after step 2 implies that the segment that has been allocated must be freed. That's why we are parsing all the arguments upfront: when BASIC can't parse an argument it jumps straight to the BASIC error handler, and intercepting that would involve complexity that we are better off avoiding.

## The CALL UDRIVER command

A new `CALL UDRIVER` ("uninstall driver") BASIC command can be added with this syntax:

```
CALL UDRIVER(<slot>+4*<subslot>,<segment>)
```

This command will:

1. Parse the slot and segment arguments, error if they aren't numbers or are invalid (negative or bigger than 255).
2. Invoke the `_DRVRO` function call to trigger the driver's shut down query. If an .IDRVR error is returned, exit with "Invalid driver" error.
3. Free the segment. Remember that the shutdown query isn't allowed to fail, so unless we got .IDRVR in the previous step we always do this step.

## The DRVROP.COM tool

Complementarily, a DRVROP.COM tool should be developed (preferably in C) with this syntax:

```
DRVROP i <file name> [/s] [/m] [/d <data>[,<data>...]]
DRVROP u <slot>[-<subslot>] <segment> [/s]
```

Where:

- "i" will install a driver using the same procedure as `CALL IDRIVER`. Optional initialization data is in decimal by default, but can be hexadecimal when prepended with `#`.
- `/s` switch is for silent mode (passes a pointer to "ret" to the driver as the character printing routine).
- `/m` switch is for automatically mapping a drive, same as `CALL IDRIVERM` does.

Take a look at the other C tools to get a grasp on how they are structured and the build process for these.

## Changes in FDISK

The built-in FDISK tool shows a driver selection screen that should now include the RAM drivers. The syntax for displaying locations for RAM drivers can be `<slot>[-<subslot>]:<segment>`. Similarly to how it's done in the boot menu, driver names should be appropriately truncated to fit in one line, and in 40 columns mode the word "slot" for the "in slot..." part next to each driver name can be suppressed.

Now, there's in principle an issue here: until now we had up to four drivers, easily printable in one single driver selection screen. Now we have a theorically unlimited number of drivers, so that's no longer the case.

However, the existing driver selection screen still fits up to eight drivers. Having more drivers available (ROM and RAM drivers combined) will be a rare case, and the user needing to use FDISK on all of them will be even more rare; so let's leave the driver selection screen as is and stop querying for drivers when information about the first eight has been retrieved. This will be a documented limitation for the sake of simplicity of the FDISK code, and the user always has the option to load only the drivers where he needs to apply FDISK.

## Changes in CALL DRIVERS and DRIVERS.COM

The `CALL DRIVERS` command and `DRIVERS.COM` tool will already pick the RAM drivers for display as they are using `_GDRVR` in enumaration mode. However they need to be modified so that they show the proper segment number for RAM drivers. We can use the same notation as FDISK: `<slot>[-<subslot>]:<segment>`.

## Example RAM disk drivers

In order to test the feature and provide an example driver for developers, we can implement a very simple RAM disk driver that uses the free space in its own segment to store the data (I think this will be be at least 14KB). On initialization this driver needs to initialize the boot sector, FAT and root directory of its memory area used for the data (in FAT12); and there's no special action needed on shutdown.

Additionally, we could modify the Turbo-R FDD driver to be loadable in RAM. We have two options here: make the same code be aware of whether it's running in RAM or ROM (depending on how it's initialized), or compiling two separate versions (we can pass a RAM_DRIVER constant to the assembler and use conditional assembly within the same source code). I tend to think that the second option is better, but that should get some analysis.

A caveat is that the Turbo-R FDD driver is allocating page 3 space for a data transfer routine, which is something that a RAM driver can't do. So the routine could be  copied to stack space every time it's needed and executed from there. This imposes a slight penalty on performance, but it should be negligible compared to the typical FDD disk access times.

---

## Claude's Initial Analysis

### Questions

**1. Unit Descriptor (UD) storage for segment numbers**
When a drive is mapped to a RAM driver's device, the UD needs to store the segment number alongside the slot. Does the current UD structure already have a field for this, or does it need to be extended? The API functions mention segment numbers, but the internal storage hasn't been addressed.

**2. CALL_MAP across mapper slots**
The plan says to use `CALL_MAP` for invoking RAM driver code, with a preference for allocating segments in non-primary mapper slots. Does `CALL_MAP` work across different mapper slots, or is it primary-mapper only? If the latter, we'd need a wrapper that does slot switching + `PUT_P1` + call + restore, or we'd need to restrict driver segments to the primary mapper.

**3. Double-initialization of the same slot+segment**
The plan says "a second call to initialize for an already initialized driver should do nothing and return no error" — but is that the *driver's* behavior or the *kernel's* behavior? Should `_DRVRO` check its registration table and reject (or silently succeed) if the slot+segment is already registered, before even calling the driver?

**4. Function number for `_DRVRO`**
What function number will it get? And the new driver query IDs for "initialize RAM driver" / "shut down RAM driver" — do they need specific numbers?

**5. RAM driver work area**
ROM drivers get work area allocated via "Get driver initialization parameters" during boot. For RAM drivers, the plan skips that query. Are RAM drivers expected to use their own segment for persistent state (there's unused space between the code and the end of the 16K)? That should be stated explicitly so driver developers know.

**6. Printing routine accessibility**
The printing routine in DE is passed to the driver, which runs with its own segment in page 1. This routine needs to be at a page 0 or page 3 address to be callable from that context. Is that guaranteed? If the printing routine is kernel code in a ROM bank (page 1), it would be paged out.

### Potential Issues

**7. Interrupt safety during registration/unregistration**
When modifying the RAM driver list, an interrupt could fire and the timer handler could try to iterate the same list. Interrupts should be disabled during list modifications. Worth noting explicitly.

**8. TIMER_INT during shutdown**
Between step 2 (unmap drives) and step 6 (unregister) of the shutdown sequence, the driver is still in the list and its TIMER_INT could still fire. Should unregistration from the timer hook happen first, before calling the driver's shutdown routine?

**9. Warm reboot behavior**
On warm reboot, the kernel re-initializes but the RAM segments still contain driver code. The registration will be lost, so the drivers effectively disappear — but the segments remain allocated. Is that acceptable, or should there be cleanup? (I think it's fine, just worth documenting.)

### Suggestions

**10. Registration data structure**
Rather than a linked list (which needs dynamic memory management and is fragile), consider a fixed-size array in page 3. 8 entries × 3 bytes (slot, segment, flags) = 24 bytes is tiny and would fit in the safe area at F327+. The 8-driver limit already exists de facto (FDISK's display limit), so it aligns naturally. This is much simpler to implement and iterate during interrupts.

**11. Storing linked-list pointers in driver segments**
If you do go with a linked list, avoid storing the "next" pointers in the driver segments themselves. A misbehaving driver could corrupt the list. Keep all management data in kernel-controlled space.

**12. Shutdown sequence order**
I'd suggest reordering shutdown to:
1. Unregister from timer interrupt hook (stops TIMER_INT calls immediately)
2. Unmap all drives
3. Call the driver's shutdown routine
4. Remove from registration table

This is safer — the driver can still clean up in step 3, but won't receive unexpected TIMER_INT calls during or after shutdown.

**13. .IDRVR for already-registered check on init**
On initialize, if the slot+segment is already registered as a RAM driver, it might be better to return a specific error (or .IDRVR) rather than registering it twice. The caller probably made a mistake.

### Things That Look Good

- The "dual driver" approach (same code for ROM and RAM, distinguished by which init query succeeds) is clean.
- Using the first 256 bytes for init data is clever reuse.
- The graceful shutdown model (driver can't veto unregistration) prevents hangs.
- Keeping the feature DOS 2-only is the right call given the mapper dependency.
- The FDISK 8-driver limit is a pragmatic and reasonable tradeoff.

Overall the plan is solid. The biggest architectural questions are #1 (UD structure) and #2 (CALL_MAP across slots) — everything else is refinement.

---

## User's Responses to Questions

1. The unit descriptor has a `UD_RAM_SEGMENT` field that currently always holds the value FFh, so we can use it.

2. Yes, CALL_MAP takes slot number, segment number, routine address and input parameters for the routine; so that's already covered.

3. That's driver's behavior. The kernel will never initialize again a driver that's already initialized, but the driver needs to cover the case of that being done by an external application.

4. Let's use numbers consecutive to what's there now. So 7Fh for _DRVRO, 6 and 7 for the init and shut down routines, and AFh for the .INITE error.

5. Yes, RAM drivers are expected to use their own RAM segment for any data storage, including the first 256 bytes when they don't hold initialization data that needs to persist.

6. The printing routine needs to be in page 0, 2 or 3 indeed. The driver doesn't need to worry about that, it can assume that the routine is available when the init/shut down query is invoked.

7. Yes, indeed, better to disable interrupts during driver registration/shutdown.

8. Yes, good point, first make sure that the timer interrupt routine is unregistered, then do everything else.

9. You mean a computer reset (warm or not)? The segments won't be allocated anymore in this case: segment allocations don't survive resets.

10 and 11. What about a "clustered" linked list? So initially we allocate a memory area that's enough for 4 entries, and a pointer to the next cluster right after the last entry, the pointer will be initially zero. If that initial table gets full, then we allocate another one with another 4 entries, linked from the first one. Usually there will be no more than 4 drivers loaded in RAM so this approach is memory-effective yet flexible.

When a driver is uninstalled its entry in the table is set to zero. If a new driver is installed afterwards we reuse that entry. I think there's no need to worry about compacting or "garbage collecting" tables.

Also I think there's no need for an explicit flags byte, the only needed flag is the one for the timer interrupt and we can use one of the unused bits of the slot number for that.

12. Yes, good point.

13. Indeed. I think that returning .IDRVR is enough, applications aren't expected to ever do this double initialization.

---

## Additional Clarification on Data Structure

All the clusters for the linked list, including the first one, must be allocated in the kernel data segment, in the same way as the other kernel data structures are (e.g. disk buffers and file descriptors). The pointer to that first cluster must be at a fixed location, also in the kernel data segment (e.g. after GHO_PAIR; see source/kernel/kvar.mac).

---

## Agreed Design Decisions Summary

- **Function numbers**: _DRVRO = 7Fh, init RAM driver query = 6, shut down RAM driver query = 7, .INITE error = AFh
- **UD storage**: Use existing `UD_RAM_SEGMENT` field (currently always FFh)
- **Driver invocation**: CALL_MAP (supports slot number, segment number, routine address, input parameters)
- **Registration data structure**: Clustered linked list in kernel data segment. Each cluster holds 4 entries (2 bytes each: slot with timer flag in unused bit + segment) plus a 2-byte pointer to next cluster. First cluster pointer at a fixed location in kernel data segment (after GHO_PAIR in kvar.mac). Zero entries = empty/uninstalled, reusable.
- **Interrupt safety**: Disable interrupts during registration/unregistration
- **Shutdown sequence**: (1) Unregister from timer hook, (2) Unmap drives, (3) Call driver's shutdown routine, (4) Remove from registration table
- **Double-init check**: Kernel returns .IDRVR if slot+segment already registered
- **RAM driver work area**: Drivers use their own 16K segment, including the first 256 bytes after init data is consumed
- **Printing routine**: Must be in page 0, 2, or 3; driver assumes it's available
- **Reset behavior**: Segment allocations don't survive resets, so no cleanup needed
