# "Drive per partition" feature - Design Session

## Initial Task

User asked to review `.ai/PLAN.md` which describes a "drive per partition" feature for Nextor.

### PLAN.md contents:

> # "Drive per partition" feature
>
> ## The problem
>
> Right now, at boot time Nextor will assign one drive per device reported as existing for each driver (except those that are offline and are non-removable). Users have expressed that it would be convenient to have several partitions already mapped to drives at boot time.
>
> ## The solution
>
> Idea: at boot time, and while querying drivers for available devices, also scan the partitions chain of each device, and for that given device assign one drive for each partition that is marked as active (bit 7 of first byte in the partition table is set). If the device is offline (and is removable), there's no valid partition table, or there are no active partitions, one single drive is assigned for that device as before.
>
> The built-in FDISK tool allows to toggle the active status of existing partitions, so that's a convenient method for users to lay out the boot state.
>
> ## Nuances and technical details
>
> - There's no need to effectively assign drives to partitions at boot time: that can happen at first access, as it's being the case now. However this can lead to surprising results, depending on the order in which the user accesses the drives after boot. Maybe the internal drive descriptor could somehow encode a hint about the partition index number it's expected to receive?
> - This needs to work for both DOS 1 and DOS 2 modes, even though it's understood that FAT16 partitions won't be available in DOS 1 (but any partition type having the active bit set should be counted at boot time).
>
> ## Task for you
>
> Validate the idea and the design. What works and what can be improved? Do you have any questions?

---

## Codebase Exploration

Two exploration agents were launched in parallel to understand the existing architecture:

### Boot-time drive assignment flow

**DOS 2 mode:**
1. `KINIT` (`bank2/kinit.mac:101-255`) reads `DRVTBL`, calls `INIT_UNITS` to pre-allocate one UD per reported drive
2. Then `AUTODRV` (`bank4/partit.mac:467`) is called via `CALBNK` to populate those pre-allocated UDs
3. `AUTODRV_DOS2` (line 643) iterates `UNIT_TAB`, for each Nextor UD calls `AUTO_ASSIGN`
4. `AUTO_ASSIGN` (line 840) scans devices and partitions, assigns one partition to the UD

**DOS 1 mode:**
1. `DRVS1` (line 5834) optionally limits each Nextor driver to 1 drive ("5" key)
2. `AUTODRV_DOS1` (line 532) loops through drivers, for each driver loops through drives, calls `AUTO_ASSIGN`
3. Each call creates one entry in the `KERNEX_DOS1` table

### Critical discovery: Drive count determination

The number of drives per driver is determined BEFORE DPBs and UDs are allocated:

1. **Bank 0 init** (`init.mac:1235-1237`) calls `GET_BOOT_DRIVES_COUNT` (bank 4) via `C4PBK`
2. The returned count goes into `DRVTBL` via `SET_DPBS` (`init.mac:2438`)
3. `SET_DPBS` allocates **DPBs** (21 bytes each via `PALLOC` in page 3 RAM) and populates `$DPBLIST`
4. Later, `KINIT` reads `DRVTBL` -> creates one UD per drive via `INIT_UNITS`
5. Then `AUTODRV` assigns devices/partitions to those UDs

**`GET_BOOT_DRIVES_COUNT` is THE control point.** Currently it counts devices (1 drive per device). For "drive per partition", it needs to count active partitions instead. Everything downstream flows from this count.

### Key data structures

**Unit Descriptor (UD) - DOS 2 Mode** (kvar.mac:146-221):
- `UD_SLOT` (offset 0, 1 byte) - Driver slot address
- `UD_DEVICE_NUMBER` (offset 2, 1 byte) - Device number
- `UD_DPB_ADDRESS` (offset 3, 2 bytes) - Address of MSX DPB
- `UD_FLAGS` (offset 4, 1 byte) - FAT12/FAT16 flags etc.
- `UD_DFLAGS` (offset 5, 1 byte) - Device-based flags:
  - bit 0: device-based driver
  - bit 2: removable
  - bit 3: partition assignment pending
  - bit 4 (UF_FOK): UD_FIRST_DEVICE_SECTOR is valid
  - bit 5: UD_CHKSUM is valid
