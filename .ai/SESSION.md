# Nextor Boot Menu - Design Review Session

## Date: 2026-02-08

## Context

Review and validation of the boot menu design described in `.ai/PLAN.md`. No code was written - this was a pure design discussion session.

## The Design (Summary from PLAN.md)

When a Nextor 3 kernel boots and detects it's the master, if the user is pressing the N key, it displays an interactive boot menu showing:
- All detected Nextor 3 kernels (with names from driver query) and their toggle keys
- Numeric boot key options (0-5 plus CTRL=6, SHIFT=7)
- ESC to cancel, ENTER to apply, N to disable all kernels and boot

## Key Design Decisions Made

### Signature placement
- "NXT3\0" signature already placed at 407Bh in doshead.mac (5-byte gap between `jp BIOS_IN##` at 4078h and RAMENT at 4080h)
- Avoids using 400Ah-400Fh reserved bytes (MSX standard says those should be zero)
- Slot scanning uses RDSLT to check for "NXT3" at 407Bh - safe, read-only operation

### One-time boot keys as the anti-re-display mechanism
- After the menu is shown, RAM keys (at A100h) are ALWAYS written for all three exit paths (ENTER, N, ESC), with the N bit cleared
- Subsequent kernels' `SCANKEYS_RAM` picks these up, never sees N, never triggers the menu
- No separate "menu already shown" flag is needed - the mechanism is self-consistent
- For ESC: write RAM keys = current BOOTKEYS with N bit cleared (preserves all other key states)
- For ENTER: write RAM keys = user's toggled selections with N bit cleared
- For N (disable all): write RAM keys = all slot disable keys set, N bit cleared

### Programmatic RAM keys (NEXBOOT.COM)
- If RAM keys already exist at A100h when booting (set by NEXBOOT.COM or similar), the menu is NEVER shown, even if those RAM keys have N set
- Rationale: programmatic boot keys imply the user knows exactly what boot configuration they want
- N-in-RAM-keys continues to mean "disable all kernels" the old way

### DISABLE_KEY modification
- Current `DISABLE_KEY` checks N first (disables all kernels) then per-slot keys
- New design: split the N check out of `DISABLE_KEY`, so DISABLE_KEY only checks per-slot keys
- N key triggers the menu instead of directly disabling
- After menu updates BOOTKEYS, DISABLE_KEY is re-run to check if the current kernel was disabled by the user's selections

### Init flow for the menu
```
CHK_DUP → DO_KEYS_INIT → DISABLE_KEY (per-slot only) → CHK_DRIVER →
DISKID check → [if master: reach OVERRIDE] →
OVERRIDE: set LINL40, call INITXT →
[if N pressed AND keys came from keyboard (not RAM): SHOW_MENU, update BOOTKEYS, write RAM keys] →
[re-run DISABLE_KEY with updated BOOTKEYS] →
continue normal OVERRIDE (set DOS_VER, NXT_VER, H.RUNC hook) → SLAVE (driver init)
```

The menu is shown AFTER INITXT (screen is ready) and AFTER CHK_DRIVER (own driver validated), but BEFORE driver initialization.

### Screen initialization
- The existing INITXT call at OVERRIDE (line 1032-1039) already sets 40/80 columns based on MSXVER
- The menu check is inserted right after INITXT, before version/hook setup
- The CLS at line 893 (first cartridge path) becomes redundant but harmless

### Korean MSX gotcha
- From `PrintRuler()` in fdisk.c: Korean MSX computers do weird things when printing at the last screen column
- Check if H_CHPH (FDA4h) != 0xC9 (hooked); if so, print separator lines with `width - 1` characters
- The boot menu's separator/ruler lines must implement this same check

### "(BAD DRIVER)" handling
- For the scanning kernel itself: CHK_DRIVER runs before the menu and halts on failure, so the scanning kernel always has a valid driver
- For other kernels found during scanning:
  1. RDSLT to check "NXT3" at 407Bh (safe)
  2. Try DRVQ_GET_STRING via CALSLT+CALDRV to get driver name
  3. If query returns QUERY_OK or QUERY_TRUNCATED_STRING: display the name
  4. If query returns anything else: display "(BAD DRIVER)"
- A kernel shown as "(BAD DRIVER)" is locked as disabled (no asterisk, toggle ignored)
- Risk of crash from corrupt ROM accepted as unlikely (NXT3 signature implies properly built kernel)

### Dead code cleanup
- The commented-out code at init.mac lines 1046-1063 (simulating N key via RAM keys to kill Nextor v2 kernels) is leftover and should be removed as part of this implementation

