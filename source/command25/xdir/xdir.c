/*****************************************************************************/
/*                                                                           */
/*                  MSX-DOS 2 XDIR Utility,  Main program                    */
/*                                                                           */
/*                      Copyright (c) IS Systems 1986                        */
/*                                                                           */
/*****************************************************************************/

#asm
	DSEG
	db      13,10
	db      'MSX-DOS 2 XDIR program',13,10
	db      'Version '
	db	VERSION##+'0', '.', RELEASE##/256+'0', RELEASE##+'0',13,10
	db      'Copyright (c) '
	db	CRYEAR##/1000 MOD 10 +'0'
	db	CRYEAR##/ 100 MOD 10 +'0'
	db	CRYEAR##/  10 MOD 10 +'0'
	db	CRYEAR##      MOD 10 +'0'
	db	' ASCII Corporation',13,10
	db      13,10,26
#endasm

		     /*   C O N S T A N T S   */


#define FALSE           0
#define TRUE            -1

#define DEBUG           FALSE

/* maximum length of CP/M command line (+1), and its absolute address */
#define MAX_CMD_LEN     129
#define CMD_ADDRESS     0x0080

/* error codes returned by MSX_DOS */
#define FE_DIR_EXISTS   0xCC
#define FE_NO_FILE      0xD7
#define FE_IOPT         0x88
#define FE_IPARM        0x8B
#define FE_NORAM        0xDE
#define FE_WRONG_VER    0x01            /* internally generated */

/* offsets into MSX_DOS FIBs */
#define FIB_LENGTH      64
#define FIB_ATTRIBUTES  14
#define FIB_DRIVE       25
#define FIB_NAME        1
#define FIB_SIZE        21

#define FIL_NAME_LEN    13

/* masks to select bits in the attribute byte */
#define MASK_READ_ONLY  0x01
#define MASK_HIDDEN     0x02
#define MASK_SYSTEM     0x04
#define MASK_SUB_DIR    0x10
#define MASK_DEVICE     0x80



	       /*   G L O B A L   V A R I A B L E S   */


unsigned reg_bc, reg_de;
unsigned st_file;               /* Start of file name in path string         */
unsigned st_wpth;               /* Start of file name in whole path string   */

static int source_drive;        /* logical source drive                      */

static int ambig_flag;          /* true if last item user types is ambiguous */
static int hidden_flag;         /* true if we should list hidden files       */

static unsigned total_sizes [2]; /* accumulates (32 bit) sum of file sizes   */

static char s_path [MAX_CMD_LEN];
static char w_path [MAX_CMD_LEN];
static char f_name [FIL_NAME_LEN+1];
static char null_file[1];

static int f_count;             /* count of files found                      */


	/*    E X T E R N A L     M E S S A G E S    */

extern int M_SIZ1;
extern int M_SIZ2;
extern int M_IN;
extern int M_KIN;
extern int M_FIL1;
extern int M_FIL2;
extern int M_KFREE;
extern int M_VOL1;
extern int M_VOL2;
extern int M_VOL3;
extern int M_XDIR;
extern int M_WVER;

#asm
	.Z80
MSX_DOS EQU     5
;
	.8080
#endasm

main()
    {
    static char out_buffer[9];

    if (!check_ver()) error(FE_WRONG_VER);

    null_file[0] = '\0';

    hidden_flag = FALSE;
    f_count = 0;                /* initially no files have been found */

    total_sizes[0] = 0;   total_sizes[1] = 0;

    get_path();                 /* find out which logical drives to use */

    xdir(0, s_path);

/*    put_char(' '); */
    if ((total_sizes[0] < 1024) && (total_sizes[1] == 0))
	{
	put_unsigned(total_sizes[0]);
	if (total_sizes[0] != 1) put_mes(M_SIZ2);       /* "bytes" */
	     else put_mes(M_SIZ1);                      /* "byte"  */
	put_mes(M_IN);                                  /* " in "  */
	}
    else
	{
	shift32_right(total_sizes,10);
	put_32bits(0,total_sizes,out_buffer);
	put_string(out_buffer);
	put_mes(M_KIN);                                 /* "K in " */
	}

    put_unsigned(f_count);
    if (f_count != 1) put_mes(M_FIL2);                  /* "files" */
	else put_mes(M_FIL1);                           /* "file"  */
/*  put_string("   ");  */

    put_unsigned(get_kbytes_free(source_drive));
    put_mes(M_KFREE);                                   /* "K free\r\n" */
    }




