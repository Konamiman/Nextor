# Implementation Plan: Drivers Loadable in RAM

## Context

Nextor currently requires each disk driver to occupy its own ROM slot. This limits driver availability to the number of physical ROM slots. This feature allows loading/unloading drivers in mapped RAM segments, enabling users to dynamically add drivers post-boot. The full design spec is at `.ai/PLAN.md` and `.ai/AGENT_PLAN.md`.

## Reference: Agreed Design Decisions

- `_DRVRO` = function 7Fh, `DRVQ_INIT_RAM` = 6, `DRVQ_SHUTDOWN_RAM` = 7, `.INITE` = 0AFh
- Use existing `UD_RAM_SEGMENT` field (offset +7, currently always FFh)
- Use `CALL_MAP` (page 3 vector: IYh=slot, IYl=segment, IX=address, AF/BC/DE/HL=params)
- Clustered linked list in kernel data segment (4 entries × 2 bytes + 2-byte next ptr = 10 bytes/cluster)
- Disable interrupts during registration/unregistration
- Shutdown order: unregister timer → unmap drives → call shutdown query → remove from table
- DOS 2 mode only

---

## Phase 1: Infrastructure — Constants and Definitions

### 1.1 New function number and error code
**File**: `source/kernel/codes.mac`
- After `_GETCLUS` (line 152), add: `const _DRVRO, 7Fh`
- At line 247 (reserved area), add: `const .INITE, 0AFh` with comment "Initialization error"

### 1.2 New driver query constants
**File**: `source/kernel/data.mac`
- After `DRVQ_GET_MAX_DEVICE,5` (line 542), add:
  - `const DRVQ_INIT_RAM,6`
  - `const DRVQ_SHUTDOWN_RAM,7`
- After existing QUERY constants (around line 536), add:
  - `const QUERY_INIT_ERROR,1`

### 1.3 RAM driver registration list pointer
**File**: `source/kernel/kvar.mac`
- After `GHO_PAIR` (line 789), before `RAM_PTR` (line 791), add:
  - `var2 RDRV_PTR` — pointer to first cluster of RAM driver registration list (0 = none)

### 1.4 Error messages for .INITE
**File**: `source/kernel/bank1/msg.mac`
- English section (after `.ICLUS` err at line ~243): `err .INITE, <"Initialization error">`
- Japanese section (after `.ICLUS` err at line ~411): `err .INITE, <"初期化エラー">`

Note: The label `INITE` already exists as a boot `msg` — that's a different namespace (msg vs err), no conflict.

### 1.5 BASIC error mapping for .INITE
**File**: `source/kernel/bank0/dskbasic.mac`
- Change `.FRSTER` from `0B0h` to `0AFh` (line 3450)
- Add `DERNUM *` (generic BASIC error) at the very beginning of `DERR_TAB` (before the `.ICLUS`/ERRICL entry at line 3465) — this maps .INITE (0AFh) to a generic error

### 1.6 Extend kernel PRSLOT to handle segment numbers
**File**: `source/kernel/bank4/partit.mac` — `PRSLOT` (line 5963)
- Currently only prints `slot[-subslot]` and ignores B
- Add segment printing after the subslot: if B != FFh, print `:<segment>` in decimal
- Model after SHARED.MAC's PRSLOT (line 328) which already does this using BY2ASC

---

## Phase 2: Core Kernel — Registration Table and Driver Invocation

### 2.1 RAM driver registration table management
**File**: `source/kernel/bank4/partit.mac` (new section)

Implement these internal routines:

