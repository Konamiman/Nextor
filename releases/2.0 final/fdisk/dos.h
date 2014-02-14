#ifndef __DOS_H
#define __DOS_H

#include "types.h"

#define MAX_INSTALLED_DRIVERS 8
#define MAX_DEVICES_PER_DRIVER 7
#define MAX_LUNS_PER_DEVICE 7
#define DRIVER_NAME_LENGTH 32
#define MAX_INFO_LENGTH 64


/* MSX-DOS data structures */

#define DRIVER_IS_DOS250 (1 << 7)
#define DRIVER_IS_DEVICE_BASED 1

typedef struct {
    byte slot;
    byte segment;
    byte numDrivesAtBootTime;
    byte firstDriveLetterAtBootTime;
    byte flags;
    byte versionMain;
    byte versionSec;
    byte versionRev;
    char driverName[DRIVER_NAME_LENGTH];
    byte reserved[64 - DRIVER_NAME_LENGTH - 8];
} driverInfo;


typedef struct {
	byte lunCount;
	char deviceName[MAX_INFO_LENGTH];
} deviceInfo;


#define BLOCK_DEVICE 0
#define READ_ONLY_LUN (1 << 1)
#define FLOPPY_DISK_LUN (1 << 2)

typedef struct {
	byte mediumType;
	uint sectorSize;
	ulong sectorCount;
	byte flags;
	uint cylinders;
	byte heads;
	byte sectorsPerTrack;
	bool suitableForPartitionning;
} lunInfo;


/* MSX-DOS functions */

#define _DIRIO 0x06
#define _BUFIN 0x0A
#define _EXPLAIN 0x66
#define _GDRVR 0x78
#define _GPART 0x7A
#define _CDRVR 0x7B


/* MSX-DOS error codes */

#define _IPART 0xB4


/* Disk driver routines */

#define CALLB0 0x403F
#define CALBNK 0x4042
#define DEV_RW 0x4160
#define DEV_INFO 0x4163
#define LUN_INFO 0x4169


#define BK4_ADD 0xF84C


#endif   //__DOS_H