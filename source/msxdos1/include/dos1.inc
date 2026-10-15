;
;        This file is included by MSXDOS.SYS 1.03 and COMMAND.COM 1.11. It
; defines the CP/M style page zero locations, the FCB layout, the MSX-DOS 1
; system variables and disk ROM entry points used by them, and the layout of
; the data area that MSXDOS.SYS shares with COMMAND.COM.
;
;        The DOS function codes (_CONOUT, _FOPEN...) and some BIOS entry points
; and work area addresses are taken from the Nextor SDK constant files (the
; SDK root is passed to the assembler with --include-directory, see the
; Makefile). The names of the system variables are those used for the same
; addresses in the Nextor kernel (source/kernel/data.mac).
;
	include	"asm/constants/dos_calls.inc"
	include	"asm/constants/msx_bios.inc"
	include	"asm/constants/msx_workarea.inc"
;
;
CR	EQU	0DH
LF	EQU	0AH
TAB	EQU	09H
ESC	EQU	1BH
EOFCHR	EQU	1AH		; End of file mark (Ctrl-Z).
;
;
;------- Page zero
;
JWBOOT	EQU	0000H		; JP to the BIOS warm boot entry point.
BDOS	EQU	0005H		; JP to the BDOS function dispatcher.
FCB	EQU	005CH		; Default FCB.
FCB2	EQU	006CH		; Second default FCB (overlaps the first one).
CMDLIN	EQU	0080H		; Command line tail (length byte, then the text
				;   terminated with a CR). Default DMA area too.
