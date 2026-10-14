;-----------------------------------------------------------------------------
;
;    Page-3 resident part of the mapper support routines: RD_SEG, WR_SEG,
; CAL_SEG, CALLS, PUT_PH, GET_PH, PUT_Pn and GET_Pn, and the UNAPI RAM helper
; compatible routines CALL_MAP, RD_MAP, CALL_MAPI and WR_MAP.
;
;    This file is included by DOSINIT (as part of the page-3 RAM code that
; DOWNLOAD_CODE copies and relocates) and by MSR.COM (the standalone mapper
; support routines installer for MSX-DOS 1, source/tools/MSR.MAC).
;
;    It must be included inside a ".PHASE 0" block, after p3reloc.inc (the
; absolute references to the routines themselves are marked with "..." for
; run time relocation).
;
;    The variables P0_SEG, P1_SEG, P2_SEG, P3_SEG (current segment in each
; page) and MAP_TAB (pointer to the mapper variables table) are referenced
; via the "p3var" macro. These are absolute addresses in the kernel (the
; includer must declare them as EXTRN), but if RELOC_P3_VARS is defined they
; are assumed to be labels inside the relocated block and the instructions
; that reference them are marked for relocation too.
;
;    All the routines expect the RAM slot to be already switched in the
; involved page (they do not do any slot switching), except CALL_MAP, RD_MAP,
; CALL_MAPI and WR_MAP which take a slot number too.
;
;-----------------------------------------------------------------------------
;
p3var		MACRO	inst
		ifdef	RELOC_P3_VARS
		 ...	<inst>
		else
		 inst
		endif
		ENDM
;
P0_REG		equ	0FCh
P1_REG		equ	0FDh
P2_REG		equ	0FEh
P3_REG		equ	0FFh
;
RDPRIM	equ	0F380h		;Inter-slot read, write and call primitives
WRPRIM	equ	0F385h		; in the MSX data area.
CLPRIM	equ	0F38Ch
_JP_IX	equ	CLPRIM+12	;Where "JP (IX)" instruction is stored
_JP_HL	equ	_JP_IX+1	;Where "JP (HL)" instruction is stored
;
;
;-----------------------------------------------------------------------------
;
; UNAPI RAM helper compatible routines

        ;--- Routine to call code in a RAM segment
        ;    Input:
        ;    IYh = Slot number
        ;    IYl = Segment number
        ;    IX = Target routine address (must be a page 1 address)
        ;    AF, BC, DE, HL = Parameters for the target routine
        ;Output:
        ;    AF, BC, DE, HL, IX, IY = Parameters returned from the target
        ;    routine

_U_CALLRAM:
        ex      af,af
...	      < call	_GET_P1 >
        push    af
        ld_ a,iyl
...	      < call	_PUT_P1 >
        ex      af,af
        call    CALSLT
        ex      af,af
        pop     af
...	      < call	_PUT_P1 >
        ex      af,af
        ret


        ;--- Routines to read or writea byte from a RAM segment
        ;Input:
        ;    A = Slot number
        ;    B = Segment number
        ;    E = Data to write (only WRITERAM)
        ;    HL = Address to be read/written from
        ;       (higher two bits will be ignored)
        ;Output:
        ;    A = Data read from the specified address (only READRAM)
        ;    BC, DE, HL preserved

_U_READRAM:
        push ix
        ld ix,RDSLT
        jr _U_RW

_U_WRITERAM:
        push ix
        ld ix,WRSLT

_U_RW:
        push bc
        push de
        push hl
        ex      af,af
...	      < call	_GET_P1 >
        push    af
        ld      a,b
...	      < call	_PUT_P1 >
        res     7,h
        set     6,h
        ex      af,af
...	      < call	_U_JPIX >
        ex      af,af
        pop     af
...	      < call	_PUT_P1 >
        ex      af,af
        pop hl
        pop de
        pop bc
        pop ix
        ret

_U_JPIX:
        jp (ix)


        ;--- Routine to call code in a RAM segment
        ;    (code location specified in the stack)
        ;Input:
        ;    AF, BC, DE, HL = Parameters for the target routine
        ;Output:
        ;    AF, BC, DE, HL, IX, IY = Parameters returned from the target
        ;    routine
        ;Call as:
        ;    CALL CALLRAM2
        ;
        ;CALLRAM2:
        ;    CALL <routine address>
        ;    DB bMMAAAAAA
        ;    DB <segment number>
        ;
        ;    MM = Slot as the index of the entry in the mapper table.
        ;    AAAAAA = Routine address index:
        ;             0=4000h, 1=4003h, ..., 63=40BDh