/*****************************************************************************/
/*                                                                           */
/*                           Get Drives Module                               */
/*                                                                           */
/*****************************************************************************/
/*                                 */
/*     - get_path                  */
/*         - parse_flags           */
/*                                 */
/***********************************/


/* parse_flags: this routine parses the C string pointed to by 'ptr' as a    */
/* set of flags of the form "/d/g /h". The flags may be separated by blanks  */
/* however the first character pointed to must be non-blank                  */
parse_flags(ptr)
    char *ptr;
    {
    while (*ptr)
	{
	if (*ptr != '/') error(FE_IOPT);
	switch (upper(*(++ptr)))
	    {
	    case 0  : error(FE_IOPT);
	    case 'H': hidden_flag = TRUE;  break;
	    default : error(FE_IOPT);
	    }
	while (*(++ptr) == ' ') ;
	}
    }

/* get_path: finds out the path that is required to start the XDIR command. */
/* It does this by parsing the command line, and extracts any file that  it */
/* has  to look for.   It also parses any flags that may have been typed in */
/* by the user.                                                             */
get_path()
    {
    static char vol_id[12];
    static char cmd_line[MAX_CMD_LEN];
    char *ptr, *cmd, *path;
    int cmd_length, i, err_flag;
#if DEBUG
    int j;
#endif
    unsigned s_flags;

#if DEBUG
    put_string ("Entering get_path");
    newline();
#endif

    /* copy the command line from the CP/M area (at 0x80) to */
    /* a C string in the array 'cmd_line' */
    ptr = CMD_ADDRESS;
    cmd = cmd_line;
    cmd_length = (*ptr++) & 255;
    for (i=0; i<cmd_length; i++,ptr++,cmd++)
	if (*ptr) *cmd = *ptr; else *cmd = ' ';

    *cmd = 0;           /* terminate the copy with a null */

    cmd = cmd_line;                     /* point 'cmd' to the command line */
    while (*cmd == ' ') cmd++;          /* strip leading blanks */

    if (*cmd == '/') { parse_flags(cmd);  *cmd = 0; }

    ptr = cmd;
    path = s_path;
    do *path++ = *ptr; while (*ptr++);

#if DEBUG
    put_string ("Doing MSX_DOS call PARSE_PATH"); newline();
    put_string ("  String to pass = "); put_string (s_path); newline();
#endif

    if (err_flag = path_parse (0,s_path)) error(err_flag);

    s_flags = reg_bc/256;               /* Get parse flags   */
    cmd = st_file;                      /* Start of filename */

    for (i=0; i<FIL_NAME_LEN+1; i++) {f_name[i] = 0;}
    for (i=0; i<(reg_de-st_file); i++) {f_name[i] = *cmd++;}
    ptr = cmd;                          /* Temporary store   */

#if DEBUG
put_string ("Path: "); put_string (s_path); newline();
put_string ("Filename: "); put_string (f_name); newline();
#endif

    while (*cmd == ' ') cmd++;

    if (*cmd == '/') parse_flags(cmd);
	else if (*cmd) error(FE_IPARM);

    *ptr = 0;           /* Termainate filename with null */

#if DEBUG
put_string ("Path to search: "); put_string (s_path); newline();
put_string ("Filename: "); put_string (f_name); newline();
#endif

    if ((s_flags & 0x20) || ((s_flags & 0x18) == 0)) ambig_flag = TRUE;
	else ambig_flag = FALSE;

    put_mes(M_VOL1);                            /* "Volume in drive " */

    get_vol_name(vol_id,s_path,&source_drive);

    put_char(source_drive - 1 + 'A');

    if (*vol_id)
	{ put_mes(M_VOL2);                      /* ": is "            */
	  put_string(vol_id); }
    else
	put_mes(M_VOL3);                        /* ": has no name"    */
    newline();
    }




