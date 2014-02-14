#ifndef __SYSTEM_H
#define __SYSTEM_H


#define MAX_LINLEN_MSX1 40
#define MAX_LINLEN_MSX2 80

#define ESC 27
#define CURSOR_RIGHT 28
#define CURSOR_LEFT 29


/* MSX BIOS routines */

#define SYNCHR 0x0008
#define INITXT 0x006C
#define INIT32 0x006F
#define CHPUT 0x00A2
#define CLS 0x00C3
#define POSIT 0x00C6
#define ERAFNK 0x00CC
#define DSPFNK 0x00CF
#define CALBAS 0x0159


/* MSX BASIC ROM routines */

#define FRMEVL 0x4C64


/* MSX work area variables */

#define LINL40 0xF3AE
#define LINL32 0xF3AF
#define LINLEN 0xF3B0
#define CRTCNT 0xF3B1
#define CSRY 0xF3DC
#define CSRX 0xF3DD
#define CNSDFG 0xF3DE
#define VALTYP 0xF663
#define DAC 0xF7F6
#define SCRMOD 0xFCAF



#endif   //__SYSTEM_H