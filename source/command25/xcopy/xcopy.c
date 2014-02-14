/*****************************************************************************/
/*                                                                           */
/*                   MSX-DOS 2 XCOPY Utility, Main Program                   */
/*                                                                           */
/*                     Copyright (c) IS Systems Ltd 1986                     */
/**                                                                         **/
/*****************************************************************************/

#asm
	DSEG
	db      13,10
	db      'MSX-DOS XCOPY program',13,10
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

#define GLOBAL                  /* Need to actually declare globals */
#include xcopy.h

extern char * src_parse();
extern char * dst_parse();

/* variables to detect recursive, infinite copies */

char    tl_cluster, th_cluster;
char    t_drive;

main()
{
    if (!check_ver()) error(FE_WRONG_VER);

    newline();

    empty_flag = FALSE;         /* By default, don't create empty sub-dirs   */
    wait_flag = FALSE;          /* and don't wait at the start               */
    prompt_flag = FALSE;        /* and we don't prompt before each file      */
    subdirectory_flag = FALSE;  /* and don't recurse copy subdirectories     */
    archive_flag = FALSE;       /* and don't just copy archive bit files     */
    update_arc_flag = FALSE;    /* and don't update archive bit              */
    time_flag = FALSE;          /* and use old dates and times               */
    hidden_flag = FALSE;        /* and don't copy hidden files               */

    f_count = 0;                /* initially there are no files copied       */
    null_file[0] = 0;           /* Null file = "*.*".                        */

    file_not_ensured = FALSE;

    verify_flag = get_ver_flag(); /* Save, so it can be restored at end.     */

    set_abort_routine();    /* Set up abort routine to restore system at end */

    get_drives();               /* find out which logical drives to use */

    if (wait_flag) { put_mes(M_WAIT);   /* wait if required */
		     clr_in();
		     get_char();
		     newline();
		     newline();
		   }

    src_fib = src_parse();
    dst_fib = dst_parse();

    xcopy(0, src_fib, dst_fib);

    put_char(' ');
    put_unsigned(f_count);
    if (f_count != 1) put_mes(M_FIL2);                  /* "files"       */
	else put_mes(M_FIL1);                           /* "file"        */
    put_mes(M_COP);                                     /* " copied\r\n" */

}



/*****************************************************************************/
/*                                                                           */
/*                           Get Drives Module                               */
/*                                                                           */
/*****************************************************************************/
/*                                 */
/*     - get_drives                */
/*         - parse_flags           */
/*         - path_parse            */
/*                                 */
/***********************************/

/*---------------------------------------------------------------------------*/
/* parse_flags: this routine parses the C string pointed to by 'ptr' as a    */
/* set of flags of the form "/d/g /h". The flags may be separated by blanks  */
/* however the first character pointed to must be non-blank                  */
parse_flags(ptr)
    char *ptr;
    {
    while (*ptr)
	{
	if (*ptr != '/') error(FE_IPARM);
	switch (upper(*(++ptr)))
	    {
	    case 'S': set_true(&subdirectory_flag); break;
	    case 'H': set_true(&hidden_flag); break;
	    case 'T': set_true(&time_flag); break;
	    case 'E': set_true(&empty_flag); break;
	    case 'W': set_true(&wait_flag); break;
	    case 'P': set_true(&prompt_flag); break;
	    case 'V': set_verify_flag(); break;
	    case 'A': archive_flag = TRUE; break;
	    case 'M': archive_flag = TRUE;
		      set_true(&update_arc_flag);
		      break;
	    case 0  : error(FE_IPARM);
	    default : error(FE_IOPT);
	    }
	while (*(++ptr) == ' ') ;
	}
    }

/*-------------------------------------------------------------------------*/
/* true: returns true, but prints out a warning if the flag is already set */
/* to true.                                                                */
set_true(flag)
    int *flag;
    {
    if (*flag) warning (W_MULT_DEF_OPT);
    *flag = TRUE;
    }

