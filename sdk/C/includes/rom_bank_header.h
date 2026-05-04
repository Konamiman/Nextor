/*
 * Nextor SDK - Page-1 ROM bank header entry points.
 *
 * The names mirror the asm SDK file sdk/asm/constants/rom_bank_header.inc.
 *
 * Fixed-address entry points and data items in the Nextor ROM bank header
 * (the 4000h-40FFh area, identical in every kernel bank) plus the bank
 * switching routine. ROM drivers may use these as they see fit.
 */

#ifndef __ROM_BANK_HEADER_H
#define __ROM_BANK_HEADER_H

#define GSLOT1                   0x402D  /* Get current slot for page 1 */
#define RDBANK                   0x403C  /* Read byte from another bank */
#define CALLB0                   0x403F  /* Call routine in main bank (0/3) */
#define CALBNK                   0x4042  /* Call routine in another bank */
#define GWORK                    0x4045  /* Get SLTWRK entry for a slot */
#define CALLB0_IX_IY             0x404B  /* CALLB0 with IX/IY from TMP_IX / TMP_IY */
#define K_SIZE                   0x40FE  /* Byte: number of kernel banks (= first driver bank) */
#define CUR_BANK                 0x40FF  /* Byte: current bank paged in at page 1 */
#define CHGBNK                   0x7FD0  /* Bank switching routine (selects bank in A) */

#endif   //__ROM_BANK_HEADER_H
