#ifndef __ASMCALL_H
#define __ASMCALL_H

#define AsmCall(dir, regs, in, out) AsmCallAlt(dir, regs, in, out, 0)

void DriverCall(byte slot, uint routineAddress);
void DosCall(byte function, register_usage outRegistersDetail);
void SwitchSystemBankThenCall(int routineAddress, register_usage outRegistersDetail);
void AsmCallAlt(uint address, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail, int alternateAf);

#endif //__ASMCALL_H