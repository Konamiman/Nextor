/*****************************************************************************/
/*                                                                           */
/*                 MSX-DOS 2 DISKCOPY Utility, Main Program                  */
/*                                                                           */
/*                       Copyright (c) IS Systems Ltd.                       */
/*                                                                           */
/*****************************************************************************/

#asm
	DSEG
	db	13,10
	db	'MSX-DOS 2 DISKCOPY program',13,10
	db	'Version '
	db	VERSION##+'0', '.', RELEASE##/256+'0', RELEASE##+'0',13,10
	db	'Copyright (c) '
	db	CRYEAR##/1000 MOD 10 +'0'
	db	CRYEAR##/ 100 MOD 10 +'0'
	db	CRYEAR##/  10 MOD 10 +'0'
	db	CRYEAR##      MOD 10 +'0'
	db	' ASCII Corporation',13,10
	db	13,10,26
#endasm

                     /*   C O N S T A N T S   */


#define FALSE		0
#define TRUE		-1

#define DEBUG		FALSE

#define CR		13

/* maximum length of CP/M command line (+1), and its absolute address */
#define MAX_CMD_LEN	129
#define CMD_ADDRESS	0x0080

/* length of a sector in bytes */
#define SECT_LENGTH	512		/*NB If this is changed, the program */
					/* will only work as long as the no. */
					/* sectors that can fit in memory is */
					/* less than 255 !!!!!		     */

/* Disc parameter offsets */
#define DP_DRIVE	0
#define DP_DDF		19		/* Dirty disc flag. */
#define DP_VOL_ID	20

#define no_entries	32		/* No of bytes of disk info */


/* Offsets in boot sector */
/* #define dirt_flag	0x46 */
#define	dirt_flag	0x26
#define	vol_id		0x27

/* internal report_error flags */
#define	RE_READING	0
#define	RE_WRITING	1

/* MSX-DOS errors */
#define	FE_IOPT		0x88
#define	FE_IPARM	0x8B
#define	FE_NORAM	0xDE

/* Errors which could be passed to the internal disc error routine that */
/* are to be trapped, and ignored.					*/
#define ERR_WR		0xFE
#define ERR_VR		0xFB
#define ERR_DATA	0xFA
#define ERR_RNF		0xF9
#define ERR_SEEK	0xF3

/* internal diskcopy errors: the only requirement is that these be */
/* unique integers below 32 (so COMMAND.COM will not print errors) */
#define E_INCOM		1
#define E_WR_SSIZ	2
#define E_SAM_DRIVE	3
#define E_NO_ID		4
#define E_WRONG_VER	5


#asm
	.Z80
MSX_DOS		EQU	5
PUBLIC		MSX_DOS

;	Offsets into disk information
S_SIZ		EQU	1		;Sector size.
NO_SECS		EQU	9		;Total number of logical sectors.
	.8080
#endasm


               /*   G L O B A L   V A R I A B L E S   */


int source_drive;		/* logical source drive			     */
int target_drive;		/* logical target drive			     */

int o_chk;			/* value of DSK_CHK variable for restoring   */
int prompt_flag;		/* true if the program should give warnings  */
int cpboot_flag;		/* true if the program should copy bootcode  */
int e_count;			/* count of errors detected during copying   */
int err_sec;			/* sector number of error in copying	     */
int err_num;			/* number of error during copying	     */

int num_sects;			/* number of sectors to copy		     */
int s_size;

unsigned buf_adr;		/* start of free memory to use as buffer     */

char src_dr_info [no_entries];
char trg_dr_info [no_entries];


	    /*   E X T E R N A L    T E X T    */

/*	All the text is stored in a .MAC file called TEXT.  This is to make */
/* it easy to change the text to a different language.  Each message has  a */
/* number which is defined here as an external.  This is passed to a  piece */
/* of code which finds the relevant message and prints it.		    */

