#ifndef __DOS_H
#define __DOS_H

#include "types.h"

#define MAX_INSTALLED_DRIVERS 8
#define MAX_DEVICES_PER_DRIVER 7
#define MAX_LUNS_PER_DEVICE 7
#define DRIVER_NAME_LENGTH 32
#define MAX_INFO_LENGTH 64

#define FILEATTR_DIRECTORY (1<<4)


/* MSX-DOS/Nextor data structures */

typedef struct {
    byte alwaysFF;
    char filename[13];
    byte attributes;
    byte timeOfModification[2];
    byte dateOfModification[2];
    unsigned int startCluster;
    unsigned long fileSize;
    byte logicalDrive;
    byte internal[38];
} fileInfoBlock;


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

#define DRIVE_STATUS_ASSIGNED_TO_DEVICE 1


typedef struct {
    byte driveStatus;
    byte driverSlotNumber;
    byte driverSegmentNumber;
    byte relativeDriveNumber;
    byte deviceIndex;
    byte logicalUnitNumber;
    ulong firstSectorNumber;
    byte reserved[64 - 10];
} driveLetterInfo;

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
	bool suitableForPartitioning;
} lunInfo;

typedef struct {
    uint fatSectorNumber;
    uint entryOffsetInFatSector;
    ulong dataSectorNumber;
    uint fatEntryValue;
    byte sectorsPerCluster;
    struct {
		unsigned isFat12:1;
		unsigned isFat16:1;
		unsigned isOddEntry:1;
		unsigned isLastClusterOfFile:1;
		unsigned isUnusedCluster:1;
		unsigned unused:3;
	} flags;
    byte reserved[4];
} clusterInfo;

/* MSX-DOS functions */

#define _TERM0 0
#define _DIRIO 0x06
#define _BUFIN 0x0A
#define _DPARM 0x31
#define _FFIRST 0x40
#define _FNEXT 0x41
#define _OPEN 0x43
#define _CREATE 0x44
#define _CLOSE 0x45
#define _READ 0x48
#define _WRITE 0x49
#define _PARSE 0x5B
#define _WPATH 0x5E
#define _TERM 0x62
#define _EXPLAIN 0x66
#define _DOSVER 0x6F
#define _GDRVR 0x78
#define _GPART 0x7A
#define _CDRVR 0x7B
#define _GDLI 0x79
#define _GETCLUS 0x7E

/* MSX-DOS error codes */

#define _IPART 0xB4
#define _NOFIL 0xD7


/* Disk driver routines */

#define CALLB0 0x403F
#define CALBNK 0x4042
#define DEV_RW 0x4160
#define DEV_INFO 0x4163
#define LUN_INFO 0x4169


#define BK4_ADD 0xF1D0


#endif   //__DOS_H