Changes to the suggested directory structure:

- Two root level directories, `sdk` and `private_sdk`. It will be easier and cleaner for repos going the git submodule route to hydrate just `sdk/` thank `sdk/public/`.
- `driver_calls`: if these are the routines available in all banks at 4000h-40FFh, let's rename to `rom_bank_header`.
- Files related to drivers: `driver_routines` (for the signature and the jump table), `driver_driver_queries` (DRVQ_*), `driver_device_queries` (DEVQ_*) and `driver_error_codes`, `driver_workarea` (MASTER_SLOT, TMP_IX/IY and other page 3 addresses relevant for drivers, excluding the standard MSX work area).
- For C: include `crt0_msxdos_advanced.asm`, `asmcall.c/h` and `printf.c` (which are at `/source/tools/C`).

As for your questions:

1. Yes, include these too.
2. In the public SDK: only what makes sense for drivers (I guess CALSLT, RDSLT and a few more). In the private SDK: only whatever the kernel needs.
3. Same: public file with what makes sense for drivers, private file with what the kernel needs.
4. Let's make `codes.mac` disappear, replaced by `bdos_calls` and `bdos_errors` (also `bods_file_handles`). Note however that in `codes.mac` all values are defined as both a constant and a public symbol, this is required for compiling the kernel but it's probably overkill for external tools (constants are enough). Suggestion: define only the constant by default, enable the public symbols with `--define_symbols=GENERATE_PUBLIC_SYMBOLS`, and adjust the kernel makefile to include this when assembling the files.
5. Yes, refactor the kernel as part of the process.
6. It's at `/source/tools/C`.
7. Yes, confirmed.
8. These aren't the same: "not implemented" is equivalent to ok with default return values assumed, "init error" means that the driver initialization has failed.
9. Yes, `.inc`
10. The submodule is expected to map `sdk/` to a directory like `nextor_sdk/` in its own repository. Or if that's easier, `nextor/sdk/`.
11. Yes, public SDK.
12. Keep `_NAME` for BDOS function calls, `.NAME` for BDOS error codes, normalize everything else.