extern int M_E_INCOM;
extern int M_E_WR_SSIZ;
extern int M_E_SAM_DR;
extern int M_E_NO_ID;
extern int M_E_VER;
extern int M_FROM;
extern int M_TO;
extern int M_KEY;
extern int M_WARN;
extern int M_ERS;
extern int M_FIN;
extern int M_SRC;
extern int M_TRG;
extern int M_WER;
extern int M_RER;
extern int M_VER;
extern int M_RNF;
extern int M_SEEK;
extern int M_MORE;

	/*========================================*/

main()
{
    prompt_flag = TRUE;		/* By default we warn the user */
    cpboot_flag = FALSE;	/* By default we don't copy boot */

    if (! check_ver()) error(E_WRONG_VER);

    get_drives();		/* Find out which logical drives to use */

    set_abort_routine();	/* Set up an abort routine */
    o_chk = get_dchk();		/* Save disk check status */
    set_chk(0xFF);		/* Disable disk checking for this program */

    do
    {
	e_count = 0;			/* Initially there are no errors. */

	if (prompt_flag) wait_for_ready();

	chk_drives(source_drive, target_drive);
					/* check both are compatible	    */

	set_error_routine ();
	copy_data(source_drive, target_drive);
	unset_error_routine ();

	if (e_count != 0)
	{
	    put_mes(M_WARN);		/* "Warning: " */
	    put_unsigned(e_count);
	    put_mes(M_ERS);		/* " errors were detected. */
					/* Target disk may be unusable." */
	}
	else
	    put_mes(M_FIN);		/* "\r\nDiskcopy finished ok" */
	newline();
    }
    while (prompt_flag && yes_ans(M_MORE));
    newline();
}




/*****************************************************************************/
/*                                                                           */
/*                           Get Drives Module                               */
/*                                                                           */
/*****************************************************************************/
/*				   */
/*     - get_drives		   */
/*	   - parse_flags	   */
/*	   - ask_drives		   */
/*	     - ask_user		   */
/*				   */
/***********************************/


/* parse_flags: this routine parses the C string pointed to by 'ptr'  as  a  */
/* set of flags of the form "/d/g /h". The flags may be separated by blanks  */
/* however the first character pointed to  must  be  non-blank.   Only  one  */
/* option is currently supported - "/X"                                      */

parse_flags(ptr)
char *ptr;
{
    while (*ptr)
    {
	if (*ptr != '/') error(FE_IPARM);
	switch (upper(*(++ptr)))
	{
	    case 'X': prompt_flag = FALSE;
		      break;
	    case 'S': cpboot_flag = TRUE;
		      break;
	    case 0  : error(FE_IPARM);
	    default : error(FE_IOPT);
	}
	while (*(++ptr) == ' ') ;
    }
}

/*---------------------------------------------------------------------------*/
/* ask_user: prompts the user with the given 'prompt' until it (the user)    */
/* enters a valid drive. The function then returns this as a drive no.(1..26)*/

ask_user(mes_num)
int mes_num;
{
    char drv_ch;
    do
    {
	put_mes(mes_num);
	clr_in();
	drv_ch = upper(get_char());
	newline();
    }
    while (! isdrive_ch(drv_ch));
    return (drv_ch - 'A' + 1);
}


/*--------------------------------------------------------------------*/
/* ask_drives: prompts the user to enter the source and target drives */

ask_drives()
{
    source_drive = ask_user(M_SRC);	/* "Source drive? " */
    target_drive = ask_user(M_TRG);	/* "Target drive? " */
}

/*---------------------------------------------------------------------------*/
/* get_drives: finds out which logical drives  to  use  as  the  source  and */
/* destination ('source_drive'  and  'target_drive'), by parsing the command */
/* line and (if neccesary) asking the user. It also parses any flags present */
/* the command line.                                                         */

