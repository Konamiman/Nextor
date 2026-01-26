#ifndef __DOS_H
#define __DOS_H

#include "types.h"

#define MAX_INSTALLED_DRIVERS 8
#define MAX_LUNS_PER_DEVICE 7
#define DRIVER_NAME_LENGTH_80 51
#define DRIVER_NAME_LENGTH_40 36
#define MAX_INFO_LENGTH 64

#define FILEATTR_DIRECTORY (1<<4)

#define DEV_TYPE_BLOCK 0
#define DEV_FLAG_REMOVABLE 1
#define DEV_FLAG_READ_ONLY (1 << 1)
#define DEV_FLAG_FLOPPY (1 << 2)


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


#define DRIVER_IS_NEXTOR (1 << 7)
#define GDRVR_EXTENDED_DRIVER_NAME_FLAG 0x80
#define CDRVR_NEXTOR_3_FLAG 0x10

typedef struct {
    byte slot;
    byte segment;
    byte numDrivesAtBootTime;
    byte firstDriveLetterAtBootTime;
    byte flags;
    byte versionMain;
    byte versionSec;
    byte versionRev;
    char driverName[64-8];
} driverInfo;

typedef struct {
	byte mediumType;
	uint sectorSize;
	ulong sectorCount;
	byte flags;
	uint cylinders;
	byte heads;
	byte sectorsPerTrack;
	bool suitableForPartitioning;
} deviceParams;

typedef struct {
	char deviceName[MAX_INFO_LENGTH];
    byte deviceNumber;
    bool isValid;
    bool isOnline;
    bool canCreatePartitions;
	deviceParams params;
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
#define _CONOUT 0x02
#define _DIRIO 0x06
#define _BUFIN 0x0A
#define _SETDTA 0x1A
#define _ALLOC 0x1B
#define _RDABS 0x2F
#define _WRABS 0x30
#define _DPARM 0x31
#define _FFIRST 0x40
#define _FNEXT 0x41
#define _OPEN 0x43
#define _CREATE 0x44
#define _CLOSE 0x45
#define _READ 0x48
#define _WRITE 0x49
#define _SEEK 0x4A
#define _PARSE 0x5B
#define _WPATH 0x5E
#define _TERM 0x62
#define _EXPLAIN 0x66
#define _DOSVER 0x6F
#define _RDDRV 0x73
#define _WRDRV 0x74
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

//Nextor 2
#define DEV_RW 0x4160
#define DEV_INFO 0x4163
#define LUN_INFO 0x4169

// Nextor 3
#define READ_WRITE 0x4137
#define DRIVER_QUERY 0x412B
#define DEVICE_QUERY 0x412E
#define DRVQ_GET_VERSION 1
#define DRVQ_GET_INFO_STRING 2
#define DRVQ_GET_INIT_PARAMS 3
#define DRVQ_INIT_DRIVER 4
#define DRVQ_GET_BOOT_DRIVES_COUNT 5
#define DRVQ_GET_DRIVE_BOOT_CONFIG 6
#define DRVQ_GET_MAX_DEVICE_NUMBER 7
#define DEVQ_GET_STRING 1
#define STRING_DEVICE_NAME 2
#define DEVQ_GET_PARAMS 2
#define DEVQ_GET_STATUS 3
#define DEVQ_GET_AVAILABILITY 4
#define ERR_QUERY_OK 0
#define ERR_QUERY_TRUNCATED_STRING 1
#define ERR_QUERY_INVALID_DEVICE 2
#define ERR_QUERY_NOT_IMPLEMENTED 0xFF

#define DEFAULT_MAX_DEVICE_NUMBER 4

#define BK4_ADD 0xF1D0


#endif   //__DOS_H