_U_CALLRAM2:
        exx
        ex      af,af'

        pop     ix
        ld      e,(ix+1)        ;Segment number
        ld      d,(ix)  ;Slot and entry point number
        dec     ix
        dec     ix
        push    ix
        ld_ iyl,e

        ld      a,d
        and     00111111b
        ld      b,a
        add     a,a
        add     a,b     ;A = Address index * 3
        ld      l,a
        ld      h,40h
        push    hl
        pop     ix      ;IX = Address to call

        ld      a,d
        and     11000000b
        rlca
        rlca
        rlca            ;A = Mapper table index * 8
        rlca
        rlca
        ld      c,a
        ld      b,0
        p3var   <ld      hl,(MAP_TAB)>
        add     hl,bc
        ld      a,(hl)  ;A = Slot to call
        ld_ iyh,a

        ex      af,af'
        exx
        inc     sp
        inc     sp

        ;The NO_UNDOC variants of `ld_ iyl,e` and `ld_ iyh,a` above
        ;expand by 5 bytes each, pushing the target out of jr range.

        ifdef	NO_UNDOC_CPU_INSTRUCTIONS
...	      < jp      _U_CALLRAM >
        else
        jr      _U_CALLRAM
        endif
;
;-----------------------------------------------------------------------------
;
;    These are the inter-segment read and write routines.  They must be called
; with  the RAM slot in page-2.  This ensures that they can be reasonably fast
; because no slot switching is necessary.  Interrupts are  disabled  by  these
; routines and not enabled again.  All registers except AF are preserved.
;
_RD_SEG:	di
		push	hl
		push	bc
		ld	b,a
...	      < call	_GET_P2 >		;Get current page-2 segment
		ld	c,a			; number and save it in C.
		ld	a,b
...	      < call	_PUT_P2 >		;Put required segment into
		res	6,h			; page-2 and force address
		set	7,h			; to page-2.
		ld	b,(hl)			;Get the byte.
		ld	a,c
...	      < call	_PUT_P2 >		;Restore original page-2
		ld	a,b			;A := byte value read
		pop	bc
		pop	hl
		ret
;
;
_WR_SEG:	di
		push	hl
		push	bc
		ld	b,a
...	      < call	_GET_P2 >		;Get the current page-2
		ld	c,a			; segment & save it in C.
		ld	a,b
...	      < call	_PUT_P2 >		;Put the required segment
		res	6,h			; in page-2 and force the
		set	7,h			; address into page-2.
		ld	(hl),e			;Store the byte.
		ld	a,c
...	      < call	_PUT_P2 >		;Restore original segment
		pop	bc			; to page-2.
		pop	hl
		ret
;
;
;-----------------------------------------------------------------------------
;
;    These are the routines for doing inter-segment calls.   They  are  almost
; identical  to  the  inter-slot  call  routines  except that they use segment
; switching routines rather than slot switching routines.  They can be used to
; call  any  segment  in  any page (except page-3).  However the RAM slot must
; already be selected in the desired page since these routines do not  do  any
; slot switching at all in order to keep them fast.
;
;
_CALLS:		exx
		ex	(sp),hl
		ld	d,(hl)
		inc	hl			;Extract parameters from in-
		push	de			; line after the "CALL CALLS"
		pop	iy			; instruction, and adjust
		ld	e,(hl)			; the return address.
		inc	hl
		ld	d,(hl)
		inc	hl
		push	de
		pop	ix
		ex	(sp),hl
		exx
;
_CAL_SEG:	exx				;Preserve main register set.
		ex	af,af'
		push	ix
		pop	hl			;HL := address to call and get
...	      < call	_GET_PH >		; current segment for this page
		push	af			; and save it for return.
		push	hl
		push	iy
		pop	af			;Enable required segment for
...	      < call	_PUT_PH >		; this address.
		ex	af,af'
		exx
		call	_JP_IX			;Call the routine via IX
		exx
		ex	af,af'
		pop	hl			;Restore the original
		pop	af			; segment to the appropriate
...	      < call	_PUT_PH >		; page.
		ex	af,af'
		exx
		ret
;
;
;-----------------------------------------------------------------------------
;
_PUT_PH:	bit	7,h			;Jump to appropriate "PUT_Pn"
		jr	nz,_put_p2_or_p3	; routine, depending on the
		bit	6,h			; top two bits of H.
		jr	z,_put_p0
		jr	_put_p1
_put_p2_or_p3:	bit	6,h
		jr	z,_put_p2
		jr	_put_p3
;
_GET_PH:	bit	7,h			;Jump to appropriate "GET_Pn"
		jr	nz,_get_p2_or_p3	; routine, depending on the
		bit	6,h			; top two bits of H.
		jr	z,_get_p0
		jr	_get_p1
_get_p2_or_p3:	bit	6,h
		jr	z,_get_p2
		jr	_get_p3
;
;
_PUT_P0:	p3var	<ld	(P0_SEG),a>
		out	(P0_REG),a
		ret
_GET_P0:	p3var	<ld	a,(P0_SEG)>
		ret
;
;
_PUT_P1:	p3var	<ld	(P1_SEG),a>
		out	(P1_REG),a
		ret
_GET_P1:	p3var	<ld	a,(P1_SEG)>
		ret
;
;
_PUT_P2:	p3var	<ld	(P2_SEG),a>
		out	(P2_REG),a
		ret
_GET_P2:	p3var	<ld	a,(P2_SEG)>
		ret
;
;
_GET_P3:	p3var	<ld	a,(P3_SEG)>		;Cannot alter page-3.
_PUT_P3:	ret
;
