#!/usr/bin/env python3
"""
Verifies that the Nextor sources don't use undocumented Z80 instructions.

Nextor must also work on Z180-based MSX machines, which don't support them.
Code that needs the effect of an instruction operating on ixh, ixl, iyh or iyl
must use the macros in sdk/asm/macros/undoc.inc, which expand to documented
instructions.

The sources are checked and not the generated binaries, since code and data
are mixed in those and telling them apart isn't feasible (for example the
"FD 6A" that a "call 6AFDh" contains looks like "ld l,d" using IY as prefix).

These are checked, in the assembler sources and in the __asm blocks of the
C sources:

- The ixh, ixl, iyh and iyl registers (except as arguments for the macros).
- The sll instruction.
- The forms of in and out having no equivalent documented instruction:
  in f,(c), in (c), out (c),0 and out (c),f.
- The forms of the rotate/shift, set and res instructions that copy the result
  to a register: set 0,(ix+1),b and the like.

It also verifies that no makefile enables undocumented instructions in SDCC.

Usage: check-undoc.py [<repository root>]   (default: the current directory)
"""

import os
import re
import sys

SCAN_DIRS = ("source", "sdk")
ASM_EXTENSIONS = (".mac", ".asm", ".inc", ".s", ".z80")
C_EXTENSIONS = (".c", ".h")
SDCC_OPTION = "--allow-undocumented-instructions"

# The macros in undoc.inc, whose arguments are precisely the index register halves,
# plus the assembler statements that can be used to implement these macros.
HALVES_ALLOWED = {
    "ld_", "cp_", "or_", "and_", "xor_", "add_", "adc_", "sub_", "sbc_", "inc_", "dec_",
    "ifidn", "ifidni", "ifdif", "ifdifi", "_undoc_err",
}

# Optional label, optional "..." (the page 3 relocation macro) and optional opening
# angle bracket (the argument of that macro), then the statement itself.
STATEMENT = re.compile(r"^\s*(?:[\w.$?@]+:{1,2})?\s*(?:\.\.\.\s*)?<?\s*([^\s,<>]+)\s*(.*)$")

HALVES = re.compile(r"\b(ixh|ixl|iyh|iyl)\b", re.IGNORECASE)
INDEXED = r"\(\s*i[xy]\b[^)]*\)"
REGISTER = r"[abcdehl]\b"

CHECKS = [
    ("sll", re.compile(r"^.*$"), "sll instruction"),
    ("in", re.compile(r"^(f\s*,\s*)?\(\s*c\s*\)\s*$", re.IGNORECASE), "undocumented form of the in instruction"),
    ("out", re.compile(r"^\(\s*c\s*\)\s*,\s*(0+h?|f)\s*$", re.IGNORECASE), "undocumented form of the out instruction"),
    (("set", "res"), re.compile(rf"^[^,]+,\s*{INDEXED}\s*,\s*{REGISTER}", re.IGNORECASE),
     "undocumented form of the set/res instruction"),
    (("rlc", "rrc", "rl", "rr", "sla", "sra", "srl"), re.compile(rf"^{INDEXED}\s*,\s*{REGISTER}", re.IGNORECASE),
     "undocumented form of the rotate/shift instruction"),
]


def strip_comment(line):
    """Returns the code of an assembler line without the comment and with the contents of the strings blanked."""
    code = []
    i = 0
    while i < len(line):
        char = line[i]
        if char == ";":
            return "".join(code)
        # The closing quote can be missing for ex af,af' (that's not a string)
        is_alternate_af = char == "'" and "".join(code)[-2:].lower() == "af"
        end = line.find(char, i + 1) if char in "'\"" and not is_alternate_af else -1
        if end != -1:
            code.append(char + " " * (end - i - 1) + char)
            i = end + 1
        else:
            code.append(char)
            i += 1
    return "".join(code)


def check_asm_line(line):
    """Returns the description of the undocumented instruction that an assembler line has, or None."""
    code = strip_comment(line)
    match = STATEMENT.match(code)
    if not match:
        return None
    mnemonic = match.group(1).lower()
    operands = match.group(2).strip().rstrip(">").strip()

    halves = HALVES.search(code)
    if halves and mnemonic not in HALVES_ALLOWED:
        return f"{halves.group(1)} register (use the macros in sdk/asm/macros/undoc.inc)"

    for mnemonics, pattern, description in CHECKS:
        if mnemonic in ((mnemonics,) if isinstance(mnemonics, str) else mnemonics) and pattern.search(operands):
            return description
    return None


def asm_lines(path, lines):
    """The lines of a source file that contain assembler code, as (number, text) tuples."""
    if not path.lower().endswith(C_EXTENSIONS):
        yield from enumerate(lines, 1)
        return
    in_block = False
    for number, line in enumerate(lines, 1):
        if not in_block and "__asm" in line and "__endasm" not in line:
            in_block = True
        elif "__endasm" in line:
            in_block = False
        elif in_block:
            yield number, line


def check_file(path):
    """The problems of a source file, as (line number, description) tuples."""
    with open(path, "r", encoding="latin-1", newline=None) as f:
        lines = f.read().split("\n")
    if os.path.basename(path).lower() == "makefile":
        return [(n, f"{SDCC_OPTION} must not be used") for n, l in enumerate(lines, 1) if SDCC_OPTION in l]
    return [(n, problem) for n, l in asm_lines(path, lines) if (problem := check_asm_line(l))]


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else "."
    extensions = ASM_EXTENSIONS + C_EXTENSIONS
    checked = 0
    problems = 0
    for scan_dir in SCAN_DIRS:
        for directory, _, files in os.walk(os.path.join(root, scan_dir)):
            for name in sorted(files):
                if not name.lower().endswith(extensions) and name.lower() != "makefile":
                    continue
                path = os.path.join(directory, name)
                checked += 1
                for number, problem in check_file(path):
                    relative = os.path.relpath(path, root)
                    print(f"::error file={relative},line={number}::{problem}")
                    problems += 1

    print(f"{checked} files checked, {problems} problems found")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