/*****************************************************************************/
/*                                                                           */
/*                             Xdir Module                                   */
/*                                                                           */
/*****************************************************************************/


/* put_file: this function outputs a description of the file in the located  */
/* fib passed. This includes its name, file size and hidden/readonly bits    */
put_file(fib)
    char *fib;
    {
    static char buffer[FIL_NAME_LEN+1+2+8+1], *ptr, *bufptr;
    static int i;

    for (i=0; i<22; i++) buffer[i] = ' ';

    bufptr = buffer;
    ptr = fib + FIB_NAME;                       /* Copy filename. */

    while (*ptr) *bufptr++ = *ptr++;

    bufptr = buffer + FIL_NAME_LEN + 2;

    if (isreadonly(fib[FIB_ATTRIBUTES])) *bufptr-- = 'r';
    if (ishidden(fib[FIB_ATTRIBUTES])) *bufptr = 'h';

    put_32bits(' ',fib+FIB_SIZE,buffer + FIL_NAME_LEN + 3);
    put_string(buffer);
    newline();

    addon_32bits(total_sizes,fib+FIB_SIZE);
    }


/* first: this performs a search for first with either the File info block */
/* or path passed.                                                         */
first(search,filename,new,attr)
    char *search, *filename, *new;
    int attr;
    {
#asm
	.z80
	POP     IY              ;Return address
	POP     BC              ;C := attributes
	POP     IX              ;IX := new FIB address
	POP     HL              ;HL := filename
	POP     DE              ;DE := ASCIIZ string or FIB
	PUSH    DE
	PUSH    HL
	PUSH    IX
	PUSH    BC
	PUSH    IY
	LD      B,C             ;B := attributes
	LD      C,_FFIRST##
	CALL    MSX_DOS
	LD      L,A             ;Return any error.
	LD      H,0
	RET
	.8080
#endasm
    }

/* next: this performs a search for next with the fib provided.              */
next(new)
    char *new;
    {
#asm
	.z80
	POP     HL              ;Return address.
	POP     IX              ;FIB address.
	PUSH    IX
	PUSH    HL
	LD      C,_FNEXT##
	CALL    MSX_DOS
	LD      L,A             ;Return any error.
	LD      H,0
	RET
	.8080
#endasm
    }

/* get_name: this transfers the file name pointed to by ptr and transfers */
/* it to the output buffer 'buffer'.                                      */
get_fname(ptr,buffer)
    char *ptr, *buffer;
    {
    int i;
    for (i=0; i<12; i++) *buffer++ = *ptr++;
    *buffer = 0;
    }

/* modify_path: this function resolves the path in w_path with respect to    */
/* file name 'fname'. If fname is ".", the path is unmodified. If it is ".." */
/* then the path is reduced by one level. Otherwise fname is added on as     */
/* eg:  w_path before       fname      w_path after                          */
/*        \one\two            .          \one\two                            */
/*        \one\two            ..         \one                                */
/*        \one                ..         \                                   */
/*        \                   fred       \fred                               */
/*        \fred               joe        \fred\joe                           */
modify_path(fname)
    char *fname;
    {
    static char *ptr;

    ptr = w_path;
    while (*ptr) ptr++;

    if (str_eq(fname,".."))
	{
	if (! ((w_path[0] == '\\') && (w_path[1] == 0)))
	    {
	    ptr = ptr - 1;
	    while (*ptr != '\\') ptr--;
	    *ptr = 0;
	    if (w_path[0] == 0) { w_path[0] = '\\';  w_path[1] = 0; }
	    }
	}
    else if (! str_eq(fname,"."))
	{
	if (*(ptr-1) != '\\') *ptr++ = '\\';
	copy_str(fname,ptr);
	}
    }


/* xdir: this is the heart of the xdir program. If performs a recursive      */
/* descent through the directory structure. In each directory it:            */
/*  a) searches for files that match the global pattern in 'search_fib'      */
/*  b) calls xdir on all subdirectories (if any) in the directory            */
/* there is a fiddle for the 1st search (in the root directory). If the path */
/* given on the command line was unambiguous and it matches with a directory */
/* then an xdir is done IN that directory. This means that (if DIR1 is a dir)*/
/* going "xdir DIR1" gives "Xdirectory of A:\DIR1..." rather than            */
/* "Xdirectory of a:\, \DIR1, ...."                                          */