TPA	EQU	0100H		; Start of the TPA, transient programs load here.
;
;
;------- FCB fields
;
;        The search functions return in the DMA area the drive number plus
; the 32 byte directory entry, with the kernel moving the attributes byte
; from the directory entry into byte 0Dh (FCB_S1) and putting the extent
; number in byte 0Ch.
;
FCB_DRV	EQU	00H		; Drive number (0 = default, 1 = A:...).
FCB_NAM	EQU	01H		; File name, 8 characters.
FCB_EXT	EQU	09H		; Extension, 3 characters.
FCB_EX	EQU	0CH		; Extent number.
FCB_S1	EQU	0DH		; Attributes in search results.
FCB_RSZ	EQU	0EH		; Record size.
FCB_SIZ	EQU	10H		; File size (4 bytes).
FCB_DAT	EQU	14H		; File date.
FCB_TIM	EQU	16H		; File time.
FCB_CR	EQU	20H		; Current record.
FCB_RR	EQU	21H		; Random record (4 bytes).
FCB_LEN	EQU	25H		; Size of a complete FCB.
;
SR_TIM	EQU	17H		; Time, date and size fields in a search
SR_DAT	EQU	19H		;   result (the drive byte followed by the
SR_SIZ	EQU	1DH		;   directory entry).
;
ATR_HID	EQU	02H		; File attribute bits.
ATR_SYS	EQU	04H
ATR_VOL	EQU	08H
ATR_DIR	EQU	10H
;
;
;------- MSX-DOS 1 page 3 system variables
;
PRTBUF	EQU	0F1C9H		; String output routine (BDOS function 09h).
PRTECHO	EQU	0F23BH		; Console output echo to printer flag.
DTA_ADDR EQU	0F23DH		; Current DMA address.
SPSAVE	EQU	0F304H		; Saved user stack pointer.
CPMCALL	EQU	0F306H		; NZ => return HL=BA (CP/M style) from BDOS.
SRCHFCB	EQU	0F307H		; FCB passed to "search first".
COUNTRY	EQU	0F30EH		; Date format: 0=Y-M-D, 1=M-D-Y, 2=D-M-Y.
DISKVECT EQU	0F323H		; Pointer to pointer to disk error handler.
BREAKVECT EQU	0F325H		; Pointer to pointer to Ctrl-C handler.
TIMFLG	EQU	0F338H		; NZ => the machine has a clock chip.
NOTFIRST EQU	0F340H		; Zero on the very first boot of MSX-DOS.
$DOSON	EQU	0F368H		; Switch the disk ROM into page 1.
$DOSOFF	EQU	0F36BH		; Switch the DOS RAM into page 1.
;
;
;------- Other MSX BIOS entry points and work area
;
INITXT	EQU	006CH		; Initialise SCREEN 0.
INIT32	EQU	006FH		; Initialise SCREEN 1.
EXBRSA	EQU	0FAF8H		; Slot of the MSX2 SUB-ROM (zero on MSX1).
;
;
;------- MSX-DOS 1 disk ROM entry points
;
;        These are at fixed addresses in the disk ROM, and the disk ROM must
; be switched into page 1 ($DOSON) when calling them.
;
K_BASIC	EQU	4022H		; Start Disk BASIC.
K_FORMAT EQU	4025H		; Format a disk, HL=buffer, BC=buffer size.
K_SYSBOT EQU	4030H		; Get in HL the lowest address used by MSX-DOS.
BIOS_STAT EQU	4034H		; BIOS level console status.
BIOS_IN	EQU	4078H		; BIOS level console input.
BIOS_OUT EQU	408FH		; BIOS level console output.
K_FLUSH	EQU	50A9H		; Flush all disk buffers (part of F_DSKRST).
K_CRLF	EQU	5183H		; Print CR+LF on the console.
K_CTLOUT EQU	535DH		; Print character, control ones as ^x.
;
;        BDOS function handlers.
;
F_CONIN	EQU	5445H		; 01h
F_CONOUT EQU	53A7H		; 02h
F_AUXIN	EQU	546EH		; 03h
F_AUXOUT EQU	5474H		; 04h
F_LSTOUT EQU	5465H		; 05h
F_DIRIO	EQU	5454H		; 06h
F_DIRIN	EQU	5462H		; 07h
F_INNOE	EQU	544EH		; 08h
F_BUFIN	EQU	50E0H		; 0Ah
F_CONST	EQU	543CH		; 0Bh
F_CPMVER EQU	41EFH		; 0Ch
F_DSKRST EQU	509FH		; 0Dh
F_SELDSK EQU	50D5H		; 0Eh
F_FOPEN	EQU	4462H		; 0Fh
F_FCLOSE EQU	456FH		; 10h
F_SFIRST EQU	4FB8H		; 11h
F_SNEXT	EQU	5006H		; 12h
F_FDEL	EQU	436CH		; 13h
F_RDSEQ	EQU	4775H		; 14h
F_WRSEQ	EQU	477DH		; 15h
F_FMAKE	EQU	461DH		; 16h
F_FREN	EQU	4392H		; 17h
F_LOGIN	EQU	504EH		; 18h
F_CURDRV EQU	50C4H		; 19h
F_SETDTA EQU	5058H		; 1Ah
F_ALLOC	EQU	505DH		; 1Bh
F_RDRND	EQU	4788H		; 21h
F_WRRND	EQU	4793H		; 22h
F_FSIZE	EQU	501EH		; 23h
F_SETRND EQU	50C8H		; 24h
F_WRBLK	EQU	47BEH		; 26h
F_RDBLK	EQU	47B2H		; 27h
F_WRZER	EQU	47D1H		; 28h
F_GDATE	EQU	553CH		; 2Ah
F_SDATE	EQU	5552H		; 2Bh
F_GTIME	EQU	55DBH		; 2Ch
F_STIME	EQU	55E6H		; 2Dh
F_VERIFY EQU	55FFH		; 2Eh
F_RDABS	EQU	46BAH		; 2Fh
F_WRABS	EQU	4720H		; 30h
;
;
;------- MSXDOS.SYS shared data area
;
;        The data area at the start of the MSXDOS.SYS BDOS page (the page
; that the jump at 0005h points into) is used by COMMAND.COM to keep the
; state that must survive a re-load of COMMAND.COM: the information needed
; to check whether the copy in memory is still intact, and everything about
; the batch file being executed. These are the offsets of the fields from
; the start of the page.
;
SH_SERNO EQU	00H		; CP/M serial number (6 bytes).
SH_BDOS	EQU	06H		; JP to the BDOS function dispatcher.
SH_DSKVEC EQU	09H		; Address of the disk error handler.
SH_BRKVEC EQU	11H		; Address of the Ctrl-C handler.
SH_CMDADR EQU	13H		; Entry point of COMMAND.COM in high memory.
SH_CMDLEN EQU	15H		; Number of words of COMMAND.COM to checksum.
SH_CMDSUM EQU	17H		; Checksum of COMMAND.COM.
SH_BATFLG EQU	19H		; NZ => a batch file is being executed.
SH_BOOTFLG EQU	1AH		; NZ => not the first boot (no AUTOEXEC.BAT).
SH_PARMTAB EQU	1BH		; Pointers to the batch parameters %0-%9.
SH_BATFCB EQU	2FH		; FCB of the batch file being executed.
SH_BATLIN EQU	54H		; Copy of the batch file command line, where
				;   the batch parameters point to.