/*---------------------------------------------------------------------------*/
/* get_drives:  finds  which logical drives are to be used as the source and */
/* destination ('source_drive' and 'target_drive'),  by parsing the  command */
/* line and (if neccesary) asking the user. It also parses any flags present */
/* the command line.                                                         */
get_drives()
    {
    static char cmd_line[MAX_CMD_LEN];
    static char *ptr, *cmd, *path;
    static int cmd_length, i;
    static unsigned t_flags, s_flags;

    /* copy the command line from the CP/M area (at 0x80) to */
    /* a C string in the array 'cmd_line' */
    ptr = CMD_ADDRESS;
    cmd = cmd_line;
    cmd_length = (*ptr++) & 255;
    for (i=0; i<cmd_length; i++, ptr++, cmd++)
	if (*ptr) *cmd = *ptr; else *cmd = ' ';
    *cmd = 0;           /* terminate the copy with a null */

    *s_path = 0;        /* make both the source and target path names null */
    *t_path = 0;
    t_flags = 0;        /* zero the target and source flags */
    s_flags = 0;

    cmd = cmd_line;                     /* point 'cmd' to the command line */
    while (*cmd == ' ') cmd++;          /* strip leading blanks */

    s_flags = path_parse(&cmd, s_path, sf_name, &s_st_file);
    if ((s_flags & 0x20) || ((s_flags & 0x18) == 0)) s_ambig_flag = TRUE;

    while (*cmd == ' ') cmd++;

    t_flags = path_parse(&cmd, t_path, tf_name, &t_st_file);
    if ((t_flags & 0x20) || ((t_flags & 0x18) == 0)) t_ambig_flag = TRUE;

    if (*cmd)
	{
	while (*cmd == ' ') cmd++;
	if (*cmd == '/') parse_flags(cmd); else if (*cmd) error(FE_IPARM);
	}
    }

/*-------------------------------------------------------------------------*/
/* path_parse: is passed a path string, which is parsed by  MSX-DOS.   The */
/* final  path is left in path,  terminated by a null, whilst any filename */
/* is copied into the filename array passed.  The parse flags will also be */
/* returned.                                                               */
path_parse (cmd, path, f_name, st_file)
    char *path, *f_name;
    char **cmd, **st_file;
    {
    static int flags, err_flag;
    static char *ptr;

    ptr = *cmd;                 /* Makes code neater !!!! */

    if (err_flag = do_parse(ptr)) error(err_flag);
    *st_file = path - ptr + reg_hl;             /* Start of filename */
		/* Was *st_file = path + (reg_hl - ptr); */

    while (ptr < reg_de) *path++ = *ptr++;      /* Copy path + filename */
    *path = 0;
    ptr = reg_hl;

    while (ptr < reg_de) *f_name++ = *ptr++;    /* Copy filename */
    *f_name = 0;

    *cmd = ptr;

    return (reg_bc/256);
    }


/*****************************************************************************/
/*                                                                           */
/*                              Xcopy Module                                 */
/*                                                                           */
/*****************************************************************************/


/*---------------------------------------------------------------------------*/
/* xcopy: this function is the core of the system.  It is passed either fibs */
/* or path strings to the current source and target directories that are  to */
/* be used.   Firstly, it will copy all the files, and then it will copy any */
/* files from sub-directories, if the relevant flag is set.                  */
/*      To prevent files from being copied onto themselves, or  copying  two */
/* files  to the same destination, file handles are never closed.   For each */
/* sub-directory, a fork is done.  On leaving the sub-directory, a join will */
/* close any open files.  This has not taken on a nice tree structure, since */
/* it would leave too many FABs in memory, and memory could run out.         */
/*    WARNING : This is a total REWRITE by Gavin of the Original Version     */
/*---------------------------------------------------------------------------*/
/* !!%% */

