# "Drive per partition" feature

## The problem

Right now, at boot time Nextor will assign one drive per device reported as existing for each driver (except those that are offline and are non-removable). Users have expressed that it would be convenient to have several partitions already mapped to drives at boot time.

## The solution

Idea: at boot time, and while querying drivers for available devices, also scan the partitions chain of each device, and for that given device assign one drive for each partition that is marked as active (bit 7 of first byte in the partition table is set). If the device is offline (and is removable), there's no valid partition table, or there are no active partitions, one single drive is assigned for that device as before.

The built-in FDISK tool allows to toggle the active status of existing partitions, so that's a convenient method for users to lay out the boot state.

## Nuances and technical details

- There's no need to effectively assign drives to partitions at boot time: that can happen at first access, as it's being the case now. However this can lead to surprising results, depending on the order in which the user accesses the drives after boot. Maybe the internal drive descriptor could somehow encode a hint about the partition index number it's expected to receive?
- This needs to work for both DOS 1 and DOS 2 modes, even though it's understood that FAT16 partitions won't be available in DOS 1 (but any partition type having the active bit set should be counted at boot time).

## Task for you

Validate the idea and the design. What works and what can be improved? Do you have any questions?
