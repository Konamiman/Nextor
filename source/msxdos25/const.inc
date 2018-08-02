	.xlist
;
;------------------------------------------------------------------------------
;
; ASCII and keyboard character constants.
;
BELL            EQU     07h
BS              EQU     08h
TAB             EQU     09h
LF              EQU     0Ah
HOME            EQU     0Bh
FF              EQU     0Ch
CR              EQU     0Dh
INS             EQU     12h
CEOL            EQU     19h
ESC             EQU     1Bh
RIGHT           EQU     1Ch
LEFT            EQU     1Dh
UP              EQU     1Eh
DOWN            EQU     1Fh
DEL             EQU     7Fh
;
CTRL_C          EQU     "C"-40h
CTRL_N          EQU     "N"-40h
CTRL_P          EQU     "P"-40h
CTRL_S          EQU     "S"-40h
CTRL_U          EQU     "U"-40h
CTRL_X          EQU     "X"-40h
CTRL_Z          EQU     "Z"-40h
;
;
;------------------------------------------------------------------------------
;
;                 BITS IN PARSE FLAGS (Register B in parse functions)
;               =======================================================
;
PF_ANY  equ     0       ;Set if any characters parsed after drive name
PF_DIR  equ     1       ;Set if any directory path specified
PF_DRV  equ     2       ;Set if drive was specified (unit number in TEMP_UNIT)
PF_FILE equ     3       ;Set if filename specified (in FNAME_BUF)
PF_EXT  equ     4       ;Set if extension specified (in FNAME_BUF+8)
PF_AMB  equ     5       ;Set if filename or extension is ambiguous
PF_DOT  equ     6       ;Set if "." or ".." filename found
PF_2DOT equ     7       ;Set if ".." filename found
;
;         MASKS FOR PARSE FLAGS
;
PFM_A   equ     (1 SHL PF_ANY)
PFM_DI  equ     (1 SHL PF_DIR)
PFM_DR  equ     (1 SHL PF_DRV)
PFM_F   equ     (1 SHL PF_FILE)
PFM_E   equ     (1 SHL PF_EXT)
PFM_AM  equ     (1 SHL PF_AMB)
PFM_D   equ     (1 SHL PF_DOT)
PFM_2D  equ     (1 SHL PF_2DOT)
;
;
;
;                 BITS IN PARSE CONTROL FLAGS VARIABLE (PC_FLAGS)
;               ===================================================
;
PC_LAST equ     0       ;Set if last item treated as directory
PC_NULL equ     1       ;Set if null last item replaced with "*.*"
PC_ERR  equ     2       ;Set to suppress errors from "ADD_WPTH"
PC_DISK equ     3       ;Set to allow accessing of disk
PC_END  equ     4       ;Set if this is the last item
PC_ROOT equ     5       ;Set if root directory forced
PC_VAL  equ     6       ;Set if disk has been validated
PC_NDRV equ     7       ;Set if no drive allowed (for MOVE)
;
;         MASKS FOR PARSE CONTROL FLAGS
;
PCM_LA  equ     (1 SHL PC_LAST)
PCM_NU  equ     (1 SHL PC_NULL)
PCM_ER  equ     (1 SHL PC_ERR)
PCM_DI  equ     (1 SHL PC_DISK)
PCM_EN  equ     (1 SHL PC_END)
PCM_RO  equ     (1 SHL PC_ROOT)
PCM_VA  equ     (1 SHL PC_VAL)
PCM_ND  equ     (1 SHL PC_NDRV)
;
;
;                 BITS IN "CH_FLAGS" VARIABLE
;               ===============================
;
CF_UP   equ     0       ;Set if characters should be upper-cased
CF_16B  equ     1       ;Set if 1st byte of 16-bit character
CF_2ND  equ     2       ;Set if 2nd byte of 16-bit character
CF_VOL  equ     3       ;Set if parsing volume name (must equal AT_VOL)
CF_TE   equ     4       ;Set if character is a terminator
;
;         MASKS FOR "CH_FLAGS" BITS
;
CFM_UP  equ     (1 SHL CF_UP)
CFM_16B equ     (1 SHL CF_16B)
CFM_2ND equ     (1 SHL CF_2ND)
CFM_VOL equ     (1 SHL CF_VOL)
CFM_TE  equ     (1 SHL CF_TE)
;
;
;
;                 BITS IN "FAT_FLAGS" VARIABLE
;               ================================
;
FF_OVR  equ     0       ;Set if FAT entry overflows sector
FF_DBUF equ     1       ;Set if directory sector buffer needs checking
;
;
;
;                 BITS IN "SE_FLAGS" VARIABLE
;               ===============================
;
SE_FREE equ     0       ;Set if free directory entry located
SE_ERR  equ     1       ;Set if CHK_CLU cluster found in PARSE_PATH
SE_SRCH equ     2       ;Set if searching, clear if creating
SE_NEW  equ     3       ;Set if create must not overwrite existing files
;
;
;         MASKS FOR "SE_FLAGS" BITS
;
SEM_FR  equ     (1 SHL SE_FREE)
SEM_ER  equ     (1 SHL SE_ERR)
SEM_SR  equ     (1 SHL SE_SRCH)
;
;
;
;                 BITS IN "RW_FLAGS" VARIABLE
;               ===============================
;
RF_READ equ     0       ;Set=>read     Clear=>write
RF_ZERO equ     1       ;Set => zero fill
RF_PAGE equ     2       ;Set => current segments,    Clear => TPA segments
RF_NEXT equ     3       ;Set => must step to next sector
RF_SIZE equ     4       ;Set => writing zero bytes to change file size
RF_DOS2	equ	5	;Set => running RDDRV or WRDRV
;
;
;         MASKS FOR "RW_FLAGS" BITS
;
RFM_RD  equ     (1 SHL RF_READ)
RFM_ZF  equ     (1 SHL RF_ZERO)
RFM_PG  equ     (1 SHL RF_PAGE)
RFM_NXT equ     (1 SHL RF_NEXT)
RFM_SZ  equ     (1 SHL RF_SIZE)
RFM_D2	equ	(1 SHL RF_DOS2)
;
;
;
;                 BITS IN ATTRIBUTES BYTE
;               ===========================
;
AT_RO   equ     0       ;Set => Read only file
AT_HID  equ     1       ;Set => Hidden file
AT_SYS  equ     2       ;Set => System file
AT_VOL  equ     3       ;Set => Volume name entry
AT_DIR  equ     4       ;Set => Sub-directory entry
AT_ARC  equ     5       ;Set => File needs to be archived
;===== start add FAT16 (for CP/M compatible FCB file access)
AT_RES  equ     6	;Reserved bit
;===== end add FAT16
AT_DEV  equ     7       ;Set => Device file (all other bits clear)
;
;
;       MASKS FOR ATTRIBUTE FLAGS
;
ATM_RO  equ     (1 SHL AT_RO)
ATM_HID equ     (1 SHL AT_HID)
ATM_SYS equ     (1 SHL AT_SYS)
ATM_VOL equ     (1 SHL AT_VOL)
ATM_DIR equ     (1 SHL AT_DIR)
ATM_ARC equ     (1 SHL AT_ARC)
ATM_DEV equ     (1 SHL AT_DEV)
;
;
;                 BITS IN "FAB_ACC" ACCESS MODE BYTE
;               =====================================
;
AC_WR   equ     0       ;Set => Read only access
AC_RD   equ     1       ;Set => Write only access
AC_INH  equ     2       ;Set => Should be inherited by child process
AC_DEAD equ     3       ;Set => File has been deleted
AC_MOD  equ     7       ;Set => File has been written to
;
;
;       MASKS FOR ACCESS MODE BITS
;
ACM_WR  equ     (1 SHL AC_WR)
ACM_RD  equ     (1 SHL AC_RD)
ACM_INH equ     (1 SHL AC_INH)
ACM_DED equ     (1 SHL AC_DEAD)
ACM_MOD equ     (1 SHL AC_MOD)
;
;
;                 BITS IN "FAB_MODE" BYTE
;               =====================================
;
MD_CIN  equ     0       ;Set => Console input device
MD_COUT equ     1       ;Set => Console output device
MD_CPM  equ     2       ;Set => CP/M style input
MD_ASC  equ     5       ;Set => ASCII mode (otherwise binary)
MD_EOF  equ     6       ;Set => End of file on read
MD_DEV  equ     7       ;Set => Device file (otherwise disk file)
;
;
;       MASKS FOR "FAB_MODE" BITS
;
MDM_CIN equ     (1 SHL MD_CIN)
MDM_COT equ     (1 SHL MD_COUT)
MDM_CPM equ     (1 SHL MD_CPM)
MDM_ASC equ     (1 SHL MD_ASC)
MDM_EOF equ     (1 SHL MD_EOF)
MDM_DEV equ     (1 SHL MD_DEV)
;
;
;                 BITS IN "UD_FLAGS" VARIABLE
;               ===============================
;
UF_F12	equ     0       ;Set => disk is FAT12
UF_F16  equ     1       ;Set => disk is FAT16
UF_UNDL equ     2       ;Set => disk supports undeleting
UF_RALL	equ	3	;Set => reduced allocation information mode
UF_LCK	equ	4	;Set => drive is locked
UF_Z80	equ	5	;Set => drive needs Z80 mode
;
;
;         MASKS FOR "UD_FLAGS" BITS
;
UFM_F2  equ     (1 SHL UF_F12)
UFM_F6  equ     (1 SHL UF_F16)
UFM_UN  equ     (1 SHL UF_UNDL)
UFM_RA  equ     (1 SHL UF_RALL)
UFM_MT  equ     (1 SHL UF_LCK)
UFM_Z8	equ	(1 SHL UF_Z80)
;
;
;                 BITS IN "UD_DFLAGS" VARIABLE
;               ================================
;
UF_DBD	equ	0	;Set => drive assigned to a device-based driver
UF_HPL	equ	1	;Set => device supports hot-plug
UF_RMV	equ	2	;Set => drive assigned to a removable device
UF_PAP	equ	3	;Set => drive is pending partition assignment
UF_FOK	equ	4	;Set => value at UD_FSEC is valid
UF_NCH  equ 5   ;Set => discard next device change status and return "not changed"
;
;
;         MASKS FOR "UD_DFLAGS" BITS
;
UFM_DB	equ	(1 SHL UF_DBD)
UFM_HP	EQU	(1 SHL UF_HPL)
UFM_RM	equ	(1 SHL UF_RMV)
UFM_PP	equ	(1 SHL UF_PAP)
UFM_FO	equ	(1 SHL UF_FOK)

NUMKEYS	equ	0FCBBh

;                 DRIVER CONFIGURATION KEYS
;               =============================

CFG_DRVS   equ 1
CFG_DEVLUN equ 2

;-----------------------------------------------------------------------------
;
	.list
;
