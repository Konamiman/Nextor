/*
 * Standard MSX system work area addresses commonly accessed by Nextor drivers.
 *
 * Reference: MSX2 Technical Handbook, Appendix 4 ("Work area listing")
 * https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Appendix4.md
 */

#ifndef __MSX_WORKAREA_H
#define __MSX_WORKAREA_H

#define LINL40  0xF3AE  /* Screen width for SCREEN 0 (1 byte) */
#define LINL32  0xF3AF  /* Screen width for SCREEN 1 (1 byte) */
#define LINLEN  0xF3B0  /* Current screen width in characters (1 byte) */
#define CRTCNT  0xF3B1  /* Number of rows on current screen (1 byte) */
#define CSRY    0xF3DC  /* Current cursor Y position (1-based) */
#define CSRX    0xF3DD  /* Current cursor X position (1-based) */
#define CNSDFG  0xF3DE  /* Function-key display flag (0 = hidden) */
#define BUF     0xF55E  /* BASIC line input buffer; commonly reused as scratch space */
#define VALTYP  0xF663  /* Type of the most recent expression evaluated by FRMEVL.
                           Values: 2=integer, 3=string, 4=single-precision, 8=double-precision */
#define DAC     0xF7F6  /* Result accumulator used by the BASIC numeric routines (FRMEVL, FRESTR, ...) */
#define NLONLY  0xF87C  /* BASIC "newline only" flag (set during program load to suppress output) */
#define SCRMOD  0xFCAF  /* Current screen mode (0..8) */
#define FLBMEM  0xFCAE  /* BASIC "string mode" flag for the line buffer */
#define EXPTBL  0xFCC1  /* Expanded slot flags (one byte per main slot) */
#define SLTATR  0xFCC9  /* Attributes for each slot */
#define SLTWRK  0xFD09  /* Work area address for each slot (4 bytes per slot) */
#define PROCNM  0xFD89  /* Name of the BASIC procedure being parsed by a CALL command (16 bytes) */
#define H_CHPH  0xFDA4  /* CHPUT extension hook */

#endif   //__MSX_WORKAREA_H