get_drives()
{
    static char cmd_line[MAX_CMD_LEN];
    char *p, *cmd;
    int cmd_length, i;

/* copy the command line from the CP/M area (at 0x80) to */
/* a C string in the array 'cmd_line' & terminate it with a null */
    p = CMD_ADDRESS;
    cmd = cmd_line;
    cmd_length = (*p++) & 255;
    for (i=0; i<cmd_length; i++,p++,cmd++)
/*	if (*p) *cmd = *p; else *cmd = ' ';	*/
	if (*p) *cmd = upper(*p); else *cmd = ' ';
    *cmd = 0;

    cmd = cmd_line;			/* point 'cmd' to the command line */
    while (*cmd == ' ') cmd++;		/* strip leading blanks */
    if (*cmd)
    {
	if (! isdrive_ch(*cmd))
	    /* No drive given. Therefore the only thing given is flags */
	    /* Parse these and then prompt the user for the drives     */
	{
	    parse_flags(cmd);
	    ask_drives();
	}
	else
	{
	    /* First parameter is a drive name. Check for ':' */
	    source_drive = (*cmd) - 'A' + 1;
	    if (*(++cmd) != ':') error(FE_IPARM);
	    do cmd++; while (*cmd == ' ');	/* strip intermediate blanks */
	    if (isdrive_ch(*cmd))
		/* we have the target drive specified as well */
	    {
		target_drive = (*cmd) - 'A' + 1;
		if (*(++cmd) != ':') error(FE_IPARM);
		/* strip blanks: anything left must be flags */
		do ++cmd; while (*cmd == ' ');
		if (*cmd) parse_flags(cmd);
	    }
	    else if (*cmd)
		/* only the source is specified: pick up the flags and */
		/* let the target default to the current drive */
	    {
		parse_flags(cmd);
		target_drive = get_default_drive();
	    }
	    else
		/* only one drive given, and no flags. So the target */
		/* defaults to the current drive */
		target_drive = get_default_drive();
	}
    }
    else
	/* given a null command line prompt for both drives */
	ask_drives();

#if DEBUG
    if (! prompt_flag) put_string("   *** NoPrompt_flag is set\r\n");
#endif

    if (source_drive == target_drive) error(E_SAM_DRIVE);
}



/*****************************************************************************/
/*                                                                           */
/*                           Check Drives Module                             */
/*                                                                           */
/*****************************************************************************/
/*				   */
/*     - chk_drives		   */
/*	  - get_info		   */
/*				   */
/***********************************/


/* chk_drives: This routine is passed the logical numbers of the two  drives */
/* which  are  involved  in the disc copy.  It finds out any data about them */
/* that is needed for the subsequent copying, including setting up the value */
/* of  num_sects.   It  also  checks that the two drives are compatible with */
/* each other, and prints an error message if this is not the case.          */

chk_drives(drive_1,drive_2)
int drive_1, drive_2;
{
    static int i;

    get_info(drive_1, src_dr_info);		/* Will set up s_size */
    num_sects = get_info(drive_2, trg_dr_info);

#if DEBUG
    put_string ("Number of sectors: "); put_unsigned(num_sects); newline();
    put_string ("Sector size: "); put_unsigned(s_size); newline();
#endif

    for (i=1; i<13; i++)			/* Are drives compatible ?? */
	if (src_dr_info [i] != trg_dr_info [i]) error(E_INCOM);
    if (s_size != SECT_LENGTH) error(E_WR_SSIZ);

    if (src_dr_info [DP_DRIVE] == trg_dr_info [DP_DRIVE]) error(E_SAM_DRIVE);
		/* Give an error if both physical drives are the same */

    if (is_VOL_ID(src_dr_info) && (! is_VOL_ID(trg_dr_info))) error(E_NO_ID);
		/* Error if volume ID on source, but not target. */
}

/*--------------------------------------------------------------------------*/
/* get_info: This routine is passed a buffer and a drive number,  which  it */
/* passes on to the MSX_DOS function to get disc parameters.		    */

