#include "asmcall.h"

byte ASMRUT[4];
byte OUT_FLAGS;


/*
 * Call an arbitrary routine using Z80_registers structa to pass the input
 * values of the Z80 registers and receive the output values.
 *
 * Most callers should use the AsmCall macro instead (declared in asmcall.h),
 * and use AsmCallAlt only when AF' needs to be set at input.
 *
 * Input:
 *   address            Address of the routine to call.
 *   regs               Pointer to a Z80_registers struct holding the
 *                      input register values.
 *   inRegistersDetail  Which input registers to load before calling the routine:
 *                        REGS_NONE - no registers loaded.
 *                        REGS_AF   - AF only.
 *                        REGS_MAIN - AF, BC, DE, HL.
 *                        REGS_ALL  - AF, BC, DE, HL, IX, IY.
 *   outRegistersDetail Which output registers to read back. Same set
 *                      of values as inRegistersDetail.
 *   alternateAf        Value to load into AF' before the call.
 *
 * Output:
 *   regs               Updated with the output registers selected by
 *                      outRegistersDetail. The post-call F register
 *                      is also reflected in regs->Flags (Z, C, S,
 *                      etc.) when AF is part of the output set.
 */
void AsmCallAlt(uint address, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail, int alternateAf) __naked __sdcccall(0)
{
	__asm
	;Patch ASMRUT[0] with the JP opcode every call. Harmless if already set,
	;and avoids relying on initialized-globals support in the CRT0.
	ld	a,#0xC3
	ld	(_ASMRUT),a

	push    ix
    ld      ix,#4
    add     ix,sp
    ld  e,6(ix) ;Alternate AF
    ld  d,7(ix)
    ex  af,af
    push    de
    pop af
    ex  af,af
    ld      l,(ix)  ;HL=Routine address
    ld      h,1(ix)
    ld      e,2(ix) ;DE=regs address
    ld      d,3(ix)
	ld      a,5(ix)
	ld	    (_OUT_FLAGS),a
	ld	    a,4(ix)	;A=in registers detail

	ld	(_ASMRUT+1),hl

    push    de
	or	a
	jr	z,ASMRUT_DO

    push    de
    pop     ix      ;IX=&Z80regs

	exx
	ld	l,(ix)
	ld	h,1(ix)	;AF
	dec	a
	jr	z,ASMRUT_DOAF
	exx

	ld      c,2(ix) ;BC, DE, HL
    ld      b,3(ix)
    ld      e,4(ix)
    ld      d,5(ix)
    ld      l,6(ix)
    ld      h,7(ix)
	dec	a
	exx
	jr	z,ASMRUT_DOAF

    ld      c,8(ix)	 ;IX
    ld      b,9(ix)
    ld      e,10(ix) ;IY
    ld      d,11(ix)
	push	de
	push	bc
	pop	ix
	pop	iy

ASMRUT_DOAF:
	push	hl
	pop	af
	exx

ASMRUT_DO:
    call  _ASMRUT

;ASMRUT: call    0

    ex      (sp),ix ;IX to stack, now IX=&Z80regs
	ex	af,af	;Alternate AF

	ld	a,(_OUT_FLAGS)
	or	a
	jr	z,CALL_END

	exx		;Alternate HLDEBC
	ex	af,af	;Main AF
	push	af
	pop	hl
	ld	(ix),l
	ld	1(ix),h
	exx		;Main HLDEBC
	ex	af,af	;Alternate AF
	dec	a
	jr	z,CALL_END

    ld      2(ix),c ;BC, DE, HL
    ld      3(ix),b
    ld      4(ix),e
    ld      5(ix),d
    ld      6(ix),l
    ld      7(ix),h
	dec	a
	jr	z,CALL_END

	exx		;Alternate HLDEBC
    pop     hl
    ld      8(ix),l ;IX
    ld      9(ix),h
    push    iy
    pop     hl
    ld      10(ix),l ;IY
    ld      11(ix),h
	exx		;Main HLDEBC

	ex	af,af
	pop	ix
    ret

CALL_END:
	ex	af,af
	pop	hl
	pop	ix
    ret

;OUT_FLAGS:	.db	#0
	__endasm;
}

/*
 * Call a DOS function via the BDOS dispatcher at 0005h.
 * Same as AsmCall, but the function code is supplied explicitly.
 */
void DosCall(byte function, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail)
{
    regs->Bytes.C = function;
    AsmCall(0x0005,regs,inRegistersDetail < REGS_MAIN ? REGS_MAIN : inRegistersDetail, outRegistersDetail);
}