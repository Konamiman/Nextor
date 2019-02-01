/*****************************************************************************/
/*                                                                           */
/*                   MSX-DOS 2 XCOPY Utility, Function Library               */
/*                                                                           */
/*                     Copyright (c) 1986 IS Systems Ltd.                    */
/*                                                                           */
/*****************************************************************************/

#include XCOPY.H


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
    return (yes(ch) || no(ch));
    }

/* yes: returns true if the character that is passed as a parameter to it  */
/* is a valid 'Yes' character, which is stored in the table in TEXT.MAC.   */
yes(ch)
    char ch;
    {
#asm
	.Z80
	POP     HL              ;Return address.
	POP     DE              ;The character.
	PUSH    DE              ;Restore the stack.
	PUSH    HL
	LD      HL,YES_CHARS##  ;Table of allowed responses.
YESLOP: LD      A,(HL)
	OR      A
	JR      Z,NOTYES
	CP      E
	INC     HL
	JR      NZ,YESLOP
	LD      HL,-1           ;It is a yes character.
	RET
NOTYES: LD      HL,0            ;Not a valid 'Yes' character.
	RET
	.8080
#endasm
    }

/* no: returns true if the character that is passed as a parameter to it  */
/* is a valid 'No' character, which is stored in the table in TEXT.MAC.   */
no(ch)
    char ch;
    {
#asm
	.Z80
	POP     HL              ;Return address.
	POP     DE              ;The character.
	PUSH    DE              ;Restore the stack.
	PUSH    HL
	LD      HL,NO_CHARS##   ;Table of allowed responses.
NOLOP:  LD      A,(HL)
	OR      A
	JR      Z,NOTNO
	CP      E
	INC     HL
	JR      NZ,NOLOP
	LD      HL,-1           ;It is a no character.
	RET
NOTNO:  LD      HL,0            ;Not a valid 'No' character.
	RET
	.8080
#endasm
    }


/*--------------------------------------------------------------*/
/* islower: returns true if its argument is a lower case letter */
islower(c)
    char c;
    { return ( 'a' <= c && c <= 'z' ); }

/*-------------------------------------------------------------*/
/* isdrive_ch: returns true if its argument is a valid logical */
/* drive character */
isdrive_ch(c)
    char c;
    { return ( 'A' <= c && c <= 'Z' ); }


/*-------------------------------------------------------------------------*/
/* is_power_of_two: returns true if its argument is a power of two or zero */
is_power_of_two(num)
    int num;
    { return ( (num & (-num)) == num); }

/*--------------------------------------------*/
/* upper: converts its argument to upper case */
upper(c)
    char c;
    { return (islower(c) ? (c - 'a' + 'A') : c); }

/*-------------------------------------------------------------------*/
/* in_range: returns true if 'num' is in the range 'lower'..'higher' */
inrange(num, lower, higher)
    int num, lower, higher;
    { return ( (lower <= num) && (num <= higher) ); }

/* isdirectory: returns true if the attributes indicate the entry is a dir */
isdirectory(attributes)
    int attributes;
    { return(attributes & MASK_SUB_DIR); }

/* isfile: returns true if the attributes indicate the entry is a file */
isfile(attributes)
    int attributes;
    { return(! (attributes & MASK_SUB_DIR)); }

/* ishidden: returns true if the attributes indicate the entry is hidden */
ishidden(attributes)
    int attributes;
    { return(attributes & MASK_HIDDEN); }

/* ishidden: returns true if the attributes indicate the entry is system */
issystem(attributes)
    int attributes;
    { return(attributes & MASK_SYSTEM); }

/* isreadonly: returns true if the attributes indicate the entry is readonly */
isreadonly(attributes)
    int attributes;
    { return(attributes & MASK_R_ONLY); }

/* isdevice: returns true if the attributes indicate the entry is a device. */
isdevice(attributes)
    int attributes;
    { return(attributes & MASK_DEVICE); }




/*****************************************************************************/
/*                                                                           */
/*                  Operating System Interface Module                        */
/*                                                                           */
/*       This module supports the following functions:                       */
/*            get_high_mem()                                                 */
/*                                                                           */
/*****************************************************************************/