get_info(drive,info)
int drive;
char info [no_entries];
{
#asm
	.z80
	POP	BC			;Return address.
	POP	DE			;Address to store information.
	POP	HL			;Drive number -> L.
	PUSH	HL
	PUSH	DE
	PUSH	BC
	LD	C,_DPARM##
	PUSH	DE
	CALL	MSX_DOS
	OR	A			;Is there an error ??
	JP	NZ,MYSTOP		;Give up if there is.
	POP	IX			;Address of buffer.
	LD	L,(IX+S_SIZ)
	LD	H,(IX+S_SIZ+1)		;Sector size.
	LD	(s_size),HL
	LD	L,(IX+NO_SECS)
	LD	H,(IX+NO_SECS+1)	;Return no. of logical sectors.
	RET

MYSTOP:	LD	B,A			;Error.
	LD	C,_TERM##		;Terminate & explain error.
	CALL	MSX_DOS
	JP	0

	.8080
#endasm
}



/*****************************************************************************/
/*                                                                           */
/*                             Copy Data Module                              */
/*                                                                           */
/*****************************************************************************/
/*				   */
/*     - copy_data		   */
/*	   - alloc_buffers	   */
/*	   - report_error	   */
/*	   - read_sectors	   */
/*	   - write_sectors	   */
/*				   */
/***********************************/


/*------------------------------------------------------------------------*/
/* alloc_buffers: determine how many buffers we have, and their locations */

alloc_buffers()
{
    unsigned high_mem, track_length;
    int buf_count, i;

    high_mem = get_high_mem();
    if ((buf_adr = allocate(1)) == -1) error(FE_NORAM);
    if ((buf_adr = next_half(buf_adr)) > high_mem) error(FE_NORAM);

    buf_count = (high_mem - buf_adr) / SECT_LENGTH;

#if DEBUG
    put_string("--- Allocated buffers information:\r\n");
    put_string("    Start of buffer = ");
    put_hex(buf_adr);
    put_string("\r\n    High memory = ");
    put_hex(high_mem);
    put_string("\r\n    Number of allocated buffers = ");
    put_unsigned(buf_count);
    newline();
    newline();
#endif

    return(buf_count);
}

/*--------------------------------------------------------*/
/* copy_data: this routine actually copies an entire disc */

copy_data(source_dr, target_dr)
int source_dr, target_dr;
{
    static int  num_buffers, num_now, sect, err_flag;
    static char boot_sector[SECT_LENGTH] = {0};
    int i;

#if DEBUG
    put_string("--- Copy data\r\n");
#endif

    num_buffers = alloc_buffers();

    if (!cpboot_flag) {
	if (err_flag = read_sectors(0, 1, target_dr, boot_sector))
	    error (err_flag);
	if (is_VOL_ID(src_dr_info))
	    boot_sector[dirt_flag] = src_dr_info[DP_DDF];
    }
    else {
	if (err_flag = read_sectors(0, 1, source_dr, boot_sector))
	    error (err_flag);
	if (is_VOL_ID(src_dr_info))
	    for (i=0; i<4; i++)
		boot_sector[vol_id+i] = trg_dr_info[DP_VOL_ID+i];
    }

    sect = 1;
    while (sect < num_sects)
    {
	num_now = (num_sects-sect > num_buffers) ? (num_buffers)
						 : (num_sects-sect);
	if (err_flag = read_sectors(sect, num_now, source_dr, buf_adr))
	    error (err_flag);

	if (sect == 1)
	    if (err_flag = write_sectors(0, 1, target_dr, boot_sector))
		error (err_flag);
	if (err_flag = write_sectors(sect, num_now, target_dr, buf_adr))
	    error (err_flag);
	sect += num_now;
    }
}


/*------------------------------------------------------------------------*/
/* do_err:  This routine gets jumped to if an error occurs whilst a  copy */
/* is  in  progress.   Basically,  it looks to see what type of error has */
/* occurred, and if possible it will ignore it and report it to the user. */
/* If  this turns out not to be possible,  the routine will return to the */
/* systems error handling routine.					  */
/*	This routine assumes that err_num has been set up already,  which */
/* is done in the routine that is jumped to when A/R/I errors occur.	  */