xcopy (level, s_fib, t_fib)
unsigned level;
char *s_fib, *t_fib;
{
    static int err_flag, t_err_flag, err2_flag, attributes, i;
    static int s_handle, t_handle, process_id;
    static char *ptr;
    auto char *s_next_fib, *t_next_fib;
    auto int d_err_flag;        /* Error flag for creating a directory */

    if ((s_next_fib = allocate(FIB_LENGTH)) == -1) error(FE_NORAM);
    if ((t_next_fib = allocate(FIB_LENGTH)) == -1) error(FE_NORAM);

    fork (&process_id);

    err_flag = first(s_fib, sf_name, s_next_fib, 0x16);

    while (err_flag == 0)
    {
	attributes = s_next_fib[FIB_ATTRIBUTES];
	if (   (isfile(attributes))
	    && (hidden_flag || (! ishidden(attributes)))
	    && ((attributes & MASK_ARCHIVE) || (! archive_flag))
	    && (! issystem(attributes)) )
	{
	    /* we have found a matching file to copy */

	    do
	    {
		put_spaces(level*3);
		put_string(s_next_fib + FIB_FILE_NAME);
		tell_attributes(attributes);

		if (prompt_flag)
		{
		    put_mes(M_PROMPT);
		    clr_in();
		    i = get_char();
		    if (no(i)) {newline(); goto nxt_fil;};
		    i = yes(i);
		    if (!i) newline();
		}
		else
		    i = TRUE;
	    } while (!i);

	    copy_data(&process_id, s_next_fib, t_fib, t_next_fib, attributes);

	    newline();
	}
	nxt_fil: err_flag = next(s_next_fib);
    }
    if (err_flag != FE_NO_FILE) error(err_flag);

    join (process_id);          /* Close all the files */

    /* Now look for any directories to search through. */

    if (subdirectory_flag)              /* Only do if /S selected. */
    {
	if (level == 0)
	{
	    *s_st_file = 0;
	    *t_st_file = 0;
	}
	err_flag = first(s_fib, null_file, s_next_fib, 0x16);
	while (err_flag == 0)
	{
	    attributes = s_next_fib[FIB_ATTRIBUTES];
	    if (   isdirectory(attributes)
		&& (s_next_fib[FIB_FILE_NAME] != '.')
		&& (hidden_flag || (! ishidden(attributes)))
		&& (! issystem(attributes))
		&& ((t_drive != log2phy(s_next_fib[FIB_DRIVE]))
		 || (tl_cluster != s_next_fib[FIB_CLUSTER])
		 || (th_cluster != s_next_fib[FIB_CLUSTER+1])))
	    {
		put_spaces(level*3);
/*              modify_path(ws_path, s_next_fib + FIB_FILE_NAME); */
		if (modify_path(ws_path, s_next_fib + FIB_FILE_NAME))
			error(FE_PLONG);
		put_string(ws_path);
		tell_attributes(attributes);

		get_file_name (s_next_fib, t_next_fib);
		d_err_flag = find_new(t_fib, null_file, t_next_fib,
				      attributes&0x7e);
		switch (d_err_flag)
		{
		    case FALSE:
		    case FE_DIR_EXISTS:
			    newline();
			    xcopy(level+1, s_next_fib, t_next_fib);
			    if ((! d_err_flag) && (! empty_flag))
				try_delete(t_next_fib);
			    break;

		    case FE_FILE_EXISTS:        /* Non fatal errors */
		    case FE_IFNM:
		    case FE_FOPEN:
			    put_mes (M_CCSD); break;

		    default:
			    newline(); error(d_err_flag); break;
		}
		modify_path(ws_path, "..");
	    }
	    err_flag = next(s_next_fib);
	}
	if (err_flag != FE_NO_FILE) error(err_flag);
    }

    deallocate (s_next_fib);
    deallocate (t_next_fib);
}


