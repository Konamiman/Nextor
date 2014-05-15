#ifndef __ASM_H
#define __ASM_H

#ifndef uint
typedef unsigned int uint;
#endif

#ifndef byte
typedef unsigned char byte;
#endif

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


/* --- UNAPI code block  --- */

typedef struct {
	byte UnapiCallCode[11];	/* Code to call the UNAPI entry point */
	byte UnapiReadCode[13];	/* Code to read one byte from the implementation, address is passed in HL */
} unapi_code_block;


/* ---  Generic assembler interop functions  --- */

//* Invoke a generic assembler routine.
//  regs is used for both input and output registers of the routine.
//  Depending on the values of inRegistersDetail and outRegistersDetail,
//  not all the registers will be passed from regs to the routine
//  and/or copied back to regs when the routine returns.
extern void AsmCall(uint address, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail);

//* Execute a DOS function call,
//  it is equivalent to invoking address 5 with function number in register C.
extern void DosCall(byte function, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail);

//* Invoke a MSX BIOS routine,
//  this is done via CALSLT to the slot specified in EXPTBL.
extern void BiosCall(uint address, Z80_registers* regs, register_usage outRegistersDetail);


/* --- UNAPI related functions  --- */

// * Get the number of installed implementations of a given specification
extern int UnapiGetCount(char* implIdentifier);

// * Build code block for a given implementation,
//   code block can be later used in UnapiCall and UnapiRead.
//   implIdentifier may be NULL, then the identifier already at ARG 
//   (set for example by a previous call to UnapiGetCount) will be used.
extern void UnapiBuildCodeBlock(char* implIdentifier, int implIndex, unapi_code_block* codeBlock);

// * Extract information about implementation location from a code block.
//   If address>=0xC000, slot and segment must be ignored.
//   Otherwise, if segment=0xFF, implementation is in ROM.
//   Otherwise, implementation is in mapped RAM.
extern void UnapiParseCodeBlock(unapi_code_block* codeBlock, byte* slot, byte* segment, uint* address);

//* Get the location of the RAM helper jump table and mapper table.
//  If RAM helper is not installed, jumpTableAddress will be zero.
extern void UnapiGetRamHelper(uint* jumpTableAddress, uint*  mapperTableAddress);

//* Execute an UNAPI function.
//  Code block can be generated with UnapiBuildCodeBlock.
extern void UnapiCall(unapi_code_block* codeBlock, byte functionNumber, Z80_registers* registers, register_usage inRegistersDetail, register_usage outRegistersDetail);

//* Read one byte from an UNAPI implementation.
//  Code block can be generated with UnapiBuildCodeBlock.
extern byte UnapiRead(unapi_code_block* codeBlock, uint address);

#endif
