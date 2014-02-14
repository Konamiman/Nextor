FALSE   equ     0
TRUE    equ     NOT FALSE
;
;
;
warn    macro   text
	 if1
	  .printx      text 
	 endif
	endm
;
;
;
error   macro   text
	 if2
	  .printx     text 
	 endif
	endm
;
;
;
start:
;
;
pr_fin  macro   name,l
	 warn    <Size of module "&name" is l bytes>
	endm
;
;
finish  macro   name
	 if1
	  pr_fin  name,%($-start)
	 endif
	endm
;
;
;
code    macro   instr,arg
	.8080
	 db      (instr arg)
	.z80
	endm
;
;
;
exos    macro   code
	 rst     30h
	 db      code
	endm
;
;
;
