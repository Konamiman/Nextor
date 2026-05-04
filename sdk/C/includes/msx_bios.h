/*
 * MSX BIOS entry points commonly used by Nextor drivers.
 *
 * Reference: MSX2 Technical Handbook, Appendix 1 ("BIOS listing")
 * https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Appendix1.md
 */

#ifndef __MSX_BIOS_H
#define __MSX_BIOS_H

#define SYNCHR  0x0008  /* Compare next char in BASIC text vs immediate byte */
#define RDSLT   0x000C  /* Inter-slot read: A = slot, HL = addr -> A = byte */
#define WRSLT   0x0014  /* Inter-slot write: A = slot, HL = addr, E = byte */
#define CALSLT  0x001C  /* Inter-slot call: address in IYh:IX */
#define DCOMPR  0x0020  /* Compare HL with DE (sets Z if equal, C if HL < DE) */
#define ENASLT  0x0024  /* Enable a slot permanently */
#define INITXT  0x006C  /* Initialize SCREEN 0 (40x24 text mode) */
#define INIT32  0x006F  /* Initialize SCREEN 1 (32x24 text mode) */
#define CHGET   0x009F  /* Console input (returns char in A) */
#define CHPUT   0x00A2  /* Console output (A = char) */
#define CLS     0x00C3  /* Clear screen */
#define POSIT   0x00C6  /* Move cursor: H = column, L = row */
#define CKCNTC  0x00BD  /* Check Control-STOP key (Z=1 if pressed) */
#define ERAFNK  0x00CC  /* Erase the function key display */
#define DSPFNK  0x00CF  /* Display the function keys */
#define CALBAS  0x0159  /* Call a routine in the BASIC interpreter ROM (address in IX) */

#endif   //__MSX_BIOS_H
