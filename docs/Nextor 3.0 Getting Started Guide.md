# Nextor 3.0 Getting Started Guide

## Index

[1. Introduction](#1-introduction)

[2. Setting up blueMSX](#2-setting-up-bluemsx)

[2.1. Setting up a real MSX](#21-setting-up-a-real-msx)

[3. Booting and creating partitions](#3-booting-and-creating-partitions)

[4. Mapping partitions and changing the boot partition](#4-mapping-partitions-and-changing-the-boot-partition)

[5. Booting in MSX-DOS 1 mode](#5-booting-in-msx-dos-1-mode)

[6. Using a second storage device](#6-using-a-second-storage-device)

[7. Locking drives](#7-locking-drives)

[8. The reduced/zero allocation information mode](#8-the-reducedzero-allocation-information-mode)

[9. Using the boot keys and the boot menu](#9-using-the-boot-keys-and-the-boot-menu)

[10. Installing drivers in RAM](#10-installing-drivers-in-ram)


## 1. Introduction

Nextor is an enhanced version of MSX-DOS 2, the disk operating system for MSX computers. It is based on MSX-DOS 2.31, with which it is 100% compatible.

This document provides a walkthrough of the features that Nextor adds to MSX-DOS in the form of a step-by-step guide. In order to make it easier for everyone, this guide assumes that the blueMSX emulator will be used for the walkthrough together with an emulated Sunrise IDE controller. Users of other emulators will need to adapt the steps of _[2. Setting up blueMSX](#2-setting-up-bluemsx)_ appropriately; on the other hand, users of real MSX hardware should take a look at _[2.1. Setting up a real MSX](#21-setting-up-a-real-msx)_.

The reader of this document is assumed to have experience with MSX-DOS 2 at least at the user level. Also, it is a good idea to get acquainted with Nextor by reading _[Nextor 3.0 User Manual](Nextor%203.0%20User%20Manual.md)_ prior to (or in parallel with) this document.

Note: in this guide the MAPDRV, LOCK, RALLOC and DRIVERS commands will be used. These are internal commands of COMMAND3.COM, the command interpreter of Nextor 3, so no `.COM` file is needed to run them (they also exist as standalone tools, which work with older interpreter and Nextor versions as well); the DRVROP.COM tool will be used too. The other Nextor tools are simple enough for not needing a step-by-step usage explanation; remember however that all the commands and tools are explained in the _[Nextor 3.0 User Manual](Nextor%203.0%20User%20Manual.md)_, that HELP followed by a command name gives the full details when the help files of the tools disk are available, and that you can get a parameters summary for any of the standalone tools by invoking it without parameters.

## 2. Setting up blueMSX

This section explains the steps needed to set up blueMSX in order to follow this guide. The blueMSX emulator can be downloaded at http://www.bluemsx.com. 

a. Download the following files:

* The Nextor kernel with the Sunrise IDE driver, from [the latest release of the Sunrise IDE driver repository in GitHub](https://github.com/Konamiman/SunriseIDE-Nextor-driver/releases/latest). Please choose the file with _.SunriseIDE.blueMSX.ROM_ extension (the _.SunriseIDE.ROM_ version works but only recognizes the slave IDE device in blueMSX), if you are using another emulator or a physical Sunrise IDE (or compatible) controller hardware you can use the file with _.SunriseIDE.ROM_ extension instead.

* The Nextor tools disk image (_tools.dsk.zip_), from [the latest release of Nextor in GitHub](https://github.com/Konamiman/Nextor/releases/latest).

b. Run blueMSX and select the _Tools - Machine Editor_ menu.

c. Select the machine configuration named "MSX2".

d. Select the Memory tab and press the _Add..._ button.

e. In the Slot list select "Slot 1". In the _Type_ list select "Sunrise IDE".

f. In the _File_ box browse the Nextor kernel file you have downloaded in step a (or simply paste the whole file path). Then press the Ok button to close the window.

g. Press the _Save as..._ button and save the machine configuration with the name "MSX2 with Sunrise IDE and Nextor". Then press the Close button to close the window.

h. Select the _File - Disk Drive A - Insert_ menu. Select the Nextor tools disk image file you have downloaded in step a.

i. Select the _File - Hard Disk - IDE Sunrise Primary - Insert New Disk Image_ menu. In the _Disk Size_ list select "100 MB". Give the disk image any name you want and click the _Open_ button.

Now you have an emulated MSX2 with a Sunrise IDE controller that has the Nextor kernel in its flash ROM and a 100MB hard disk as the master device. Also, you have the Nextor tools disk in the floppy disk drive.

### 2.1. Setting up a real MSX

If you want to follow the steps of this guide by using a real MSX computer with a Sunrise IDE controller and a floppy disk drive instead of an emulator, you will need to do the following:

* Download the Nextor kernel with Sunrise IDE driver (the file with extension _.SunriseIDE.ROM_) from [the latest release of the Sunrise IDE driver repository in GitHub](https://github.com/Konamiman/SunriseIDE-Nextor-driver/releases/latest).

* Copy the NEXTOR.SYS and COMMAND3.COM files and the Nextor tools to a floppy disk. You have two options:

    * Download the Nextor tools disk image file and transfer its contents to a floppy disk.

    * Download the Nextor tools ZIP file (_tools.zip_) and uncompress it to a floppy disk, together with the NEXTOR.SYS and COMMAND3.COM files.

Note: NEXTOR.SYS and the Nextor tools are available as assets in [the latest release of Nextor in GitHub](https://github.com/Konamiman/Nextor/releases/latest); the Nextor kernel files with the Sunrise IDE driver are released in [the Sunrise IDE driver repository](https://github.com/Konamiman/SunriseIDE-Nextor-driver/releases/latest).

* Copy the MSXDOS.SYS and COMMAND.COM files to the floppy disk, these are necessary for the steps that involve booting in MSX-DOS 1 mode. These files are included in the Nextor tools disk image file, but can be found on other places on Internet too (search for "MSXDOS.SYS" on any search engine).

* Insert the floppy disk in the drive and attach an IDE device (hard disk or compact flash card) of at least 100MB to the Sunrise IDE controller as the master device, or to the first card slot if you are using a CF reader. The device should be completely blank, that is, it should not contain any partition or filesystem; otherwise you may get different results from those described in the next section until you repartition the device.

Please note that if you use an existing IDE device, **following the steps of this guide will delete all the data on the device**.
 
## 3. Booting and creating partitions

Once all the hardware (real or emulated) is in place, now we'll see what happens when we try to boot without having defined any partition for the IDE device, then we'll create and prepare some partitions.

a. Boot your MSX. You will see a sequence of boot messages like this:

![Nextor kernel boot messages](img/gsg/KernelBootMessage.png)

then after a few seconds you'll see that the computer boots in the DOS prompt in drive C:, as in the following image:

![The DOS prompt with drive C: as the current drive](img/gsg/NextorPrompt.png)
 
***What has happened?*** Nextor has assigned two drives to the Sunrise IDE driver: one for the master device (A:) and one for the slave device (B:), then it has assigned two drives to the floppy disk drive (C: and its ghost drive D:). Then it has attempted to search a suitable FAT12 or FAT16 partition in the hard disk attached to the master device in order to assign it to drive A:, but the hard disk has no partitions yet; as for the slave device, it's offline (not available). Therefore drive C:, assigned to the floppy disk, is the first "valid" drive, and so it is used as the boot drive.

**Note:** If you had chosen/flashed the _.SunriseIDE.blueMSX.CTRL\_INV.ROM_ variant of the ROM file you would have only drive C: assigned to the floppy disk drive (no ghost drive in D:). This variant inverts the meaning of the CTRL key at boot time. See ["Boot key inverters" in the Nextor 3.0 User Manual](Nextor%203.0%20User%20Manual.md#2101-boot-key-inverters).

b. Type BASIC and then CALL FDISK to invoke the device partitioning tool.

c. Press "1" to select the Sunrise IDE driver, then "1" again to select the hard disk (devices are listed with the name of the storage medium they contain, together with their size).

d. If you were using an already partitioned device, press "D" and then "y" to delete the existing partitions.

e. Press "P" to add a partition, then type "50" and press ENTER to create a 50MB partition.

f. Repeat step e but this time create a 25MB partition.

g. Repeat step e but this time create a 16MB partition.

h. Press "A" to use all the remaining disk space for yet another partition (the exact size to be used is displayed as part of the menu option itself).

i. Press "S" to see the defined partitions. You should see four partitions, similar to the following image:

![FDISK partition list showing the four partitions just created](img/gsg/FourPartitionsList.png)

Note: the asterisk "*" next to the partition number means that the partition will be created with the "active" flag set in the partition table. We'll see what this implies in the next section.

j. Press ESC to return to the main menu, then press "W" to create the partitions on the disk. Press "y" on the data destroy warning prompt.

k. Reset your MSX. You will see that the computer boots in the BASIC prompt.
 
***What has happened?*** Nextor has assigned drive A: to the IDE controller again. However, since now the attached device has valid partitions, Nextor has assigned the first suitable partition found (the 50MB one, which was created with the "active" flag set) to drive A:, so this is now the boot drive. But since the partition does not have the Nextor boot files yet, the system boots in the BASIC prompt.

l. Copy the Nextor system files from the floppy disk to drive A: by executing the following commands:

```
COPY "C:NEXTOR.SYS" TO "A:"
COPY "C:COMMAND3.COM" TO "A:"
```

m. Reset your computer and you will see that now it boots in the DOS prompt from drive A:. Issue a DIR command and you should see the following (notice the 50M free space):

![DIR on drive A: listing the two system files, with 50M free](img/gsg/DirTwoSystemFiles.png)

## 4. Mapping partitions and changing the boot partition

So far we have seen that Nextor has performed an automatic drive to partition mapping. Now we'll see how to instruct Nextor to select a different partition at boot time, and how to perform manual mapping after the system has booted.

a. Assign partition 2 to drive E: by executing the following command:

```
MAPDRV E: 2 1 0
```

("2" is the second partition, and "1 0" means "device index 1 controlled by driver in the primary Nextor controller". MAPDRV is an internal command of COMMAND3.COM; see _[3.4.1. MAPDRV: the drive mapping tool](Nextor%203.0%20User%20Manual.md#341-mapdrv-the-drive-mapping-tool)_ in the Nextor 3.0 User Manual for more details.)

b. Perform a `DIR E:` command. Check that drive E: has indeed 25MB of free space.

c. Copy the Nextor system files to drive E: with the following commands:

```
COPY NEXTOR.SYS E:
COPY COMMAND3.COM E:
```

d. Type BASIC and then CALL FDISK to invoke the device partitioning tool.

e. Press "1" to select the Sunrise IDE driver, then "1" again to select the hard disk, then "S" to show the partitions list.

f. Press "1" and then "y" to remove the "active" flag from the first partition in the partition table.

g. Press "2" and then "y" to set the "active" flag for the second partition in the partition table.

h. Reset your MSX, and when you are in the DOS prompt, perform a DIR command and check that this time drive A: is assigned to the second (25MB big) partition:

![DIR showing drive A: mapped to the 25M second partition](img/gsg/DirInSecondPartition.png)

***What has happened?*** When performing the automatic drive to device and partition mapping assignment at boot time, Nextor selects the first valid (FAT12 or FAT16) partition available that has the "active" flag set in the partition table (and has not been assigned to another drive already). You can set and reset this flag for any of the first 9 partitions in the device using FDISK (either partitions that you are creating, or already existing partitions).

If none of the existing partitions has the "active" flag set in the partition table, then the first suitable partition found will be mapped. We'll see this in the next section.

i. Type BASIC and then CALL FDISK again.

j. Press "1" to select the Sunrise IDE driver, then "1" again to select the hard disk, then "S" to show the partitions list.

k. Press "1" and then "y" to set the "active" partition flag for the first partition. Now both partitions 1 and 2 should have the flag set.

l. Reset the computer and run `DIR A:` then `DIR B:`, you should see that A: is now mapped to the first partition and B: to the second one:

![DIR A: and DIR B: showing A: mapped to the first partition and B: to the second one](img/gsg/DirAthenDirB.png)

m. Run `BASIC` and once in the BASIC prompt run `CALL DRVINFO`, see how aside from A: and B: mapped to the master device, there's C: now mapped to the slave device, and D: and E: mapped to the internal floppy disk drive:

![CALL DRVINFO listing A: and B: on the master device, C: on the slave device, and D: and E: on the floppy disk drive](img/gsg/DrvinfoABCDE.png)

***What has happened?*** In Nextor 3 each existing partition having the "active" flag set will get a drive mapped at boot time; this is a change from Nextor 2 where only one drive would be mapped per device. The IDE master device has now two such partitions, so it gets A: and B:. The mapping process then continues as before: the slave device, being offline, gets one single drive, C:; and the internal floppy disk drive gets D: and E:.
 
## 5. Booting in MSX-DOS 1 mode

Now we will see what happens with the automatic partition assignment process when we try to boot in MSX-DOS 1 mode.

a. Go back to the DOS prompt (run `CALL SYSTEM`) and map partition 3 to drive F: by executing the following:

```
MAPDRV F: 3 1 0
```

b. Copy the MSX-DOS 1 system files to drive F: by executing the following:

```
COPY D:MSXDOS.SYS F:
COPY D:COMMAND.COM F:
```

c. Reset the computer while keeping pressed the "1" key. You will boot in MSX-DOS 1 mode and in the COMMAND.COM prompt. Issue a DIR command and you should see the following (notice the 16M free space):

![DIR in MSX-DOS 1 mode showing 16M free](img/gsg/DirInDos1Mode.png)

***What has happened?*** When booting in MSX-DOS 1 mode, Nextor performs the automatic drive to device and partition assignment, as when booting in normal mode. However, this time the partitions that can't be handled in MSX-DOS 1 mode are skipped during the assignment procedure; MSX-DOS 1 can handle only FAT12 partitions with three or less sectors per FAT. The first partition (50MB big) is skipped because it is FAT16. The second partition (25MB big) is skipped because it is FAT12 but has more than three sectors per FAT. Thus the third partition (16MB big) becomes the first one available, and it is then mapped to drive A:.

Remember that all partitions of 16MB or less created with the Nextor built-in partitioning tool are MSX-DOS 1 compatible.

d. Reset the computer in normal mode again.

e. Map partition 4 to drive F: by executing the following:

```
MAPDRV F: 4 1 0
```

f. Copy the MSX-DOS 1 system files to drive F: as you did in step b.

g. Go to BASIC, run FDISK and go to the partitions list for the first device, as you did in the previous section. This time set the "active" flag for partition 4. You should see that partitions 1, 2 and 4 are active now:

![FDISK partition list with the active flag set on partitions 1, 2 and 4](img/gsg/Partitions124Active.png)

h. Boot the computer while keeping pressed the "1" key. You will boot in MSX-DOS 1 mode and in the COMMAND.COM prompt again. Issue a DIR command and you should see the following (notice the 9M free space):

![DIR in MSX-DOS 1 mode showing 9M free](img/gsg/DirInDos1ModeWithPartition4Active.png)

***What has happened?*** The rule "give priority to partitions having the 'active' flag set during the automatic drive to device and partition assignment at boot time" is valid when booting in MSX-DOS 1 mode too, but this time only the MSX-DOS 1 compatible partitions are checked (partitions 3 and 4 in this case; partition 4 is the "winner").

i. Go to BASIC, run FDISK and set partition 3 as active too.

j. Reset the computer while keeping pressed the "1" key again. Run `DIR A:` and you'll see 16MB free (partition 3), then run `DIR B:` and you'll see 9MB free (partition 4).

![DIR A: and DIR B: in MSX-DOS 1 mode showing 16M and 9M free respectively](img/gsg/DirAthenDirBinDos1Mode.png)

k. Go to BASIC and run `CALL DRVINFO`, you'll see A: and B: assigned to the master device, C: assigned to the slave device, and D: and E: assigned to the internal floppy disk drive, as when you booted in normal mode with only partitions 1 and 2 active.

![CALL DRVINFO listing A: and B: on the master device, C: on the slave device, and D: and E: on the floppy disk drive](img/gsg/DrvinfoABCDE.png)

***What has happened?*** The rule "each active partition gets a drive mapped at boot time" is valid when booting in MSX-DOS 1 mode too, but this time only the MSX-DOS 1 compatible partitions are checked (partitions 3 and 4 in this case).

The remaining steps will leave the environment ready for the next section.

l. Boot the computer in normal mode.

m. Go to BASIC, run FDISK and reset the "active" flag of all four partitions, so that none of them has the flag set.
 
## 6. Using a second storage device

Now we will see what happens when we add a second storage device to the IDE controller.

Step a applies when using the blueMSX emulator. If you are using a different emulator, you should perform the appropriate steps to attach a second 100M emulated hard disk as the slave device to the IDE controller. If you are using a real MSX, you should attach an (at least 100M big) real device configured as slave to the IDE controller, or insert an additional card in the second card slot.

a. Select the _File - Hard Disk - IDE Sunrise Secondary - Insert New Disk Image_ menu. In the _Disk Size_ list select "100 MB". Give the disk image any name you want and click the _Open_ button.

b. Reset the computer, go to BASIC and invoke the device partitioning tool with CALL FDISK.

c. Partition the device as you did in [3. Booting and creating partitions](#3-booting-and-creating-partitions). This time, however, you should select device 2 in the device selection screen; and you should create just two partitions, the first one having a size of 85M and the second one of 15M. Note that the first partition gets created with the "active" flag set (there's an asterisk next to its number in the partitions list); leave it like that.

d. Set the "active" flag on the first partition of the master device too: press ESC until you are back in the device selection screen, select device 1, press "S" to show its partitions, then press "1" and "y" to set the flag on the first (50M) partition.

e. Reset the computer. Once in the DOS prompt, issue a `DIR` command and check that drive A: has 50M free. Then issue a `DIR B:` command and check that drive B: has 85M free:

![DIR and DIR B: showing 50M free on drive A: and 85M free on drive B:](img/gsg/DirAAndDirB.png)

***What has happened?*** Nextor has assigned drive A: to the master device and drive B: to the slave device, as in the previous cases; the difference is that the slave device is now present and has valid partitions, so it has assigned the drive to the first active partition found in that device, which is the 85M one (note that a device without any active partition still gets one drive).

f. Go to `BASIC`, run FDISK, and remove the "active" flag of the first partition in the slave device.

g. Reset the computer and run `DIR` and `DIR B:` again, you'll see the exact same result as before: A: holds the 50M partition of the master device and B: holds the 85M partition of the slave device.

![DIR and DIR B: showing 50M free on drive A: and 85M free on drive B:](img/gsg/DirAAndDirB.png)

***What has happened?*** When suitable partitions are present in the device but none is flagged as active at boot time, Nextor defaults to assigning one single drive and mapping it to the first suitable partition found.

h. Copy the `NEXTOR.SYS` and `COMMAND3.COM` files to drive B:.

i. Go to BASIC, run FDISK and remove the "active" flag of partition 1 (50M) in the master device; also set it again in partition 1 (85M) of the slave device.

j. Reset your computer and issue a `DIR` command, then a `DIR B:` command. Notice that the mapping is reversed relative to the previous case (A: is mapped to the 85M partition on slave device, B: is mapped to the 50M partition on master device):

![DIR and DIR B: showing the reversed mapping, 85M free on drive A: and 50M free on drive B:](img/gsg/DirAAndDirBReversed.png)

***What has happened?*** The rule "give priority to partitions having the 'active' flag set during the automatic drive to device and partition assignment at boot time" applies to all partitions on all devices, not only to the first device. Taking into account both the master and the slave devices, the only partition having the "active" flag set is the first partition on the slave device, and therefore this one is assigned to drive A: (and since we copied the Nextor system files to it, the system has booted from it). Drive B: has then been mapped to the first suitable partition still unassigned, which is the 50M one on the master device.

k. Map partition 2 of slave device to drive B: (`MAPDRV B: 2 2 0`). Copy the MSXDOS.SYS and COMMAND.COM files to drive B: (remember that the floppy holding these files is now mapped in C:).
 
l. Go to BASIC, run FDISK, and set the "active" flag of partition 3 (16M) in the master device and of partition 2 (15M) in the slave device.

m. Reset the computer while keeping pressed the "1" key. You will boot in MSX-DOS 1 mode and in the `COMMAND.COM` prompt. Issue a `DIR` command, then a `DIR B:` command. Notice that in the first case you get 16M of free space, and in the second case you get 15M:

![DIR and DIR B: in MSX-DOS 1 mode showing 16M free on drive A: and 15M free on drive B:](img/gsg/DirAAndDirBDos1Mode.png)

***What has happened?*** When booting in MSX-DOS 1 mode only the MSX-DOS 1 compatible partitions are taken into account, both when assigning drives to the drivers and when mapping them. The active MSX-DOS 1 compatible partitions are partition 3 (16M) on the master device and partition 2 (15M) on the slave device, and they have been mapped in that order: drive A: to the 16M one, drive B: to the 15M one. The 85M partition on the slave device has the "active" flag set too, but being FAT16 it is completely ignored in MSX-DOS 1 mode. (By the way: if you now booted in normal mode, the IDE driver would get three drives, one per active partition.)

n. Go to BASIC, run FDISK and remove the "active" flag of partition 3 (16M) in the master device.

o. Reset the computer while keeping pressed the "1" key. Once in the `COMMAND.COM` prompt, issue a `DIR` command, then a `DIR B:` command. Notice that the mapping is reversed relative to the previous case: A: is mapped to the 15M partition on the slave device, and B: is mapped to the 16M partition on the master device.

***What has happened?*** You should have guessed it already: the only active MSX-DOS 1 compatible partition is now the 15M one on the slave device, so it has been given priority and mapped to drive A:; drive B: has then been mapped to the first suitable partition still unassigned, the 16M one on the master device.

p. Go to BASIC, run FDISK and remove the "active" flag from partitions 1 and 2 of the slave device, so that none of the partitions in either device has the flag set.

q. Reset your computer in normal mode; you should be in the DOS prompt, with the 50M partition of the master device mapped to drive A: and the 85M partition of the slave device mapped to drive B:.
 
## 7. Locking drives

In this section we'll try the drive lock feature. 

a. Create a small text file in the tools floppy disk. You can do that by executing `COPY CON C:TEST`, then writing something, then pressing CTRL+Z.

b. Execute the following command: `LOCK C: ON`

c. Execute the following: `TYPE C:TEST`. You will see the contents of the text file you just created.

d. Remove the floppy disk from the drive (if you are using blueMSX, select the `File - Disk Drive A - Eject: tools.dsk` menu option).

e. Execute `TYPE C:TEST` again. You will see the file contents again instead of getting a "Not Ready" error, even though the disk has been removed from the drive.

***What has happened?*** Whenever Nextor is about to access the contents of a drive (this mostly matters for drives holding removable media, such as floppy disks), it first asks the driver if the associated storage media has changed. If the answer is "Yes" or "Not sure", then it takes the appropriate actions: for drives mapped to devices on Nextor drivers, it assigns the first available valid primary partition on the device to the drive (valid meaning a FAT12 or FAT16 partition that isn't mapped to another drive already); for drives handled by MSX-DOS drivers, it simply clears sector buffers and creates again the disk parameters block for the drive. (The "Not sure" response gets actually a special treatment; see _[3.3. Managing media changes](Nextor%203.0%20User%20Manual.md#33-managing-media-changes)_ in the Nextor 3.0 User Manual for more details.)

When a drive is locked, Nextor will never ask the driver for device change status when accessing that drive, and will instead assume that the device will never change. This improves performance as it saves both CPU processing and device access.

In this case, we have locked the floppy disk drive, whose change status would otherwise be checked on each disk access (floppy disk drives usually report a change status of "Not sure" always). When displaying the file contents the second time, Nextor assumes that the disk has not changed, and since it still has the file contents sector cached in memory, it can display it even if the disk has been removed from the drive.

Nextor will clear the cached sector for a drive at the moment of locking it, therefore if you had reversed the order of steps b and c you would have effectively got a "Not Ready" error.

f. Insert the Nextor tools floppy disk in its drive again.
 
## 8. The reduced/zero allocation information mode

Now we will set the reduced allocation mode on the FAT16 partition available on each device and see what happens.

a. Reset your computer. Drive A: will be mapped to the 50M partition on the master device, and drive B: to the 85M partition on the slave device.

b. Issue a `VOL A:50M` command, then a `VOL B:85M` command, in order to help identify the partitions without having to rely on their free size:

 ![VOL commands setting the volume names 50M and 85M, followed by a DIR showing them](img/gsg/VolAndDir.png)

c. Execute the following commands:

```
RALLOC A: ON
RALLOC B: ON
```

d. Issue a `DIR` command, then a `DIR B:` command. Notice that the free space reported has not changed: it is still the real one for both partitions.

***What has happened?*** When a drive is in reduced allocation information mode, the classic Nextor function ALLOC, which returns the total and free space on a drive as a sector count, returns 32MB when the real amount is larger than this value. This is useful when using old software that asks the operating system how much free space is available and expects the result (in sectors) to fit in 16 bits. However, `COMMAND3.COM` displays the free space using the newer byte-based disk space function of the kernel, which always returns the real value regardless of this mode; that's why DIR still shows the real free space (the DIR command of the old `COMMAND2.COM`, which uses ALLOC, would show 32M instead).

e. Execute the following command:

```
SET ZALLOC = ON
```

f. Issue again the `DIR` and `DIR B:` commands. Notice that this time no free space figure is displayed at all, and the DIR commands finish immediately after displaying the last file name. A `FREE` command will likewise tell that there is no space information to report.

***What has happened?*** When an environment item named ZALLOC exists and has the value ON (case insensitive), the reduced allocation information mode becomes the zero allocation information mode: the free space is not calculated at all for the drives that are in this mode (the ALLOC function reports zero), and `COMMAND3.COM` detects the situation and omits the figure instead of displaying a misleading value. You can use this mode if you have a very large drive and/or are using a very slow device, to prevent the computer from hanging for a few seconds every time a DIR command is issued.

## 9. Using the boot keys and the boot menu

We have seen that if key "1" is kept pressed while the computer is booting, Nextor starts in MSX-DOS 1 mode. Now we'll see other useful keys that can be used to alter the way Nextor boots, as well as the boot menu; see _[2.10. Boot keys and the boot menu](Nextor%203.0%20User%20Manual.md#210-boot-keys-and-the-boot-menu)_ in the Nextor 3.0 User Manual for a full list of the available keys.

a.	Reset your computer while keeping the "3" key pressed. You will see that the computer boots in the BASIC prompt.

***What has happened?*** When the "3" key is kept pressed at boot time, Nextor ignores any `NEXTOR.SYS`, `MSXDOS.SYS` and `AUTOEXEC.BAS` files present in the boot drive, and boots in the MSX-BASIC prompt.

b.	Issue a `CALL SYSTEM` command.

c.	Execute the `DRIVERS` command. You will see that the IDE controller has been assigned two drives at boot time, and the floppy disk controller has been assigned another two:

![DRIVERS output showing two drives assigned to the IDE controller and two to the floppy disk controller](img/gsg/DriversTwoDrives.png)

d.	Reset your computer while keeping the "5" key pressed. Once in the DOS prompt, execute the `DRIVERS` command again. You will see that now the IDE controller has got only one drive, but the floppy disk drive still has two:

![DRIVERS output showing one drive assigned to the IDE controller and two to the floppy disk controller](img/gsg/DriversOneDrive.png)

***What has happened?*** When the "5" key is kept pressed at boot time, Nextor will assign one single drive to each Nextor driver, regardless of the number of attached devices and of how many partitions have the "active" flag set. Legacy MSX-DOS drivers are unaffected by this.
 
e.	Reset your computer while keeping the "SHIFT" key pressed. Once in the DOS prompt, execute the `DRIVERS` command again. You will see that no drives have been assigned to the floppy disk drive (note that the floppy disk drive itself is not accessible now, but the `DRIVERS` command, being internal to `COMMAND3.COM`, keeps working):

![DRIVERS output showing no drives assigned to the floppy disk controller](img/gsg/DriversOneController.png)

***What has happened?*** When the "SHIFT" key is kept pressed at boot time, all the storage controllers with an MSX-DOS kernel (including the floppy disk drive controller) will disable themselves, but Nextor will not. This is useful to maximize the amount of available memory, especially in MSX-DOS 1 mode, as shown in the next step. (There are boot keys to selectively disable the Nextor kernels as well, and the boot menu shown below can also be used for this; see _[2.10. Boot keys and the boot menu](Nextor%203.0%20User%20Manual.md#210-boot-keys-and-the-boot-menu)_ in the Nextor 3.0 User Manual for details)

f.	Reset your computer while keeping the "1" and "3" keys pressed simultaneously. Once in the BASIC prompt, issue a `PRINT FRE(0)` command. You will see that there are about 20K free for BASIC code.

g.	Reset your computer while keeping the "1", "3", "5" and "SHIFT" keys pressed simultaneously. Once in the BASIC prompt, issue a `PRINT FRE(0)` command again. You will see that the free memory is now about 25K. Also, if you execute a `CALL DRVINFO` command, you will see that indeed, there is only one drive assigned:

![CALL DRVINFO in BASIC showing that only one drive is assigned](img/gsg/DrvinfoInBasic.png)

h.	Reset your computer while keeping the "N" key pressed. Instead of booting, the computer will display the Nextor boot menu:

![The Nextor boot menu](img/gsg/BootMenu.png)

***What has happened?*** The boot menu, introduced in Nextor 3.0, allows you to configure how the system will boot without having to keep several keys pressed while the computer boots. The menu lists all the Nextor 3 kernels found in the system, and each one can be enabled or disabled for the current boot by pressing the key displayed next to it ("Q" in our case); it then lists a set of options equivalent to the boot keys we have seen in this section, which can be switched on and off by pressing the corresponding number key (an asterisk indicates the kernels and options that are currently switched on). Once you are done, press ENTER to boot applying your selection, ESC to cancel the menu and boot normally, or "N" to disable all the Nextor kernels and boot (this is useful when the kernel ROM must be updated from a storage device controlled by a non-Nextor controller). See _[2.10. Boot keys and the boot menu](Nextor%203.0%20User%20Manual.md#210-boot-keys-and-the-boot-menu)_ in the Nextor 3.0 User Manual for the details.

i.	In the boot menu, press "1" and then "5" (an asterisk will appear next to the "Boot in MSX-DOS 1 mode" and "Reduced drive allocation mode" entries), then press ENTER. You will boot in MSX-DOS 1 mode with one single drive assigned per driver, exactly as if you had kept the "1" and "5" keys pressed while the computer was booting - but this time keeping your hands free.

## 10. Installing drivers in RAM

So far all the Nextor drivers we have used were embedded in a Nextor kernel ROM. Nextor 3 introduces a new type of driver: the RAM drivers, which are loaded from a file and installed in a RAM segment of a memory mapper — no flashing involved. To try this feature we'll use the example RAM driver that is developed as part of Nextor: it implements a tiny (14K) RAM disk and, just for demonstration purposes, it also makes the CAPS LED of the computer blink and adds a CALL PRINT command to BASIC.

a. Get the example RAM driver file, _ram-driver-example.drv_ (available in [the latest release of Nextor in GitHub](https://github.com/Konamiman/Nextor/releases/latest), it can also be built from the Nextor source code), rename it to RAMDRIVR.DRV, and copy it to drive A:. If you are using blueMSX you can put the file in an empty folder of your computer, select the _File - Disk Drive A - Insert Directory_ menu option to use that folder as the floppy disk, execute `COPY C:RAMDRIVR.DRV A:`, and then insert the Nextor tools disk again.

b. Install the driver by executing the following command:

```
C:DRVROP i RAMDRIVR.DRV /m
```

You will see the initialization messages printed by the driver, the slot and segment number where it has been installed (take note of these, you'll need them later), and a message telling that a drive has been mapped (drive E: if you are following this guide step by step). You will also notice that the CAPS LED of your computer (or of the emulated machine) starts blinking:

![Initialization messages of the RAM driver, showing the slot and segment where it was installed and the drive that was mapped](img/gsg/InstallingRamDriver.png)

***What has happened?*** The `DRVROP.COM` tool has allocated a segment from the memory mapper, loaded the driver file into it, and registered it in Nextor; from this point the driver works exactly like a driver embedded in ROM (it can even hook into the timer interrupt, hence the blinking CAPS LED). The `/m` flag has additionally mapped the first free drive letter to the first available device controlled by the new driver: a small RAM disk that lives in the driver's own RAM segment. See _[3.4.13. DRVROP: the driver operations tool](Nextor%203.0%20User%20Manual.md#3413-drvrop-the-driver-operations-tool)_ in the Nextor 3.0 User Manual for the complete syntax of the tool.

c. Issue a `DIR E:` command: you will see an empty drive with just a few kilobytes of free space. Copy a small file to it and verify that it is indeed a working drive, for example:

```
COPY C:TEST E:
DIR E:
TYPE E:TEST
```

d. Execute the `DRIVERS` command. The new driver ("Simple RAM disk driver") will be listed together with the drivers in ROM, showing the slot and the segment number where it is installed (the number after the slot is the RAM segment number):

![DRIVERS output listing the Simple RAM disk driver together with the drivers in ROM, with its slot and segment number](img/gsg/RamDriverInstalled.png)

e. Go to BASIC and execute `CALL PRINT("HELLO!")`: the string will be printed. `CALL PRINT` is a new command that the driver has added to BASIC. Go back to the DOS prompt with CALL SYSTEM.

f. Uninstall the driver by executing the following command, replacing the slot and segment numbers with the values you took note of in step b (if you didn't take note, run the `DRIVERS` command as in step d); for example, if the driver was installed in slot 3-2 and segment 28:

```
C:DRVROP u 3-2 28
```

You will see the shutdown message printed by the driver, and the CAPS LED will stop blinking. Drive E: is now unmapped, and the `CALL PRINT` command is gone.

***What has happened?*** The driver has been asked to shut down (so that it could remove its hooks), all the drives mapped to it have been unmapped, its RAM segment has been freed, and finally Nextor has unregistered the driver.

**Notes:**

* Drivers can also be installed and uninstalled from BASIC, with the commands `CALL IDRIVER("RAMDRIVR.DRV",1)` and `CALL UDRIVER(<slot>,<segment>)` respectively; see _[3.6.12. The CALL IDRIVER command](Nextor%203.0%20User%20Manual.md#3612-the-call-idriver-command)_ and _[3.6.13. The CALL UDRIVER command](Nextor%203.0%20User%20Manual.md#3613-the-call-udriver-command)_ in the Nextor 3.0 User Manual.

* RAM drivers don't work in MSX-DOS 1 mode, and they don't survive a computer reset. If you want a RAM driver to be always available, you can install it from your AUTOEXEC.BAT file.

* [The MSX Turbo-R FDD driver](https://github.com/Konamiman/Turbo-R-FDD-Nextor-driver) is available as a RAM driver also, in addition to the regular ROM driver variant.
