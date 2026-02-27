# Better support for floppy disks, part 2

This document adds some planning bits that were left out in PLAN.md, related to mapping related functions when FDD/ghost drives are at play.

## Changes in _GDLI and related tools

The `_GDLI` function call should be extended so that it returns proper information about ghost drives too. It could be like this:

```
; Returns:    A = Error code
;             Buffer filled with this information:
;                +0 (1): Drive state
;                        0-4: as before
;                        5: Assigned to a ghost drive
;             When assigned to a ghost drive (+0 = 5):
;                +1 (1): Main drive this one is ghosting (0 = A:, etc)
```

Note: currently the ghost drive for drive X will always be drive X+1, but we might change this in future versions of Nextor, so better to explicitly return information about which one is the "master" drive.

The `DRVINFO.COM` and `CALL DRVINFO` tools should be modified to show the appropriate information:

```
Drive X: is assigned to:
Ghost drive of Y:
```

where Y is taken from what `_GDLI` returns instead of assuming it's X-1.

## Changes in _MAPDRV and related tools

The `_MAPDRV` function should be modified so that when a drive is mapped to a device that is a FDD, the next drive is assigned as its ghost drive only if all of the following is true:

- The drive being assigned is not H:
- The next drive is controlled by a Nextor kernel currently unmapped
- In DOS 1 mode: the next drive is controlled by a Nextor kernel (it's on KERNEX_DOS1)
- No other drive is currently mapped as a ghost drive in the system

If an application wants to know if a ghost drive has been mapped in after mapping a FDD drive, it needs to run `_GDLI` on the next drive to find out.

Also, when changing the mapping of a drive that's part of a main/ghost pair (unmapping, or mapping to a different device):

- When changing the mapping of the ghost: the flags corresponding to the main drive need to be updated to reflect the fact that it doesn't have a ghost companion anymore.
- When changing the mapping of the main drive: the ghost drive should be unmapped.

To be kept in mind: the current limitation of MAPDRV in DOS 1 mode: only a device in the same kernel where the drive was mapped at boot time can be mapped manually afterwards (although to be honest I don't remember why this limitation exists).

## The "device mutation" case

It's possible for a device to mutate from non-FDD to FDD and viceversa. For example an USB controller could have a pendrive connected at boot time, and later on the user could extract it and insert a FDD drive.

It would be great to check if this case is supported, assuming that the devices are flagged as removable and that the "FDD" flag is always reported correctly by the driver. My gut feeling is that this might be well supported in DOS 2 mode but not that much in DOS 1 mode, and that would be fine.

## Task

Analyze these changes. Do they make sense? Is there something I have missed? Something that can be improved in the design?


--------------------------------

Good plan overall. Here's my analysis:

## _GDLI Changes

The state 5 approach is clean and backward-compatible. One thing to consider:

**Should _GDLI also indicate ghost status on the master drive?** Currently the master would return state 1 (normal block device) with no indication it's paired. Since the ghost won't always be drive X+1 in the future, an application querying the master can't discover the ghost without scanning all drives. Options:
- Add a flag bit to the existing state 1 response (e.g., bit 7 of +0, or a flag in a spare byte)
- Or just document that applications should scan _GDLI for state 5 to find ghost relationships

The minimal approach (no master indication) is fine for now since there's at most one ghost pair system-wide ΓÇö a single scan of 8 drives is cheap.

**For the ghost's buffer content**: returning only +0 (state=5) and +1 (master drive) is good. The caller can always do _GDLI on the master to get full device info.

## _MAPDRV Changes

The four conditions for auto-ghost creation are solid. A few observations:

**Condition 2 clarification**: "The next drive is controlled by a Nextor kernel currently unmapped" ΓÇö after an unmap (B=0), the UD is freed via `FR_P2` and invalidated. What survives to tell us the drive "belongs" to a Nextor kernel? Probably the DRVTBL slot entry. You'd check that the next drive's DRVTBL slot is a Nextor kernel (via `IS_NEXTOR_KER` or similar), even though the UD is gone. The UD would then need to be re-created (like `CREATE_UD` in the B=1 path) for the ghost.

**DPB for the ghost drive**: When _MAPDRV creates a ghost at runtime, does the next drive already have a DPB? If it was unmapped, its $DPBLIST entry might still point to the boot-allocated DPB (unmap doesn't free DPBs ΓÇö they're allocated once by `SET_DPBS` and never freed by `ALLOC`). So no new DPB allocation should be needed in most cases. Worth verifying this assumption though ΓÇö if the DPB pointer is null (drive was never in $DPBLIST at boot), allocation would be needed.

**Ghost cleanup ordering**: When remapping the master to a different FDD device, the cleanup of the old ghost should happen before the new mapping. Then the auto-ghost creation logic runs and finds the next drive unmapped. This natural ordering works, but the code should explicitly handle the sequence: cleanup old pair ΓåÆ map master to new device ΓåÆ check FDD ΓåÆ potentially create new ghost.

**One edge case**: What if `_MAPDRV` B=0 (unmap) is called on a drive that's neither the ghost nor the master, but the ghost IS the next drive? This shouldn't trigger any ghost cleanup since the drive being unmapped isn't part of the ghost pair. The conditions in the plan handle this correctly since they check ghost pair membership explicitly.

## Device Mutation

I agree this is fine as a known limitation. The ghost mechanism relies on boot-time or explicit _MAPDRV setup. Detecting device mutation (non-FDD ΓåÆ FDD or vice versa) on every access would add overhead to a hot path. In DOS 2, the DPB/UD refresh on media change could theoretically detect it, but creating/destroying ghost pairs dynamically is complex and fragile.

**Recommendation**: Document that ghost pairs are established at boot time or via _MAPDRV. Device identity changes (USB hot-swap) don't auto-create or destroy ghost pairs.

## Missing Items

1. **DRVINFO.COM**: The plan mentions modifying DRVINFO.COM but its source doesn't seem to be in the kernel tree (it's likely a standalone tool). Only the `CALL DRVINFO` BASIC command (in `partit.mac:5135`) is in the kernel. Both need updating.

2. **_MAPDRV in DOS 1 ΓÇö KERNEX_DOS1 table mutation**: Modifying the KERNEX_DOS1 table at runtime (adding ghost state bytes, inserting entries) is more delicate than the DOS 2 path where we just manipulate UDs. The table is fixed-size after `MK_DVB_TBL` copies it. Worth thinking about whether _MAPDRV ghost creation in DOS 1 needs to modify KERNEX_DOS1 entries in-place (updating FAS bytes for ghost state) vs. something simpler.

3. **The "last accessed" initialization for _MAPDRV ghost creation**: Same as boot ΓÇö master should be marked as last accessed initially. This is implicit in the plan but worth calling out explicitly for implementation.

--------------------------------

_GDLI master flag: for simplicity let's not implement this, an application willing to detect master-ghost pairs needs to scan all drives and detect the ghost drive, which will then point to the master.

_MAPDRV: "The next drive is controlled by a Nextor kernel currently unmapped": right, let's change this to simply "the next drive is unmapped" (so if it's mapped to a MSX-DOS driver it can't be used either).

The DPB issue: let's keep it simple and only assign a ghost drive if a DPB already exists for the drive where the ghost drive would be mapped (so effectively this means that the drive has been mapped to something in the past). This limits even further the cases where a ghost drive will be mapped, but the normal case will be (or should be) that the user won't mess with the boot time mapping of a FDD drive.

Ghost cleanup: I might be misremembering, but doesn't "remapping" require an explicit unmapping followed by the actual mapping? Otherwise the kernel will throw a "duplicate mapping" error, right? In any case: yes, full master+ghost cleanup is required before attempting any new mapping.

Device mutation: I was referring not only to the ghost drive case, but also to the "device is a FDD" flag itself. Is that flag updated whenever a device change happens? If not, it might be interesting to implement that. But for now that's too much of an edge case to worry about, and we can always forbid drivers from reusing the same device number for FDD and non-FDD devices.

DRVINFO.COM: Yes I just noticed that we are currently in the kernel directory, but if you go to (root)/source/tools the code for MAPDRV.COM is there (and also for DRVINFO.COM, which also needs to be updated).

MAPDRV in DOS 1: yes, as part of the mapping process the appropriate flags in KERNEX_DOS1 need to be updated accordingly; this should be easily doable after we have determined whether a ghost drive will be assigned or not, right?