check_ver()
{
#asm
	.z80
	ld      c,_DOSVER##
	call    MSX_DOS
	ld      hl,0
	ret     nz
	ld      a,1
	cp      b
	ret     nc
	cp      d
	ret     nc
	dec     hl
	ret
	.8080
#endasm
}


/*---------------------------------------------------------------------------*/
/* set_abort_routine:  sets  up  the  address  of a user abort routine. The  */
/* abort routine itself is also defined here.                                */

set_abort_routine()
    {
#asm
	.Z80
	LD      DE,ABORT
	LD      C,_DEFAB##
	CALL    MSX_DOS
	RET


ABORT:  push    af                      ;Save the error code for return
	.8080
#endasm
    if (file_not_ensured)
	{
	reg_bc = file_not_ensured;      /* In case of recursion. */
	file_not_ensured = FALSE;
	delete (reg_bc);
	}
    restore_ver_flag (verify_flag);
#asm
	.Z80
	pop     af                      ;Return error to the system
	ret
	.8080
#endasm
    }

/*---------------------------------------------------------------------------*/
/* MYSTOP is jumped to by many of the assembler routines when a fatal error  */
/* occurs.  It simply terminates the program with that error code and ABORT  */
/* routine above will then be called to tidy up.                             */

_mystop_routine()
{
#asm
	.Z80
MYSTOP: LD      B,A
	LD      C,_TERM##
	CALL    MSX_DOS
	JP      0
	.8080
#endasm
}

/*----------------------------------------------------------------------*/
/* set_verify_flag: sets the system verify flag, so that all writes are */
/* verified.                                                            */
set_verify_flag()
    {
#asm
	.Z80
	LD      C,_VERIFY##
	LD      E,0FFh
	CALL    MSX_DOS
	RET
	.8080
#endasm
    }

/*-------------------------------------------------------------------------*/
/* get_ver_flag: finds out the current state of the system verify flag, so */
/* that it may be restored to its original value at the end of the XCOPY.  */
get_ver_flag()
    {
#asm
	.Z80
	LD      C,_GETVFY##
	CALL    MSX_DOS
	LD      L,B                     ;Return it in HL.
	LD      H,0
	RET
	.8080
#endasm
    }

/*----------------------------------------------------------------------*/
/* restore_ver_flag: restores the verify flag to it's original setting. */
restore_ver_flag (value)
    int value;
    {
#asm
	.Z80
	POP     HL
	POP     DE                      ;Original value.
	PUSH    DE
	PUSH    HL
	LD      C,_VERIFY##
	CALL    MSX_DOS
	RET
	.8080
#endasm
    }

/*-------------------------------------------------------*/
/* get_high_mem: returns the highest free byte in memory */
get_high_mem()
    {
#asm
	.Z80
	LD      HL,(6)                  ;load top of memory
	DEC     HL                      ; decrement it for caution
	RET                             ; return it
	.8080
#endasm
    }

