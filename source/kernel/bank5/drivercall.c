#include "drivercall.h"
#include "asmcall.h"
#include "dos_functions.h"
#include "drivers.h"           /* CDRVR_NEXTOR_3_FLAG */
#include "driver_workarea.h"   /* BK4_ADD */
#include "rom_bank_header.h"   /* CALLB0 */

//The main program is required to provide a global Z80_registers regs;
//(the routines below use it as the shared register-marshalling buffer).

void DriverCall(byte slot, byte segment, uint routineAddress)
{
	byte registerData[8];
	int i;

	memcpy(registerData, &regs, 8);

	regs.Bytes.A = slot | CDRVR_NEXTOR_3_FLAG;
	regs.Bytes.B = segment;
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
