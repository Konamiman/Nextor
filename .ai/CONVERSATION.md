# Nextor Boot Menu Design Review - Full Conversation Archive

## Date: 2026-02-08

---

## User

Read .ai/PLAN.md for instructions.

---

## Claude

[Read /home/konamiman/Nextor/.ai/PLAN.md]

[Launched three parallel research agents to explore: boot process code, driver query protocol, and boot keys documentation]

[After research completed, also read doshead.mac and init.mac sections for specific details]

Now I have a thorough understanding of the codebase. Here's my analysis of the boot menu design:

---

## Design Validation

The overall design is solid and well-thought-out. The core idea of leveraging the existing one-time boot keys mechanism (RAM keys at A100h) as the output mechanism for the menu is, in my view, the key to making this work cleanly. Here are my detailed observations:

### Things that work well in the design

1. **Repurposing the N key** from "disable all" to "show menu" is a natural evolution. The menu provides equivalent functionality (and more) via the "N. Disable all and boot" option inside the menu.

2. **The 5-kernel display limit** is practical. With 5 kernel lines + 8 numeric lines + header/footer, the menu fits in 24-line text mode (about 21 lines total).

3. **Using per-slot disable keys (UIOP/QWER/ASDF/ZXCV) as the toggle keys in the menu** preserves backward compatibility for experienced users and avoids inventing a new key assignment scheme.

### Important issues and suggestions

#### 1. Use the one-time boot keys mechanism to prevent double menu display

The plan mentions needing an "in-memory flag" to prevent a second kernel from showing the menu. I'd suggest that the one-time boot keys mechanism at A100h is sufficient by itself, with no extra flag needed. Here's why:

- The menu **always** writes RAM keys (for all three exit paths: ENTER, N, and ESC), with the **N bit cleared**.
- When the next kernel calls `DO_KEYS_INIT`, `SCANKEYS_RAM` finds the RAM keys and loads them instead of scanning the keyboard.
- Since the N bit is cleared in the RAM keys, the next kernel's "show menu?" check fails.
- For **ESC** specifically: write RAM keys = current BOOTKEYS with N bit cleared. This preserves all other key states while preventing menu re-display.
- For **ENTER**: write RAM keys = user's toggled selections with N bit cleared.
- For **N (disable all)**: write RAM keys = all slot disable keys set, N bit cleared.

This is fully self-consistent and requires no separate flag.

#### 2. Separate the N check from DISABLE_KEY

Currently `DISABLE_KEY` checks N first (line 226-229) and returns NZ (disabled) if N is pressed. The new design should split this:

- `DISABLE_KEY` checks **only per-slot keys** (remove the N check)
- A separate check for N triggers the menu at the right point in the init flow

The revised init flow for the master kernel would be:
```
CHK_DUP → DO_KEYS_INIT → DISABLE_KEY (per-slot only) → CHK_DRIVER →
DISKID check → [if master AND N pressed → SHOW_MENU → re-run DISABLE_KEY] →
continue normal init
```

The re-run of `DISABLE_KEY` after the menu is important: if the user disabled the current kernel in the menu, the updated BOOTKEYS will have its per-slot key set, and DISABLE_KEY will return NZ.

#### 3. Don't show the menu when using programmatic RAM keys

If the NEXTOR_BOOT_KEYS signature already exists at A100h when the kernel boots (someone used NEXBOOT.COM), the menu should **not** be shown, even if those RAM keys have the N bit set. RAM keys are meant for automated/programmatic boot control, and showing an interactive menu would defeat the purpose.

This is already handled naturally: `DO_KEYS_INIT` calls `SCANKEYS_RAM` first, which loads from RAM and skips keyboard scanning. Then KEYS_INV is not applied. But you should explicitly verify that programmatic N-in-RAM-keys doesn't trigger the menu. The check could be: "show menu if N is in BOOTKEYS AND keys came from keyboard (not from pre-existing RAM keys)." A simple flag set by `SCANKEYS_RAM` when it finds pre-existing keys would work.

#### 4. NEXTOR3 signature placement: use 400Ah-400Fh

