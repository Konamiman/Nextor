#ifndef __FDISK_H
#define __FDISK_H

#include "types.h"
#include "drivers.h"               /* deviceParams, MAX_DEVICE_NAME_LENGTH */
#include "driver_routines.h"       /* DRIVER_DEVICE_QUERY_ENTRY */
#include "driver_device_queries.h" /* DEVICE_QUERY_GET_STRING / GET_PARAMS / GET_AVAILABILITY */

/* Screen / key constants used by fdisk. */
#define MAX_LINLEN_MSX1 40
#define MAX_LINLEN_MSX2 80

#define ESC          27
#define CURSOR_RIGHT 28
#define CURSOR_LEFT  29
#define KEY_1        49

/* Maximum number of installed drivers fdisk will enumerate and manage. */
#define MAX_MANAGED_DRIVERS 8

/*
 * Application-level wrapper for one device, used by fdisk to gather and
 * cache info about each device of the selected driver.
 *
 * Populated piecewise via _CDRVR (7Bh) Call DRiVeR routine, targeting
 * DRIVER_DEVICE_QUERY_ENTRY:
 *   A = DEVICE_QUERY_GET_STRING       - fills deviceName
 *   A = DEVICE_QUERY_GET_PARAMS       - fills params
 *   A = DEVICE_QUERY_GET_AVAILABILITY - fills isOnline
 */
typedef struct {
    char deviceName[MAX_DEVICE_NAME_LENGTH];
    byte deviceNumber;
    bool isValid;
    bool isOnline;
    bool canCreatePartitions;
    deviceParams params;
} deviceInfo;

/* Maximum driver-name length fdisk will display per row, in 80- and
   40-column screen modes (used to fit the driver name plus its
   per-row trimmings into one screen line). */
#define DRIVER_NAME_LENGTH_80 46
#define DRIVER_NAME_LENGTH_40 36

#define f_CalculateFatFileSystemParameters 1
#define f_CreateFatFileSystem 2
#define f_PreparePartitioningProcess 3
#define f_CreatePartition 4
#define f_ToggleStatusBit 5

/*
 * fdisk-internal partition descriptor.
 *
 * Tracked in memory by fdisk while the user is creating,
 * deleting, or inspecting partitions on a device.
 */
typedef struct {
	byte primaryIndex;
	byte extendedIndex;
	byte partitionType;
	byte status;
	ulong sizeInK;
	uint alignmentPaddingInSectors;
} partitionInfo;

/*
 * fdisk-internal computed FAT12/16 filesystem geometry.
 *
 * Produced by fdisk when it sizes a new partition's FAT,
 * root directory, and cluster layout.
 */
typedef struct {
	ulong totalSectors;
	ulong dataSectors;
	uint clusterCount;
	uint sectorsPerFat;
	byte sectorsPerCluster;
	byte sectorsPerRootDirectory;
	bool isFat16;
} dosFilesystemParameters;

#endif //__FDISK_H