xdir(level, search_fib)
    unsigned level;
    char *search_fib;
    {
    static int err_flag, err2_flag, attributes, i;
    static char fname [20];
    static char *ptr;
    auto char *next_fib;

    next_fib = allocate(FIB_LENGTH);
    if (next_fib == -1) error(FE_NORAM);

    err_flag = first(search_fib,f_name,next_fib,0x16);

#if DEBUG
put_string("Xdir at level "); put_unsigned(level); newline();
put_string("  Allocated new Fib at "); put_hex(next_fib); newline();
put_string("  Pattern to match (search Fib) = ");
if ((search_fib[0]&255) == 255)
	{newline(); put_string(search_fib + FIB_NAME);
	put_string("Filename: "); put_string(f_name); }
	else {put_string("Path: "); put_string(search_fib); }
newline();
#endif

    if (level == 0)
	{
	if (err_flag && (err_flag != FE_NO_FILE)) error(err_flag);

	w_path[0] = '\\';
	if (err2_flag = get_w_path(w_path+1)) error (err2_flag);
	ptr = st_wpth;

	if (ptr == (w_path+1)) *ptr = '\0'; else *(ptr-1)='\0';

	attributes = next_fib[FIB_ATTRIBUTES];

#if DEBUG
put_string("Final path: "); put_string(w_path); newline();
put_string("Attributes: "); put_hex(attributes); newline();
put_string("ambig_flag: "); put_hex(ambig_flag); newline();
#endif

	if (isdirectory(attributes) && (! ambig_flag) && (err_flag == 0))
	    {
	    get_fname(next_fib+FIB_NAME,fname);
	    modify_path(fname);

	    search_fib = allocate(FIB_LENGTH);
	    if (search_fib == -1) error(FE_NORAM);
	    copy_fib(next_fib,search_fib);

	    f_name[0] = 0;                      /* Search for *.* */
	    err_flag = first(search_fib,f_name,next_fib,0x16);
	    }

	put_mes(M_XDIR);                        /* "X-Directory of " */
	put_char(source_drive - 1 + 'A');
	put_char(':');
	put_string(w_path);
	newline();   newline();

	ptr = st_file;
	*ptr = 0;               /* Remove filename from path in level 0 */
	}

    while (err_flag == 0)
	{
	attributes = next_fib[FIB_ATTRIBUTES];
	if (isfile(attributes) && (hidden_flag || (! ishidden(attributes)))
	    && (! issystem(attributes)) && (! isdevice(attributes)))
	    {
	    /* we have found a matching file to display */
	    put_spaces(level*3);
	    put_file(next_fib);
	    f_count++;  /* bump the global count of files found */
	    }
	/* search for next */
	err_flag = next(next_fib);
	}

    if (err_flag != FE_NO_FILE) error(err_flag);

    /* Now look for any directories to search through. */
    err_flag = first(search_fib,null_file,next_fib,0x16);

    while (err_flag == 0)
	{
	attributes = next_fib[FIB_ATTRIBUTES];
	if (isdirectory(attributes)
	    && (next_fib[FIB_NAME] != '.')
	    && (hidden_flag || (! ishidden(attributes))))
	    {
	    put_spaces(level*3);
	    get_fname(next_fib + FIB_NAME,fname);
	    modify_path(fname);
	    put_string(w_path);
	    if (ishidden(attributes)) put_string("        h");
	    newline();

	    /* recurse to xdir the files in this directory */
	    xdir(level+1,next_fib);
	    modify_path("..");
	    }
	/* search for next */
	err_flag = next(next_fib);
	}
    if (err_flag != FE_NO_FILE) error(err_flag);

    deallocate(next_fib);
    }


/*****************************************************************************/
/*                                                                           */
/*                    32 Bit Arithmetic Module                               */
/*                                                                           */
/*****************************************************************************/
/*                                 */
/*      - shift32_right            */
/*      - put_32bits               */
/*      - addon_32bits             */
/*                                 */
/***********************************/