char * src_parse()
{
    char *ptr;
    char *fib_ptr;
    int  error_flag;
    int  error_path;
    int  attr;
    if ((fib_ptr = allocate(FIB_LENGTH)) == -1) error(FE_NORAM);

    error_flag = first(s_path, null_file, fib_ptr, 0x16);

    ws_path[0] = '\\';
    if (error_path = get_w_path(ws_path+1)) error(error_path);

    ptr = reg_hl;
    if (ptr == (ws_path + 1)) *ptr = 0; else *(ptr-1) = 0;

    if (! s_ambig_flag)
    {
	switch(error_flag)
	{
	    case 0 :
		    if (isdirectory(fib_ptr[FIB_ATTRIBUTES]))
		    {
			error_path = modify_path(ws_path,
						 fib_ptr+FIB_FILE_NAME);
			if (error_path) error(FE_PLONG);
			sf_name[0] = 0;
		    }
		    if (isfile(fib_ptr[FIB_ATTRIBUTES]))
		    {
			deallocate(fib_ptr);
			fib_ptr = s_path;
		    }
		    break;

	    case FE_NO_FILE :
		    deallocate(fib_ptr);
		    fib_ptr = s_path;
		    break;
	    default :
		    error(error_flag);
	}
    }
    else
    {
	deallocate(fib_ptr);
	fib_ptr = s_path;
    }
    return (fib_ptr);
}


char * dst_parse()
{
    char *fib_ptr;
    char *ptr, *dptr;
    int  error_flag;
    int  error_path;
    char dirnam[64];
    char tempfib[64];

    if ((fib_ptr = allocate(FIB_LENGTH)) == -1) error(FE_NORAM);

    error_flag = first(t_path,tf_name,fib_ptr,0x16);
    if ((error_flag)&&(error_flag != FE_NO_FILE)) error(error_flag);
    if (isdevice(fib_ptr[FIB_ATTRIBUTES])&&(error_flag == 0)) error(FE_IDEV);

    t_drive = log2phy(fib_ptr[FIB_DRIVE]);      /* JeyS */

    if   (isdirectory(fib_ptr[FIB_ATTRIBUTES])
       && (! t_ambig_flag)
       && (error_flag == 0)) {
	tf_name[0] = 0;                         /* No Renaming */
	tl_cluster = fib_ptr[FIB_CLUSTER];
	th_cluster = fib_ptr[FIB_CLUSTER+1];
    } else
    {
	if (error_flag = do_parse(t_path)) error(error_flag);
	ptr = t_path;
	dptr = dirnam;
	while (ptr < reg_hl)
		*dptr++ = *ptr++;
	*dptr = 0;
	if ((*t_path) && (*(dptr-1) == '\\'))
		*(dptr-1) = 0;
	first(dirnam,tf_name,tempfib,0x16);
	tl_cluster = tempfib[FIB_CLUSTER];
	th_cluster = tempfib[FIB_CLUSTER+1];
	deallocate(fib_ptr);
	fib_ptr = t_path;
    }

    return(fib_ptr);
}



/*--------------------------------------------------------------------------*/
/* Copy_Data : This procedure is responsible for the actual transfer of data*/
/*   between Source and Destination Files. Copy_Data is an extension (11.86)*/
/*   to the original XCOPY which attempts to reduce execution time by       */
/*   careful consideration of disc accesses between drives. A useful side   */
/*   effect is that the XCOPY procedure is much easier to read !    GMA     */
/*--------------------------------------------------------------------------*/

copy_data(process_id,src_file,dst_dir,dst_file,attr)