do_err()
{
    static int ret_code;

    ret_code = 3;		/* Ignore error if possible */
    switch (err_num)
    {
	case ERR_WR:   put_mes (M_WER);  break;
	case ERR_VR:   put_mes (M_VER);  break;
	case ERR_DATA: put_mes (M_RER);  break;
	case ERR_RNF:  put_mes (M_RNF);  break;
	case ERR_SEEK: put_mes (M_SEEK); break;
	default: ret_code = 0; break;		/* MSX-DOS error routine */
    }
    if (ret_code == 3)
    {
	put_unsigned (err_sec); newline();	/* Print sector number	 */
	e_count ++;				/* Error count		 */
    }
    return (ret_code);
}

/*---------------------------------------------------------------------------*/
/* read_sector: This routine will read some absolute sectors into the buffer */
/* that is passed as a parameter.  It will return any error which happens    */
/* on the way.								     */

read_sectors(sector, no_of_sects, drive, buffer)
int sector, no_of_sects, drive;
unsigned buffer;
{
#if DEBUG
    put_string("reading from sector: "); put_number(sector);
    put_string(" for: "); put_number(no_of_sects);
    put_string(" Drive: "); put_char(drive-1+'A'); newline();
#endif
#asm
	.Z80
	POP	HL			;Return address.
	POP	DE			;Buffer to read to.
	PUSH	DE			;Restore stack.
	PUSH	HL
	LD	C,_SETDTA##
	CALL	MSX_DOS

	POP	IY			;Get information for actual read.
	POP	IX			;Buffer address.
	POP	HL			;L = Drive.
	POP	BC			;Number of sectors to read.
	POP	DE			;Sector number
	PUSH	DE
	PUSH	BC
	PUSH	HL
	PUSH	IX
	PUSH	IY
	DEC	L			;Adjust drive for 0=A:
	LD	H,C			;Sector count in H for MSX-DOS call.
	LD	C,_RDABS##
	CALL	MSX_DOS
	LD	L,A			;Return any error.
	LD	H,0
	RET

	.8080
#endasm
    }

/*------------------------------------------------------------------------*/
/* write_sectors: This routine writes absolute sectors.			  */

write_sectors(sector, no_sects, drive, buffer)
int no_sects, drive, sector;
unsigned buffer;
{
#asm
	.z80
	POP	HL			;Return address.
	POP	DE			;Buffer to write form.
	PUSH	DE			;Restore address.
	PUSH	HL
	LD	C,_SETDTA##		;Set up disc transfer address.
	CALL	MSX_DOS

	POP	IY			;Get actual parameters.
	POP	IX			;Buffer address.
	POP	HL			;L = drive number.
	POP	BC			;Number of sectors to write.
	POP	DE			;Sector to start writing to.
	PUSH	DE
	PUSH	BC
	PUSH	HL
	PUSH	IX
	PUSH	IY
	DEC	L			;Adjust drive for 0=A:
	LD	H,C			;Sector count -> H. Will be < 255 !
	LD	C,_WRABS##
	CALL	MSX_DOS			;Go and do the write.
	LD	L,A			;Pass error back to program.
	LD	H,0
	RET
#endasm
}


/*****************************************************************************/
/*                                                                           */
/*                             Odds and Sods Module                          */
/*                                                                           */
/*****************************************************************************/


check_ver()
{
#asm
	.z80
	ld	c,_DOSVER##
	call	MSX_DOS
	ld	hl,0
	ret	nz
	ld	a,1
	cp	b
	ret	nc
	cp	d
	ret	nc
	dec	hl
	ret
	.8080
#endasm
}


/* set_chk: sets the "DSK_CHK" variable to a specified value by */
/* doing a simple MSX_DOS function call				*/

