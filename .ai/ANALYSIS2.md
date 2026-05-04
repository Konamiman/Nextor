# SDK Extraction — Follow-up Questions (round 2)

Got it. A few of your answers raise follow-ups before I produce the final tree:

1. **`GENERATE_PUBLIC_SYMBOLS` mechanism.** The kernel uses a `const` macro
   (not bare `equ`) in `macros.inc`. Today `const NAME, VAL` likely emits both
   an equate *and* a `public` declaration. To honor the flag, I see two
   options:
   - (a) **Change the `const` macro itself** so the `public` half is
     conditional on `GENERATE_PUBLIC_SYMBOLS`. Cleanest, but it's a global
     behavior change for every file that uses `const`.
   - (b) **Keep `const` as-is for kernel files**, and use plain `equ` in the
     SDK `.inc` files plus a guarded `public` block at the bottom.

   Which do you prefer? (Note: Makefile already uses `--define-symbols NAME`
   with hyphens — I'll use that syntax.)

2. **`driver_workarea.inc` contents.** You named `MASTER_SLOT`, `TMP_IX`,
   `TMP_IY` as examples. I can sweep `data.mac` and the v3 drivers and propose
   a candidate list (page-3 vars that drivers actually reference, minus
   standard MSX work area). Do that, or do you already have a specific list in
   mind?

3. **Other C files in `source/tools/C/`.** You listed
   `crt0_msxdos_advanced.asm`, `asmcall.c/h`, `printf.c`. The directory also has
   `asm.h`, `dos.h`, `system.h`, `types.h`, `strcmpi.c/h`, `print_msxdos.c`,
   `partit.h`, and `asm.lib`. Which of these belong in the public C SDK?

4. **"Normalize everything else" — to what style?** `UPPER_SNAKE_CASE`? And in
   scope: just a few odd names (e.g. `?BDOS`, `FIB_FILE_NAME` vs `FIB_NAME`,
   `MASK_R_ONLY` vs `MASK_READ_ONLY`), or a broader pass? If you can name the
   symbols you want renamed, I'll keep the rest verbatim.

5. **`chgbnk_template.inc` style.** Skeleton with a `chgbnk:` label, documented
   inputs/outputs, and a comment block that says "fill in your hardware
   bank-switch logic here" — is that the right shape, or do you want a more
   abstract spec doc?

6. **`bdos_file_handles.inc` confirmation.** `codes.mac` has a "Standard File
   Handles" section (`STDIN`, `STDOUT`, `STDERR`, `STDAUX`, `STDLST`). That's
   what goes in this file, right?

7. **Tool-macros file name.** `sdk/asm/tool_macros.inc`, or do you prefer
   `shared.inc` to match the existing `SHARED.MAC`?

Once I have these I'll produce the concrete file-by-file plan.
