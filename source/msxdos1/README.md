# MSX-DOS 1 system files: MSXDOS.SYS 1.03 and COMMAND.COM 1.11

Reverse engineered, commented source code of the two disk files of MSX-DOS 1, following the style and conventions of the rest of the Nextor sources. Both build into files that are identical to the originals, except for the padding areas (which in the original files contain leftovers of whatever was in memory when they were built, and here are just zeros) and the leftover bytes at the end of the original COMMAND.COM, which are not part of the program. These are the files needed to boot to the DOS prompt when Nextor runs in MSX-DOS 1 mode; they are included in the root directory of the tools disk image and the tools zip archive.

- `msxdos/`: MSXDOS.SYS 1.03 (`msxdos.mac` is the master file).
- `command/`: COMMAND.COM 1.11 (`command.mac` is the master file).
- `include/dos1.inc`: definitions shared by both. The MSX-DOS 1 system variables, disk ROM entry points, FCB layout, and the layout of the MSXDOS.SYS data area shared with COMMAND.COM.

The work started from the MSXDOS.SYS disassembly made by Luis C. Grosso in 2006.

## Building

You need [Nestor80](https://github.com/Konamiman/Nestor80) (`N80` in the path, or set the `N80` variable). The DOS function codes and some BIOS constants are taken from the [Nextor SDK](../../sdk).

```
make            # Builds MSXDOS.SYS and COMMAND.COM and copies them to bin/tools
make clean
```

## Relocation

Both programs copy most of themselves to high memory and adjust that code for its new address. They use a table that lists every byte holding the high byte of an internal address. The table is not maintained by hand.

The Makefile assembles the relocatable part twice, at two addresses one page apart. `cmp -l` lists the bytes that differ, which are exactly the ones needing relocation, and `tools/reltab.awk` turns them into the `reltab.inc` that the final assembly includes. This is the same idea as NEXTOR.SYS, which links its main code twice, but done at build time, so the original file format is kept.

The code can therefore be modified freely. The relocation table, and the padding up to the page boundaries the layout requires (generated with `.ALIGN`), adapt automatically.