/****************************************************************************/
/*                      D I S C   I N T E R F A C E                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* do_open: this is passed an fib and an address for a file handle, and has */
/* the job of opening the file.                                             */
do_open(fib, handle)
    char *fib;
    int *handle;
    {
#asm
	.Z80
	POP     HL                      ;Return address.
	POP     BC                      ;Address of where to store handle.
	POP     DE                      ;Fib.
	PUSH    DE
	PUSH    BC
	PUSH    HL
	LD      C,_OPEN##
	XOR     A                       ;Status = read/write.
	CALL    MSX_DOS
	POP     DE                      ;Return address.
	POP     HL
	PUSH    HL
	PUSH    DE
	LD      (HL),B                  ;Store file handle.
	INC     HL
	LD      (HL),0
	LD      L,A                     ;Store any error.
	LD      H,0
	LD      (open_err##),HL
	RET
	.8080
#endasm
    }

/*-----------------------------------------------------------------*/
/* do_flush : Flushes out all disk buffers in order to guarantee   */
/*    that access is not inadvertantly gained to an inappropiate   */
/*    drive just because a buffer filled.                  GMA     */
/*-----------------------------------------------------------------*/

do_flush()
{
#asm
	.Z80
	LD      B,0FFh          ;Flush all Drives
	LD      C,_FLUSH##      ;Flush Buffers to Disc
	LD      D,0             ;Flush but do not invalidate
	CALL    MSX_DOS
	RET
	.8080
#endasm
}

/*-----------------------------------------------------------------*/
/* ensure: does an ensure call, and terminates if an error occurs. */
ensure (handle)
    int handle;
    {
#asm
	.z80
	LD      C,_ENSURE##
CL_ENT: POP     HL                      ;Return address.
	POP     DE                      ;Handle.
	PUSH    DE
	PUSH    HL
	LD      B,E
	CALL    MSX_DOS
	JP      NZ,MYSTOP
	RET
	.8080
#endasm
    }

/*-------------------------------------------------------------*/
/* close: does a close call and terminates if an error occurs. */
close(handle)
    int handle;
    {
#asm
	.Z80
	LD      C,_CLOSE##
	JP      CL_ENT                  ;Do the same as for ensure.
	.8080
#endasm
    }

/*-------------------------------------------------------*/
/* fork: carries out an MSX-DOS fork to a child process. */
fork (process_id)
    int *process_id;
    {
#asm
	.Z80
	LD      C,_FORK##
	CALL    MSX_DOS
	JP      NZ,MYSTOP
	POP     DE                      ;Return address.
	POP     HL
	PUSH    HL
	PUSH    DE
	LD      (HL),B                  ;Store process id.
	INC     HL
	LD      (HL),0
	RET
	.8080
#endasm
    }

/*----------------------------------------------------------*/
/* join: does an MSX-DOS call to rejoin the parent process. */
join (process_id)
    int process_id;
    {
#asm
	.Z80
	POP     HL                      ;Return address.
	POP     DE                      ;Process id number.
	LD      B,E
	PUSH    DE
	PUSH    HL
	LD      C,_JOIN##
	CALL    MSX_DOS
	JP      NZ,MYSTOP
	RET
	.8080
#endasm
    }



/*-------------------------------------------------------------------------*/
/* first: this performs a search for first with either the file info block */
/* or path passed.                                                         */
first(current, filename, next_fib, attr)
    char *current, *filename, *next_fib;
    int attr;
    {
#asm
	.Z80
	POP     IY                      ;Return address
	POP     BC                      ;C := attributes
	POP     IX                      ;IX -> new FIB
	POP     HL                      ;HL -> filename
	POP     DE                      ;DE -> current FIB or ASCIIZ string
	PUSH    DE
	PUSH    HL
	PUSH    IX
	PUSH    BC
	PUSH    IY
	LD      B,C                     ;B := attributes
	LD      C,_FFIRST##
	CALL    MSX_DOS
	LD      L,A                     ;Return any error code.
	LD      H,0
	RET
	.8080
#endasm
    }

/*-----------------------------------------------------------------------*/
/* f_new: performs a find new entry, with either the file info block, or */
/* path string passed.  If the filename passed is ambiguous, a template  */
/* is required to be set up the new fib.                                 */
find_new(fib, filename, next_fib, attr)
    char *fib, *filename, *next_fib;
    int attr;
    {
#asm
	.Z80
	POP     IY                      ;Return address
	POP     BC                      ;C := attributes
	POP     IX                      ;IX -> new FIB
	POP     HL                      ;HL -> filename
	POP     DE                      ;DE -> current FIB or ASCIIZ string
	PUSH    DE
	PUSH    HL
	PUSH    IX
	PUSH    BC
	PUSH    IY
	LD      B,C                     ;B := attributes
	LD      C,_FNEW##
	CALL    MSX_DOS
	LD      L,A                     ;Return any error code.
	LD      H,0
	RET
	.8080
#endasm
    }

/*-------------------------------------------------------------*/
/* next: this performs a search for next with the fib provided */
next(fib)
    char *fib;
    {
#asm
	.Z80
	POP     HL                      ;Return address.
	POP     IX                      ;FIB address.
	PUSH    IX
	PUSH    HL
	LD      C,_FNEXT##
	CALL    MSX_DOS
	LD      L,A                     ;Return any error.
	LD      H,0
	RET
	.8080
#endasm
    }

/*---------------------------------------------------------------------------*/
/* try_delete: is passed a fib, to a directory.  Its job is to try to delete */
/* this.  This means that if it is empty, it will be deleted, otherwise an   */
/* error will be returned.  This prevents XCOPY creating empty sub-dirs.     */
try_delete(fib)
    char *fib;
    {
#asm
	.Z80
	POP     HL                      ;Return address.
	POP     DE                      ;Address of fib.
	PUSH    DE                      ;Restore stack.
	PUSH    HL
	LD      C,_DELETE##
	CALL    MSX_DOS                 ;Go and try.
	CP      .DIRNE##                ;Directory not empty error ??]
	RET     Z                       ;Yes means files have been copied there
	OR      A
	JP      NZ,MYSTOP               ;Any other error is fatal.
	RET
	.8080
#endasm
    }

/*------------------------------------------------------------------*/
/* delete: will delete the file corresponding to the handle passed. */
delete (handle)
    int handle;
    {
#asm
	.Z80
	POP     HL                      ;Return address.
	POP     BC                      ;File handle.
	PUSH    BC
	PUSH    HL
	LD      B,C                     ;Load File Handle Number
	LD      C,_HDELETE##
	CALL    MSX_DOS
	RET                             ;Ignore any errors !!!
	.8080
#endasm
    }

/*--------------------------------------------------------------------*/
/* set_time: sets the date and time on the target fib passed,         */
/* to the values from the source file info block passed.        GMA   */
int set_time(src_fib, dst_handle)
char *src_fib;
int   dst_handle;
{
#asm
	.Z80
	POP     HL      ;Pull off Return Address
	POP     BC      ;Destination File Handle
	POP     IY      ;Address of Source FIB
	PUSH    IY      ;Restore Stack ready for return
	PUSH    BC
	PUSH    HL
	LD      D,(IY+time_high_fib)    ;High Byte of Time
	LD      E,(IY+time_low_fib)     ;Low Byte of Time
	PUSH    DE                      ;Stupid Z80 Method
	POP     IX                      ;to load IX with time
	LD      H,(IY+date_high_fib)    ;High Byte of Date
	LD      L,(IY+date_low_fib)     ;Low Byte of Date
	LD      A,1                     ;Set Time A=1
	LD      B,C                     ;Put the Handle in B
	LD      C,_HFTIME##
	CALL    MSX_DOS                 ;Do the Call
	LD      H,0
	LD      L,A
	RET
	.8080
#endasm
}

/*---------------------------------------------------------------------------*/
/* do_read: performs an MSX-DOS call to read the disc.  If an error is found */
/* that isn't an EOF, then the program is exited.  The routine is passed the */
/* number of bytes to read, which will be the full length of the buffer, and */
/* will return to the calling routine the number of bytes actually read.  In */
/* this way, the wnd of file will be recognised when 0 bytes are read.       */
do_read (buffer, length, handle)
    unsigned length;
    int handle;
    char *buffer;
    {
#asm
	.Z80
	POP     IX                      ;Return address.
	POP     BC                      ;File handle.
	POP     HL                      ;Number of bytes to read.
	POP     DE                      ;Address to read to.
	PUSH    DE
	PUSH    HL
	PUSH    BC
	PUSH    IX
	LD      B,C                     ;Put file handle into B.
	LD      C,_READ##
	CALL    MSX_DOS
	CP      .EOF##
	RET     Z                       ;With number read in HL.
	OR      A
	JP      NZ,MYSTOP
	RET
	.8080
#endasm
    }

/*---------------------------------------------------------------------------*/
/* do_write: is passed the address of a buffer, the number of bytes to write */
/* and the file handle to write them to.   If the number of bytes passed  is */
/* zero,  then  no writing will be done.   If an error is encountered whilst */
/* doing the write, the program will be exited.                              */
do_write (buffer, length, handle)
    int handle;
    unsigned length;
    char *buffer;
    {
#asm
	.Z80
	POP     IX                      ;Return address.
	POP     BC                      ;File handle into C.
	POP     HL                      ;Number of bytes to write.
	POP     DE                      ;Address of data.
	PUSH    DE
	PUSH    HL
	PUSH    BC
	PUSH    IX
	LD      A,H                     ;Check if trying to write nothing.
	OR      L
	RET     Z                       ;And return if so.
	LD      B,C                     ;File handle.
	LD      C,_WRITE##
	CALL    MSX_DOS
	JP      NZ,MYSTOP
	RET
	.8080
#endasm
    }





/*****************************************************************************/
/*                                                                           */
/*                I/O primitives for MSX-DOS utility programs                */
/*                                                                           */
/*       This module supports the following functions:                       */
/*            get_char()                   put_number(number)                */
/*            put_char(ch)                 put_unsigned(unsigned_num)        */
/*            put_spaces(num)              put_mes(eng_message,ger_message)  */
/*            put_string(string)           newline()                         */
/*            put_hex(16_bit_number)       put_hbyte(8_bit_number)           */
/*                                                                           */
/*****************************************************************************/

/*------------------------------*/
/* clr_in:  clears input buffer */
clr_in()
    {
#asm
	.z80
CLINLP: LD      C,_CONST##
	CALL    MSX_DOS
	OR      A
	RET     Z
	LD      C,_INNOE##
	CALL    MSX_DOS
	JR      CLINLP
	.8080
#endasm
    }


/*-----------------------------*/
/* get_char: input a character */
get_char()
    {
#asm
	.z80
	LD      C,_INNOE##
	CALL    MSX_DOS                 ;Get a single character
	PUSH    AF
	LD      E,A
	LD      C,_CONOUT##
	CP      " "                     ;Echo it unless it is a control
	CALL    NC,MSX_DOS              ; character
	POP     AF
	LD      L,A
	LD      H,0
	RET
	.8080
#endasm
    }

/*-------------------------------------------*/
/* putchar: output a character to the screen */
putchar(c)
    int c;
    {
#asm
	.z80
	POP     HL                      ;pop the return address
	POP     DE                      ; and the character into DE
	PUSH    DE                      ;restore the stack
	PUSH    HL

	LD      C,_CONOUT##
	CALL    MSX_DOS
	RET
	.8080
#endasm
    }

/*--------------------------------------------------*/
/* put_char: character output, converts \n to CR/LF */
put_char(c)
    int c;
    { putchar(c); }

/*-----------------------*/
/* newline: writes CR/LF */
newline()
    { put_char('\r'); put_char('\n'); }

/*---------------------------------------------*/
/* put_string: writes a c-string to the screen */
put_string(s)
    char *s;
    { while (*s) put_char(*s++); }

/*------------------------------------*/
/* put_spaces: print out 'num' spaces */
put_spaces(num)
    int num;
    { while (num--) put_char(' '); }

/*--------------------------------------------------------------------------*/
/* put_mes: prints the text corresponding to the numerical value that is    */
/* passed to it.  All the text is stored in a file called TEXT.MAC so that  */
/* translation into another language is easy.                               */
put_mes(mes_num)
    int mes_num;
    {
    static char *txt_adr;
    txt_adr = address_of(mes_num);
    put_string(txt_adr);
    }

/* address_of: returns the absolute address in memory of the message which  */
/* corresponds to the number that is passed as a parameter.  The address is */
/* obtained by doing a CALL to GET_MSG_ADR which is a routine in TEXT.MAC.  */
address_of(mes_num)
    int mes_num;
    {
#asm
	.Z80
	POP     HL              ;Return address.
	POP     DE              ;Number of text message.
	PUSH    DE              ;Restore the stack.
	PUSH    HL
	LD      A,E             ;The 8 bit number.
	CALL    GET_MSG_ADR##
	RET                     ;Answer is in HL as required.
	.8080
#endasm
    }

/*--------------------------------------------------------*/
/* put_unsigned: writes an unsigned integer to the screen */
put_unsigned(i)
    unsigned i;
    {
    if (i<10)
	put_char(i+'0');
    else
	{ put_unsigned(i/10);  put_char((i%10)+'0'); };
    }
