# MKDSK

_Versions: 3.0_

## Format

```
MKDSK filename[.DSK] S|D|H|size[K|M] [/R entries]
      [/F12|/F16] [/B1|/B2|/BS]
```

## Purpose

Creates an empty disk image file.

## Use

A file of the specified size is created, containing an empty FAT12 or FAT16 filesystem (a boot sector, two copies of the FAT and an empty root directory), ready to be mounted with `MAPDRV` (Nextor can mount images of up to 32M only) or used in disk emulation mode with `EMUFILE` (see the requirements below). If the file name has no extension, `.DSK` is added. An existing file with the same name is overwritten.

`S`, `D` and `H` create a standard floppy disk image: 360K single sided (1DD), 720K double sided (2DD) or 1.44M high density (2HD) respectively. Otherwise, the size of the file is given as a number from 1 to 65535, in kilobytes if it has no suffix or a `K` suffix, or in megabytes if it has an `M` suffix. The allowed sizes are 10K to 4096M.

By default the filesystem is FAT12 for sizes up to 16M and FAT16 for bigger sizes. `/F12` forces FAT12, which is possible for sizes up to 32M, and `/F16` forces FAT16, which needs a minimum of 4085 clusters: that is about 2M (2076K with the default root directory size). Thus FAT16 is not possible for the floppy disk images.

The cluster size is chosen automatically. For FAT12 filesystems up to 16M the smallest cluster size that keeps each copy of the FAT in three sectors is used, so that the image can be used in MSX-DOS 1 mode; for bigger FAT12 filesystems the cluster size is 8K. For FAT16 the clusters are 2K up to 128M, 4K up to 256M, and so on, up to 64K for sizes over 2G. If the maximum number of clusters is reached (1021 for FAT12 filesystems up to 16M, 4084 for other FAT12 filesystems, 65524 for FAT16) the filesystem is a bit smaller than the file, the rest of the file is unused.

`/R` sets the number of entries of the root directory. It is rounded up to a multiple of 16 (the entries that fit in a sector), and the maximum is 240 for FAT12 and 4080 for FAT16. The default is 512 for FAT16, and for FAT12: 64 for sizes under 360K, 112 under 1440K, 224 under 2880K and 240 otherwise.

`/B1`, `/B2` and `/BS` select the type of boot sector:

* `/B1`: An MSX-DOS 1 boot sector, with the MSX-DOS 1 boot code. When Nextor boots from a disk with this boot sector it switches to MSX-DOS 1 mode. Undeletion is not supported since the boot sector has no volume id.
* `/B2`: An MSX-DOS 2 boot sector, the same that the `FORMAT` command creates on floppy disks: the MSX-DOS 1 boot code plus a volume id. These disks boot MSX-DOS 2 (or Nextor) normally, and MSX-DOS 1 in MSX-DOS 1 mode.
* `/BS`: A standard boot sector (as in PC disks), with a volume id. These disks boot MSX-DOS 2 (or Nextor) only: booting from them in MSX-DOS 1 mode hangs the computer.

`/B1` and `/B2` are available for FAT12 only. The default is `/B2` for the FAT12 images with three or less sectors per FAT (`S`, `D` and the other FAT12 images up to 16M), and `/BS` for any other image, including `H`.

Disk emulation mode always runs in MSX-DOS 1 mode, so the images used with `EMUFILE` must be FAT12 with three or less sectors per FAT, and have an MSX-DOS 1 or MSX-DOS 2 boot sector. The images created with the default options meet these requirements for sizes up to 16M, except `H` (it has nine sectors per FAT).

`S`, `D` and `H` can be combined with the other options to create non-standard floppy disk images, for example with a smaller root directory; note however that the floppy disk drives of MSX-DOS 1 identify the disk format by the media ID only, so they will misread such disks.

When the file is created, the number and size of the clusters, the number of root directory entries and the type of boot sector are displayed. If an error occurs (or the program is aborted) while the file is being created, the file is deleted.

This command works in MSX-DOS 2 too.

## Examples

```
MKDSK GAMES D
```

Creates a standard 720K floppy disk image in the file `GAMES.DSK`.

```
MKDSK DATA.IMG 100M /R 1024
```

Creates a 100M image with a FAT16 filesystem and 1024 entries in the root directory.

```
MKDSK SMALL 200K /B1
```

Creates a 200K image with an MSX-DOS 1 boot sector.

---

[Back to the help index](INDEX.md)
