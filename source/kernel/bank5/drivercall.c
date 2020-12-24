#include "drivercall.h"
#include "../../tools/C/asmcall.h"
#include "../../tools/C/dos.h"

//The following is required in the main program:
//byte ASMRUT[4];
//byte OUT_FLAGS;
//Z80_registers regs;

void DriverCall(byte slot, uint routineAddress)
{
	byte registerData[8];
	int i;

	memcpy(registerData, &regs, 8);

	regs.Bytes.A = slot;
	regs.Bytes.B = 0xFF;
	regs.UWords.DE = routineAddress;
	regs.Words.HL = (int)registerData;

	DosCallFromRom(_CDRVR, REGS_ALL);

	if(regs.Bytes.A == 0) {
		regs.Words.AF = regs.Words.IX;
	}
}

void DosCallFromRom(byte function, register_usage outRegistersDetail)
{
    regs.Bytes.C = function;
    SwitchSystemBankThenCall((int)0xF37D, outRegistersDetail);
}


void SwitchSystemBankThenCall(int routineAddress, register_usage outRegistersDetail)
{
	*((int*)BK4_ADD) = routineAddress;
	AsmCall(CALLB0, &regs, REGS_ALL, outRegistersDetail);
}