set_chk(value)
int value;
{
#asm
	.z80
	pop	de
	pop	hl
	push	hl
	push	de
	ld	b,l
	ld	a,1
	ld	c,_DSKCHK##
	call	MSX_DOS
	ret
	.8080
#endasm
}


/* get_dchk: gets the current value of the "DSK_CHK" variable	*/

get_dchk()
{
#asm
	.z80
	ld	b,l
	xor	a
	ld	c,_DSKCHK##
	call	MSX_DOS
	ld	l,b
	ld	h,0
	ret
	.8080
#endasm
}


/* get_default_drive: returns the number of the current drive (1..26) */

get_default_drive()
{
#asm
	.z80
	LD	C,_CURDRV##		;CP/M function number.
	CALL	MSX_DOS			;L returns as no. current drive.
	INC	A
	LD	L,A
	LD	H,0
	RET
	.8080
#endasm
}

/*---------------------------------------------------------------------------*/
/* next_half: This routine is used to find the next half K boundary for  the */
/* parameter  that is passed to it.   It is included so that the buffer used */
/* the copy does not get a sector crossing a segment boundary.   This is not */
/* essential, but it will improve efficiency if any errors occur.	     */

next_half (mem_adr)
unsigned mem_adr;
{
#asm
	.z80
	POP	DE			;Return address.
	POP	HL			;Address to change.
	PUSH	HL			;Restore stack.
	PUSH	DE
	LD	A,L
	OR	A			;If L is not zero, then move to
	JR	Z,POS_OK		; next 256 byte part.
	LD	L,0
	INC	H
POS_OK:	BIT	0,H
	RET	Z			;Return if on 512 byte boundary.
	INC	H			;Will now be on a boundary.
	RET
	.8080
#endasm
}

/*---------------------------------------------------------------------------*/
/* set_abort_routine: This sets up an abort handler routine.  This is so     */
/* that the disk check status variable can be restored on exit from the      */
/* program.								     */

set_abort_routine ()
{
#asm
	.z80
	LD	DE,ab_rtn		;Address of routine.
	LD	C,_DEFAB##
	CALL	MSX_DOS
	RET
	.8080
#endasm
}

ab_rtn()
{
#asm
	.z80
	push	af
	ld	hl,(o_chk)
	ld	b,l			;Restore original state of
	ld	a,1			; disk check variable before
	ld	c,_DSKCHK##		; exiting.
	call	MSX_DOS
	pop	af
	ret
	.8080
#endasm
}


/*---------------------------------------------------------------------------*/
/* set_error_routine: This sets up a disc error handler routine.  This is so */
/* that ordinary read/write errors which occur whilst copying can be ignored */
/* and the sector number reported to the user.				     */

set_error_routine ()
{
#asm
	.z80
	LD	DE,cpy_err		;Address of routine.
	LD	C,_DEFER##
	CALL	MSX_DOS
	RET
	.8080
#endasm
}

unset_error_routine ()
{
#asm
	.z80
	LD	DE,0
	LD	C,_DEFER##
	CALL	MSX_DOS
	RET
	.8080
#endasm
}

/*-----------------------------------------------------------------------*/
/* cpy_err:  This is the routine that is jumped to first when  an  error */
/* happens  during copying.   It sets up various C variables so that the */
/* actual reporting and decisions may be made by a piece of C code.	 */

cpy_err()
{
#asm
	.z80
	BIT	3,C			;Check if sector passed is valid.
	JR	Z,NORMER		;If not, do the normal error.
	LD	(err_num),A		;Store error number.
	LD	(err_sec),DE		;Store error sector number.
	CALL	do_err			;Jump to C routine.
	LD	A,L			;0 or 3 : normal or ignore.
	RET

NORMER:	XOR	A			;Signify this to MSX-DOS.
	RET
	.8080
#endasm
}

/*------------------------------------------------------------------------*/
/* yes_ans: takes a message, and prints it; then it waits for a yes or no */
/* response, and returns TRUE if the answer is yes; FALSE otherwise.      */

