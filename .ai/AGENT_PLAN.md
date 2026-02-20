# Better Support for Floppy Disks - Implementation Plan (Phases 1-3)

## Context

The user's specification (`.ai/PLAN.md`) describes adding proper floppy disk support to Nextor for devices controlled by Nextor drivers. Currently, Nextor has no formatting support, no single/double-sided handling, no ghost drives, and always attempts partition scanning even for floppies.

**This iteration covers Phases 1-3 only:** floppy flag infrastructure, no-partition behavior, and media ID pass-through. Formatting, stop motor, and ghost drives will be addressed in subsequent iterations.

## Issues Found in the Specification

### 1. UD1_RELATIVE_DRIVE bit 7 conflict (DOS 1 mode)

The spec says: *"Bit 7 could be used as the 'floppy disk drive' flag"* for `UD1_RELATIVE_DRIVE`.

**Problem:** Bit 7 is already the "partition changed" flag:
- Set at `bank4/partit.mac:3694`: `set 7,(ix+UD1_RELATIVE_DRIVE##)`
- Checked at `drv.mac:615-618`: tests bit 7 to detect partition changes
- Masked at `drv.mac:781`: `and 01111111b` to extract the drive number

**Proposed fix:** Use bit 6 instead. The relative drive number only needs bits 0-4 (max 31, in practice < 8). Bit 5 can be reserved for the ghost drive flag. The mask at `drv.mac:781` would change from `01111111b` to `00011111b`.

### 2. Register C in READ_WRITE is available (confirmed)

The spec correctly states C is "currently unused". LUN was removed in Nextor 3 (see `kvar.mac:239`: *";field 1 ;LUN number was here in Nextor 2"*). The driver's READ_WRITE interface (`drivers/StandaloneASCII8/driver.mac:520-542`) does not list C as input.

In `rdwr_devcmd` (`val.mac:2524`), C is used as scratch for the page-2 segment number, but this can be reorganized to free C for media ID.

### 3. Stop motor mechanism already exists

The `$MTOFF` routine (`init.mac:2096-2124`) already iterates through all driver slots calling `MTOFFENT`. For Nextor drivers, `MOF_ENTRY` in `drv.mac:705-710` is currently a no-op (`ret`). This just needs to be wired to a new driver query.

### 4. Ghost drive boot key already anticipated

`bootmenu.mac:1367` already defines: `"Disable FDD ghost drives (CTRL)"`. The infrastructure for this boot key exists.

### 5. "Insert disk" message infrastructure exists

`bank1/msg.mac:125-127` has the messages, and `init.mac:2240-2263` has the PROMPT routine that displays them and waits for a keypress.

## Recommended Approach

### UD_DFLAGS layout (DOS 2 mode)

Repurpose the unused UF_HPL (bit 1) as UF_FDD. Add new bits for ghost drives:

| Bit | Name | Meaning |
|-----|------|---------|
| 0 | UF_DBD | Drive assigned to a Nextor driver |
| 1 | UF_FDD | Device is a floppy disk drive (was UF_HPL, unused) |
| 2 | UF_RMV | Drive assigned to a removable device |
| 3 | UF_PAP | Partition assignment pending |
| 4 | UF_FOK | UD_FSEC value is valid |
| 5 | UF_GHO | Drive is a ghost drive |
| 6 | UF_GLA | Drive was last accessed in ghost pair |
| 7 | (spare) | |

### UD1_RELATIVE_DRIVE layout (DOS 1 mode)

| Bits | Meaning |
|------|---------|
| 0-4 | Relative unit number |
| 5 | Ghost drive flag |
| 6 | Floppy disk drive flag |
| 7 | Partition changed flag (existing) |

Mask at `drv.mac:781` changes from `01111111b` to `00011111b`.

### New device queries

Add as standard device queries (DEVQ_* in `data.mac`):

- **DEVQ_GET_FORMAT_CHOICES (5)**: Input B=device number, HL=buffer address, D=buffer length. Output A=error, B=choice type (0/1/2/255).
- **DEVQ_DO_FORMAT (6)**: Input B=device number, C=choice. Output A=error.
- **DEVQ_STOP_MOTOR (7)**: Input B=device number. Output A=error.

