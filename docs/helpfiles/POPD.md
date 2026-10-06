# POPD

_Versions: 2.40_

## Format

```
POPD [/N]
```

## Purpose

Restores the current drive and directory.

## Use

The current drive and directory are changed to the last drive and directory, stored by the `PUSHD` command, which are subsequently removed from the list.

If the `/N` option is given, then only the last drive and directory are removed from the list. The current drive and directory are left unaltered.

See also the [`PUSHD`](PUSHD.md) command.

## Examples

```
POPD
```

The current drive and directory are changed to the last drive and directory, stored by the `PUSHD` command.

```
POPD /N
```

The last drive and directory, stored by the `PUSHD` command, are removed from the list.

---

[Back to the help index](INDEX.md)
