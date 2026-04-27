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

## And that's it!

This is an ambitious feature. To start with, verify if the plan is sound: does all make sense? Is there something wrong or missing? Is there something that could be improved? Do you have any questions? Once that's all clear we'll lay out an implementation plan.
