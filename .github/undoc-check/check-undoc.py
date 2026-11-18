#!/usr/bin/env python3
"""
Verifies that the Nextor sources don't use undocumented Z80 instructions.

Nextor must also work on Z180-based MSX machines, which don't support them.
Code that needs the effect of an instruction operating on ixh, ixl, iyh or iyl
must get it with documented instructions (e.g. "push iy / pop af" to get IYh
in A, or "ld iy,(nn)" to load the two halves at once).

The sources are checked and not the generated binaries, since code and data
are mixed in those and telling them apart isn't feasible (for example the
"FD 6A" that a "call 6AFDh" contains looks like "ld l,d" using IY as prefix).

These are checked, in the assembler sources and in the inline assembler code
of the C sources (__asm ... __endasm and __asm__("...")):

- The ixh, ixl, iyh and iyl registers.
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
    if halves:
        return f"{halves.group(1)} register"

    for mnemonics, pattern, description in CHECKS:
        if mnemonic in ((mnemonics,) if isinstance(mnemonics, str) else mnemonics) and pattern.search(operands):
            return description
    return None


def split_c_line(line, in_comment):
    """
    Returns a line of C code with the comments blanked, twice: with the contents of the
    strings and character literals blanked too ("code") and with them intact ("text").
    Both have the same length as the line. in_comment tells if a /* comment is open at
    the start of the line, and the updated value is returned too.
    """
    code = list(line)
    text = list(line)

    def blank(start, end, *targets):
        for target in targets:
            target[start:end] = " " * (end - start)

    i = 0
    while i < len(line):
        if in_comment:
            end = line.find("*/", i)
            stop = len(line) if end == -1 else end + 2
            blank(i, stop, code, text)
            in_comment = end == -1
            i = stop
        elif line.startswith("//", i):
            blank(i, len(line), code, text)
            break
        elif line.startswith("/*", i):
            blank(i, i + 2, code, text)
            in_comment = True
            i += 2
        elif line[i] in "\"'":
            end = i + 1
            while end < len(line) and line[end] != line[i]:
                end += 2 if line[end] == "\\" else 1
            blank(i + 1, min(end, len(line)), code)
            i = end + 1
        else:
            i += 1
    return "".join(code), "".join(text), in_comment


def decode_c_string(string):
    """The value of the contents of a C string literal."""
    escapes = {"n": "\n", "t": "\t", "r": "\r"}
    return re.sub(r"\\(.)", lambda m: escapes.get(m.group(1), m.group(1)), string)


def string_asm_lines(pieces):
    """
    The instructions of a __asm__("...") statement, as (number, text) tuples, given the
    (number, contents) tuples of its string literals. Each instruction gets the number of
    the line where it starts.
    """
    instruction, start = "", None
    for number, contents in pieces:
        parts = contents.split("\n")
        for index, part in enumerate(parts):
            if start is None and part.strip():
                start = number
            instruction += part
            if index < len(parts) - 1:
                if start is not None:
                    yield start, instruction
                instruction, start = "", None
    if start is not None:
        yield start, instruction


# __asm__("...") or __asm ... __endasm, outside of comments and strings
C_ASM_START = re.compile(r"\b__asm__\s*\(|\b__asm\b")
C_ASM_END = "__endasm"
C_STRING = re.compile(r'" *"')


def splice_c_lines(lines):
    """
    The lines of a C source file with the ones ending with a backslash joined to the next
    one, as the C preprocessor does, as (number of the first line, text) tuples.
    """
    start, spliced = None, ""
    for number, line in enumerate(lines, 1):
        if start is None:
            start = number
        if line.endswith("\\"):
            spliced += line[:-1]
            continue
        yield start, spliced + line
        start, spliced = None, ""
    if start is not None:
        yield start, spliced


def c_asm_lines(lines):
    """The assembler code of a C source file, as (line number, text) tuples."""
    in_comment = False
    in_block = False
    string_pieces = None  # (number, contents) of the strings of an unfinished __asm__(
    for number, line in splice_c_lines(lines):
        if in_block:
            end = line.find(C_ASM_END)
            if end == -1:
                yield number, line
                continue
            yield number, line[:end]
            in_block = False
            line = " " * (end + len(C_ASM_END)) + line[end + len(C_ASM_END):]

        code, text, in_comment = split_c_line(line, in_comment)
        position = 0
        while True:
            if string_pieces is not None:
                close = code.find(")", position)
                stop = len(code) if close == -1 else close
                string_pieces += [
                    (number, decode_c_string(text[m.start() + 1:m.end() - 1]))
                    for m in C_STRING.finditer(code, position, stop)
                ]
                if close == -1:
                    break
                yield from string_asm_lines(string_pieces)
                string_pieces = None
                position = close + 1
                continue

            start = C_ASM_START.search(code, position)
            if not start:
                break
            if start.group(0) != "__asm":
                string_pieces = []
                position = start.end()
                continue
            end = code.find(C_ASM_END, start.end())
            if end == -1:
                yield number, line[start.end():]
                in_block = True
                break
            yield number, line[start.end():end]
            position = end + len(C_ASM_END)


def asm_lines(path, lines):
    """The lines of a source file that contain assembler code, as (number, text) tuples."""
    if path.lower().endswith(C_EXTENSIONS):
        yield from c_asm_lines(lines)
    else:
        yield from enumerate(lines, 1)


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
