/*
 * Nextor SDK - Common data structures and file attribute flags.
 *
 * DOS function call codes live in dos_functions.h, DOS error codes in
 * dos_errors.h, and driver-API constants/structs in drivers.h;
 * include those separately as needed.
 */

#ifndef __DATA_STRUCTURES_H
#define __DATA_STRUCTURES_H

#include "types.h"


/* ----------------------------------------------------------------- */
/*  fileInfoBlock.attributes bit masks                               */
/* ----------------------------------------------------------------- */

#define ATTR_READ_ONLY     0x01
#define ATTR_HIDDEN        0x02
#define ATTR_SYSTEM        0x04
#define ATTR_VOLUME_ID     0x08
#define ATTR_SUB_DIR       0x10
#define ATTR_ARCHIVE       0x20
#define ATTR_DEVICE        0x80


/*
 * File Information Block (FIB).
 *
 * Populated in the DTA by the search-style DOS calls:
 *   _FFIRST (40h) - Find first matching entry
 *   _FNEXT  (41h) - Find next matching entry
 *   _FNEW   (42h) - Find new entry
 */
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

/*
 * Cluster information returned by:
 *   _GETCLUS (7Eh) - Get information about a FAT cluster
 */
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


#endif /* __DATA_STRUCTURES_H */
