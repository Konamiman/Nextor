1. Yes, modify the `const` macro with an `ifdef GENERATE_PUBLIC_SYMBOLS` block.
2. Yes, propose a candidate list.
3. All of these belong, but I suspect that `asm.h` and `asm.lib` aren't actually used; if you can confirm that, these can go away.
4. On second thought don't normalize anything, keep names as-is, we'll later decide which symbols get renamed.
5. That's fine.
6. Yes, confirmed.
7. `shared.inc` is fine.
