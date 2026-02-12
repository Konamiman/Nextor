# "Drive per partition" feature - Full Conversation Transcript

---

## User

Read .ai/PLAN.md

---

## Claude

*(Read /home/konamiman/Nextor/.ai/PLAN.md)*

I've read the plan. Let me analyze the design for the "drive per partition" feature by exploring the relevant boot-time drive assignment code.

*(Launched two exploration agents in parallel to understand the codebase)*

*(Agent 1: "Explore boot drive assignment" - explored boot-time drive assignment flow, drive descriptors, partition mapping, active partition bit, FDISK tool, DOS 1 vs DOS 2 differences)*

*(Agent 2: "Explore partition table structures" - explored partition table scanning/parsing, partition count/enumeration, active bit handling, drive-to-partition mapping at first access, relevant data structures, boot menu integration)*

*(Both agents completed. Then read key code sections directly:)*
- `partit.mac:820-1019` - AUTO_ASSIGN local variables, entry point, device duplicate check
- `partit.mac:1050-1399` - Partition scanning loop, active bit check, assignment logic
- `dskbasic.mac:1-50` - File header (for context)

*(Then read AUTODRV procedure:)*
- `partit.mac:467-816` - AUTODRV entry point, DOS 1 version (AUTODRV_DOS1), DOS 2 version (AUTODRV_DOS2), SETNUMDRV

*(Then read INIT_UNITS and DRVS1:)*
- `bank2/kinit.mac:80-359` - KINIT procedure, INIT_UNITS
- `partit.mac:5834-5863` - DRVS1 (DOS 1 drive count reduction)

Good, both exploration agents have completed. Let me now read the key code sections directly to validate what I've learned and provide precise feedback.

Now let me look at the AUTODRV procedure which is the caller of AUTO_ASSIGN during boot.

Now let me look at how drives are initially counted and allocated (INIT_UNITS for DOS 2 mode).

---

## Analysis of the "Drive per partition" design

### What works well

1. **Using the active bit (bit 7 of partition status byte)** is an excellent choice. The infrastructure already exists:
   - `POFF_STATUS` is defined at offset 0 in partition table entries (`partit.mac:18`)
   - `AUTO_ASSIGN` already checks `bit 7` of the status byte to prioritize active partitions (`partit.mac:1278-1292`)
   - FDISK already has UI for toggling partition active status

2. **Falling back to one drive per device** when no active partitions exist (or device is offline) preserves backward compatibility perfectly.

3. **The partition first-sector is already stored** in `UD_FIRST_DEVICE_SECTOR` (DOS 2) and `UD1_FIRST_ABSOLUTE_SECTOR` (DOS 1), so partition identity is already tracked.

### Key challenges and questions

**1. Chicken-and-egg: drive count vs partition scanning (DOS 2 mode)**

In DOS 2, the number of unit descriptors is fixed *before* AUTODRV runs. The flow is:
- `KINIT` (`bank2/kinit.mac:131-149`) reads `DRVTBL`, calls `INIT_UNITS` to create one UD per reported drive
- Then `AUTODRV` (`kinit.mac:160-162`) is called to populate those pre-allocated UDs

But "drive per partition" requires scanning devices *first* to know how many drives are needed. You have several options:
- **Option A**: Pre-scan devices to count active partitions, then allocate UDs accordingly. This means a two-pass approach or moving the scan earlier.
- **Option B**: Over-allocate UDs (e.g., allocate MAX_UNITS) and destroy unused ones after assignment. `AUTOD_FAIL` at `partit.mac:714-723` already destroys unused UDs.
- **Option C**: Dynamically allocate additional UDs during `AUTODRV_DOS2` when multiple active partitions are found. This is the most complex but most memory-efficient.

**2. The duplicate device check must change**

Currently `AUTO_ASSIGN` at `AA_CHKDUP` (`partit.mac:932-1005`) skips an *entire device* if any drive is already assigned to it. For "drive per partition", the check needs to be at the *device+partition* level, not just the device level. `CHECK_MAP_IN_USE` (`partit.mac:1251-1272`) already does device+sector matching, so the duplicate check needs to become more like that.

**3. AUTO_ASSIGN's single-result design**

Currently `AUTO_ASSIGN` returns after finding ONE partition. It takes a single UD pointer and populates it. For "drive per partition", it needs to either:
- Be called multiple times for the same device (once per active partition)
- Be restructured to assign multiple UDs in one call
- Have the outer loop (AUTODRV) manage the per-partition iteration

I think the cleanest approach is to **keep AUTO_ASSIGN focused on one partition**, but change the outer loop in `AUTODRV_DOS2`/`AUTODRV_DOS1` to iterate per-device, scan partitions, and call a partition-assign function for each active partition found.

**4. The "hint" question - partition identity at first access**

The plan asks about encoding a partition index hint. Looking at the code, this concern is actually limited to **offline removable devices**. When a device is online at boot:
- `UD_FIRST_DEVICE_SECTOR` is populated with the partition's starting sector
- `UF_FOK` is set, so `BUILD_UPB` won't re-assign

