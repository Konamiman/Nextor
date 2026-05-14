	.xlist
;
FALSE   equ     0
TRUE    equ     NOT FALSE
??????  equ     0
;
;
;


	include ../../sdk/asm/macros/const.inc
	include ../../sdk/asm/macros/undoc.inc
;
;
;
warn    macro   text
	 if1
	  .printx % text
	 endif
	endm
;
;
;
error   macro   text
	 if1
	  .error text
	 endif
	endm
;
;
;
pr_dec  macro   msg1,value,msg2
	 warn    <msg1 value msg2>
	endm
;
;
pr_hex  macro   msg1,value,msg2
	 warn    <msg1 value!h msg2>
	endm
;
;
code    macro   instr,arg

_code_ defl 0

	ifidn <instr>,<RET>
_code_ defl 0C9h
    endif

	ifidn <instr>,<ret>
_code_ defl 0C9h
    endif

    ifidn <instr>,<CPI>
_code_ defl 0FEh
    endif

    ifidn <instr>,<cpi>
_code_ defl 0FEh
    endif

    ifidn <instr>,<sta>
_code_ defl 032h
    endif

    ifidn <instr>,<call>
_code_ defl 0CDh
    endif

    ifidn <instr>,<CALL>
_code_ defl 0CDh
    endif

    ifidn <instr>,<jz>
_code_ defl 0CAh
    endif

    ifidn <instr>,<ori>
_code_ defl 0F6h
    endif

    ifidn <instr>,<CNZ>
_code_ defl 0C4h
    endif

    ifidn <instr>,<JMP>
_code_ defl 0C3h
    endif

    ifidn <instr>,<push>
    ifidn <arg>,<psw>
_code_ defl 0F5h
    endif
    endif

    ifidn <instr>,<MVI>
    ifidn <arg>,<A>
_code_ defl 3Eh
    endif
    ifidn <arg>,<M>
_code_ defl 36h
    endif
    ifidn <arg>,<C>
_code_ defl 0Eh
    endif
    endif

    ifidn <instr>,<mvi>
    ifidn <arg>,<a>
_code_ defl 3Eh
    endif
    ifidn <arg>,<b>
_code_ defl 06h
    endif
    ifidn <arg>,<m>
_code_ defl 36h
    endif
    ifidn <arg>,<c>
_code_ defl 0Eh
    endif
    endif

    ifidn <instr>,<LXI>
    ifidn <arg>,<H>
_code_ defl 21h
    endif
    ifidn <arg>,<SP>
_code_ defl 31h
    endif
    ifidn <arg>,<HL>
_code_ defl 21h
    endif
    endif

    ifidn <instr>,<lxi>
    ifidn <arg>,<h>
_code_ defl 21h
    endif
    ifidn <arg>,<hl>
_code_ defl 21h
    endif
    endif

    if _code_ eq 0
    if1
    .printx Undefined CODE arguments!!!!!!!!!!!!!!!!!!!!!!!!!!!
	.printx instr,arg
    endif
    endif

db _code_

endm
;
;
;-----------------------------------------------------------------------------
;
OFFSET  equ     4100h
PHASED  defl    FALSE
;
rammod  macro
PHASED   defl    TRUE
	 .phase  $-OFFSET
	endm
;
;
start:
;
finish  macro   name
	 if      PHASED
	  .dephase
	 endif
	 if1
	  pr_dec  <Size of module "&name" is>,%($-start),<bytes>
	 endif
	endm
;
;

;--- The "proc" macro is used to define a label that is invoked
;    at RAM in page 0 (when PHASED) or at ROM bank 4 (when not PHASED).

proc    macro   name                    ;;Macro for declaring a public
	if	PHASED
name&:                                  ;; label to be called.
	 .dephase
?&name&::
	 .phase  $-OFFSET
	else
name&:		;;For local call
?&name&::	;;For external call
	endif
	endm
;
;
;
pcall   macro   cc,name                 ;;Macro for calling an external
					;; routine.
	 ifb    <name>
	  call   ?&cc&##-OFFSET         ;;Unconditional CALL.
	 else
	  call   cc,?&name&##-OFFSET    ;;Conditional CALL.
	 endif
	endm
;
;
;


djpnz macro address
    dec b
	jp nz,address
  endm

printchar macro
	rst 18h
  endm

;------------------------------------------------------------------------------
;
	.list
;
