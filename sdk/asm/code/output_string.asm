	title	Nextor SDK - OUTPUT_STRING

;-----------------------------------------------------------------------------
;
; Copy a zero-terminated string to a destination buffer with a length limit.
; The terminator zero is included in the maximum length; if the source string
; does not fit, the destination is truncated and zero-terminated.
;
; Input:  HL = Source string (zero-terminated)
;         DE = Destination buffer
;         B  = Maximum length including the terminator
; Output: A  = RESULT_OK or RESULT_TRUNCATED_STRING
;         DE = Pointer to the terminator zero in the destination
; Modifies: AF, B, DE, HL

	ifrel
	public OUTPUT_STRING
	endif

OUTPUT_STRING:
	ld	a,b
	or	a
	ret	z

OUTPUT_STRING_LOOP:
	ld	a,(hl)
	or	a
	ld	(de),a
	ret	z		;A=0=RESULT_OK

	inc	hl
	inc	de
	djnz	OUTPUT_STRING_LOOP

	dec	de
	xor	a
	ld	(de),a
	ld	a,RESULT_TRUNCATED_STRING
	ret