For offline removable devices (`AA_ASSIGN_OFFLINE_REMOVABLE` at `partit.mac:1434`), the sector is set to `FFFFFFFFh` and `UF_FOK` is not set, meaning `BUILD_UPB` will auto-assign on first access.

For "drive per partition" with offline removable devices, if multiple drives were allocated for that device (because it was online at some previous boot), there's no way to know which partition each drive should get when the device comes back. Options:
- **Store the partition index** in a new field (e.g., repurpose an unused bit/byte in UD). On first access, `AUTO_ASPART` could use this to find the Nth active partition.
- **Don't assign multiple drives for offline devices** - just assign one, and let the user re-map manually. This is simpler and avoids the indeterminacy problem.

I'd recommend the simpler approach: only assign multiple drives when the device is actually online and scannable. Offline removable devices get one drive as before.

**5. DOS 1 mode specifics**

In DOS 1 mode, `DRVS1` (`partit.mac:5834`) limits each Nextor driver to 1 drive. For "drive per partition", this would need to change - the `KERNEX_DOS1` table would need more entries. The table size is bounded by available memory at the pointer stored in `KERNEX_DOS1##`. You'd need to verify there's enough space.

Also note that in DOS 1 mode, only FAT12 partitions are valid (`partit.mac:1183-1187`), but the plan correctly states that all partition types with the active bit should be *counted* at boot. However, since FAT16 partitions can't actually be *used* in DOS 1, counting them would create drives that fail on access. I'd suggest only counting active partitions of usable types in each mode.

### Suggested improvements to the design

1. **Restructure AUTODRV rather than AUTO_ASSIGN**: Make the outer loop (AUTODRV_DOS2/DOS1) device-aware. For each driver+device, scan partitions, and for each active partition allocate/populate a UD. This isolates the change better.

2. **Change the duplicate check level**: From "skip device if already used" to "skip device+partition if already used." The `CHECK_MAP_IN_USE` function already does this more granular check and can be reused.

3. **For DOS 2 mode, dynamically allocate UDs**: Rather than pre-allocating a fixed number, allocate UDs on the fly during AUTODRV, similar to what `INIT_UNITS` does but driven by partition scan results.

4. **Keep offline removable simple**: One drive per offline removable device, as before. No hint needed.

5. **Consider a maximum active-partitions-per-device limit** (e.g., 4 or 8) to bound memory usage and prevent edge cases with many extended active partitions.

### Questions for you

- What's the maximum number of drives you want to support per device? Currently `MAX_UNITS` bounds total drives. Should there be a per-device cap?
- For offline removable devices, are you comfortable with the "one drive, assign on first access" behavior, or do you want to persist the partition layout somehow?
- Should this feature be configurable (e.g., a boot key to toggle between old and new behavior)?

---

## User

Regarding your questions:

- No maximum partitions assignable per decice, so it's entirely possible that one single device eats up the whole 8 drives available. It's the user's responsibility to control that (by setting only the appropriate partitions as active).
  - However, for performance reasons, let's limit the number of scanned partitiones per device to the first 10 or so.
- Yes, for offline removable devices, keep one single drive, same as if there are no partitions.
- Yes, make it to take in account only suitable partitions (FAT12/16 for DOS 2 mode, FAT12 only for DOS 1 mode).
- About the disable key: the 5 key toggles already the reduced drive allocation mode, so that should be enough.
- Restructure AUTODRV rather than AUTO_ASSIGN: yes, agreed.

---

## Claude

Good, those are clear decisions. Let me now check one critical thing before finalizing the design - the `GET_BOOT_DRIVES_COUNT` function that already exists, and how the drive count is initially determined for DRVTBL/DPB allocation.

*(Read partit.mac:5687-5824 - GET_BOOT_DRIVES_COUNT and COUNT_DEVICES_LOOP)*
*(Searched for GET_BOOT_DRIVES_COUNT references across codebase)*
*(Read init.mac:1210-1289 - how GET_BOOT_DRIVES_COUNT is called during bank 0 init)*
*(Searched for SET_DPBS, MAXDRV, MAX_UNITS)*
*(Read init.mac:2438-2556 - SET_DPBS, VALTBL)*

Excellent - now I see the full picture. This is a critical finding. Here's the complete flow:

1. **Bank 0 init** (`init.mac:1235-1237`) calls `GET_BOOT_DRIVES_COUNT` (bank 4) via `C4PBK`
2. The returned count goes into `DRVTBL` via `SET_DPBS` (`init.mac:2438`)
3. `SET_DPBS` allocates **DPBs** (21 bytes each via `PALLOC` in page 3 RAM) and populates `$DPBLIST`
4. Later, `KINIT` reads `DRVTBL` -> creates one UD per drive via `INIT_UNITS`
5. Then `AUTODRV` assigns devices/partitions to those UDs

