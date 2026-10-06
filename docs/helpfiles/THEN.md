# THEN

_Versions: 2.40_

## Format

```
THEN [command]
```

## Purpose

Executes the `command`.

## Use

`THEN` is ignored, and the `command`, if given, is executed. This may be any internal or external command, batchfile or alias.

`THEN` is only provided for reasons of compatibility, when it is used in an `IF` or `IFF` command.

## Examples

```
THEN
```

Does nothing at all. A new command can be typed.

```
THEN BEEP
```

The command `THEN` is ignored and the command `BEEP` is executed.

---

[Back to the help index](INDEX.md)