int  *process_id;               /* Process Id                               */
char *src_file;                 /* Source File FIB                          */
char *dst_dir;                  /* Destination Directory FIB                */
char *dst_file;                 /* Destination File FIB                     */
int  attr;                      /* Required Attributes for Destination File */
{
    static int src_handle;
    static int dst_handle;
    static int error_flag;
    static int err2_flag;
    static unsigned num_read,buf_len;
    static char *buffer;

    buffer = buf_adr(&buf_len);           /* Get our buffer */

    chk_and_open(process_id, src_file, &src_handle);

    get_file_name(src_file, dst_file);
    dst_handle = 0xff;                    /* Not yet created   */
    file_not_ensured = FALSE;             /* No need to Ensure */

    do
    {
	do_flush();
	num_read = do_read(buffer, buf_len, src_handle);

	if (dst_handle == 0xff)
	{
	    error_flag = find_new(dst_dir, tf_name, dst_file, attr&0x7e);
	    if (! error_flag)
	    {
		open(dst_file, &dst_handle);
		file_not_ensured = dst_handle;
	    }
	}

	if (! error_flag) do_write(buffer, num_read, dst_handle);
    }
    while ((num_read == buf_len) && (! error_flag)) ;


    switch (error_flag)
    {
	case FALSE :
		ensure(dst_handle);
		file_not_ensured = FALSE;
						/* On Destination */
		if (attr&0x01)
		    if (err2_flag = set_file_attr(dst_handle, attr|0x20))
			error(err2_flag);

		do_flush();
						/* On Source then Destn */

		if (! time_flag) set_time(src_file, dst_handle);

						/*    On Source   */
		if (update_arc_flag)
		    if (err2_flag = set_file_attr(src_handle, attr&0xDF))
			error(err2_flag);

		f_count ++;
		break;

	case FE_DIR_EXISTS :
		pr_err(M_T_DE,src_handle);
		break;
	case FE_SYS_EXISTS :
		pr_err(M_T_SE,src_handle);
		break;
	case FE_R_ONLY :
		pr_err(M_T_RO,src_handle);
		break;
	case FE_IFNM :
		pr_err(M_T_IN,src_handle);
		break;
	case FE_FOPEN :
		pr_err(M_T_FO,src_handle);
		break;

	default :
		newline();
		error(error_flag);
    }
}


/*-------------------------------------------------------------------------*/
/* tell_attributes: prints out the attributes descrided by the byte passed */
/*-------------------------------------------------------------------------*/

tell_attributes(attributes)
    int attributes;
    {
    static int attr_count;

    attr_count = 0;
    if (attributes & MASK_HIDDEN)
	{
	put_string(" (");
	attr_count++;
	put_mes(M_HID);                 /* "hidden"     */
	}
    if (attributes & MASK_R_ONLY)
	{
	if (attr_count == 0) put_string(" (");
	    else put_string(",");
	attr_count++;
	put_mes(M_RD_ONLY);             /* "read only"  */
	}
    if (attr_count != 0) put_char(')');
    }

/*---------------------------------------------------------------------------*/
/* modify_path: this function resolves the path in whole_path with respect   */
/* to file name 'fname'. If fname is ".", the path is unmodified. If it is   */
/* ".." then the path is reduced by one level. Otherwise fname is added on   */
/* eg:  w_path before       fname      w_path after                          */
/*        \one\two            .          \one\two                            */
/*        \one\two            ..         \one                                */
/*        \one                ..         \                                   */
/*        \                   fred       \fred                               */
/*        \fred               joe        \fred\joe                           */
modify_path(path,fname)
    char path[], *fname;
    {
    static char *ptr;
    static int count;

    ptr = path; count = 0;
    while (*ptr) {ptr++; count++;}

    if (str_eq(fname,".."))
	{
	if (! ((path[0] == '\\') && (path[1] == 0)))
	    {
	    ptr = ptr - 1;
	    while (*ptr != '\\') ptr--;
	    *ptr = 0;
	    if (path[0] == 0) { path[0] = '\\';  path[1] = 0; }
	    }
	}
    else if (! str_eq(fname,"."))
	{
	if (*(ptr-1) != '\\') {*ptr++ = '\\'; count++;}
	do { *ptr++ = *fname; count++;}
	while ((*fname++) && (count <= MAX_PATH_LEN));
	}
    return (count > MAX_PATH_LEN);
    }

