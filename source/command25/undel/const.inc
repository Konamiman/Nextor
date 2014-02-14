;
MSX_DOS		EQU	5


;		  ASCII CONTROL CHARACTERS
;		============================
;
BELL	equ	07h
TAB	equ	09h
LF	equ	0Ah
CR	equ	0Dh
EOF	equ	1Ah
;
;
;
;		  BITS IN FAT ENTRY FLAGS
;		===========================
;
FL_UNDEL	equ	4	;Undeletable cluster
FL_CHAIN	equ	5	;Referred to by undeletable cluster,
FL_REF		equ	6	;Referred to by allocated cluster
;
;
;
;		  BITS IN PARSE FLAGS
;		=======================
;
PF_ANY	equ	0	;Set if any characters parsed after drive name
PF_DIR	equ	1	;Set if initial double quote character
PF_DRV	equ	2	;Set if drive was specified (unit number in TEMP_UNIT)
PF_FILE	equ	3	;Set if filename specified (in FNAME_BUF)
PF_EXT	equ	4	;Set if extension specified (in FNAME_BUF+8)
PF_AMB	equ	5	;Set if filename or extension is ambiguous
PF_DOT	equ	6	;Set if "." or ".." filename found
PF_2DOT	equ	7	;If PF_DOT is set then this indicates "." or ".."
;
;
;	  MASKS FOR PARSE FLAGS
;
PFM_A	equ	(1 SHL PF_ANY)
PFM_DI	equ	(1 SHL PF_DIR)
PFM_DR	equ	(1 SHL PF_DRV)
PFM_F	equ	(1 SHL PF_FILE)
PFM_E	equ	(1 SHL PF_EXT)
PFM_AM	equ	(1 SHL PF_AMB)
PFM_D	equ	(1 SHL PF_DOT)
PFM_2D	equ	(1 SHL PF_2DOT)
;
;
;
;		  BITS IN ATTRIBUTES BYTE
;		===========================
;
AT_RO	equ	0	;Set => Read only file
AT_HID	equ	1	;Set => Hidden file
AT_SYS	equ	2	;Set => System file
AT_VOL	equ	3	;Set => Volume name entry
AT_DIR	equ	4	;Set => Sub-directory entry
AT_ARC	equ	5	;Set => File needs to be archived
AT_NU6	equ	6	;Not used - clear
AT_DEV	equ	7	;Set => File is a device
;
;
;	MASKS FOR ATTRIBUTE FLAGS
;
ATM_RO	equ	(1 SHL AT_RO)
ATM_HID	equ	(1 SHL AT_HID)
ATM_SYS	equ	(1 SHL AT_SYS)
ATM_VOL	equ	(1 SHL AT_VOL)
ATM_DIR	equ	(1 SHL AT_DIR)
ATM_ARC	equ	(1 SHL AT_ARC)
;
;
;
;		  BITS IN MSX-DOS's CHKCHR FLAGS
;		==================================
;
CF_16B	EQU	1	;Set => First byte of a 16 bit character.
CF_2ND	EQU	2	;Set => Second byte of a 16 bit character.
CF_TE	EQU	4	;Set => filename terminator character
;
;
;
;		  OFFSETS IN FIBs.
;		====================
;
FIB_FILNAME	EQU	1	;ASCII version of filename.
FIB_ATTR	EQU	14	;Attributes byte.
FIB_TIME	EQU	15
FIB_DATE	EQU	17
FIB_CLUSTER	EQU	19	;Start cluster.
FIB_SIZE	EQU	21	;File size.
FIB_DRIVE	EQU	25	;Drive number.
FIB_VOL_ID	EQU	26	;Volume ID.
FIB_DEV		EQU	30	;Device number.
FIB_SA		EQU	31	;Search attributes.
FIB_PAT		EQU	32	;Extended pattern filename (FCB form).
FIB_DVAR	EQU	43	;Directory variables.
;
D_START		EQU	10	;Start of directory information.
D_CLUSTER	EQU	8
D_NEXT_CLU	EQU	6
D_SECTOR	EQU	4
D_S_LEFT	EQU	3
D_E_LEFT	EQU	2
D_ENT		EQU	1
D_ODD		EQU	0
;
;
;
;		  OFFSETS IN DIRECTORY ENTRIES
;		================================
;
DR_FILE		equ	0		;Filename
DR_EXT		equ	8		;Filename extension
DR_ATTR		equ	11		;Attributes byte
DR_DCHAR	equ	12		;Deleted filename character
DR_TIME		equ	22		;Time of last update
DR_DATE		equ	24		;Date of last update
DR_CLU		equ	26		;Fisrt cluster number
DR_SIZE		equ	28		;File size
;
;