### Override case (newer kernel becomes master)
- If kernel A (3.0) shows the menu and writes RAM keys, then kernel B (3.1) overrides A and becomes master
- Kernel B's DO_KEYS_INIT loads RAM keys (no N set) → menu doesn't trigger again
- This is correct: the previous master already showed the menu

## Scanning Algorithm

For each slot/subslot combination (up to 16, but stop at 5 kernels found):
1. Check EXPTBL (FCC1h) to determine if primary slot is expanded
2. RDSLT at 4000h/4001h → check for "AB" ROM signature
3. RDSLT at 407Bh-407Eh → check for "NXT3" signature
4. If both match: set BK4_ADD = DRIVER.DRIVER_QUERY (412Bh), CALSLT with IX=CALDRV (4048h), IYh=target slot, A=DRVQ_GET_STRING, B=DRVQ_STR_DRIVER_NAME, D=buffer size, HL=buffer address
5. Handle result: QUERY_OK → name, QUERY_TRUNCATED_STRING → name+"...", other → "(BAD DRIVER)"

Buffer size for driver name query should be `available_width - 3` to leave room for "..." suffix when truncated.

## Screen Layout Analysis

Total lines with 5 kernels + 8 numeric options: ~21 lines. Fits in 24-line MSX text mode.

### 40-column mode (MSX1)
- Shorten "in slot X-Y" to "in X-Y" for 5 extra characters of driver name
- Separator: 40 (or 39 for Korean MSX) hyphens
- Max driver name display width: roughly 25-28 characters

### 80-column mode (MSX2+)
- Full "in slot X-Y" text
- Separator: 80 (or 79 for Korean MSX) hyphens
- Max driver name display width: roughly 55-60 characters

### Non-Turbo-R
- Keys 2 and 4 (R800-related) are hidden, saving 2 lines

## Key Mappings in the Menu

Kernel toggle keys use the existing per-slot disable keys from DISABLE_TBL:
- Slot 0-0 to 0-3: U, I, O, P
- Slot 1-0 to 1-3: Q, W, E, R
- Slot 2-0 to 2-3: A, S, D, F
- Slot 3-0 to 3-3: Z, X, C, V

Numeric keys: 0-5 as labeled, plus 6=CTRL, 7=SHIFT

Special: N = disable all and boot, ESC = cancel, ENTER = apply and boot

## Default States (Asterisks)

- All kernels: enabled (asterisk shown) by default
- Numeric keys: asterisk shown if the corresponding bit is set in KEYS_INV_0/KEYS_INV_1
- Default KEYS_INV_0 = 00h → no numeric keys inverted
- Default KEYS_INV_1 = 20h → CTRL inverted (key 6 "Single FDD drive" shows asterisk by default)

## Technical Notes for Implementation

### BOOTKEYS layout (5 bytes at F9B9h)
- Byte 0 (B): 76543210 (digit keys)
- Byte 1 (E): FEDCBA98
- Byte 2 (D): NMLKJIHG
- Byte 3 (L): VUTSRQPO
- Byte 4 (H): CAPS.GRAPH.CTRL.SHIFT.ZYXW

### RAM keys at A100h
- 17 bytes: "NEXTOR_BOOT_KEYS\0"
- 5 bytes: key data (same layout as BOOTKEYS: B, E, D, L, H)
- RAM keys bypass KEYS_INV (inversion already applied or not needed)

### Inter-slot call safety
- BK4_ADD is safe to use at this boot stage (no timer interrupt hooks set up yet)
- CALDRV/CALBNK in target ROM are doshead.mac code (ROM, no initialization needed)
- Bank switching code at CHGBNK is ROM code, works without initialization

### Existing code references
- `PRINT_INIT_KERNEL_AT_ASLOT` at init.mac:695 - existing driver name query logic (local slot only)
- `CALL_DRIVER_QUERY` at init.mac:3556 - calls DRIVER_QUERY in local slot via CALBNK
- `DISABLE_KEY` at init.mac:226 - current N + per-slot key check (to be split)
- `DO_KEYS_INIT` at init.mac:3024 - keyboard/RAM key scanning
- `SCANKEYS_RAM` at init.mac:634 - one-time RAM keys detection
- `OVERRIDE` at init.mac:1028 - master kernel setup (menu insertion point)
- `CLEAN_V2_KERNELS` at init.mac:3574 - Nextor 2 cleanup (separate from menu)

## Open Items / Nice-to-haves

- Redefine asterisk character pattern as a checkmark (VDP pattern table modification) - defer until working implementation exists
- Consider what happens with exactly 5 kernels (KERNEX only has 4 slots) - show all 5 in menu, accept that the 5th can't register
- Key 0 "Disable permanent disk emulation" always shown (can't check device before driver init)
- Driver query without initialization requirement must be documented for third-party driver developers
