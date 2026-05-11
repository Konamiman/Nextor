;-----------------------------------------------------------------------------
;
; Macros that replace the undocumented Z80 instructions which operate on
; the high or low halves of the IX and IY registers (ixh, ixl, iyh, iyl).
;
; When the symbol NO_UNDOC_CPU_INSTRUCTIONS is defined at assembly time
; (e.g. via N80's `--define-symbols NO_UNDOC_CPU_INSTRUCTIONS` flag), each
; macro expands to a sequence of documented instructions that has the same
; effect on registers AND flags as the original undocumented opcode.
; Otherwise the macro expands to the original undocumented opcode.
;
; This allows a single source tree to produce ROMs that run on the
; original Z80 (where the undocumented opcodes are smaller and faster) and
; on Z180-based MSX machines (e.g. Victor HC-95) that do not support them,
; just by toggling a build flag.
;
; Supported macros and operand combinations:
;
;   ld_  a,{ixh|ixl|iyh|iyl}
;   ld_  {b|c|d|e},{ixh|ixl|iyh|iyl}
;   ld_  ixh,{a|b|c|d|e|<n>}     (n = 8-bit immediate)
;   ld_  ixl,{a|b|c|d|e|<n>}
;   ld_  iyh,{a|b|c|d|e|<n>}
;   ld_  iyl,{a|b|c|d|e|<n>}
;   ld_  ix,de                   (compound: ld_ ixl,e + ld_ ixh,d)
;   ld_  iy,de                   (compound: ld_ iyl,e + ld_ iyh,d)
;
;   cp_  {ixh|ixl|iyh|iyl}
;   or_  {ixh|ixl|iyh|iyl}
;   and_ {ixh|ixl|iyh|iyl}
;   xor_ {ixh|ixl|iyh|iyl}
;   add_ a,{ixh|ixl|iyh|iyl}
;   adc_ a,{ixh|ixl|iyh|iyl}
;   sub_ {ixh|ixl|iyh|iyl}
;   sbc_ a,{ixh|ixl|iyh|iyl}
;
;   inc_ {ixh|ixl|iyh|iyl}
;   dec_ {ixh|ixl|iyh|iyl}
;
; Operand names are matched case-insensitively (via `ifidni`), so for
; example `LD_ IYH,A` is equivalent to `ld_ iyh,a`. Any combination
; not listed above is rejected at assembly time with `.error`.
;
; ---------------------------------------------------------------------------
; Replacement strategy
; ---------------------------------------------------------------------------
;
; A single-instruction undocumented opcode that touches an index half
; (e.g. `ld iyh,a`) is replaced by the documented sequence
;
;       push  iy
;       ex    (sp),hl   ; HL = original IY,  stack top = original HL
;       <op via h or l>
;       ex    (sp),hl   ; HL restored,       stack top = new IY value
;       pop   iy        ; IY = new value
;
; This is 7 bytes / ~71 T-states, vs 2 bytes / 8 T-states for the
; original undocumented opcode. Flags are preserved exactly because
; `push`, `pop` and `ex (sp),hl` do not touch them; only the inner
; `<op>` (which is the documented counterpart of the original) sets
; flags as the original would. The HL and stack top are temporarily
; clobbered, but both are restored before the macro returns.
;
; The compound `ld_ iy,de` / `ld_ ix,de` shortcuts expand to a single
; `push de` + `pop iy/ix` (3 bytes / 25 T-states), which is far smaller
; than two independent `ld_ iyl,e` / `ld_ iyh,d` expansions.
;
; IMPORTANT: Do not rewrite a replacement to "save bytes" by using AF
; as scratch (`push af / ld a,h / ... / pop af`). That would corrupt the
; flag register if the instruction being replaced is in the middle of a
; conditional flow — and several Nextor sites depend on flag preservation
; around these instructions (e.g. the `cp iyh` chain in bank0/init.mac).
;
;-----------------------------------------------------------------------------

	ifndef _SDK_UNDOC_MACROS
_SDK_UNDOC_MACROS equ 1


;-----------------------------------------------------------------------------
; Internal helper: emit a pass-1 .error with a descriptive message.

_undoc_err macro text
	if1
	 .error text
	endif
	endm


;=============================================================================
; ld_ dest,src
;=============================================================================
;
; Each combination is matched explicitly. Unknown combinations produce
; `.error`. The flag `_undoc_ld_ok` (defl) is set to 1 by any matching
; branch and checked at the end of the macro.

ld_ macro dest,src
	_undoc_ld_ok defl 0

;--- ld a, {ixh|ixl|iyh|iyl}

	ifidni <dest>,<a>

	 ifidni <src>,<ixh>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	a,h
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	a,ixh
	  endif
	 endif

	 ifidni <src>,<ixl>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	a,l
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	a,ixl
	  endif
	 endif

	 ifidni <src>,<iyh>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	a,h
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	a,iyh
	  endif
	 endif

	 ifidni <src>,<iyl>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	a,l
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	a,iyl
	  endif
	 endif

	endif

;--- ld b, {ixh|ixl|iyh|iyl}

	ifidni <dest>,<b>

	 ifidni <src>,<ixh>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	b,h
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	b,ixh
	  endif
	 endif

	 ifidni <src>,<ixl>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	b,l
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	b,ixl
	  endif
	 endif

	 ifidni <src>,<iyh>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	b,h
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	b,iyh
	  endif
	 endif

	 ifidni <src>,<iyl>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	b,l
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	b,iyl
	  endif
	 endif

	endif

;--- ld c, {ixh|ixl|iyh|iyl}

	ifidni <dest>,<c>

	 ifidni <src>,<ixh>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	c,h
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	c,ixh
	  endif
	 endif

	 ifidni <src>,<ixl>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	c,l
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	c,ixl
	  endif
	 endif

	 ifidni <src>,<iyh>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	c,h
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	c,iyh
	  endif
	 endif

	 ifidni <src>,<iyl>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	c,l
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	c,iyl
	  endif
	 endif

	endif

;--- ld d, {ixh|ixl|iyh|iyl}

	ifidni <dest>,<d>

	 ifidni <src>,<ixh>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	d,h
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	d,ixh
	  endif
	 endif

	 ifidni <src>,<ixl>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	d,l
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	d,ixl
	  endif
	 endif

	 ifidni <src>,<iyh>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	d,h
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	d,iyh
	  endif
	 endif

	 ifidni <src>,<iyl>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	d,l
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	d,iyl
	  endif
	 endif

	endif

;--- ld e, {ixh|ixl|iyh|iyl}

	ifidni <dest>,<e>

	 ifidni <src>,<ixh>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	e,h
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	e,ixh
	  endif
	 endif

	 ifidni <src>,<ixl>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	e,l
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	e,ixl
	  endif
	 endif

	 ifidni <src>,<iyh>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	e,h
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	e,iyh
	  endif
	 endif

	 ifidni <src>,<iyl>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	e,l
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	e,iyl
	  endif
	 endif

	endif

;--- ld {ixh|ixl|iyh|iyl}, src
;
; For these we accept any source the documented `ld h,X` / `ld l,X`
; would accept: registers a/b/c/d/e and 8-bit immediates. Each block
; enumerates the recognized register sources and falls back to "assume
; immediate" for anything else.

	ifidni <dest>,<ixh>

	 ifidni <src>,<a>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	h,a
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixh,a
	  endif
	 endif

	 ifidni <src>,<b>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	h,b
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixh,b
	  endif
	 endif

	 ifidni <src>,<c>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	h,c
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixh,c
	  endif
	 endif

	 ifidni <src>,<d>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	h,d
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixh,d
	  endif
	 endif

	 ifidni <src>,<e>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	h,e
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixh,e
	  endif
	 endif

	 if _undoc_ld_ok eq 0  ;; assume immediate
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	h,src
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixh,src
	  endif
	 endif

	endif

	ifidni <dest>,<ixl>

	 ifidni <src>,<a>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	l,a
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixl,a
	  endif
	 endif

	 ifidni <src>,<b>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	l,b
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixl,b
	  endif
	 endif

	 ifidni <src>,<c>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	l,c
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixl,c
	  endif
	 endif

	 ifidni <src>,<d>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	l,d
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixl,d
	  endif
	 endif

	 ifidni <src>,<e>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	l,e
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixl,e
	  endif
	 endif

	 if _undoc_ld_ok eq 0  ;; assume immediate
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   ld	l,src
	   ex	(sp),hl
	   pop	ix
	  else
	   ld	ixl,src
	  endif
	 endif

	endif

	ifidni <dest>,<iyh>

	 ifidni <src>,<a>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	h,a
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyh,a
	  endif
	 endif

	 ifidni <src>,<b>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	h,b
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyh,b
	  endif
	 endif

	 ifidni <src>,<c>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	h,c
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyh,c
	  endif
	 endif

	 ifidni <src>,<d>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	h,d
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyh,d
	  endif
	 endif

	 ifidni <src>,<e>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	h,e
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyh,e
	  endif
	 endif

	 if _undoc_ld_ok eq 0  ;; assume immediate
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	h,src
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyh,src
	  endif
	 endif

	endif

	ifidni <dest>,<iyl>

	 ifidni <src>,<a>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	l,a
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyl,a
	  endif
	 endif

	 ifidni <src>,<b>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	l,b
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyl,b
	  endif
	 endif

	 ifidni <src>,<c>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	l,c
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyl,c
	  endif
	 endif

	 ifidni <src>,<d>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	l,d
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyl,d
	  endif
	 endif

	 ifidni <src>,<e>
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	l,e
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyl,e
	  endif
	 endif

	 if _undoc_ld_ok eq 0  ;; assume immediate
	  _undoc_ld_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   ld	l,src
	   ex	(sp),hl
	   pop	iy
	  else
	   ld	iyl,src
	  endif
	 endif

	endif

;--- ld iy,de  /  ld ix,de  (compound shortcuts: 3 bytes vs 14)

	ifidni <dest>,<iy>

	 ifidni <src>,<de>
	  _undoc_ld_ok defl 1
	  push	de
	  pop	iy
	 endif

	endif

	ifidni <dest>,<ix>

	 ifidni <src>,<de>
	  _undoc_ld_ok defl 1
	  push	de
	  pop	ix
	 endif

	endif

;--- Final validation

	if _undoc_ld_ok eq 0
	 _undoc_err <ld_: invalid operand combination>
	endif
	endm


;=============================================================================
; cp_ src    -    cp {ixh|ixl|iyh|iyl}
;=============================================================================

cp_ macro src
	_undoc_op_ok defl 0

	ifidni <src>,<ixh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  cp	h
	  ex	(sp),hl
	  pop	ix
	 else
	  cp	ixh
	 endif
	endif

	ifidni <src>,<ixl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  cp	l
	  ex	(sp),hl
	  pop	ix
	 else
	  cp	ixl
	 endif
	endif

	ifidni <src>,<iyh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  cp	h
	  ex	(sp),hl
	  pop	iy
	 else
	  cp	iyh
	 endif
	endif

	ifidni <src>,<iyl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  cp	l
	  ex	(sp),hl
	  pop	iy
	 else
	  cp	iyl
	 endif
	endif

	if _undoc_op_ok eq 0
	 _undoc_err <cp_: operand must be ixh, ixl, iyh or iyl>
	endif
	endm


;=============================================================================
; or_ src     and_ src     xor_ src     sub_ src
;=============================================================================
;
; Single-operand ALU instructions on an index half register.
; The operation is performed against accumulator A; the result goes to A
; and the flags are updated as documented for the corresponding documented
; instruction.

or_ macro src
	_undoc_op_ok defl 0

	ifidni <src>,<ixh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  or	h
	  ex	(sp),hl
	  pop	ix
	 else
	  or	ixh
	 endif
	endif

	ifidni <src>,<ixl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  or	l
	  ex	(sp),hl
	  pop	ix
	 else
	  or	ixl
	 endif
	endif

	ifidni <src>,<iyh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  or	h
	  ex	(sp),hl
	  pop	iy
	 else
	  or	iyh
	 endif
	endif

	ifidni <src>,<iyl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  or	l
	  ex	(sp),hl
	  pop	iy
	 else
	  or	iyl
	 endif
	endif

	if _undoc_op_ok eq 0
	 _undoc_err <or_: operand must be ixh, ixl, iyh or iyl>
	endif
	endm

and_ macro src
	_undoc_op_ok defl 0

	ifidni <src>,<ixh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  and	h
	  ex	(sp),hl
	  pop	ix
	 else
	  and	ixh
	 endif
	endif

	ifidni <src>,<ixl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  and	l
	  ex	(sp),hl
	  pop	ix
	 else
	  and	ixl
	 endif
	endif

	ifidni <src>,<iyh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  and	h
	  ex	(sp),hl
	  pop	iy
	 else
	  and	iyh
	 endif
	endif

	ifidni <src>,<iyl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  and	l
	  ex	(sp),hl
	  pop	iy
	 else
	  and	iyl
	 endif
	endif

	if _undoc_op_ok eq 0
	 _undoc_err <and_: operand must be ixh, ixl, iyh or iyl>
	endif
	endm

xor_ macro src
	_undoc_op_ok defl 0

	ifidni <src>,<ixh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  xor	h
	  ex	(sp),hl
	  pop	ix
	 else
	  xor	ixh
	 endif
	endif

	ifidni <src>,<ixl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  xor	l
	  ex	(sp),hl
	  pop	ix
	 else
	  xor	ixl
	 endif
	endif

	ifidni <src>,<iyh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  xor	h
	  ex	(sp),hl
	  pop	iy
	 else
	  xor	iyh
	 endif
	endif

	ifidni <src>,<iyl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  xor	l
	  ex	(sp),hl
	  pop	iy
	 else
	  xor	iyl
	 endif
	endif

	if _undoc_op_ok eq 0
	 _undoc_err <xor_: operand must be ixh, ixl, iyh or iyl>
	endif
	endm

sub_ macro src
	_undoc_op_ok defl 0

	ifidni <src>,<ixh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  sub	h
	  ex	(sp),hl
	  pop	ix
	 else
	  sub	ixh
	 endif
	endif

	ifidni <src>,<ixl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  sub	l
	  ex	(sp),hl
	  pop	ix
	 else
	  sub	ixl
	 endif
	endif

	ifidni <src>,<iyh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  sub	h
	  ex	(sp),hl
	  pop	iy
	 else
	  sub	iyh
	 endif
	endif

	ifidni <src>,<iyl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  sub	l
	  ex	(sp),hl
	  pop	iy
	 else
	  sub	iyl
	 endif
	endif

	if _undoc_op_ok eq 0
	 _undoc_err <sub_: operand must be ixh, ixl, iyh or iyl>
	endif
	endm


;=============================================================================
; add_ a,src    adc_ a,src    sbc_ a,src
;=============================================================================

add_ macro dest,src
	_undoc_op_ok defl 0

	ifidni <dest>,<a>

	 ifidni <src>,<ixh>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   add	a,h
	   ex	(sp),hl
	   pop	ix
	  else
	   add	a,ixh
	  endif
	 endif

	 ifidni <src>,<ixl>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   add	a,l
	   ex	(sp),hl
	   pop	ix
	  else
	   add	a,ixl
	  endif
	 endif

	 ifidni <src>,<iyh>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   add	a,h
	   ex	(sp),hl
	   pop	iy
	  else
	   add	a,iyh
	  endif
	 endif

	 ifidni <src>,<iyl>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   add	a,l
	   ex	(sp),hl
	   pop	iy
	  else
	   add	a,iyl
	  endif
	 endif

	endif

	if _undoc_op_ok eq 0
	 _undoc_err <add_: only `add_ a,{ixh|ixl|iyh|iyl}` is supported>
	endif
	endm

adc_ macro dest,src
	_undoc_op_ok defl 0

	ifidni <dest>,<a>

	 ifidni <src>,<ixh>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   adc	a,h
	   ex	(sp),hl
	   pop	ix
	  else
	   adc	a,ixh
	  endif
	 endif

	 ifidni <src>,<ixl>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   adc	a,l
	   ex	(sp),hl
	   pop	ix
	  else
	   adc	a,ixl
	  endif
	 endif

	 ifidni <src>,<iyh>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   adc	a,h
	   ex	(sp),hl
	   pop	iy
	  else
	   adc	a,iyh
	  endif
	 endif

	 ifidni <src>,<iyl>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   adc	a,l
	   ex	(sp),hl
	   pop	iy
	  else
	   adc	a,iyl
	  endif
	 endif

	endif

	if _undoc_op_ok eq 0
	 _undoc_err <adc_: only `adc_ a,{ixh|ixl|iyh|iyl}` is supported>
	endif
	endm

sbc_ macro dest,src
	_undoc_op_ok defl 0

	ifidni <dest>,<a>

	 ifidni <src>,<ixh>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   sbc	a,h
	   ex	(sp),hl
	   pop	ix
	  else
	   sbc	a,ixh
	  endif
	 endif

	 ifidni <src>,<ixl>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	ix
	   ex	(sp),hl
	   sbc	a,l
	   ex	(sp),hl
	   pop	ix
	  else
	   sbc	a,ixl
	  endif
	 endif

	 ifidni <src>,<iyh>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   sbc	a,h
	   ex	(sp),hl
	   pop	iy
	  else
	   sbc	a,iyh
	  endif
	 endif

	 ifidni <src>,<iyl>
	  _undoc_op_ok defl 1
	  ifdef NO_UNDOC_CPU_INSTRUCTIONS
	   push	iy
	   ex	(sp),hl
	   sbc	a,l
	   ex	(sp),hl
	   pop	iy
	  else
	   sbc	a,iyl
	  endif
	 endif

	endif

	if _undoc_op_ok eq 0
	 _undoc_err <sbc_: only `sbc_ a,{ixh|ixl|iyh|iyl}` is supported>
	endif
	endm


;=============================================================================
; inc_ reg    dec_ reg
;=============================================================================
;
; The original `inc ixh` etc. set Z, S, P/V (overflow), H and N as a
; documented `inc h` would; only the C flag is left untouched. The
; replacement uses `inc h` (or `inc l`) inside the push/pop wrapper so
; the same flag effect is preserved.

inc_ macro reg
	_undoc_op_ok defl 0

	ifidni <reg>,<ixh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  inc	h
	  ex	(sp),hl
	  pop	ix
	 else
	  inc	ixh
	 endif
	endif

	ifidni <reg>,<ixl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  inc	l
	  ex	(sp),hl
	  pop	ix
	 else
	  inc	ixl
	 endif
	endif

	ifidni <reg>,<iyh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  inc	h
	  ex	(sp),hl
	  pop	iy
	 else
	  inc	iyh
	 endif
	endif

	ifidni <reg>,<iyl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  inc	l
	  ex	(sp),hl
	  pop	iy
	 else
	  inc	iyl
	 endif
	endif

	if _undoc_op_ok eq 0
	 _undoc_err <inc_: operand must be ixh, ixl, iyh or iyl>
	endif
	endm

dec_ macro reg
	_undoc_op_ok defl 0

	ifidni <reg>,<ixh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  dec	h
	  ex	(sp),hl
	  pop	ix
	 else
	  dec	ixh
	 endif
	endif

	ifidni <reg>,<ixl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	ix
	  ex	(sp),hl
	  dec	l
	  ex	(sp),hl
	  pop	ix
	 else
	  dec	ixl
	 endif
	endif

	ifidni <reg>,<iyh>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  dec	h
	  ex	(sp),hl
	  pop	iy
	 else
	  dec	iyh
	 endif
	endif

	ifidni <reg>,<iyl>
	 _undoc_op_ok defl 1
	 ifdef NO_UNDOC_CPU_INSTRUCTIONS
	  push	iy
	  ex	(sp),hl
	  dec	l
	  ex	(sp),hl
	  pop	iy
	 else
	  dec	iyl
	 endif
	endif

	if _undoc_op_ok eq 0
	 _undoc_err <dec_: operand must be ixh, ixl, iyh or iyl>
	endif
	endm


	endif    ;_SDK_UNDOC_MACROS