yes_ans(mes)
int mes;
{
char ans;

    do
    {
	put_mes(mes);
	clr_in();
	ans = get_char();
	newline();
    }
    while (! is_yes_or_no(ans));

    return (yes(ans));
}

/*------------------------------------------------------------------------*/
/* wait_for_ready: prints out what the disc copy is about to do, and then */
/* will wait for a key to be typed.					  */

wait_for_ready()
{
    newline();
    put_mes(M_FROM); put_char(source_drive + 'A'-1); put_char(':'); newline();
/*  newline(); */
    put_mes(M_TO);   put_char(target_drive + 'A'-1); put_char(':'); newline();
/*  newline(); */
    put_mes(M_KEY); clr_in(); get_char(); newline();
/*  newline(); */
}



/*****************************************************************************/
/*                                                                           */
/*                      Basic Predicates Module                              */
/*                                                                           */
/*****************************************************************************/


/* is_yes_or_no: determines whether its argument is a valid character */
/* in response to a yes/no question */

is_yes_or_no(ch)
char ch;
{
    if (yes(ch)) return(TRUE);
#asm
	.z80
	POP	HL		;Return address.
	POP	DE		;The character.
	PUSH	DE		;Restore the stack.
	PUSH	HL
NOTYES:	LD	HL,NO_CHARS##
NOLOP:	LD	A,(HL)
	OR	A
	JR	Z,NOTNO
	CP	E
	INC	HL
	JR	NZ,NOLOP
	LD	HL,-1
	RET
NOTNO:	LD	HL,0
	RET
	.8080
#endasm
}

yes(ch)
char ch;
{
#asm
	.Z80
	POP	HL
	POP	DE
	PUSH	DE
	PUSH	HL
	LD	HL,YES_CHARS##
YSLOP2:	LD	A,(HL)
	OR	A
	JR	Z,NOYES2
	CP	E
	INC	HL
	JR	NZ,YSLOP2
	LD	HL,-1
	RET
NOYES2:	LD	HL,0
	RET
	.8080
#endasm
}


/* islower: returns true if its argument is a lower case letter */
islower(c)
char c;
{ return ( 'a' <= c && c <= 'z' ); }


/* isdrive_ch: returns true if its argument is a valid logical */
/* drive character */
isdrive_ch(c)
char c;
{ return ( 'A' <= c && c <= 'Z' ); }


/* is_power_of_two: returns true if its argument is a power of two or zero */
is_power_of_two(num)
int num;
{ return ( (num & (-num)) == num); }


/* upper: converts its argument to upper case */
upper(c)
char c;
{ return (islower(c) ? (c - 'a' + 'A') : c); }


/* in_range: returns true if 'num' is in the range 'lower'..'higher' */
inrange(num, lower, higher)
int num, lower, higher;
{ return ( (lower <= num) && (num <= higher) ); }


/* is_VOL_ID: looks in dr_info to see if the disc has a volume ID */
is_VOL_ID(dr_info)
char dr_info[];
{ return ( ((dr_info[DP_VOL_ID]) & 0xFF)  <= 127); }



/*****************************************************************************/
/*                                                                           */
/*                         Error Handling Module                             */
/*                                                                           */
/*****************************************************************************/


/* error: prints an error message and terminates the program */

error(num)
int num;
{
    switch (num)
    {
    case E_WR_SSIZ:
	put_mes(M_E_WR_SSIZ); break; /* "Wrong sector size"	     */
    case E_INCOM:
	put_mes(M_E_INCOM); break; /* "Two drives incompatible"      */
    case E_SAM_DRIVE:
	put_mes(M_E_SAM_DR); break; /* "Can't copy onto itself"      */
    case E_NO_ID:
	put_mes(M_E_NO_ID); break;  /* "Target not MSX_DOS 2 disc"   */
    case E_WRONG_VER:
	put_mes(M_E_VER); break; /* "Wrong version of MSX-DOS"	     */
    }
    newline();
    exit(num);		/* Return to MSX_DOS with our error code. */
}



