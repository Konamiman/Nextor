/*
 * Persistent storage: the operations of the _PSOPS function call, and the
 * layout of the data that Nextor keeps at the very beginning of the storage.
 *
 * See "Persistent storage" in the Nextor Programmers Reference for the full
 * explanation, including the rules that a program modifying this data must
 * follow (keep the size field and every byte it doesn't know about, and
 * recalculate the checksum).
 */

#ifndef __PERSISTENT_STORAGE_H
#define __PERSISTENT_STORAGE_H

#include "types.h"

/* Operations for the _PSOPS function call (in register A) */

#define PSOPS_GET_INFO 0   /* Get information about the storage */
#define PSOPS_READ     1   /* Read sectors from the storage */
#define PSOPS_WRITE    2   /* Write sectors to the storage */

/* Layout of the information returned by the PSOPS_GET_INFO operation */

#define PSI_SLOT         0 /* Slot of the driver that provides the storage */
#define PSI_FLAGS        1 /* Flags, currently always zero */
#define PSI_DEVICE       2 /* Device holding the storage file, never zero */
#define PSI_SECTOR_SIZE  3 /* Sector size in bytes, 1 to 512 (2 bytes) */
#define PSI_SECTOR_COUNT 5 /* Number of sectors, at least 1 (2 bytes) */
#define PSI_LENGTH       7 /* Size of the information block */

/* In Nextor 3.0 the storage is always a file in a device, so PSI_FLAGS is
   zero, PSI_DEVICE is never zero, PSI_SECTOR_SIZE is 512 and PSI_SECTOR_COUNT
   is 1. Read these fields instead of assuming those values, and don't choke
   on a device number of zero: a later version may keep the storage
   elsewhere, and would use a flag and a device number of zero to say so. */

/* The same information as a structure (it maps exactly on the block
   above: the Z80 compiler doesn't insert any padding). */

typedef struct {
    byte driverSlot;
    byte flags;
    byte deviceIndex;
    uint sectorSize;
    uint sectorCount;
} persistentStorageInfo;

/* Layout of the data that Nextor keeps at the beginning of the storage.
   Everything past PSD_MIN_SIZE bytes is free for programs to use. */

#define PSD_SIGNATURE    0 /* "NEXTOR", zero terminated (7 bytes) */
#define PSD_SIZE         7 /* Size of the data, this field and the
                              signature included (2 bytes) */
#define PSD_VERSION      9 /* Version of the data format */
#define PSD_KEYS        10 /* Boot key inverters, FFFFh if not set
                              (2 bytes, see PSD_KEYSM_* below) */
#define PSD_EMU_DEVICE  12 /* Persistent disk emulation: device number,
                              0 or FFh if not set */
#define PSD_EMU_SECTOR  13 /* Persistent disk emulation: absolute sector
                              number (4 bytes) */
#define PSD_EMU_FLAGS   17 /* Persistent disk emulation: flags */
#define PSD_MIN_SIZE    19 /* Size of the data in format version 1 */
#define PSD_MAX_SIZE   512 /* The data must always fit in one sector */

#define PSD_FORMAT_VERSION 1 /* Current value of the PSD_VERSION field */

/* The keys that have an inverter bit, in the two bytes at PSD_KEYS:
   1 to 6 in the first one, SHIFT and CTRL in the second one. */

#define PSD_KEYSM_LOW  0x7E
#define PSD_KEYSM_HIGH 0x30

/* A valid, empty set of data: signature, size, version, no inverters,
   no emulation pointer, and the checksum that makes the sum zero.
   Use it to initialize a byte array of PSD_MIN_SIZE elements. */

#define PSD_FRESH_DATA { \
    'N','E','X','T','O','R',0, \
    PSD_MIN_SIZE, 0, \
    PSD_FORMAT_VERSION, \
    0xFF, 0xFF, \
    0, 0, 0, 0, 0, 0, \
    0x0E }

/* Routines in sdk/C/code/persistent_storage.c. None of them terminates
   the program nor prints anything: they return the error code of the
   _PSOPS function call. */

byte PsGetInfo(persistentStorageInfo* info);
byte PsRead(byte* buffer, uint firstSector, byte sectorCount);
byte PsWrite(byte* buffer, uint firstSector, byte sectorCount);
byte PsSectorsThatFit(persistentStorageInfo* info, uint bufferSize, uint* availableBytes);
bool PsDataIsValid(byte* buffer, uint availableBytes);

/* Put a fresh, empty set of data in the buffer. The buffer should be at
   least PSD_MIN_SIZE bytes long (PSD_MAX_SIZE to also hold what programs
   keep past Nextor's data); a shorter one gets only what fits in it, which
   won't be valid data. */
void PsInitData(byte* buffer, uint bufferSize);
void PsSetChecksum(byte* buffer);

/* Read the data Nextor keeps in the storage into 'buffer'. If there is
   none, or what is there isn't valid, the buffer gets a fresh empty set
   of data instead. 'sectorsUsed' is what PsSaveData must be given. */
byte PsLoadData(byte* buffer, uint bufferSize, persistentStorageInfo* info, byte* sectorsUsed);

/* Recalculate the checksum and write the data back. */
byte PsSaveData(byte* buffer, byte sectorsUsed);

#endif /* __PERSISTENT_STORAGE_H */