**RAMDRV_FIND(A=slot, B=segment) → Cy=1 found + HL=entry ptr, Cy=0 not found**
- Walk clustered list from (RDRV_PTR##). If 0, return not found.
- Each cluster: 4 entries of 2 bytes (slot+segment) + 2-byte next pointer.
- Entry with slot=0 means empty. Match on both slot (masked: bits 0-3 + bit 7 for expanded slot flag) and segment.

**RAMDRV_REGISTER(A=slot with timer flag in bit 4, B=segment) → A=0 ok, .NORAM on alloc fail**
- DI before modification, EI after.
- First scan existing clusters for an empty entry (slot=0). If found, store and return.
- If no empty slot, allocate a new 10-byte cluster via `ALL_P2##` (call bank 2). Initialize all entries to 0, link it to the end of the chain (or set as head if RDRV_PTR=0). Store the new entry.

**RAMDRV_UNREGISTER(A=slot, B=segment) → A=0 ok, .IDRVR on not found**
- DI before, EI after. Call RAMDRV_FIND, zero out the entry.

**RAMDRV_ENUMERATE(A=1-based index) → A=slot (0 if not found), B=segment**
- Walk list, skip zero entries, decrement index for each non-zero entry. Return when index reaches 0.

Cluster layout (10 bytes):
```
+0,+1: entry 0 (slot byte, segment byte)
+2,+3: entry 1
+4,+5: entry 2
+6,+7: entry 3
+8,+9: next cluster pointer (0 = end)
```

Slot byte encoding: bits 0-1 = primary slot, bits 2-3 = sub-slot, bit 4 = timer interrupt flag, bit 7 = expanded slot flag. A slot byte of 0 means empty entry.

### 2.2 Add _DRVRO to function dispatch table
**File**: `source/kernel/bank2/kinit.mac`
- After `F_GETCLUS##` entry (line 814), add: `dw F_DRVRO##`

### 2.3 Add F_DRVRO and helpers to bank 4 jump table
**File**: `source/kernel/bank4/jtable.inc`
- Add: `jentry F_DRVRO`
- Also later (Phase 6): `jentry CIDRIVER`, `jentry CIDRIVERM`, `jentry CUDRIVER`

### 2.4 Implement F_DRVRO
**File**: `source/kernel/bank4/partit.mac` (new PROC)

**Input**: A=slot, B=segment, DE=print routine (0→point to `ret`), H=operation (1=init, 2=shutdown)

**Operation 1 — Initialize**:
1. If H is not 1 or 2, return `.ISBFN##`
2. Call RAMDRV_FIND. If found, return `.IDRVR##`
3. If DE=0, point DE to a local `ret` instruction
4. Verify driver signature: use `RD_MAP##` to read bytes at 4100h-410Fh and compare with `"NEXTORv3_DRIVER",0`. Return `.IDRVR##` if mismatch.
5. Invoke `DRVQ_INIT_RAM` (query 6): set up AF'=6 (query number in A), DE=print routine, then call `CALL_MAP##` with IYh=slot, IYl=segment, IX=412Dh (DRIVER.DRIVER_QUERY). Check result: if A=QUERY_INIT_ERROR, return `.INITE##`. If A is not 0 (QUERY_OK) and not 255 (QUERY_NOT_IMPLEMENTED), return `.INITE##`.
6. Extract timer flag from B bit 0. Encode into slot byte bit 4.
7. Call RAMDRV_REGISTER. Return its result.

**Operation 2 — Shutdown**:
1. Call RAMDRV_FIND. If not found, return `.IDRVR##`
2. Clear timer flag in the entry (bit 4 of slot byte) — DI/EI around this
3. Unmap all drives mapped to this driver: walk unit descriptors, for each UD where UD_SLOT matches and UD_RAM_SEGMENT matches, call the existing drive unmapping logic (same as _MAPDRV with action=0). Invalidate disk buffers for those drives.
4. Verify driver signature via RD_MAP. If valid, invoke `DRVQ_SHUTDOWN_RAM` (query 7) via CALL_MAP, passing DE. Ignore result.
5. Call RAMDRV_UNREGISTER.
6. Return A=0.

---

## Phase 3: Driver Invocation Path for Disk I/O

### 3.1 Modify CALL_DRV for RAM drivers
**File**: `source/kernel/bank2/val.mac` — CALL_DRV / CALDRV_DEVBASED (line ~2792)

In `CALDRV_DEVBASED`, after loading `(ix+UD_SLOT##)` into B:
- Load `(ix+UD_RAM_SEGMENT##)` and check if FFh
- If FFh: proceed with existing ROM path (set BK4_ADD, use CALDRV via GO_DRV)
- If not FFh: branch to new `CALDRV_RAM` path that uses `CALL_MAP##`:
  - IYh = slot (from UD_SLOT), IYl = segment (from UD_RAM_SEGMENT)
  - IX = driver routine address (from HL' which holds the driver entry address, e.g., DRIVER.READ_WRITE = 4139h)
  - Set up AF, BC, DE, HL with the parameters the driver expects
  - Call `CALL_MAP##`
  - Handle return values same as existing path

This is the most critical change — all disk I/O to RAM driver devices flows through here.

---

## Phase 4: Timer Interrupt, EXTBIO, BASDEV, OEMSTAT Hooks

### 4.1 Timer interrupt for RAM drivers
**File**: `source/kernel/bank0/init.mac` — after TIMI_NEXTOR_DRV (line ~2235)

Add `TIMI_RAM_DRV`:
1. Load `(RDRV_PTR##)`. If 0, return immediately.
2. Walk the clustered list. For each non-zero entry with bit 4 set (timer flag):
   - Extract slot and segment
   - Call `CALL_MAP##` with IX=410Fh (DRIVER.TIMER_INT), IYh=slot, IYl=segment
3. Call this from both `DOS2INT` (line ~2155) and the DOS 1 handler right after `call TIMI_NEXTOR_DRV`

### 4.2 EXTBIO hook for RAM drivers
**File**: `source/kernel/bank1/mapinit.mac`

After the existing KERNEX EXTBIO loop, add a RAM driver EXTBIO loop:
- Walk RDRV_PTR list, for each registered driver call CALL_MAP with IX=4118h (DRIVER.EXTBIO)
- Respect D' register convention (D'=0 means stop calling further)

### 4.3 BASDEV for RAM drivers
**File**: `source/kernel/bank0/doshead.mac` or `dskbasic.mac`

After the ROM driver BASDEV call returns with carry set (not handled):
- Iterate RAM drivers, call each one's BASDEV (address 4115h) via CALL_MAP
- Stop when one clears carry (handled)

### 4.4 OEMSTAT for RAM drivers
**File**: `source/kernel/bank0/dskbasic.mac` — STATEMENT handler (line ~327)

After the ROM OEMSTAT call returns with carry set:
- Iterate RAM drivers, call each one's OEMSTAT (address 4112h) via CALL_MAP
- Stop when one clears carry

---

## Phase 5: Existing Function Call Modifications

### 5.1 Extend GDRIVER to enumerate RAM drivers
**File**: `source/kernel/bank4/partit.mac` — GDRIVER (line 383)

After the KERNEX loop exhausts without finding the requested index (line ~460), instead of returning A=0:
- Compute remaining index (subtract count of ROM drivers found)
- Call RAMDRV_ENUMERATE with the remaining index
- Return slot and segment from the RAM driver entry

### 5.2 Modify F_GDRVR for RAM driver info
**File**: `source/kernel/bank4/partit.mac` — F_GDRVR (line ~1941)

When GDRIVER returns a non-FFh segment (RAM driver):
- Store segment at buffer offset +1 (instead of always FFh)
- Set offset +2 (drive count) = 0, offset +3 (first drive) = 0
- Use CALL_MAP to invoke DRVQ_GET_VERSION and DRVQ_GET_STRING queries for version/name info
- Set flags at offset +4 appropriately (Nextor driver, device-based)

Also update the direct slot+segment lookup path (GDRVR_CHECK_DRVR) to accept non-FFh segments by checking the RAM driver registration table.

### 5.3 Verify F_GDLI works
**File**: `source/kernel/bank4/partit.mac` — F_GDLI (line ~2176)

Already reads `UD_RAM_SEGMENT` and stores at buffer +2. Should work automatically once _MAPDRV correctly stores the segment in the UD. Verify and adjust if needed.

### 5.4 Modify F_GPART for RAM drivers
**File**: `source/kernel/bank4/partit.mac` — F_GPART (line ~71)

When B (segment) != FFh, use CALL_MAP instead of CALSLT/CALDRV to call driver device queries.

### 5.5 Modify F_CDRVR for RAM drivers
**File**: `source/kernel/bank4/partit.mac` — F_CDRVR (line ~2455)

When B (segment) != FFh, use CALL_MAP to invoke the specified routine (in DE) with the input register set from the buffer (at HL).

### 5.6 Modify F_MAPDRV for RAM driver mapping
**File**: `source/kernel/bank4/partit.mac` — MAP_SPECIFIC (line ~3024)

When the mapping data specifies a non-FFh segment:
- F_GDRVR validation already works (step 5.2)
- Device queries within MAP_SPECIFIC must use CALL_MAP instead of CALSLT/CALDRV
- UD creation already stores segment in UD_RAM_SEGMENT (verify)

---

## Phase 6: BASIC CALL Commands

### 6.1 Add commands to command table
**File**: `source/kernel/bank0/dskbasic.mac` — COMMAND table (before line 424 `defb 0`)

Add entries (IDRIVERM before IDRIVER for longest-match-first):
```
defb 'IDRIVERM',0
defw IDRIVERM
defb 'IDRIVER',0
defw IDRIVER
defb 'UDRIVER',0
defw UDRIVER
```

### 6.2 Add thin wrappers in bank 0
**File**: `source/kernel/bank0/dskbasic.mac`

```
IDRIVER:   ld a,4 / ld ix,CIDRIVER## / jp CALBNK##
IDRIVERM:  ld a,4 / ld ix,CIDRIVERM## / jp CALBNK##
UDRIVER:   ld a,4 / ld ix,CUDRIVER## / jp CALBNK##
```

### 6.3 Implement CIDRIVER / CIDRIVERM in bank 4
**File**: `source/kernel/bank4/partit.mac` (new PROCs)

**CIDRIVER** flow:
1. Parse file name string argument
2. Parse optional init data (integer values → bytes, or address+length from memory)
3. Store parsed data in SECBUF
4. Allocate segment via `ALL_SEG##` (system mode, prefer non-primary mapper)
5. Open file, read up to 3F00h bytes to offset 100h of segment (using WR_MAP or segment switching)
6. Zero first 256 bytes of segment (using WR_MAP)
7. Copy init data: byte 0 = length, bytes 1+ = data (via WR_MAP)
8. Call _DRVRO (H=1, A=slot, B=segment, DE=print routine). On error, free segment and report BASIC error.
9. Print "Driver loaded in slot X[-Y]:Z"

**CIDRIVERM** additionally:
10. Query driver for devices via CALL_MAP + DEVICE_QUERY
11. Find first available device, find best partition (prefer active flag)
12. Map first free drive via _MAPDRV
13. Print "Drive X: mapped to device N"

On any error after step 4, free the allocated segment before returning.

### 6.4 Implement CUDRIVER in bank 4
**File**: `source/kernel/bank4/partit.mac` (new PROC)

1. Parse two numeric arguments: slot, segment
2. Validate ranges (0-255)
3. Call _DRVRO (H=2). If .IDRVR, raise "Invalid driver" BASIC error
4. Free segment via `FRE_SEG##`

### 6.5 Add to bank 4 jump table
**File**: `source/kernel/bank4/jtable.inc`
- Add: `jentry F_DRVRO`, `jentry CIDRIVER`, `jentry CIDRIVERM`, `jentry CUDRIVER`

---

## Phase 7: Display Changes

### 7.1 CDRIVERS and CDRVINFO segment display
**File**: `source/kernel/bank4/partit.mac`

Both already call kernel PRSLOT with A=slot, B=segment. Since Phase 1.6 updates PRSLOT to handle segments, these will work automatically once F_GDRVR returns the correct segment value.

### 7.2 DRIVERS.COM already works
**File**: `source/tools/DRIVERS.MAC`

Uses SHARED.MAC's PRSLOT which already prints `:segment` when B!=FFh. No changes needed — it will work once F_GDRVR returns correct segment data.

---

## Phase 8: FDISK Changes

### 8.1 FDISK driver display
**File**: `source/kernel/bank5/fdisk.c`

- `GetDriversInformation()` already calls _GDRVR in a loop — RAM drivers will appear automatically
- `ComposeSlotString()` needs modification: when `segment != 0xFF`, append `:<decimal_segment>` to the string
- Driver call mechanism uses _CDRVR which is updated in Phase 5.5

---

## Phase 9: DRVROP.COM Tool
**File**: `source/tools/C/drvrop.c` (new)

Follow the pattern of existing C tools (e.g., `drvtest.c`). Uses dos.h, asmcall.h.
- Install mode: parse args → ALL_SEG → open file → write to segment → write init data → call _DRVRO → optional _MAPDRV
- Uninstall mode: parse slot+segment → call _DRVRO → FRE_SEG
- Add to `source/tools/C/Makefile`

---

## Phase 10: Example RAM Disk Driver
**File**: `source/kernel/drivers/RAMDisk/driver.mac` (new)

Minimal FAT12 driver using its own segment's free space (~14KB = ~28 sectors):
- Standard NEXTORv3_DRIVER structure
- DRVQ_INIT_RAM: initialize boot sector, FAT, root directory in segment data area
- READ_WRITE: copy sectors between segment data area and DTA
- 1 device, 1 LUN, no timer needed

---

## Phase 11: Turbo-R FDD Driver RAM Variant

**File**: `source/kernel/drivers/TurboRFDD/driver.mac`
- Add `RAM_DRIVER` conditional assembly constant
- When defined: respond to DRVQ_INIT_RAM/SHUTDOWN_RAM, return NOT_IMPLEMENTED for old init queries
- Copy XFER_CODE to stack space instead of page 3 (can't allocate page 3 in RAM drivers)
- Add Makefile target for standalone `.DRV` binary

---

## Implementation Order

1. **Phase 1** — All infrastructure (can be done in one pass)
2. **Phase 2** — Registration table + F_DRVRO (core functionality)
3. **Phase 3** — CALL_DRV modification (enables disk I/O through RAM drivers)
4. **Phase 5.1-5.2** — GDRIVER + F_GDRVR (enables driver enumeration)
5. **Phase 10** — Example RAM disk driver (first end-to-end test: manually call _DRVRO + _MAPDRV from a test program)
6. **Phase 4** — Timer/EXTBIO/BASDEV/OEMSTAT hooks
7. **Phase 5.3-5.6** — Remaining function call modifications
8. **Phase 6** — BASIC CALL commands (enables `CALL IDRIVER` testing)
9. **Phase 7-8** — Display changes (DRIVERS, DRVINFO, FDISK)
10. **Phase 9** — DRVROP.COM tool
11. **Phase 11** — Turbo-R FDD RAM variant

---

## Critical Files Summary

| File | Changes |
|------|---------|
| `source/kernel/codes.mac` | _DRVRO, .INITE constants |
| `source/kernel/data.mac` | DRVQ_INIT_RAM, DRVQ_SHUTDOWN_RAM, QUERY_INIT_ERROR |
| `source/kernel/kvar.mac` | RDRV_PTR variable |
| `source/kernel/bank1/msg.mac` | .INITE error messages (EN + JP) |
| `source/kernel/bank0/dskbasic.mac` | .FRSTER, DERR_TAB, CALL command entries + wrappers, OEMSTAT hook |
| `source/kernel/bank0/init.mac` | TIMI_RAM_DRV timer interrupt handler |
| `source/kernel/bank0/doshead.mac` | BASDEV RAM driver chain |
| `source/kernel/bank1/mapinit.mac` | EXTBIO RAM driver chain |
| `source/kernel/bank2/kinit.mac` | Function dispatch table entry for _DRVRO |
| `source/kernel/bank2/val.mac` | CALL_DRV RAM driver I/O path |
| `source/kernel/bank4/jtable.inc` | Jump table entries |
| `source/kernel/bank4/partit.mac` | F_DRVRO, RAMDRV_*, GDRIVER, F_GDRVR, F_GPART, F_CDRVR, F_MAPDRV, PRSLOT, CIDRIVER/CUDRIVER |
| `source/kernel/bank5/fdisk.c` | ComposeSlotString segment display |
| `source/tools/C/drvrop.c` | New tool |
| `source/kernel/drivers/RAMDisk/` | New example driver |

## Verification

Each phase can be tested incrementally:
- **After Phase 2+3**: Write a small test program that calls ALL_SEG, loads a driver binary, calls _DRVRO to init, _MAPDRV to map a drive, and then reads/writes files on it
- **After Phase 5**: Verify _GDRVR enumeration returns RAM drivers with correct info
- **After Phase 6**: Test `CALL IDRIVER("RAMDISK.DRV")` from BASIC
- **After Phase 10**: End-to-end test with the RAM disk driver: load, map, create files, read back, unload
- **Full regression**: Verify ROM-only systems still work unchanged (all new code paths gate on segment != FFh)