Drivers return `QUERY_NOT_IMPLEMENTED` by default.

### Ghost drive pairing

A ghost drive is always the next drive letter after its real partner. State tracking:
- DOS 2: UF_GHO and UF_GLA bits in UD_DFLAGS on both drives
- DOS 1: Bit 5 of UD1_RELATIVE_DRIVE + 1 byte in page 3 spare area for "last accessed" state
- Only one ghost pair per system (flag prevents multiple allocations)

## Phased Implementation

### Phase 1: Floppy Flag Infrastructure
**Files:** `const.inc`, `kvar.mac`, `bank4/partit.mac`, `drv.mac`

1. Rename `UF_HPL`/`UFM_HP` to `UF_FDD`/`UFM_FD` in `const.inc:244,253`
2. Update comment in `kvar.mac:163`
3. In `partit.mac` AUTO_ASSIGN flow (~line 985-1000, 1350-1357): extract floppy flag (bit 2 of device params byte at offset+7) and store in UD_DFLAGS as UFM_FD, similarly to how removable flag is already extracted
4. In `partit.mac` for DOS 1 mode: set bit 6 of UD1_RELATIVE_DRIVE when device is floppy
5. Update masks in `drv.mac:781` from `01111111b` to `00011111b`; verify `drv.mac:615-618` still works (it tests bit 7 specifically, so no change needed there)

### Phase 2: No-Partition Behavior for Floppies
**Files:** `bank4/partit.mac`, `bank2/val.mac`

1. In AUTO_ASSIGN (~line 965-980): if device is floppy, skip partition scanning entirely, set first absolute sector to 0
2. In BUILD_UPB (`val.mac`): if UF_FDD set, skip partition search and read sector 0 directly
3. In F_GPART: for floppy devices, return "no partition" result
4. Verify FDISK already handles floppy devices correctly (shows them but prevents operations)

### Phase 3: Media ID Pass-Through
**Files:** `bank2/val.mac`, `drv.mac`, driver template

1. In `rdwr_devcmd` (`val.mac:2516-2552`): reorganize register usage to free C, then load C with media descriptor byte from the unit descriptor's DPB when UF_FDD is set (0 otherwise), before calling CALL_DRV
2. Update driver READ_WRITE documentation to note C = media ID for floppy devices
3. In `drv.mac` DOS 1 path: similarly set C from DPB media descriptor

### Phases 4-6 (deferred to next iteration)

Formatting support, stop motor, and ghost drives will be planned and implemented after Phases 1-3 are verified.

## Verification

- Build with `make` after each phase
- Phase 1: Verify the floppy flag is correctly stored in UD_DFLAGS (bit 1) for DOS 2 and UD1_RELATIVE_DRIVE (bit 6) for DOS 1 when a floppy device is detected
- Phase 2: Boot with a floppy device driver, verify it gets exactly one drive with first absolute sector = 0 (no partition scan); verify F_GPART returns "no partition" for floppy devices
- Phase 3: Verify media ID from the DPB reaches the driver's READ_WRITE in register C for floppy devices, and C=0 for non-floppy devices

## Critical Files (Phases 1-3)

| File | Phase | Changes |
|------|-------|---------|
| `source/kernel/const.inc` | 1 | Rename UF_HPL/UFM_HP to UF_FDD/UFM_FD |
| `source/kernel/kvar.mac` | 1 | Update UD_DFLAGS bit 1 comment |
| `source/kernel/bank4/partit.mac` | 1, 2 | Extract floppy flag from device params, skip partitions for floppies, set DOS 1 floppy bit |
| `source/kernel/bank2/val.mac` | 2, 3 | Skip partition search in BUILD_UPB for floppies; reorganize rdwr_devcmd to pass media ID in C |
| `source/kernel/drv.mac` | 1, 3 | Update UD1_RELATIVE_DRIVE masks; set C from DPB in DOS 1 read/write path |
| `source/kernel/drivers/StandaloneASCII8/driver.mac` | 3 | Update READ_WRITE documentation for C register |
