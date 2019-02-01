#ifndef __ASM_H
#define __ASM_H

#include "types.h"

#ifndef NULL
#define NULL 0
#endif


/* ---  Register detail levels  --- */

// This value tells which registers to pass in/out
// to the routine invoked by AsmCall, DosCall, BiosCall
// and UnapiCall.

typedef enum {
	REGS_NONE = 0,	//No registers at all
	REGS_AF = 1,	//AF only
	REGS_MAIN = 2,	//AF, BC, DE, HL
	REGS_ALL = 3	//AF, BC, DE, HL, IX, IY
} register_usage;


/* ---  Structure representing the Z80 registers  ---
        Registers can be accesses as:
        Signed or unsigned words (ex: regs.Words.HL, regs.UWords.HL)
        Bytes (ex: regs.Bytes.A)
        Flags (ex: regs.Flags.Z)
 */

typedef union {
	struct {
	    byte F;
	    byte A;
	    byte C;
		byte B;
		byte E;
		byte D;
		byte L;
		byte H;
		byte IXl;
		byte IXh;
		byte IYl;
		byte IYh;
    } Bytes;
	struct {
	    int AF;
	    int BC;
	    int DE;
	    int HL;
	    int IX;
	    int IY;
    } Words;
	struct {
	    uint AF;
	    uint BC;
	    uint DE;
	    uint HL;
	    uint IX;
	    uint IY;
    } UWords;
	struct {
		unsigned C:1;
		unsigned N:1;
		unsigned PV:1;
		unsigned bit3:1;
		unsigned H:1;
		unsigned bit5:1;
		unsigned Z:1;
		unsigned S:1;
	} Flags;
} Z80_registers;

#endif
