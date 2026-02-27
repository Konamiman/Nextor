# Floppy Disk Formatting Support Plan

## Context

Phases 1-3 (FDD flag, no-partition, media ID) and ghost drives are implemented. This plan adds floppy disk formatting support for Nextor drivers, per `.ai/PLAN.md` spec. Currently `ch_devcmd_routine` and `fmt_devcmd_routine` in `bank2/val.mac:2690-2701` are stubs returning `.IFORM`.

## Space Constraint

Bank 2 binary is 16129/16384 bytes (~255 free). New code must be minimal. The plan keeps bank 2 additions under ~200 bytes by reusing existing infrastructure and keeping built-in format strings short.

## Implementation Steps

### Step 1: New device query constants (`data.mac`)

Add after `DEVQ_GET_AVAILABILITY` (line 549):
```
const DEVQ_GET_FORMAT_CHOICES,5
const DEVQ_DO_FORMAT,6
```

### Step 2: `ch_devcmd_routine` — get format choice string (`val.mac:2690`)

Replace the stub. When called via CALL_UNIT with CH_CMD, the devcmd routine must return DE=choice string pointer and A=error. For Nextor FDD drives:

- Check `UF_FDD` in `UD_DFLAGS`. If not FDD, return `.IFORM`.
- Return DE=0 (null pointer = no string) and A=0. This tells DSK_CHOICE (line 33-35) the pointer is null, which returns A=0 (success) — meaning "format supported, no ROM string".

This is ~10 bytes. The actual choice string retrieval is handled by the new A=80h path (Step 5).

### Step 3: `fmt_devcmd_routine` — do format (`val.mac:2698`)

Replace the stub. When called via CALL_UNIT with FMT_CMD for choices 1-9:

Register state at entry (from CUN_NO_MOUNT dispatch):
- A = relative unit number (ignored for Nextor drivers)
- IX = unit descriptor
- Alt register set: HL'=DTA addr, DE'=buffer size, B'=choice number, C'=page-2 segment
- A' = P2_TPA

Action: Call `DEVQ_DO_FORMAT` via CALL_DRV, following the `mc_devcmd_remv` pattern:

```asm
fmt_devcmd_routine:
    bit  UF_FDD,(ix+UD_DFLAGS##)
    ld   a,.IFORM##
    ret  z                    ; Not FDD → error

    ld   hl,DRIVER.DEVICE_QUERY##
    ld   a,c                  ; A = page-2 segment (from C' after exx in CUN_NO_MOUNT)
    exx
    ; Now: B=choice, C=page-2 seg (original), DE=buf size, HL=DTA

    ld   c,(ix+UD_DEVICE_NUMBER##)
    ex   af,af'
    ld   a,DEVQ_DO_FORMAT##   ; Query number in A'
    ex   af,af'
    call CALL_DRV

    cp   DRIVER.QUERY_NOT_IMPLEMENTED##
    jr   z,fmt_devcmd_ni
    or   a
    ret                       ; Return driver's error code

fmt_devcmd_ni:
    ld   a,.IFORM##
    or   a
    ret
```

