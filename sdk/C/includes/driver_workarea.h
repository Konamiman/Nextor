/*
 * Page 3 work area locations relevant to drivers.
 */

#ifndef __DRIVER_WORKAREA_H
#define __DRIVER_WORKAREA_H

/* Nextor-specific work area. */

#define BK4_ADD       0xF1D0  /* Address of routine called via CALDRV / C4PBK */
#define TMP_IX        0xF1D2  /* IX storage around CALLB0_IX_IY calls */
#define TMP_IY        0xF1D4  /* IY storage around CALLB0_IX_IY calls */
#define NXT_VER       0xF318  /* Nextor version (e.g. 0x30 = 3.0) */
#define MAIN_BANK     0xF319  /* 0 in DOS 2 mode, 3 in DOS 1 mode */

/* Work area inherited from MSX-DOS. */

#define DOS_VER       0xF313  /* DOS version (high nybble.low nybble); 0 for MSX-DOS 1 */
#define DOSFLG        0xF346  /* Disk-BASIC / DOS state flag */
#define NUMDRV        0xF347  /* Number of drives currently in use ($NUMDRV in asm) */
#define MASTER_SLOT   0xF348  /* Slot of the master driver currently addressed */
#define DRVTBL        0xFB21  /* Disk driver slot table */

#endif   //__DRIVER_WORKAREA_H
