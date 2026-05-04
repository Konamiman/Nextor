/*
 * MSX BASIC interpreter ROM routines that are useful when implementing
 * new BASIC CALL commands. All addresses are in the BASIC interpreter ROM
 * (page 1, "MAIN" slot); they must be invoked via CALBAS (BIOS, 0159h)
 * so that the interpreter ROM is paged in correctly.
 *
 * Reference: MSX2 Technical Handbook, Chapter 2 section 4.4
 * ("Expansion of CMD command").
 * https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Chapter2.md#44-expansion-of-cmd-command
 */

#ifndef __MSX_BASIC_H
#define __MSX_BASIC_H

#define NEWSTT  0x4601  /* Execute one BASIC text statement */
#define CHRGTR  0x4666  /* Extract one character from BASIC text (skipping spaces) */
#define FRMEVL  0x4C64  /* Evaluate an expression in BASIC text */
#define GETBYT  0x521C  /* Evaluate an expression as a 1-byte integer (0..255) */
#define FRMQNT  0x542F  /* Evaluate an expression as a 2-byte integer */
#define PTRGET  0x5EA4  /* Look up a variable in the symbol table (allocating it if missing) */
#define FRESTR  0x67D0  /* Register a string value (used together with FRMEVL when evaluating strings) */

#endif   //__MSX_BASIC_H