The MSX ROM header has 6 reserved bytes at 400Ah-400Fh (the last three `defw 0` in doshead.mac at lines 25-27). These are safe to use. A short signature like `"NXT3"` (4 bytes) at 400Ah would be sufficient when combined with the "AB" check at 4000h. The remaining 2 bytes could hold a flags/version byte.

The scanning algorithm for each slot would be:
1. `RDSLT` at 4000h/4001h → check for "AB" (safe, read-only)
2. `RDSLT` at 400Ah-400Dh → check for "NXT3" (safe, read-only)
3. If both match: `CALSLT` + `CALDRV` at 4048h → `DRIVER_QUERY` with A=2, B=1 to get the driver name string

Step 3 is safe because CALDRV/CALBNK in the target ROM are in doshead.mac (ROM code, no initialization needed), and the bank switching code at CHGBNK is also ROM code. BK4_ADD is safe to use at this boot stage since no timer interrupt hooks have been set up yet.

Important: **don't use the full 8 bytes** including 4008h-4009h. That word is the "BASIC program text" pointer in the MSX ROM header, and some BIOSes or tools might check it. The 6 bytes at 400Ah-400Fh are truly unreserved.

#### 5. The driver name query without initialization

This requirement is well-identified in the plan. Looking at the existing drivers (StandaloneASCII8, SunriseIDE, MegaFlashRomSD), `DRVQ_GET_STRING` just copies a constant string - it has no dependency on `DRVQ_INIT`. This should be fine for well-behaved drivers.

However, the `DRVQ_GET_STRING` implementation uses `OUTPUT_STRING` which takes `HL`=source, `DE`=destination buffer, `D`=buffer size. The buffer address is in the master kernel's RAM space. Since `CALSLT` doesn't change the RAM page mapping (page 3 is always RAM), the target driver can write to the buffer address provided by the master. This should work.

One subtle point: the buffer size `D` should account for the "..." suffix. If the driver returns `QUERY_TRUNCATED_STRING`, the menu needs 3 extra characters for "...". So pass `D = available_width - 3` when querying, and append "..." if truncated.

#### 6. Screen state considerations

At the point where the menu would be shown, the screen is in whatever state the BIOS left it (usually 40-column text mode after power-on). The code needs to:

- Detect MSX version (check MSXVER at 002Dh or the version byte at FCC1h) for 40/80 column decision
- Call `INITXT` (006Ch) or set screen mode via BIOS to ensure text mode
- For 80 columns on MSX2+: set `LINL40` (F3AEh) to 80 before calling INITXT, or use the BIOS `WIDTH` routine

After the menu is dismissed, the normal init flow's `OVERRIDE` code sets up the screen properly, so no special cleanup is needed.

#### 7. What about mixed Nextor 2 and Nextor 3 scenarios?

Nextor 2 kernels won't have the "NXT3" signature, so they won't appear in the menu's kernel list. They'll be cleaned up by `CLEAN_V2_KERNELS` regardless. This seems correct - the boot menu is a Nextor 3 feature and shouldn't try to manage Nextor 2 kernels.

MSX-DOS kernels (non-Nextor) also won't appear. The "7. Disable MSX-DOS kernels (SHIFT)" option in the menu provides the mechanism for controlling those.

### Questions

1. **Key 0 (Disable permanent disk emulation)**: Should this option always appear in the menu, or only when disk emulation is actually active? Showing it always is simpler and serves as a memory aid, but might confuse users who don't use disk emulation.

2. **5th kernel display**: If 5 Nextor 3 kernels are detected, the 5th can't be registered (KERNEX has 4 slots). Should the menu indicate this limitation? Or should the 5th kernel simply be shown like the others, with the understanding that the existing KERNEX overflow behavior applies?

3. **Key 3 (boot to BASIC) default state**: With `KEYS_INV_0 = 00h` (default), key 3 is not inverted, meaning the asterisk appears only when the user toggles it in the menu. Is the intended behavior that key 3 is **off** by default (normal boot) and the user toggles it **on** to force BASIC? That's what the design implies and it seems correct, just want to confirm.