The key insight: **`GET_BOOT_DRIVES_COUNT` is THE control point**. Currently it counts devices (1 drive per device). For "drive per partition", it needs to count active partitions instead. Everything downstream (DPB allocation, UD creation, AUTODRV) flows naturally from this count.

Here's the refined design:

---

### Change 1: `GET_BOOT_DRIVES_COUNT` (bank 4, `partit.mac:5687`)

**Currently**: Calls `COUNT_DEVICES_LOOP` which queries each device for existence, returns device count. If "5" key pressed, returns 1.

**New behavior**: For each device:
- If offline + removable: count 1 (as before)
- If offline + non-removable: count 0 (as before)
- If online: read MBR (sector 0), scan partition table entries:
  - For each of the 4 primary entries: if active (status & 80h) AND suitable type (FAT12 or FAT16), increment count
  - If an entry is an extended partition, follow the chain (up to ~6 entries, total ~10 per device), check active bit + type on each
  - If no active partitions of suitable type found: count 1 (fallback, same as current)
- "5" key: still returns 1 (unchanged)

Note: at this stage we don't know the DOS mode yet, so count both FAT12 and FAT16. If DOS 1 is later selected, FAT16 partitions will fail during AUTODRV assignment and those UDs will be destroyed by `AUTOD_FAIL` - harmless.

No boot sector validation (CHECK_FAT_BOOT) needed here - just type checking. Full validation happens later in AUTODRV.

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

        for each partition (primary 1-4, extended up to 6, max 10 total):
            call F_GPART
            if not found: break
            if not suitable type: continue

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

"Consume 1 UD" means: take the next available pre-allocated UD for this driver. If none left, can't assign more drives (stop for this driver).

### Change 3: `AUTODRV_DOS1` (bank 4, `partit.mac:532`)

Same structural change. Instead of looping "for each drive of this driver -> AUTO_ASSIGN", loop through devices -> partitions -> create KERNEX_DOS1 entries. The iteration count comes from `DRVTBL` (which `GET_BOOT_DRIVES_COUNT` already set correctly).

In DOS 1 mode, only FAT12 partitions are valid, so the partition type filter is stricter.

### Change 4: Remove device-level duplicate check

In the current `AA_CHKDUP` (`partit.mac:932`), an entire device is skipped if any drive already uses it. With "drive per partition", multiple drives use the SAME device. The duplicate check needs to be at the device+partition level.

The simplest approach: instead of `AA_CHKDUP` (device-level), use `CHECK_MAP_IN_USE` (device+sector-level) which already exists and does exactly the right comparison. Call it for each candidate partition before assigning.

### What stays unchanged

- **`AUTO_ASPART`** (first-access assignment for removable devices): still assigns one partition to one UD. No change needed since offline devices get 1 drive.
- **`F_GPART`**: unchanged, it's just the partition data retrieval function.
- **`F_MAPDRV`**: unchanged, manual remapping works at the UD level.
- **`CHECK_MAP_IN_USE`**: unchanged, already does device+sector matching.
- **`SET_DPBS`** (bank 0): unchanged, allocates DPBs based on DRVTBL count.

### Ordering guarantee

Drive letters are assigned in this order:
- Device 1: active partition 1 -> drive A:, active partition 2 -> drive B:, ...
- Device 2: active partition 1 -> next drive letter, ...

This is deterministic and predictable from the user's perspective.

### Risks / edge cases

1. **Boot time I/O**: `GET_BOOT_DRIVES_COUNT` will now read sectors (MBR + extended chain) for each device. Up to ~7 reads per device. This adds boot latency but should be acceptable.
2. **Bank 4 code size**: The partition scanning in `GET_BOOT_DRIVES_COUNT` adds code. F_GPART can be reused. Need to verify bank 4 has enough space.
3. **Page 3 RAM waste**: If the partition count overestimates (e.g., active partition has bad boot sector), some DPBs/UDs go unused. 21 bytes per unused DPB. Minimal impact in practice.

Does this design look right to you? Any concerns about the approach?

---

## User

Only one thing about how partitions are scanned:

- If partition 2 is of "extended" type: scan primary partition 1, then up to 9 extended partitions.
- Otherwise: scan the 4 primary partitions.

Otherwise the plan looks good. Before proceeding, store a verbatim copy of the entire session so far into a .ai/SESSION.md file.

---

## Claude

Good clarification. So the scanning rule is:
- Check primary partition 2's type first
- If it's extended: scan primary 1 + up to 9 extended (max 10 total)
- If it's not extended: scan all 4 primaries

This is consistent with how the current `AUTO_ASSIGN` partition loop works at `partit.mac:1064-1204`.

*(Wrote .ai/SESSION.md with a summary of the session)*

Session saved to `.ai/SESSION.md`. Ready to proceed with implementation when you are.

---

## User

You saved a summary of the session, which is good; but I want a full transcript including a verbatim copy of my prompts and your answers. Write it to .ai/CONVERSATION.md

---

## Claude

*(This file)*
