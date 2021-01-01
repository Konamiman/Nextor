#ifndef __ASMCALL_H
#define __ASMCALL_H

#include "types.h"

typedef enum {
	REGS_NONE = 0,	//No registers at all
	REGS_AF = 1,	//AF only
	REGS_MAIN = 2,	//AF, BC, DE, HL
	REGS_ALL = 3	//AF, BC, DE, HL, IX, IY
} register_usage;

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


#define AsmCall(dir, regs, in, out) AsmCallAlt(dir, regs, in, out, 0)

void DosCall(byte function, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail);
void AsmCallAlt(uint address, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail, int alternateAf);

#endif //__ASMCALL_H