- `UD_FIRST_DEVICE_SECTOR` (4 bytes) - Partition's first absolute sector
- Total size: ~100+ bytes without mount fields

**DOS 1 Unit Descriptor Entry (UD1)** (kvar.mac:233-242):
- `UD1_SLOT` (1 byte)
- `UD1_RELATIVE_DRIVE` (1 byte, bit 7 = partition changed)
- `UD1_DEVICE_NUMBER` (1 byte)
- `UD1_FIRST_ABSOLUTE_SECTOR` (4 bytes)
- Total: 7 bytes per entry (UD1_SIZE)

**Partition table constants** (partit.mac:16-28):
- `MBR_PSTART = 01BEh` - Start of partition table in MBR
- `MBR_PSIZE = 16` - Size of each partition entry
- `POFF_STATUS = 0` - Status byte (bit 7 = active/bootable)
- `POFF_TYPE = 4` - Partition type
- `POFF_PSTART = 8` - LBA start sector (32-bit)
- `POFF_PSIZE = 12` - Size in sectors (32-bit)
- Types: `PT_FAT12=1, PT_FAT16_S=4, PT_EXT=5, PT_FAT16=6, PT_FAT16_L=14, PT_EXT_LBA=15`

**Other key constants:**
- `MAX_UNITS = 8` (const.inc:3) - Maximum total drives
- `MAXDRV = 8` (init.mac:102)

### AUTO_ASSIGN algorithm (partit.mac:840-1337)

Current behavior:
- For each device 1..max_device:
  - `AA_CHKDUP`: skip if device already assigned to another drive (device-level check)
  - Query device params (must be block, 512-byte sectors, not skip-automap)
  - If offline+removable: assign without partition info (AA_ASSIGN_OFFLINE_REMOVABLE)
  - If online: scan partitions (primary 1-4, then extended up to 9):
    - Check type (FAT12, or FAT16 if DOS 2)
    - Read + validate boot sector (CHECK_FAT_BOOT)
    - Check duplicate mapping (CHECK_MAP_IN_USE, device+sector level)
    - If active (status & 80h): assign immediately
    - If not active: remember as candidate (first suitable only)
  - If no active partition found: use candidate
- If no suitable device found: return error (UD destroyed by AUTOD_FAIL)

### Key functions

- `F_GPART` (partit.mac:71-370) - Core partition discovery, handles primary + extended
- `AUTO_ASSIGN` / `AUTO_ASPART` (line 836-891) - Assigns one device+partition to one UD
- `AUTODRV` (line 467) - Entry point for boot-time assignment
- `GET_BOOT_DRIVES_COUNT` (line 5687) - Counts devices per driver for drive allocation
- `COUNT_DEVICES_LOOP` (line 5740) - Device enumeration loop
- `SET_DPBS` (init.mac:2438) - Allocates DPBs and populates $DPBLIST + DRVTBL
- `CHECK_MAP_IN_USE` (line 5880) - Checks if device+sector already mapped
- `IS_EXT_PART` (line 6064) - Checks if partition type is extended
- `CHECK_FAT_BOOT` (line 1544) - Validates FAT boot sector
- `AA_ASSIGN_OFFLINE_REMOVABLE` (line 1434) - Assigns removable device without partition info
- `SETNUMDRV` (line 750) - Updates $NUMDRV based on existing UDs

---

## Initial Analysis

### What works well in the plan

1. **Using the active bit (bit 7 of partition status byte)** is excellent. The infrastructure already exists:
   - `POFF_STATUS` is defined at offset 0 in partition table entries
   - `AUTO_ASSIGN` already checks bit 7 to prioritize active partitions
   - FDISK already has UI for toggling partition active status