/*-------------------------------------------------------------------------*/
/* chk_and_open: tries to open two file handles.   The first is the source */
/* file,  and is described by parameters passed.   The second is to "nul"; */
/* this is done to check that there is enough memory/file handles for  the */
/* target file.  If an error is found, then a fork & join is done to close */
/* any open file handles, thus freeing them.  If, on retrying, an error is */
/* still found, the program terminates.                                    */
chk_and_open(process_id, fib, handle)
    char *fib;
    int *process_id, *handle;
    {
    int temp_handle;

    do_open (fib, handle);              /* Try and open source. */

    if (! open_err) do_open ("nul", &temp_handle);
    if (open_err)
	{
	join (*process_id);
	fork (process_id);
	do_open (fib, handle);
	if (! open_err) do_open ("nul", &temp_handle);
	if (open_err) error (open_err);
	}
    close (temp_handle);                /* Close null handle. */
    }


/*-------------------------------------------------------------------------*/
/* open: this routine simply does a call to the lowest level open routine, */
/* and if an error occurs, jumps to error.                                 */
open(fib, handle)
    char *fib;
    int *handle;
    {
    do_open(fib, handle);
    if (open_err) error(open_err);
    }



/*****************************************************************************/
/*                                                                           */
/*                             Bits Module                                   */
/*                                                                           */
/*****************************************************************************/

/*---------------------------------------------------------------*/
/* str_eq: returns TRUE if the two strings 'a' and 'b' are equal */
str_eq(a,b)
    char *a, *b;
    {
    while (*a) if (*a++ != *b++) return FALSE;
    return (*b ? FALSE : TRUE);
    }

/*-------------------------------------------------------------------------*/
/* get_name: this transfers the file name pointed to by ptr to buffer. The */
/* format will be a standard ASCIIZ string.                                */
get_fname(ptr, buffer)
    char *ptr, *buffer;
    { do *buffer++ = *ptr; while (*ptr++); }

/* get_file_name: copies the filename part of one fib, to another.         */
get_file_name(from_fib, to_fib)
    char *from_fib, *to_fib;
    {
    static int i;

    from_fib += FIB_FILE_NAME; to_fib += FIB_FILE_NAME;
    for (i=0; i<13; i++) *to_fib++ = *from_fib++;
    }

/*----------------------------------------------------------*/
/* get_w_path: puts the 'whole path' into the buffer given. */
get_w_path(buffer)
    char *buffer;
    {
#asm
	.Z80
	POP     HL              ;return address
	POP     DE              ;buffer address
	PUSH    DE
	PUSH    HL
	LD      C,_WPATH##
	CALL    MSX_DOS
	LD      (REG_HL),HL
	LD      L,A
	LD      H,0
	RET                     ;Return pointer to last item.
	.8080
#endasm
    }

/*-----------------------------------------------------------------------*/
/* do_parse: does the actual call to MSX-DOS, to parse a path string.    */
do_parse(ptr)
    char *ptr;
    {
#asm
	.z80
	POP     HL              ;Return address.
	POP     DE              ;String to parse.
	PUSH    DE
	PUSH    HL
	LD      B,0             ;Not volume name.
	LD      C,_PARSE##
	CALL    MSX_DOS
	LD      (reg_hl),HL     ;Save any results passed back for C !!!!
	LD      (reg_de),DE
	LD      (reg_bc),BC
	LD      L,A             ;Return any error.
	LD      H,0
	RET
	.8080
#endasm
    }

/*--------------------------------------------------------------*/
/* copy_fib: takes two fib's and copies the first to the second */
copy_fib(source,dest)
    char *source, *dest;
    {
    static int i;
    for (i=0; i<FIB_LENGTH; i++) *dest++ = *source++;
    }


/*-------------------------------------------------------------------------*/
/* set_file_attr : is passed a fib and the attributes for the file. This   */
/* routine is used by XCOPY when a read only file is copied and also for   */
/* the reseting of the Archive Bit when required.                          */
/*-------------------------------------------------------------------------*/

