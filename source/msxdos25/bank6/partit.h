#ifndef __PARTIT_H
#define __PARTIT_H

#include "types.h"

#define MAX_PARTITIONS_TO_HANDLE 256
#define MIN_DEVICE_SIZE_IN_K 10
#define MIN_DEVICE_SIZE_FOR_PARTITIONS_IN_K 1024
#define MIN_REMAINING_SIZE_FOR_NEW_PARTITIONS_IN_K 100
#define MIN_PARTITION_SIZE_IN_K 100
#define MAX_DEVICE_SIZE_FOR_DIRECT_FORMAT_IN_K 32768
#define MAX_FAT16_PARTITION_SIZE_IN_M 2048
#define MAX_FAT16_PARTITION_SIZE_IN_K ((ulong)MAX_FAT16_PARTITION_SIZE_IN_M * (ulong)1024)
#define MAX_FAT12_PARTITION_SIZE_IN_K 32768
#define PARTITION_ALIGNMENT_IN_SECTORS 64

#define PARTYPE_UNUSED 0
#define PARTYPE_FAT12 1
#define PARTYPE_FAT16_SMALL 4
#define PARTYPE_EXTENDED 5
#define PARTYPE_FAT16 6

#define PSTATE_EXISTS 0
#define PSTATE_ADDED 1
#define PSTATE_DELETED 2

#define MAX_FAT12_CLUSTER_COUNT 4086
#define MAX_FAT16_CLUSTER_COUNT 65526
#define FAT12_ROOT_DIR_ENTRIES 112
#define FAT16_ROOT_DIR_ENTRIES 240
#define DIR_ENTRIES_PER_SECTOR (512 / 32)
#define MAX_FAT12_SECTORS_PER_FAT 12
#define MAX_FAT16_SECTORS_PER_FAT 256
#define FAT_COPIES 2

typedef struct {
	byte primaryIndex;
	byte extendedIndex;
	byte partitionType;
	ulong sizeInK;
} partitionInfo;

typedef struct {
	ulong x;
	ulong totalSectors;
	ulong dataSectors;
	uint clusterCount;
	uint sectorsPerFat;
	byte sectorsPerCluster;
	byte sectorsPerRootDirectory;
	bool isFat16;
} dosFilesystemParameters;

#endif   //__PARTIT_H