2. **Falling back to one drive per device** when no active partitions exist (or device is offline) preserves backward compatibility.

3. **The partition first-sector is already stored** in `UD_FIRST_DEVICE_SECTOR` (DOS 2) and `UD1_FIRST_ABSOLUTE_SECTOR` (DOS 1), so partition identity is already tracked.

### Challenges identified

1. **Chicken-and-egg: drive count vs partition scanning (DOS 2 mode)** - UDs are pre-allocated before AUTODRV runs. Solution: modify `GET_BOOT_DRIVES_COUNT` to count active partitions instead of devices.

2. **The duplicate device check must change** - `AA_CHKDUP` skips an entire device if any drive is already assigned to it. Needs to become device+partition level. `CHECK_MAP_IN_USE` already does this.

3. **AUTO_ASSIGN's single-result design** - Returns after finding ONE partition. The outer loop (AUTODRV) needs restructuring to handle multiple partitions per device.

4. **DPB constraint** - DPBs are allocated by `SET_DPBS` based on `GET_BOOT_DRIVES_COUNT` result. This is why modifying `GET_BOOT_DRIVES_COUNT` is the right approach - it controls DPB allocation.

5. **Offline removable devices** - Can't scan partitions if offline. Recommendation: keep one drive per offline device.

---

## Questions and Answers

**Q: Maximum drives per device?**
A: No maximum. One device can consume all 8 drives. User's responsibility to control via active bit. However, limit scanning to first 10 partitions per device for performance.

**Q: Offline removable devices?**
A: Keep one drive, same as if there are no active partitions. Assign on first access.

**Q: Partition type filter?**
A: Only count suitable types: FAT12+FAT16 for DOS 2, FAT12 only for DOS 1.

**Q: Disable key?**
A: The "5" key (reduced drive allocation) is sufficient. "5" pressed = old behavior (1 drive per driver). "5" not pressed = new behavior (1 drive per active partition).

**Q: Restructure approach?**
A: Agreed - restructure AUTODRV rather than AUTO_ASSIGN.

---

## Partition Scanning Rule Clarification

- If primary partition 2 is of "extended" type: scan primary partition 1 + up to 9 extended partitions (max 10 total)
- Otherwise: scan all 4 primary partitions

This is consistent with how the current `AUTO_ASSIGN` partition loop works at `partit.mac:1064-1204`.

---

## Final Agreed Design

### Change 1: `GET_BOOT_DRIVES_COUNT` (bank 4, `partit.mac:5687`)

**Currently**: Calls `COUNT_DEVICES_LOOP` which queries each device for existence, returns device count. If "5" key pressed, returns 1.

**New behavior**: For each device:
- If offline + removable: count 1 (as before)
- If offline + non-removable: count 0 (as before)
- If online: read MBR (sector 0), scan partition table entries:
  - If primary partition 2 is extended type: check primary 1 + up to 9 extended partitions
  - Otherwise: check all 4 primary partitions
  - Count partitions that are active (status & 80h) AND suitable type (FAT12 or FAT16)
  - If no active partitions of suitable type found: count 1 (fallback, same as current)
- "5" key: still returns 1 (unchanged)

No boot sector validation (CHECK_FAT_BOOT) needed here - just type checking. Full validation happens later in AUTODRV.

Note: at this stage we don't know the DOS mode yet, so count both FAT12 and FAT16. If DOS 1 is later selected, FAT16 partitions will fail during AUTODRV assignment and those UDs will be destroyed by `AUTOD_FAIL` - harmless.

### Change 2: `AUTODRV_DOS2` (bank 4, `partit.mac:643`)

**Currently**: Iterates `UNIT_TAB` entries, for each Nextor UD calls `AUTO_ASSIGN` which finds ONE device+partition.