4. **What if the master kernel's own driver signature check (`CHK_DRIVER`) fails?** Currently the system halts. With the new flow, should the menu be shown before `CHK_DRIVER`? That would allow the user to disable a broken kernel, but it would also mean showing the menu from a kernel that can't validate its own driver. I'd suggest keeping `CHK_DRIVER` before the menu check - a kernel with a bad driver shouldn't be showing a menu.

5. **Timing of the menu relative to the "beep"**: Currently the first kernel plays a beep during init (line ~897). Should the menu appear before or after the beep? Before seems better (immediate visual feedback), and the beep could happen after the menu is dismissed.

---

## User

First, about your questions:

1. The "permanent disk emulation mode" is the one that's reigstered by writing data in the partition table of the first device available in the main kernel (see the second table in https://github.com/Konamiman/Nextor/blob/v2.1/docs/Nextor%202.1%20Programmers%20Reference.md#722-entering-disk-emulation-mode). We would need to read that to verify if the data exists, but we can't access any device before the kernels are initialized, therefore we must always show this menu entry.
2. Five kernels will be a very rare case, so let's keep thing simple and just show them with the understanding that no all of them will be initialized.
3. Yes, as it's the case with all the other keys: initially show the asterisk only if the key is inverted (so considered pressed by default) in KEYS_INV_0/1.
4. Good point. But also, what if any of the other drivers has a bad driver too? It won't be possible to get its name. In this case the name shown should be "(BAD DRIVER)" and it shouldn't be possible to mark it as active.
5. Beep generally means "all good, let's boot" so yes, after the menu makes sense.

Now about other things you mentioned:

- Signature placement: I don't like the idea of using 400Ah-400Fh because the MSX standard explicitly says that this should be zero. It's very likely safe to use it anyway, but sill, I prefer to keep things clean. There's actually one 5 byte gap in that code at 407B, that's enough for "NXT3" with a zero terminator so let's do that (I actually placed the signature there already).
- Screen state considerations: the screen is already being initialized before passing control to the initialization routine of the first driver, so maybe we just need to move that part back in the boot procedure. By the way look at PrintRuler() in fdisk.c for an important gotcha.

---

## Claude

[Read doshead.mac lines 119-133 - confirmed NXT3_SIGNATURE already at 407Bh]
[Read init.mac lines 870-999 - reviewed the first cartridge / OVERRIDE flow]
[Searched for PrintRuler in fdisk.c - found the Korean MSX gotcha]
[Read init.mac lines 1000-1089 - reviewed VER_CHK and OVERRIDE code]
[Read init.mac lines 3540-3564 - reviewed CALL_DRIVER_QUERY]
[Searched for H_CHPH references - confirmed address FDA4h]

Good, the Korean MSX gotcha in PrintRuler and the existing signature placement are clear. Let me also check the OVERRIDE screen init flow and how `DRVQ_GET_STRING` would be called on other slots.

Now I have a complete picture. Here's my updated analysis incorporating your feedback:

---

## Updated Design Analysis

### Signature: confirmed at 407Bh

The `NXT3_SIGNATURE` at 407Bh fits perfectly in the 5-byte gap between `jp BIOS_IN##` (4078h-407Ah) and RAMENT (4080h). Clean and doesn't touch any reserved header bytes.

### Korean MSX gotcha

The `PrintRuler` in fdisk.c checks if H_CHPH (FDA4h) is hooked (not 0xC9). Korean MSX computers do weird things when printing a character at the last screen column. The boot menu's separator lines need the same check: print `width - 1` hyphens when H_CHPH is hooked.

### Screen initialization: move INITXT earlier

The INITXT call at `OVERRIDE` (line 1032-1039) already sets 40/80 columns based on MSXVER. This naturally becomes the screen setup for the menu. The menu check would be inserted right after INITXT, before the version/hook setup:

```
OVERRIDE:
    [set LINL40, call INITXT]       ← existing code
    [check N key + show menu]       ← NEW
    [set DOS_VER, NXT_VER, hooks]   ← existing code continues
```

The CLS at line 893 (first cartridge path) becomes redundant since INITXT clears the screen, but it's harmless.

### "(BAD DRIVER)" handling

