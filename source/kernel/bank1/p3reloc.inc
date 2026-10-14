;-----------------------------------------------------------------------------
;
;    Macros for code that is assembled with ".PHASE 0" and relocated at run
; time to an address that is only known then (e.g. a page-3 area allocated
; below HIMSAV). Used by the page-3 resident code of DOSINIT and by MSR.COM.
;
;    Every instruction whose operand (at byte 1 of the instruction) is an
; absolute reference to the relocated block itself must be written as
;
;	...	<instruction>
;
; which labels it as "_adjust_N". The ADJUST_TABLE macro then generates a
; table with the offset of the operand of each of these instructions within
; the block (terminated by a zero word), and at run time the start address
; of the relocated copy must be added to each of these words.
;
; Since the operand is assumed to be at byte 1, instructions with a prefix
; byte (e.g. "ld bc,(nn)" or "ld ix,nn") can't be relocated this way.
;
;-----------------------------------------------------------------------------
;
adjlbl		MACRO	label,inst
_adjust_&label:  inst
		ENDM
;
_adjust_label	defl	0
;
...		MACRO	inst
_adjust_label	 defl	_adjust_label+1
		 adjlbl	%_adjust_label,<inst>
		ENDM
;
;
adjtab		MACRO	label
		 dw	 _adjust_&label&+1
		ENDM
;
ADJUST_TABLE	MACRO
_adjtab		 defl	 0
		REPT	_adjust_label		;Addresses within the
_adjtab		 defl	 _adjtab+1		; relocated code which need
		 adjtab  %_adjtab		; to be adjusted (absolute
		ENDM				; references).
		dw	0
		ENDM
;