**New structure**:
```
for each Nextor driver slot (from KERNEX, up to 4):
    query DRVQ_GET_MAX_DEVICE
    collect unassigned UDs for this slot (UD_SLOT matches, UD_DEVICE_NUMBER == 0)

    for each device 1..max:
        query device params
        if not suitable (non-block, wrong sector size, skip-automap): skip

        if offline:
            if removable: consume 1 UD, assign as offline (AA_ASSIGN_OFFLINE_REMOVABLE)
            else: skip
            continue

        ; Device is online - scan partitions
        active_found = false
        first_suitable_sector = none

        for each partition (scanning rule: primary 2 extended? -> p1 + 9 ext; else -> p1-p4):
            call F_GPART
            if not found: break
            if not suitable type (FAT12 or FAT16 in DOS 2): continue

            if status & 80h:  ; Active partition
                read + validate boot sector
                if valid AND not already mapped (CHECK_MAP_IN_USE):
                    consume 1 UD, assign device+partition
                    active_found = true
            elif first_suitable_sector == none:
                remember as fallback candidate

        if NOT active_found AND first_suitable_sector != none:
            validate boot sector for fallback
            if valid AND not already mapped:
                consume 1 UD, assign device+partition

    destroy any remaining unconsumed UDs for this driver

call SETNUMDRV
```

"Consume 1 UD" = take next available pre-allocated UD for this driver. If none left, stop for this driver.

### Change 3: `AUTODRV_DOS1` (bank 4, `partit.mac:532`)

Same structural change as DOS 2. Instead of looping "for each drive of this driver -> AUTO_ASSIGN", loop through devices -> partitions -> create KERNEX_DOS1 entries:
- For each device:
  - If offline+removable: create 1 KERNEX_DOS1 entry
  - If online: scan partitions (same scanning rule)
    - For each active FAT12 partition: create 1 KERNEX_DOS1 entry
    - If no active: create 1 entry for first suitable
- The total number of entries is bounded by the drive count in DRVTBL (set by GET_BOOT_DRIVES_COUNT)

### Change 4: Remove device-level duplicate check

In the current `AA_CHKDUP` (`partit.mac:932`), an entire device is skipped if any drive already uses it. With "drive per partition", multiple drives use the SAME device. Replace with `CHECK_MAP_IN_USE` (device+sector level) which already exists and does the right comparison.

### What stays unchanged

- **`AUTO_ASPART`** (first-access assignment for removable devices): still assigns one partition to one UD
- **`F_GPART`**: unchanged, just the partition data retrieval function
- **`F_MAPDRV`**: unchanged, manual remapping works at the UD level
- **`CHECK_MAP_IN_USE`**: unchanged, already does device+sector matching
- **`SET_DPBS`** (bank 0): unchanged, allocates DPBs based on DRVTBL count
- **`CHECK_FAT_BOOT`**: unchanged, boot sector validation

### Drive letter ordering

Drive letters assigned in scan order:
- Device 1: active partition 1 -> drive A:, active partition 2 -> drive B:, ...
- Device 2: active partitions -> next drive letters, ...

Deterministic and predictable from user's perspective.

### Edge cases / risks

1. **Boot time I/O**: `GET_BOOT_DRIVES_COUNT` will now read sectors (MBR + extended chain) for each device. Up to ~7 reads per device. Adds boot latency but acceptable.
2. **Bank 4 code size**: Partition scanning in `GET_BOOT_DRIVES_COUNT` adds code. F_GPART can be reused. Need to verify bank 4 has enough space.
3. **Page 3 RAM waste**: If partition count overestimates (e.g., active partition has bad boot sector), some DPBs/UDs go unused. 21 bytes per unused DPB. Minimal impact in practice.
4. **DOS mode unknown at count time**: Count FAT12+FAT16 at scan time. If DOS 1 selected later, FAT16 UDs get destroyed by AUTOD_FAIL - no harm done.
