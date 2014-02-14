	.XLIST
;
;
;        This file is included by all the major COMMAND.COM source files,  and
; defines some constants and macros that are useful to the files.
;
;
	.Z80
;
;
FALSE   EQU     0
TRUE    EQU     NOT FALSE
;
;
;
;------- ASCII Characters
;
TAB     EQU     9
LF      EQU     0AH
FF      EQU     0CH
CR      EQU     0DH
EOF     EQU     1AH
ESC     EQU     1BH
RUB	EQU	7FH
;
;
;
;------- Parse Path flags
;
$ODRV   EQU     1               ; Bit 0=0 => only drive name parsed.
$PATH   EQU     2               ; Bit 1=1 => directory path specified.
$DRV    EQU     4               ; Bit 2=1 => drive name specified.
$FN     EQU     8               ; Bit 3=1 => filename specified in last item.
$EXT    EQU     10H             ; Bit 4=1 => extension specified in last item.
$AMBIG  EQU     20H             ; Bit 5=1 => last item is ambiguous.
$DOTS   EQU     40H             ; Bit 6=1 => '.' or '..' given as last item.
;
;
;------- Offsets to Interesting Bits of File Info Blocks
;
FIB_FN  EQU     1               ; Start of ASCIZ filename string.
FIB_AT  EQU     FIB_FN+13       ; Attributes byte.
FIB_TM  EQU     FIB_AT+1        ; Time of last modification.
FIB_DT  EQU     FIB_TM+2        ; Date of last modification.
FIB_CL  EQU     FIB_DT+2
FIB_FS  EQU     FIB_CL+2        ; File size.
FIB_DR  EQU     FIB_FS+4        ; Drive, 1 => drive A: etc.
;
;
;------- Directory Entry Attribute Bits
;
ATRO    EQU     1               ; Set => file is read only.
ATHID   EQU     2               ; Set => only accessed if /H option given.
ATSYS   EQU     4               ; Set => system file, difficult to get at.
ATVOL   EQU     8               ; Set => volume name.
ATDIR   EQU     10H             ; Set => sub-directory.
ATARC   EQU     20H             ; Set => file updated.
ATDEV   EQU     80H             ; Set => character device (eg. CON, PRN).
;
;
;------- Bits in ATTRS and SWITCHES (mostly / options)
;
OPTP    EQU     1               ; /P option in 'DIR', 'TYPE' etc.
OPTH    EQU     ATHID           ; /H option, applies to many commands.
OPTT    EQU     4               ; /T in COPY.
OPTW    EQU     4               ; /W option in 'DIR'.
OPTV    EQU     8               ; /V in COPY and CONCAT.
				; 10H used for 'find only directories' flag.
OPTA    EQU     20H             ; /A in COPY.
OPTB    EQU     40H             ; /B in CONCAT.
				; 80H used for 'find character devices' flag.
;
;
;------- Flag bits in FLAGS
;
DIR     EQU     2               ; Bit DIR=NZ => 'DIR' command (used by FIRST).
				; Bit ATDIR=NZ => 'directory only' command.
;
;
;------- Very MSX-y things
;
INITXT  EQU     6CH             ; Main MSX ROM routine for hi-res text.
INIT32  EQU     6FH             ; Main MSX ROM routine for lo-res text.
LINL40  EQU     0F3AEH          ; No. columns for hi-res text.
LINL32  EQU     0F3AFH          ; No. columns for lo-res text.
;
;
;------- Miscellaneous Constants
;
CPYMEM  EQU     2*1024          ; COPY requires at least 2K free.
USERR   EQU     32              ; Errors 0..31 do not generate a message.
;
;
;
;------- Macro Definitions
;
CONST   MACRO   NAME, VALUE
NAME     EQU    VALUE
GLOBAL   NAME
	ENDM
;
;
CODE    MACRO   OPCODE, ARG
	 .8080
	 DB     (OPCODE ARG)
	 .Z80
	ENDM
;
;
.BSR    MACRO   OP, MASK, REG   ;; Support macro for .BIT, .SET and .RES below.
	 LOCAL  BIT
BIT      DEFL   0
	 IRP    M,<1,2,4,8,16,32,64,128>
	  IF    M EQ MASK
	   EXITM
	  ELSE
BIT        DEFL BIT+1
	  ENDIF
	 ENDM
	 OP     BIT, REG        ;; Actual instruction, using bit number.
	ENDM
;
;
.BIT    MACRO   MASK, REG       ;; BIT instruction using a bit mask.
	 .BSR   BIT, MASK, REG
	ENDM
;
;
.SET    MACRO   MASK, REG       ;; SET instruction using a bit mask.
	 .BSR   SET, MASK, REG
	ENDM
;
;
.RES    MACRO   MASK, REG       ;; RES instruction using a bit mask.
	 .BSR   RES, MASK, REG
	ENDM
;
;
;
	.LIST