~35 bytes. Note: B already has the choice number from the alt registers (B' was choice in `do_format` flow: `ld b,a` at line 109). The driver receives B=choice via CALL_DRV.

**Wait** — re-checking the register flow. In `DSK_FORMAT` (line 106-111): `ld e,c / ld c,d / ld d,b / ld b,a` then `call CALL_UNIT`. So at CALL_UNIT entry: A=FMT_CMD, B=choice, C=page-2 seg, DE=buffer size, IX=buffer ptr, HL=unit descriptor. Then CUN_NO_MOUNT does `exx` to save, so B'=choice, C'=seg, DE'=bufsize, HL'=buffer. The devcmd routine gets these in the alt set. After the `exx` in the routine, B=choice is available. So CALL_DRV will pass B=choice to the driver — but DEVQ_DO_FORMAT expects C=choice per the spec. Need to add `ld c,b` before the call. Actually, looking at the spec more carefully: the driver's DEVICE_QUERY entry gets A'=query_number, and BC, DE, HL as params. So we need C=choice. Fix: add `ld c,b` after `exx`.

### Step 4: Restrict DSK_FORMAT for non-FDD Nextor drives (`val.mac:56`)

At the top of DSK_FORMAT (after entry, before the `cp 0FEh` at line 101), add a gate: if the unit is a Nextor device-based driver (`UF_DBD` set) but NOT FDD (`UF_FDD` clear), return `.IFORM` for choices FBh-FFh. Choices 1-9 already go through `fmt_devcmd_routine` which has its own check.

```asm
    ; Reject FBh-FFh for non-FDD Nextor drives
    push hl
    pop  iy          ; IY = unit descriptor (HL has it at this point)
    bit  UF_DBD,(iy+UD_DFLAGS##)
    jr   z,fmt_ok_gate       ; MSX-DOS driver → allow
    bit  UF_FDD,(iy+UD_DFLAGS##)
    jr   nz,fmt_ok_gate      ; FDD Nextor → allow
    ld   a,.IFORM##
    ret
fmt_ok_gate:
```

~18 bytes. Insert before the `cp 0FEh` check at line 101.

### Step 5: F_FMT A=80h handler — get choices to buffer (`misc.mac:680`)

In F_FMT, between the `or a / jr nz,do_format` check (line 710-711) and the `pcall DSK_CHOICE` call, add a check for A=80h:

```asm
    cp   80h
    jr   nz,do_format
    ; A=80h: get choice string into buffer at IX, length in D (from DE low byte)
    ; HL = unit descriptor, IX = buffer pointer, DE = buffer size
    pcall DSK_CHOICE_BUF      ; New routine in val.mac
    ret
```

~10 bytes in misc.mac. The heavy lifting is in DSK_CHOICE_BUF (Step 6).

### Step 6: `DSK_CHOICE_BUF` — new routine (`val.mac`)

Add near DSK_CHOICE. This routine handles A=80h for both MSX-DOS and Nextor drives:

**For MSX-DOS drives** (`UF_DBD` clear):
- Call CH_CMD via CALL_UNIT to get ROM string pointer (DE) and slot (from UD_SLOT)
- Copy string from ROM to buffer at IX using RDSLT, up to D bytes

**For Nextor FDD drives** (`UF_DBD` set, `UF_FDD` set):
- Call `DEVQ_GET_FORMAT_CHOICES` via device query
- If B=0 on return: copy empty string (just 0 terminator) to buffer
- If B=1: copy built-in string "1 - SS/DD  2 - DS/DD\r\n" to buffer
- If B=2: copy built-in string "1 - SS/DD  2 - DS/DD  3 - DS/HD\r\n" to buffer
- If B=255: driver already copied string to buffer (HL was passed through)

**For Nextor non-FDD drives**: return `.IFORM`

Entry: HL = unit descriptor, IX = buffer pointer, DE = buffer size
Returns: A = error code, HL = buffer pointer (for caller)

The device query call follows the `mc_devcmd_remv` pattern but needs to set up HL (buffer) and D (length) for the driver. This means: before calling CALL_DRV, set HL=buffer address, D=buffer length in the main register set (which becomes the params for the driver after `exx; ex af,af'` in CALL_DRV).

Estimated size: ~80 bytes for logic + ~50 bytes for built-in strings = ~130 bytes. This is the largest new addition and the main space concern.

**Built-in format strings** (null-terminated):
```
FMT_STR_1: db "1-SS 2-DS",0          ; ~11 bytes (type 1)
FMT_STR_2: db "1-SS 2-DS 3-HD",0     ; ~16 bytes (type 2)
```

Keep them short to save space. The CALL FORMAT / CALL QFORMAT UI in bank 0 can elaborate on the meaning.

### Step 7: HOKFMT/CALL FORMAT changes (`init.mac:3353`)

Currently HOKFMT (line 3386-3394) loops drives 1-8, calling `_FORMAT` with A=0 to check if formatting is supported. For MSX-DOS drives this returns the ROM string pointer; for Nextor FDD drives our ch_devcmd_routine now returns DE=0 (null pointer) + A=0 (success).

The drive selection loop (FMT_CHK_DRIVE) already works: A=0 + no error = drive supports format. The null pointer means "no choice string" which already has handling at line 3467-3468 (`ld a,l / or h / jr z,NO_CHOICE`).

**Problem**: For Nextor drives, we need A=80h path to get choice strings into RAM. The current code at line 3461-3479 uses A=0 then RDSLT to read from ROM.

**Change**: After getting the drive number (line 3459), before calling `_FORMAT`:
1. First call `_FORMAT` with A=80h, HL=`$SECBUF`, D=128 (buffer in RAM)
2. If no error: display string from `$SECBUF` buffer (simple loop, no RDSLT needed)
3. If error (e.g., MSX-DOS drive that doesn't support 80h... but actually we handle MSX-DOS in DSK_CHOICE_BUF too, so this should work for both)

Actually, simpler approach: **always use A=80h**. DSK_CHOICE_BUF handles both MSX-DOS (copies from ROM to buffer) and Nextor (driver copies or built-in string). Then HOKFMT just prints from the RAM buffer. This eliminates the RDSLT loop entirely.

Replace lines 3461-3479 (the A=0 call + RDSLT display loop) with:
```asm
    ld   c,_FORMAT##
    push bc              ; Save drive + _FORMAT
    ld   a,80h           ; Get choices into buffer
    ld   hl,($SECBUF##)  ; Buffer address
    ld   d,128            ; Max length
    call BDOS##
    or   a
    jr   nz,NO_CHOICE    ; Error → no choices available
    ld   hl,($SECBUF##)  ; Print from buffer
CHOILP2:
    ld   a,(hl)
    or   a
    jr   z,CHOIED
    call $OUT
    inc  hl
    jr   CHOILP2
```

This is actually shorter than the original RDSLT loop, saving a few bytes in bank 0.

### Step 8: CALL QFORMAT command (`dskbasic.mac` + `bank4/partit.mac`)

**`dskbasic.mac`**: Add entry to COMMAND table (before the terminating `defb 0` at line 425):
```asm
    defb 'QFORMAT',0
    defw QFORMAT
```

Add dispatcher:
```asm
QFORMAT:
    ld   a,4
    ld   ix,CQFORMAT##
    jp   CALBNK##
```

**`bank4/jtable.inc`**: Add `jentry CQFORMAT`

**`bank6/jtable.inc`**: No change needed (CQFORMAT is in bank 4)

**`bank4/partit.mac`**: Implement CQFORMAT. Similar to existing CALL commands:
1. Parse drive letter argument from BASIC
2. Call `_FORMAT` with A=FBh (quick format) and drive number
3. Print success/error message

~60 bytes in bank 4 (plenty of space there).

### Step 9: Update driver template documentation

Update `source/kernel/drivers/StandaloneASCII8/driver.mac` DEVICE_QUERY section to document DEVQ_GET_FORMAT_CHOICES (5) and DEVQ_DO_FORMAT (6) query interfaces.

## Files Modified

| File | Step | Changes |
|------|------|---------|
| `source/kernel/data.mac` | 1 | Add DEVQ_GET_FORMAT_CHOICES, DEVQ_DO_FORMAT constants |
| `source/kernel/bank2/val.mac` | 2,3,4,6 | ch_devcmd_routine, fmt_devcmd_routine, DSK_FORMAT gate, DSK_CHOICE_BUF |
| `source/kernel/bank2/misc.mac` | 5 | F_FMT A=80h handler |
| `source/kernel/bank0/init.mac` | 7 | HOKFMT use A=80h, print from RAM buffer |
| `source/kernel/bank0/dskbasic.mac` | 8 | COMMAND table + QFORMAT dispatcher |
| `source/kernel/bank4/jtable.inc` | 8 | Add CQFORMAT jentry |
| `source/kernel/bank4/partit.mac` | 8 | CQFORMAT implementation |
| `source/kernel/drivers/StandaloneASCII8/driver.mac` | 9 | Document new queries |

## Bank 2 Space Budget

| Component | Est. bytes |
|-----------|-----------|
| ch_devcmd_routine (Step 2) | ~10 |
| fmt_devcmd_routine (Step 3) | ~35 |
| DSK_FORMAT gate (Step 4) | ~18 |
| DSK_CHOICE_BUF + strings (Step 6) | ~130 |
| **Total** | **~193** |

Available: ~255 bytes. Margin: ~62 bytes.

## Existing Infrastructure Reused

- **mc_devcmd_remv** (`val.mac:2633`): Pattern for calling device queries from CALL_UNIT context
- **CALL_DRV** (`val.mac:2705`): Calls disk driver with proper bank/slot switching
- **DSK_CHOICE** (`val.mac:16`): Existing CH_CMD handler for MSX-DOS drivers
- **$SECBUF** (`data.mac:318`, F34Dh): 512-byte sector buffer, usable as temporary format string buffer
- **HOKFMT/$FORMAT** (`init.mac:3353`): Existing CALL FORMAT implementation
- **CALBNK** (`doshead.mac`): Cross-bank call mechanism for CALL commands

## Verification

1. Build with `make` after each step
2. Test _FORMAT A=80h with a Nextor FDD driver → should return choice string in buffer
3. Test _FORMAT A=1 with a Nextor FDD driver → should call DEVQ_DO_FORMAT on driver
4. Test _FORMAT A=FBh with FDD → quick format should work
5. Test _FORMAT A=FBh with non-FDD Nextor drive → should return .IFORM
6. Test CALL FORMAT from BASIC → should show FDD drives, display choices, format
7. Test CALL QFORMAT from BASIC → should quick-format selected drive
8. Test MSX-DOS drives still work with CALL FORMAT (regression)
