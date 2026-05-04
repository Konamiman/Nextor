;-----------------------------------------------------------------------------
;
; Definition of the address-layout macros var / var0 / var1 / var2 / var3 /
; spare, used to lay out a sequence of variables and constants whose values
; come from a global `defl` symbol named `ram_ad`.
;
; - `var name,size[,fixed]` defines `name` equal to the current ram_ad and
;   advances ram_ad by `size` bytes. If `fixed` is supplied it is asserted
;   against the actual ram_ad value (a mismatch produces an assembly-time
;   message via `pr_hex`).
; - `var0 name[,fixed]` is shorthand for `var name,0,fixed` (just defines
;   `name = ram_ad` without advancing it).
; - `var1` / `var2` / `var3` are `var name,1` / `var name,2` / `var name,3`.
; - `spare size` advances ram_ad without defining a name.
;
; The caller must provide a `defl` symbol named `ram_ad` set to the desired
; starting address before invoking any of these macros (e.g.
; `ram_ad defl 4100h`).
;
; The address mismatch message uses `pr_hex`, which the SDK does NOT define
; here (the kernel's macros.inc provides it). Code that doesn't include a
; definition of `pr_hex` should simply omit the optional `fixed` argument.
;
; The `ifndef _SDK_VAR_MACRO` guard makes this file idempotent: it can be
; included from multiple SDK files, and the kernel can suppress the SDK's
; definitions by setting `_SDK_VAR_MACRO equ 1` before any SDK file is
; included (the kernel doesn't need to do this currently because it uses
; the SDK definitions verbatim).
;
;-----------------------------------------------------------------------------

ifndef _SDK_VAR_MACRO
_SDK_VAR_MACRO equ 1

	include const.inc

spare	macro	size
ram_ad	 defl	 ram_ad+(size)
	endm

var	macro	name,size,fixed
	 if1
	  ifnb	 <fixed>
	   if	  ram_ad ne x'&fixed'
	    .radix  16
	    pr_hex  <name address is>,%ram_ad
	    .radix  10
	   endif
	  endif
	 endif
	 const	 name,ram_ad
ram_ad	 defl	 ram_ad+(size)
	endm

var0	macro	name,fixed
	 var	 name,0,fixed
	endm

var1	macro	name,fixed
	 var	 name,1,fixed
	endm

var2	macro	name,fixed
	 var	 name,2,fixed
	endm

var3	macro	name,fixed
	 var	 name,3,fixed
	endm

endif
