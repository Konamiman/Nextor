/*
 * Driver / device API: limits, flags and struct layouts.
 *
 * See also the other driver_*.h files and rom_bank_header.h
 */

#ifndef __NEXTOR_DRIVERS_H
#define __NEXTOR_DRIVERS_H

#include "types.h"


/* ----------------------------------------------------------------------------
 * Driver and device limits
 * ---------------------------------------------------------------------------- */

#define MAX_DEVICE_NAME_LENGTH       64
#define DEFAULT_MAX_DEVICE_NUMBER     4


/* ----------------------------------------------------------------------------
 * Driver/device flags
 * ---------------------------------------------------------------------------- */

/* deviceParams.flags bits */
#define DEV_TYPE_BLOCK        0
#define DEV_FLAG_REMOVABLE    1
#define DEV_FLAG_READ_ONLY   (1 << 1)
#define DEV_FLAG_FLOPPY      (1 << 2)

/* LUN type codes (returned by the Nextor 2 legacy LUN_INFO driver entry at 4169h) */
#define BLOCK_DEVICE          0
#define READ_ONLY_LUN        (1 << 1)
#define FLOPPY_DISK_LUN      (1 << 2)

/* driverInfo.flags */
#define DRIVER_IS_NEXTOR                 (1 << 7)
#define GDRVR_EXTENDED_DRIVER_NAME_FLAG  0x80
#define CDRVR_NEXTOR_3_FLAG              0x10

/* driveLetterInfo.driveStatus */
#define DRIVE_STATUS_ASSIGNED_TO_DEVICE  1


/* ----------------------------------------------------------------------------
 * Driver / device info structs (returned by various DOS calls)
 * ---------------------------------------------------------------------------- */

/*
 * Driver information record returned by:
 *   _GDRVR (78h) - Get information about a disk driver
 */
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

/*
 * Device parameters block.
 *
 * Not returned directly by any DOS call; this matches the layout that the
 * driver-side DEVICE_QUERY_GET_PARAMS sub-function (issued through
 * _CDRVR (7Bh) Call DRiVeR routine, with A = DEVICE_QUERY_GET_PARAMS,
 * targeting DRIVER_DEVICE_QUERY_ENTRY) writes to the caller's buffer.
 */
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

/*
 * Drive-letter information record returned by:
 *   _GDLI (79h) - Get information about a drive letter
 */
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


#endif /* __NEXTOR_DRIVERS_H */