/* shift32_right: this function shifts right the 32 bit number pointed to by */
/* its first argument, by the number of bits given by the second argument    */
shift32_right(num_ptr,bits)
    char *num_ptr;
    int bits;
    {
#asm
		.z80
		pop     ix              ;Get the return address
		pop     bc              ; the number of bits to shift (in C)
		pop     hl              ; the pointer to the number in HL
		push    hl
		push    bc
		push    ix

		ld      de,3            ;point to the MSB of the number
		add     hl,de

shift_loop:     srl     (hl)            ;shift the top byte down one bit
		ld      b,e             ;load B with 3
inner_loop:     dec     hl              ;The loop round shifting the bottom
		rr      (hl)            ; three bytes
		djnz    inner_loop

		add     hl,de           ;point to the MSB again, by adding 3
		dec     c               ;Have we finished?
		jr      nz,shift_loop

		ld      hl,0            ;return zero
		ret
		.8080
#endasm
    }


/* put_32bits: This routine prints a 32 bit number pointed to by 'dataptr'.  */
/* The number is printed to 'buffer'. It is derived by simple division using */
/* 32-bit power of 10 table.  The number will be printed in a field width of */
/* 8 characters. */
put_32bits(leader,dataptr,buffer)
    char leader, *dataptr, *buffer;
    {
#asm
		.z80
		pop     ix              ;Get the return address
		pop     bc              ; pointer to the output buffer
		pop     hl              ; pointer to the 32 bit number
		pop     de              ; character for leading zeros
		push    de
		push    hl
		push    bc
		push    ix

		ld      a,e
		ld      (ZERO_CHAR),a           ;Store lead character

		push    bc
		ld      de,NUMBER
		ld      bc,4
		ldir

		ld      hl,POWER_TAB            ;HL -> power of 10 table
		ld      de,NUMBER               ;DE -> number to be printed

wr_32_loop:     ld      a,(hl)                  ;If we are at last entry in
		dec     a                       ; the table then force the
		jr      nz,not_last_char        ; lead character to be "0"
		ld      a,"0"                   ; to ensure that zero gets
		ld      (ZERO_CHAR),a           ; printed.
not_last_char:
		ld      c,0                     ;Divide by 32 bit subtraction
subtract_loop:  call    SUB_32                  ; and add last one back on to
		inc     c                       ; keep result +ve.
		jr      nc,subtract_loop
		call    ADD_32

		dec     c                       ;If the digit is zero then
		ld      a,(ZERO_CHAR)           ; use the lead character.
		jr      z,use_lead_char         ;If non-zero then set lead
		ld      a,"0"                   ; character to "0" for future
		ld      (ZERO_CHAR),a           ; zeroes and convert digit
		add     a,c                     ; to ASCII.
use_lead_char:  or      a                       ;Print the character unless
		jr      z,no_print              ; it is null.
		ex      (sp),hl
		ld      (hl),a
		inc     hl
		ex      (sp),hl

no_print:       ld      a,(hl)                  ;Test whether last entry in
		dec     a                       ; table yet.
		inc     hl
		inc     hl
		inc     hl                      ;Point HL at next entry
		inc     hl
		jr      nz,wr_32_loop           ;Loop 'til end of table

		pop     hl                      ;return new buffer ptr
		ld      (hl),0                  ;(terminate string with 0)
		ret

POWER_TAB:      dw       9680h,   98h           ;   10,000,000
		dw       4240h,   0Fh           ;    1,000,000
		dw       86A0h,    1h           ;      100,000
		dw       2710h,    0h           ;       10,000
		dw        3E8h,    0h           ;        1,000
		dw         64h,    0h           ;          100
		dw         0Ah,    0h           ;           10
		dw          1h,    0h           ;            1

NUMBER:         dw      0,0             ;Buffer for number calculation
ZERO_CHAR:      db      0               ;Character for leading zeroes

;
;    These two routines are almost identical.  They simply add or subtract the
; 32 bit number pointed to by HL to the 32 bit number pointed to by DE.  All
; registers are preserved except for AF, and the carry flag will be set
; correctly for the result.  The number at (HL) is not modified.
;
ADD_32:         push    hl
		push    de
		push    bc
		ld      b,4
		or      a
add_32_loop:    ld      a,(de)
		adc     a,(hl)
		ld      (de),a
		inc     hl
		inc     de
		djnz    add_32_loop
		pop     bc
		pop     de
		pop     hl
		ret

SUB_32:         push    hl
		push    de
		push    bc
		ld      b,4
		or      a
sub_32_loop:    ld      a,(de)
		sbc     a,(hl)
		ld      (de),a
		inc     hl
		inc     de
		djnz    sub_32_loop
		pop     bc
		pop     de
		pop     hl
		ret
		.8080
#endasm
    }