/*****************************************************************************/
/*                                                                           */
/*                  Operating System Interface Module                        */
/*                                                                           */
/*       This module supports the following functions:                       */
/*            get_high_mem()                                                 */
/*                                                                           */
/*****************************************************************************/


/* get_high_mem: returns the highest free byte in memory */

get_high_mem()
{
#asm
	.Z80
	LD	HL,(6)			;load top of memory
	DEC	HL			; decrement it for caution
	RET				; return it
	.8080
#endasm
}


/*****************************************************************************/
/*                                                                           */
/*                 I/O primitives for IS-DOS utility programs                */
/*                                                                           */
/*       This module supports the following functions:                       */
/*            get_char()                   put_number(number)                */
/*            put_char(ch)                 put_unsigned(unsigned_num)        */
/*            put_string(string)           newline()                         */
/*            put_hex(16_bit_number)       put_hbyte(8_bit_number)           */
/*                                                                           */
/*****************************************************************************/


put_mes(mes_num)
int mes_num;
{
    static char *txt_adr;
    txt_adr = get_mes_adr(mes_num);
    put_string(txt_adr);
}

get_mes_adr(mes_num)
int mes_num;
{
#asm
	.z80
	POP	HL
	POP	DE
	LD	A,E
	PUSH	DE
	PUSH	HL
	CALL	GET_MSG_ADR##		;Get address of text.
	RET
	.8080
#endasm
}


/* clr_in:  clears input buffer */
clr_in()
{
#asm
	.z80
CLINLP:	LD	C,_CONST##
	CALL	MSX_DOS
	OR	A
	RET	Z
	LD	C,_INNOE##
	CALL	MSX_DOS
	JR	CLINLP
	.8080
#endasm
}


/* get_char: input a character */
get_char()
{
#asm
	.z80
	LD	C,_INNOE##
	CALL	MSX_DOS			;Read one character from keyboard
	PUSH	AF
	LD	E,A
	LD	C,_CONOUT##
	CP	" "			;Only echo the character if it
	CALL	NC,MSX_DOS		; is not a control char.
	POP	AF
	LD	L,A
	LD	H,0
	RET
	.8080
#endasm
}


/* putchar: output a character to the screen */
putchar(c)
int c;
{
#asm
	.z80
	POP	HL			;pop the return address
	POP	DE			; and the character into DE
	PUSH	DE			;restore the stack
	PUSH	HL
	LD	C,_CONOUT##		;now write the character to the
	CALL	MSX_DOS			;Char passed in E.
	RET
	.8080
#endasm
}

/* put_char: character output, converts \n to CR/LF */
put_char(c)
int c;
{ putchar(c); }

/* newline: writes CR/LF */
newline()
{ put_char('\r'); put_char('\n'); }

/* put_string: writes a c-string to the screen */
put_string(s)
char *s;
{ while (*s) put_char(*s++); }

/* put_unsigned: writes an unsigned integer to the screen */
put_unsigned(i)
unsigned i;
{
    if (i>=10)
	put_unsigned(i/10);
    put_char((i%10)+'0');
}



#if DEBUG	/* Extra ouput routines for DEBUGging option */

/* put_number: writes a signed integer (-32768..32767) to the screen */
put_number(i)
int i;
{
    if (i<0)
	{ put_char('-');  i=(-i); }
    else
	put_char('+');
    put_unsigned(i);
}

put_hex(u)
unsigned u;
{
    put_hbyte(u/256);
    put_hbyte(u & 0xFF);
}

put_hbyte(h)
int h;
{
    puthexchar(h/16);
    puthexchar(h & 0x0F);
}

puthexchar(num)
int num;
{
    num &= 0x0F;
    if (num <10) put_char(num + '0');
	else put_char(num - 10 + 'A');
}

#endif

/************************ E N D   O F   P R O G R A M ************************/