set_file_attr(handle,attribs)
    int handle;
    int attribs;
    {
#asm
	.Z80
	POP     DE                      ;Return address.
	POP     HL                      ;The Attributes
	POP     BC                      ;File Handle
	PUSH    BC                      ;Restore stack
	PUSH    HL
	PUSH    DE
	LD      B,C                     ;File handle.
	LD      C,_HATTR##
	LD      A,1                     ; Set Attributes
	CALL    MSX_DOS
	LD      L,A
	LD      H,0
	RET
	.8080
#endasm
    }

/*--------------------------------------------------------------------------*/
/* buf_adr: finds the address of the end of the used  memory,  and  returns */
/* this  so  that  it may be the start of the buffer for copying.   It also */
/* works out the length of the buffer, and puts that in a variable to which */
/* a pointer is passed.   The start and end addresses  of  the  buffer  are */
/* rounded to the nearest 512 bytes for efficiency purposes.                */
buf_adr (length)
    int *length;
    {
#asm
	.Z80
	LD      DE,($LM##)      ;$LM points to the bottom of free memory.
	DEC     DE              ;It is more efficient to round it to the
	INC     D               ; nearest 512 byte boundary.
	INC     D
	LD      E,0
	RES     0,D
	LD      HL,(6)          ;Now calculate the buffer size.
	DEC     HL              ;Take off one for safety.
	LD      L,0
	RES     0,H             ;Nearest 512 byte boundary.
	OR      A
	SBC     HL,DE           ;Work out length of buffer.
	POP     BC              ;Return address.
	EX      DE,HL           ;DE -> length of buffer.
	EX      (SP),HL         ;HL -> address of length.
	LD      (HL),E          ;Store length there.
	INC     HL
	LD      (HL),D
	DEC     HL
	EX      (SP),HL         ;hl -> address of buffer.
	PUSH    BC              ;Resave return address.
	RET
	.8080
#endasm
    }

/*****************************************************************************/
/*                                                                           */
/*                         Error Handling Module                             */
/*                                                                           */
/*****************************************************************************/


/*-----------------------------------------------------------*/
/* error: prints an error message and terminates the program */
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

/*-----------------------------------*/
/* warning: prints a warning message */
warning(num)
    int num;
    {
    switch (num)
	{
	case W_MULT_DEF_OPT: put_mes(M_OPT); break;
	}
    newline();
    }

/*------------------------------------------------------------------------*/
/* pr_err: prints " -- " followed by the error passed, and closes the file */
/* handle passed.                                                         */
pr_err(err, handle)
    int err, handle;
    {
    static char buffer[64];

    put_string (" -- ");
    put_mes (err);
    close (handle);
    }

get_err_text(err, ptr)
    int err;
    char *ptr;
    {
#asm
	.Z80
	POP     HL              ;Return address.
	POP     DE              ;Buffer to put error text in.
	POP     BC              ;Error code.
	PUSH    BC
	LD      B,C
	PUSH    DE
	PUSH    HL
	LD      C,_EXPLAIN##
	CALL    MSX_DOS
	RET
	.8080
#endasm
    }

log2phy( drive )
char    drive;
{
#asm
	.z80
	pop     hl
	pop     de
	push    de
	push    hl
	ld      b,e             ; set logical drive number
	ld      d,0ffh          ; inquire physical drive number
	ld      c,_assign##
	call    msx_dos
	ld      l,d             ; return physical drive number
	ld      h,0
	ret
	.8080
#endasm
}


/*****************************************************************************/
/*                                                                           */
/*                  D E B U G G I N G   A D D I T I O N S                    */
/*                                                                           */
/*****************************************************************************/

#if DEBUG
/* put_number: writes a signed integer (-32768..32767) to the screen */

put_fname(name)
char *name;
{
    if (*name == 0)
    {
	put_string("*.*");
    }
    else
    {
	put_string(name);
    }
}

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

/************************ E N D   O F   P R O G R A M ************************/