/* addon_32bits: add the number pointed to by 'addition' on to the number */
/* pointed to by 'accum' */
addon_32bits(accum,addition)
    char *accum, *addition;
    {
#asm
		.z80
		pop     ix
		pop     hl
		pop     de
		push    de
		push    hl
		push    ix

		call    ADD_32          ;call ADD_32 to actually do the work

		ld      hl,0            ;return 0
		ret
		.8080
#endasm
    }



/*****************************************************************************/
/*                                                                           */
/*                             Bits Module                                   */
/*                                                                           */
/*****************************************************************************/
/*                                 */
/***********************************/


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


/* str_eq: returns true if the strings 'a' and 'b' are identical */
str_eq(a,b)
    char *a, *b;
    {
    while (*a) if (*a++ != *b++) return FALSE;
    return (*b ? FALSE : TRUE);
    }


/* copy_str: copies string 'a' to 'b' */
copy_str(a,b)
    char *a, *b;
    { do *b++ = *a; while (*a++); }


/* get_kbytes_free: returns the amount of free space on the given logical */
/* drive, in kbytes */
get_kbytes_free(drive)
    int drive;
    {
    drive;
#asm
		.z80
		ld      e,l             ;Get the drive number to e
		ld      c,_ALLOC##      ;Do MSX-DOS "get allocation info"
		call    MSX_DOS         ;Returns sect/cluster in A, and number
free_loop:      srl     a               ; of free clusters in HL
		jr      c,finito        ;A=power of two, so shift A to right
		rl      l               ; and HL to the left, until we get a
		rl      h               ; carry from A
		jr      free_loop
finito:         srl     h               ;shift to right once, since a sector
		rr      l               ; 512 bytes (== half a k)
		ret
		.8080
#endasm
    }


/* get_w_path: puts the 'whole path' (as returned by get whole path string) */
/* into the buffer given */
get_w_path(buffer)
    char *buffer;
    {
#asm
	.Z80
	POP     IX              ;return address
	POP     DE              ; buffer address
	PUSH    DE
	PUSH    IX
	LD      C,_WPATH##
	CALL    MSX_DOS         ;Actually go and get it.
	LD      (st_wpth),hl    ;Store pointer to last item
	LD      L,A
	LD      H,0
	RET
	.8080
#endasm
    }


/* copy_fib: copies the first fib to the second */
copy_fib(source,dest)
    char *source, *dest;
    {
    static int i;
    for (i=0; i<FIB_LENGTH; i++) *dest++ = *source++;
    }


/* path_parse:  This  routine  is  passed a string which is then passed to */
/* MSX_DOS parse path routine.  The registers that are returned are stored */
/* so that the C program may access them.                                  */
path_parse(vol_flag,path)
    int vol_flag;
    char *path;
    {
#asm
	.Z80
	POP     HL                      ;Return address.
	POP     DE                      ;String.
	POP     BC                      ;C = volume flag.
	PUSH    BC                      ;Restore stack.
	PUSH    DE
	PUSH    HL
	LD      B,C
	LD      C,_PARSE##
	CALL    MSX_DOS
	LD      (st_file),HL
	LD      (reg_de),DE
	LD      (reg_bc),BC
	LD      L,A                     ;Return any error.
	LD      H,0
	RET
	.8080
#endasm
    }

/* get_vol_name:  this function will try to read the volume  name  from */
/* the given path.  It will return both the volume name and the logical */
/* drive.                                                               */

