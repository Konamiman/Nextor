#include "AsmCall.h"

//The following is required in the main program:
//byte ASMRUT[4];
//byte OUT_FLAGS;
//Z80_registers regs;
//
//Also the following initialization is required:
//ASMRUT[0] = 0xC3;


void DosCall(byte function, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail)
{
    regs->Bytes.C = function;
    AsmCall(0x0005,regs,inRegistersDetail < REGS_MAIN ? REGS_MAIN : inRegistersDetail, outRegistersDetail);
}


void AsmCallAlt(uint address, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail, int alternateAf) __naked
{
	__asm
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
