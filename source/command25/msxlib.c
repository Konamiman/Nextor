/*****************************************************************************/
/*                                                                           */
/*                            MSX-DOS Library                                */
/*                                                                           */
/*              Modified by Martin Lea,  1st September 1986                  */
/*                                                                           */
/*                  Copyright (c) I S Systems Ltd. 1986                      */
/*                                                                           */
/*****************************************************************************/

program() 
{       
#asm
	.z80
$INIT:  LD      SP,__CSTK
	LD      HL,$END
	LD      ($LM),HL
	CALL    MAIN##
	LD      HL,0
EXIT::  LD      B,L
	LD      C,_TERM##
	CALL    5
	RST     0       
	.8080
#endasm

}




/* Allocate memory; return -1 if none available */

allocate (n) int n; 
{
#asm
	.Z80
	POP     DE
	POP     BC
	PUSH    BC
	PUSH    DE

	LD      HL,($LM)
	PUSH    HL                      ;save a pointer to the free memory

	LD      D,H                     ;copy free memory pointer into DE
	LD      E,L
	ADD     HL,BC                   ;add offset to create proposed $LM ptr
	PUSH    HL                      ; save two copies of it
	PUSH    HL

	CALL    c.ugt##                 ;is HL (new ptr) >= DE (old ptr)?
	POP     DE
	JR      NZ,al.1

	LD      HL,(6)                  ;mod: use top of memory (6)
	DEC     HL                      ; not the stack pointer

	CALL    c.uge##
al.1:   POP     DE
	POP     BC
	LD      HL,-1
	RET     NZ
	EX      DE,HL
	LD      ($LM),HL
	PUSH    BC
	POP     HL
	RET
;
$LM::   DW      0                       ;Must be set to end of memory
	.8080
#endasm
}



/* Deallocate memory; */

deallocate (n) int n; {
#asm
	.Z80
	POP     DE
	POP     HL
	PUSH    HL
	PUSH    DE
	LD      ($LM),HL
	RET
	.8080
#endasm
}

#asm
	DS      2048,0                  ;C runtime stack
__CSTK: 
#endasm

zz_dummy()
{
#asm                    /* End of library (code segment) */
	RET
$END::
#endasm
}

char DSEGEND;           /* End of library (data segment) */

#asm
	END     $INIT
#endasm
