;-----------------------------------------------------------------------------
;
; Definition of the `const` macro used by all the SDK constant include files
; (dos_calls.inc, dos_errors.inc, etc.) to declare each public constant.
;
; By default `const NAME, VALUE` is equivalent to `NAME equ VALUE`.
; When --define-symbols GENERATE_PUBLIC_SYMBOLS is supplied to the assembler,
; the constant is also exported as a public symbol.
;
; The `ifndef _SDK_CONST_MACRO` guard makes the macro definition idempotent,
; so that:
;
;   - This file can be included from multiple SDK files without redefining
;     the macro;
;   - Code that already provides its own `const` macro (for example, the
;     kernel's macros.inc) can suppress the SDK's definition by setting
;     `_SDK_CONST_MACRO equ 1` before any SDK file is included.
;
;-----------------------------------------------------------------------------

ifndef _SDK_CONST_MACRO
_SDK_CONST_MACRO equ 1
const macro name,value
name equ (value)
	ifdef GENERATE_PUBLIC_SYMBOLS
	 public name
	endif
	endm
endif
