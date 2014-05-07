;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.3.0 #8604 (May 11 2013) (MINGW32)
; This file was generated Wed May 07 09:08:17 2014
;--------------------------------------------------------
	.module psft
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _DosCall
	.globl _printf
	.globl _strCRLF
	.globl _strInvParam
	.globl _strUsage
	.globl _strTitle
	.globl _driveNumber
	.globl _isFat16
	.globl _isNextor
	.globl _regs
	.globl _Terminate
	.globl _print
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_regs::
	.ds 12
_isNextor::
	.ds 1
_isFat16::
	.ds 1
_driveNumber::
	.ds 1
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
_strTitle::
	.ds 2
_strUsage::
	.ds 2
_strInvParam::
	.ds 2
_strCRLF::
	.ds 2
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;psft.c:142: int main(char** argv, int argc)
;	---------------------------------
; Function main
; ---------------------------------
_main_start::
_main:
;psft.c:146: print(strTitle);
	ld	hl,(_strTitle)
	push	hl
	call	_print
	pop	af
;psft.c:148: if(argc == 0) {
	ld	hl, #4+1
	add	hl, sp
	ld	a, (hl)
	dec	hl
	or	a,(hl)
	jr	NZ,00102$
;psft.c:149: print(strUsage);
	ld	hl,(_strUsage)
	push	hl
	call	_print
;psft.c:150: Terminate(null);
	ld	hl, #0x0000
	ex	(sp),hl
	call	_Terminate
	pop	af
00102$:
;psft.c:153: Terminate(null);
	ld	hl,#0x0000
	push	hl
	call	_Terminate
	pop	af
;psft.c:154: return 0;
	ld	hl,#0x0000
	ret
_main_end::
;psft.c:160: void Terminate(const char* errorMessage)
;	---------------------------------
; Function Terminate
; ---------------------------------
_Terminate_start::
_Terminate:
;psft.c:162: if(errorMessage != NULL) {
	ld	hl, #2+1
	add	hl, sp
	ld	a, (hl)
	dec	hl
	or	a,(hl)
	jr	Z,00102$
;psft.c:163: printf("\r\x1BK*** %s\r\n", errorMessage);
	ld	hl,#__str_0
	pop	de
	pop	bc
	push	bc
	push	de
	push	bc
	push	hl
	call	_printf
	pop	af
	pop	af
00102$:
;psft.c:166: regs.Bytes.B = (errorMessage == NULL ? 0 : 1);
	ld	bc,#_regs + 3
	ld	hl, #2+1
	add	hl, sp
	ld	a, (hl)
	dec	hl
	or	a, (hl)
	sub	a,#0x01
	ld	a,#0x00
	rla
	or	a, a
	jr	Z,00105$
	ld	a,#0x00
	jr	00106$
00105$:
	ld	a,#0x01
00106$:
	ld	(bc),a
;psft.c:167: DosCall(_TERM, &regs, REGS_MAIN, REGS_NONE);
	ld	de,#_regs
	ld	hl,#0x0002
	push	hl
	push	de
	ld	a,#0x62
	push	af
	inc	sp
	call	_DosCall
	pop	af
	pop	af
	inc	sp
	ret
_Terminate_end::
__str_0:
	.db 0x0D
	.db 0x1B
	.ascii "K*** %s"
	.db 0x0D
	.db 0x0A
	.db 0x00
;psft.c:171: void print(char* s) __naked
;	---------------------------------
; Function print
; ---------------------------------
_print_start::
_print:
;psft.c:193: __endasm;    
	push ix
	ld ix,#4
	add ix,sp
	ld l,(ix)
	ld h,1(ix)
	loop:
	ld a,(hl)
	or a
	jr z,end
	ld e,a
	ld c,#2
	push hl
	call #5
	pop hl
	inc hl
	jr loop
	end:
	pop ix
	ret
_print_end::
	.area _CODE
__str_1:
	.ascii "Partition Size Fix Tool v1.0"
	.db 0x0D
	.db 0x0A
	.ascii "By Konamiman, 5/2014"
	.db 0x0D
	.db 0x0A
	.db 0x0D
	.db 0x0A
	.db 0x00
__str_2:
	.ascii "Usage: psft <drive>: [fix] "
	.db 0x0D
	.db 0x0A
	.db 0x0D
	.db 0x0A
	.ascii "This tool checks the cluster "
	.ascii "count calculated by DOS for a given volume"
	.db 0x0D
	.db 0x0A
	.ascii "and offers the p"
	.ascii "ossibility of fixing it if it is over the standard limits"
	.db 0x0D
	.db 0x0A
	.ascii "("
	.ascii "4084 clusters for FAT12, 65524 clusters for FAT16)"
	.db 0x0D
	.db 0x0A
	.ascii "by sligh"
	.ascii "tly reducing the volume size in the boot sector."
	.db 0x0D
	.db 0x0A
	.db 0x0D
	.db 0x0A
	.ascii "Run the "
	.ascii "tool as psft <drive>: first, and if it says that a fix is ne"
	.ascii "eded,"
	.db 0x0D
	.db 0x0A
	.ascii "run again adding the "
	.db 0x22
	.ascii "fix"
	.db 0x22
	.ascii " parameter to actually perf"
	.ascii "orm the fix."
	.db 0x00
__str_3:
	.ascii "Invalid parameter"
	.db 0x00
__str_4:
	.db 0x0D
	.db 0x0A
	.db 0x00
	.area _INITIALIZER
__xinit__strTitle:
	.dw __str_1
__xinit__strUsage:
	.dw __str_2
__xinit__strInvParam:
	.dw __str_3
__xinit__strCRLF:
	.dw __str_4
	.area _CABS (ABS)
