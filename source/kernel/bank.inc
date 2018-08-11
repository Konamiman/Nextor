B.MAIN	equ	0
B.INIT	equ	1
B.KERNEL equ	2
B.KANJI	equ	3

ifdef	BANK
;
	public	B.MAIN, B.INIT, B.KERNEL, B.KANJI
;
; This is the bank header for the banked version of MSX-DOS2 ROM,
; and must be always linked at 40FFh.
;
	public	BNK_ID, $LOCAL
BNK_ID:
	defb	BANK		;Bank number stored in ROM
;
if BANK ne B.MAIN
;
; Define DOSHEAD entry.
;
	public  GETSLOT
GETSLOT	equ	402Dh	;GETSLOT routine referenced by _XFER
;
	public  CHGBNK
CHGBNK  equ     7FD0h
;
; Generate inter-bank call anchors.
;
BPROC	macro	BNO,NAME
if BANK eq B.&BNO
	jp	NAME##
endif
endm

else
;
; Define inter-bank call entry points.
;
QQQQ	defl	$
;
BPROC	macro	BNO,NAME
ifnb <NAME>
	public	NAME
NAME	equ	QQQQ
endif
QQQQ	defl	QQQQ+3
endm
;
;
; Resolve message id external references
;
msg_num defl	0
;
msg	macro	name,text
msg_num defl	msg_num+1
	public	__&name
__&name equ	msg_num
	endm
;
;	----------------------------------------
;
	;msg	NORAM,	<Not enough memory>
	msg	TOOMD,	<Driver table is corrupted, too many drives allocated>
	msg	NOEMP,	<Driver table is corrupted, unexpected non-empty entry found>
	msg	BADDI,	<Boot error: disk controller ID does not match size of drive table>
	msg	INITE,	<Boot error, kernel initialization code failed>
	msg	BHIM,   <Boot error, not enough memory for BASIC variables>
;
	msg	FMT1,	<Drive name? (>
	msg	FMT2,	<) >
	msg	STRIKE,	<Strike a key when ready >
	msg	ABORT,	<Aborted>
	msg	FORMAT,	<Format complete>
;
	msg	PRM1,	<Insert disk for drive >
	msg	PRM2,	<:>
	msg	PRM3,	<and strike a key when ready>
;
	msg	ASTS,	<*** >
;
	msg	SYS_ER,	<System error >
	msg	USR_ER,	<User error >
;
endif

else

;
; Generate inter-bank call entries.
;
BPROC	macro	BNO,NAME
	call	CALL_&BNO
endm

endif

$LOCAL:
;
;	Entries for DOSINIT and Message module.
;
INITVECT:
	BPROC	INIT,	MAP_SCAN
	BPROC	INIT,	DOSINIT
	BPROC	INIT,	ROM_MSG
	BPROC	INIT,	ERR_MSG
	BPROC	INIT,	MAPBIO
;
;	Entries for KERNEL code.
;
KERNELVECT:
	;
	;
;
;
;
ifndef BANK

CALL_INIT:
	pop	ix
	push	bc
	ld	bc,$LOCAL-INITVECT-3
	ex	af,af'
	ld	a,B.INIT
	add	ix,bc
	pop	bc
	jp	CALBNK##
;
endif
;
;