get_vol_name(buffer,path,drive)
    char *buffer, *path;
    int *drive;
    {
    auto char temp_fib [FIB_LENGTH];
    auto char *temp_ptr;
    auto int err_flag;

    temp_ptr = temp_fib;

    err_flag = first(path, null_file, temp_ptr, 8);

    *buffer = '\0';
    *drive = temp_fib [FIB_DRIVE];

    switch (err_flag)
	{
	case 0: get_fname (temp_ptr+FIB_NAME,buffer); return;
	case FE_NO_FILE: return;
	default: error (err_flag);
	}
    }


/*****************************************************************************/
/*                                                                           */
/*                         Error Handling Module                             */
/*                                                                           */
/*****************************************************************************/


/* error: exits the program with an error code */
error(num)
int num;
{
    switch (num)
    {
    case FE_WRONG_VER: put_mes(M_WVER);  /* Wrong version of MSX-DOS */
		       break;
    }
    newline();
    exit(num);
}




/*****************************************************************************/
/*                                                                           */
/*                      Basic Predicates Module                              */
/*                                                                           */
/*****************************************************************************/


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

/* issystem: returns true if the attributes indicate the entry is system */
issystem(attributes)
    int attributes;
    { return(attributes & MASK_SYSTEM); }

/* isreadonly: returns true if the attributes indicate the entry is readonly */
isreadonly(attributes)
    int attributes;
    { return(attributes & MASK_READ_ONLY); }

/* isdevice: returns true if the attributes indicate the entry is a device. */
isdevice(attributes)
    int attributes;
    { return(attributes & MASK_DEVICE); }

/* upper: converts its argument to upper case */
upper(c)
    char c;
    { return (islower(c) ? (c - 'a' + 'A') : c); }

/* islower: returns true if its argument is a lower case letter */
islower(c)
    char c;
    { return ( 'a' <= c && c <= 'z' ); }

#ifneed in_range
/* in_range: returns true if 'num' is in the range 'lower'..'higher' */
inrange(num, lower, higher)
    int num, lower, higher;
    { return ( (lower <= num) && (num <= higher) ); }
#endif


/*****************************************************************************/
/*                                                                           */
/*                 I/O primitives for MSX-DOS utility programs               */
/*                                                                           */
/*       This module supports the following functions:                       */
/*            get_char()                   put_number(number)                */
/*            put_char(ch)                 put_unsigned(unsigned_num)        */
/*            put_spaces(num)              put_mes(eng_message,ger_message)  */
/*            put_string(string)           newline()                         */
/*            put_hex(16_bit_number)       put_hbyte(8_bit_number)           */
/*                                                                           */
/*****************************************************************************/

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

	LD      C,_CONOUT##             ;Write character code.
	CALL    MSX_DOS                 ;Character passed in E.
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

/* put_spaces: prints out 'num' spaces on the screen */
put_spaces(num)
    int num;
    { while (num--) put_char(' '); }


/* put_mes: this is a version of put_string that is passed a message number  */
/* rather than the actual text.  This is so that all the text may be stored  */
/* in a file called TEXT.MAC and assembled separately so that it may easily  */
/* be translated into another language.                                      */
put_mes(mes_num)
    int mes_num;
    {
    static char *text_adr;
    text_adr = get_mes_adr(mes_num);
    put_string(text_adr);
    }

/* get_mes_adr: this is passed the number of the message to be printed and   */
/* calls a routine in TEXT.MAC to find out the absolute address of the text  */
/* and then it returns this so that it may be passed onto put_string for the */
/* actual printing.                                                          */
get_mes_adr(mes_num)
    int mes_num;
    {
#asm
	.Z80
	POP     HL              ;Return address.
	POP     DE              ;Message number.
	PUSH    DE              ;Restore the stack.
	PUSH    HL
	LD      A,E
	CALL    GET_MSG_ADR##
	RET
	.8080
#endasm
    }


/* put_unsigned: writes an unsigned integer to the screen */
put_unsigned(i)
    unsigned i;
    {
    if (i<10)
	put_char(i+'0');
    else
	{ put_unsigned(i/10);  put_char((i%10)+'0'); };
    }


#ifneed put_number
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
#endif

#ifneed put_hex
put_hex(u)
    unsigned u;
    {
    put_hbyte(u/256);
    put_hbyte(u & 0xFF);
    }
#endif

#ifneed put_hbyte
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