For the scanning kernel itself: `CHK_DRIVER` runs before the menu and halts on failure, so the scanning kernel always has a valid driver.

For other kernels found during scanning: after confirming "NXT3" at 407Bh via RDSLT (safe), the menu code would try `DRVQ_GET_STRING` via CALSLT+CALDRV. The existing `PRINT_INIT_KERNEL_AT_ASLOT` at line 695 already handles three outcomes:
- `QUERY_OK` (A=0): display the name from buffer
- `QUERY_TRUNCATED_STRING` (A=1): display name + "..."
- Anything else: display fallback string

For the menu, the fallback would be "(BAD DRIVER)" instead of "(unnamed driver)". And crucially: a kernel shown as "(BAD DRIVER)" should be **locked as disabled** (no asterisk, toggling ignored). This prevents users from trying to boot with a broken driver.

One risk: if the ROM at the other slot has "NXT3" at 407Bh but corrupt bank-switching code at CHGBNK, the inter-slot call through CALDRV could crash rather than return an error. The NXT3 signature gives high confidence this won't happen (it implies a properly built Nextor 3 kernel), but a truly corrupted ROM is always possible. I think this is an acceptable risk - a ROM corrupt enough to have valid "NXT3" but broken CHGBNK would likely crash during normal boot anyway.

### Remaining design points I want to confirm

1. **One-time boot keys as the sole anti-re-display mechanism**: After the menu is shown, RAM keys are always written (all three exit paths), with N bit cleared. Subsequent kernels' `SCANKEYS_RAM` picks these up, never sees N, never triggers the menu. No extra flag needed. Does this match your intent?

2. **Programmatic RAM keys (NEXBOOT.COM) with N bit**: If someone writes RAM keys with N set via NEXBOOT.COM, the current design would NOT show the menu (since `SCANKEYS_RAM` returns Z and the keyboard is skipped). N-in-RAM-keys would need to be handled as "disable all kernels" the old way. Do you want to preserve this behavior, or should programmatic N-in-RAM-keys also trigger the menu?

3. **The disabled-via-commented-out code at lines 1046-1063**: This was an attempt to kill Nextor v2 kernels by simulating an N key press via RAM keys. It's disabled (`if 0`) because it also kills Nextor v3 kernels. With the boot menu, this code becomes unnecessary since `CLEAN_V2_KERNELS` handles v2 cleanup. Should this dead code be removed as part of the boot menu implementation?

4. **Menu insertion point for the "override" case (line 1001-1026)**: When a newer kernel overrides an older one, it also reaches OVERRIDE. At that point, a PREVIOUS Nextor 3 kernel may have already shown the menu and written RAM keys. So the overriding kernel's `DO_KEYS_INIT` would have loaded the RAM keys (no N). The menu wouldn't trigger. This is correct behavior - the previous master already showed the menu. Agreed?

---

## User

1 and 2: if the user uses NEXBOOT.COM or a similar tool to explicitly set the boot keys and reboot, it's naturally assumed that he exactly knows how the new boot must look like so the menu isn't needed.
3. This is leftover code that can go away.
4. Yes, agreed.

However, don't write any code now. Write the entire conversation for this session (including this) in .ai/SESSION.md and we'll continue at another time.

---

## Claude

[Wrote .ai/SESSION.md with a structured summary of all design decisions, technical details, and open items]

Done. The full session is saved in `.ai/SESSION.md` covering:

- All design decisions made (signature at 407Bh, one-time boot keys as anti-re-display, DISABLE_KEY split, init flow, screen init, Korean MSX gotcha, BAD DRIVER handling)
- Your answers to my questions (programmatic RAM keys skip menu, dead code removal, override case)
- Technical reference details (BOOTKEYS layout, RAM keys, inter-slot calls, key mappings, code locations)
- The scanning algorithm and screen layout analysis

Ready to pick up implementation from here in a future session.

---

## User

That document is fine for continuing the work. But for archival purposes, I want a verbatim copy of the entire converstaion, including my prompts and you answers. Write that at .ai/ARCHIVE.md

---

## Claude

[Wrote this file]
