/* XCOPY - Copy files and directory trees
 *
 * This is a port to SDCC C of the original MSX-DOS 2 XCOPY program by
 * IS Systems / ASCII Corporation (which lived in source/command/xcopy
 * in older revisions of this repository, written in c80 C in two source
 * files, merged here), adapted to the Nextor 3 tools conventions (a
 * single binary with both message languages, Kanji mode probed once at
 * startup; see also the banner in crt0_xcopy.mac, which TYPE XCOPY.COM
 * displays).
 *
 * Syntax: XCOPY src [tgt] [/S] [/E] [/P] [/W] [/V] [/H] [/T] [/A] [/M]
 *               [/Dx]
 *
 *   Copies the files matching 'src' (a file, a directory or a pattern)
 *   to 'tgt' (default: the current directory).  /S also copies the
 *   subdirectories and their contents, recursively; /E keeps the
 *   subdirectories that end up empty (by default they are removed
 *   again); /P asks for confirmation before each file; /W waits for a
 *   key before starting; /V enables disk write verification during the
 *   copy (the previous setting is restored at the end, aborts
 *   included); /H also copies hidden files and directories; /T gives
 *   the copies the current date and time instead of keeping the
 *   originals'; /A copies only files with the archive attribute set;
 *   /M is /A plus clearing that attribute on the source files after
 *   copying them.  System files are never copied.
 *
 *   The /Dx options say what to do when the destination file already
 *   exists: /DW overwrites it (the default), /DK skips it, /DN keeps
 *   the newer of the two files, /DO the older, /DS the smaller, /DB
 *   the bigger, /DD overwrites only when the sizes differ, and /DP
 *   shows both files (name, date and time, size) and asks: overwrite,
 *   skip, cancel the whole run, or switch to /DW or /DK for the
 *   remaining files.  A file skipped this way is listed with
 *   " - Skipped" after its name, and a copy that replaces an existing
 *   destination file is listed with " - Overwritten" (in every mode,
 *   /Dx given or not).  On a tie (/DN, /DO: same date and time;
 *   /DS, /DB: same size) the existing destination is kept.
 *
 * The copy strategy is inherited from the original program: within each
 * directory the source file handles are deliberately kept open until
 * the whole directory is done (a fork/join pair per directory level
 * closes them all at once).  A file already open cannot be overwritten
 * by the "find new" DOS call, so this protects every source file of the
 * directory from being overwritten by the copy of another one - that is
 * what refuses "XCOPY A.TXT A.TXT" and what makes two source files
 * renamed to the same destination name give "Duplicate destination
 * filename" for the second one.  A destination file half written when
 * the copy is aborted (CTRL-C, disk error abort, etc.) is deleted by
 * the abort routine, and destinations that cannot be overwritten (a
 * subdirectory, a system or read only file, an invalid name) are
 * reported per file without stopping the run.
 *
 * Differences from the original program:
 *
 * - The source specification is mandatory: with no arguments (or with
 *   just switches) the usage text is displayed and the program exits
 *   with no error, following the convention of the other ported tools
 *   (the original would happily copy the current directory onto
 *   itself, reporting each file as a duplicate).
 *
 * - The /Dx duplicate handling options are new in this port; the
 *   original always overwrote an existing destination file, which
 *   remains the default (/DW).  Files that replace an existing
 *   destination are listed with " - Overwritten" (the original
 *   printed just the name).
 *
 * Everything else works as the original: same syntax, same switches,
 * same messages (English and Japanese), same per-file output.  Like the
 * original, the program runs on any MSX-DOS 2 or later system, Nextor
 * included.
 *
 * Exit codes: 0 = success, 1 = wrong MSX-DOS version; DOS error codes
 * (invalid parameter, disk full...) are returned as such and COMMAND
 * prints their messages.
 */

#include <string.h>
#include "asmcall.h"
#include "types.h"
#include "dos_functions.h"
#include "dos_errors.h"


/*   C O N S T A N T S   */

#define MAX_CMD_LEN	129	/* maximum command line length (+1) */
#define MAX_PATH_LEN	63	/* maximum length of a path string */
#define MAX_FIL_LEN	13	/* name, '.', extension, terminator */

#define E_WRONG_VER	1	/* internal error code */

/* Offsets into file information blocks (FIBs) */
#define FIB_LENGTH	64
#define FIB_FILE_NAME	1
#define FIB_ATTRIBUTES	14
#define FIB_TIME	15
#define FIB_DATE	17
#define FIB_CLUSTER	19
#define FIB_SIZE	21
#define FIB_DRIVE	25

/* Bits in the attributes byte */
#define MASK_R_ONLY	0x01
#define MASK_HIDDEN	0x02
#define MASK_SYSTEM	0x04
#define MASK_SUB_DIR	0x10
#define MASK_ARCHIVE	0x20
#define MASK_DEVICE	0x80

/* Room left for the stack above the copy buffer: enough for the
   deepest possible recursion (paths are limited to 63 characters, so
   at most ~31 levels of ~150 bytes each) plus the DOS calls. */
#define STACK_SLACK	6144


/*   M E S S A G E S   */

/* The message texts live in xcopy_msgs.mac (assembled with Nestor80 in
   SDCC relocatable mode and linked in; the Japanese table is converted
   to Shift-JIS at assembly time by its .strenc directive).  Each table
   is a sequence of zero-terminated strings; the message numbers below
   are ordinal positions in the tables, so THE ORDER OF THIS ENUM AND OF
   THE TABLES IN xcopy_msgs.mac MUST MATCH EXACTLY (the count parity
   between the two tables is checked at assembly time). */

enum {
    M_WAIT, M_FIL1, M_FIL2, M_COP, M_HID, M_RD_ONLY,
    M_T_DE, M_T_SE, M_T_RO, M_T_IN, M_T_FO, M_CCSD,
    M_OPT, M_PROMPT, M_WVER, M_USAG,
    M_SKIP, M_SRC, M_TGT, M_DUPQ, M_BYTES, M_OVER
};

/* Duplicate file handling modes (the /Dx options) */
enum {
    DUP_OVERWRITE,		/* /DW: always overwrite (the default) */
    DUP_SKIP,			/* /DK: always skip */
    DUP_NEWER,			/* /DN: keep the newer file */
    DUP_OLDER,			/* /DO: keep the older file */
    DUP_SMALLER,		/* /DS: keep the smaller file */
    DUP_BIGGER,			/* /DB: keep the bigger file */
    DUP_DIFF,			/* /DD: overwrite if the sizes differ */
    DUP_PROMPT			/* /DP: ask for each file */
};

extern const byte msgs_en[];	/* defined in xcopy_msgs.mac */
extern const byte msgs_ja[];

static const char yes_chars[] = "Yy";
static const char no_chars[]  = "Nn";


/*   G L O B A L   V A R I A B L E S   */

Z80_registers regs;

byte kanji_flag;		/* non-zero => print Japanese messages */

bool prompt_flag;		/* true => confirm each file (/P) */
bool empty_flag;		/* true => keep empty subdirectories (/E) */
bool wait_flag;			/* true => wait before starting (/W) */
bool subdirectory_flag;		/* true => recurse subdirectories (/S) */
bool archive_flag;		/* true => only archive-bit files (/A, /M) */
bool update_arc_flag;		/* true => clear source archive bit (/M) */
bool time_flag;			/* true => stamp copies with now (/T) */
bool hidden_flag;		/* true => copy hidden files too (/H) */
bool s_ambig_flag;		/* true => ambiguous source filename */
bool t_ambig_flag;		/* true => ambiguous target filename */

byte dup_mode;			/* what to do with duplicate files (DUP_*) */
bool dup_mode_set;		/* true => a /Dx option was given */
bool dup_existed;		/* true => the file being copied replaces
				   an existing destination file */

byte verify_flag;		/* original verify setting, restored on
				   every exit by the abort routine */
byte file_not_ensured;		/* handle of a half-written destination
				   file (deleted on abort), or 0 */

byte open_err;			/* error of the last do_open() */
byte process_id;		/* parent process id of the current fork */

byte t_drive;			/* physical drive of the target directory */
byte tl_cluster, th_cluster;	/* its start cluster (recursion guard) */

char s_path[MAX_CMD_LEN];	/* source path */
char t_path[MAX_CMD_LEN];	/* target path */
char ws_path[MAX_PATH_LEN + 6];	/* whole source path */

char sf_name[MAX_FIL_LEN];	/* source filename (pattern) */
char tf_name[MAX_FIL_LEN];	/* target filename (rename pattern) */
char null_file[1];		/* an empty filename (= "*.*") */

char* s_st_file;		/* start of the filename within s_path */
char* t_st_file;		/* start of the filename within t_path */

byte* src_fib;			/* source as returned by src_parse() */
byte* dst_fib;			/* target as returned by dst_parse() */

uint f_count;			/* count of files copied */

byte* buffer;			/* the copy buffer (all the free TPA) */
uint buf_len;

/* Results of the last _PARSE / _WPATH call */
char* parse_last;		/* HL: start of the last item */
char* parse_end;		/* DE: the termination character */
byte parse_pflags;		/* B: parse flags */

uint stack_top_raw;		/* SP as sampled by read_sp() */
byte term_code;			/* exit code for do_terminate() */

static byte sp_fib[FIB_LENGTH];	/* src_parse() result FIB */
static byte dp_fib[FIB_LENGTH];	/* dst_parse() result FIB */
static byte dp_tempfib[FIB_LENGTH];
static char cmd_line[MAX_CMD_LEN];

static byte dup_fib[FIB_LENGTH];/* the existing destination entry found
				   by check_duplicate() */
static char dup_path[MAX_CMD_LEN];

extern byte HEAP_start;		/* first free byte after the program */


/*   P R O T O T Y P E S   */

void kanji_probe(void);
void put_char(char c);
void read_sp(void);
void abort_handler(void);
void do_terminate(void);
void error(byte num);
void terminate(byte code);
void xcopy(byte level, byte* s_fib, byte* t_fib);


/*   B A S I C   P R E D I C A T E S   */

bool islowercase(char c) { return c >= 'a' && c <= 'z'; }
char upper(char c) { return islowercase(c) ? c - 'a' + 'A' : c; }

bool in_set(char c, const char* set)
{
    while (*set)
        if (*set++ == c) return true;
    return false;
}

bool yes(char c) { return in_set(c, yes_chars); }
bool no(char c) { return in_set(c, no_chars); }

bool str_eq(const char* a, const char* b)
{
    while (*a)
        if (*a++ != *b++) return false;
    return *b == '\0';
}


/*   M E S S A G E   A N D   C O N S O L E   O U T P U T   */

void put_char(char c) __naked
{
    __asm
	ld	e,a		;char argument arrives in A
	ld	c,#2		;_CONOUT
	push	ix
	call	5
	pop	ix
	ret
    __endasm;
}

const char* get_msg(byte n)
{
    const byte* p;

    p = kanji_flag ? msgs_ja : msgs_en;
    while (n--)
        while (*p++) ;
    return (const char*)p;
}

void put_string(const char* s)
{
    while (*s) put_char(*s++);
}

void put_msg(byte n)
{
    put_string(get_msg(n));
}

void newline(void)
{
    put_char('\r');
    put_char('\n');
}

void put_spaces(byte num)
{
    while (num--) put_char(' ');
}

void put_unsigned(uint i)
{
    if (i >= 10)
        put_unsigned(i / 10);
    put_char((char)(i % 10) + '0');
}

void put_u32(ulong i)
{
    if (i >= 10)
        put_u32(i / 10);
    put_char((char)(i % 10) + '0');
}

/* put_2d: a number of at most two digits, zero padded */
void put_2d(byte n)
{
    put_char('0' + n / 10);
    put_char('0' + n % 10);
}

/* put_summary: the final " N file(s) copied" line */
void put_summary(void)
{
    put_char(' ');
    put_unsigned(f_count);
    put_msg(f_count == 1 ? M_FIL1 : M_FIL2);	/* " file(s)" */
    put_msg(M_COP);				/* " copied"  */
}


/*   C O N S O L E   I N P U T   */

/* clr_in: gobble up any pending keypresses */
void clr_in(void)
{
    for (;;) {
        DosCall(_CONST, &regs, REGS_MAIN, REGS_AF);
        if (regs.Bytes.A == 0) return;
        DosCall(_INNOE, &regs, REGS_MAIN, REGS_AF);
    }
}

/* get_char: read one character, echoing it unless it is a control char */
char get_char(void)
{
    char c;

    DosCall(_INNOE, &regs, REGS_MAIN, REGS_AF);
    c = regs.Bytes.A;
    if (c >= ' ') put_char(c);
    return c;
}


/*   S T A R T U P   */

/* kanji_probe: ask the Kanji driver (if present) for the current screen
   mode via EXTBIO, so all messages are printed in Japanese when a Kanji
   mode is active.  The Kanji driver answers with inter-slot calls that
   switch pages 0-2 out, so the stack must be in page 3 here: the crt0
   put the whole stack at the top of the TPA (see crt0_xcopy.mac), so no
   stack switching is needed (or allowed: moving SP up to (0006h) from a
   routine would overwrite the live stack data below it). */
void kanji_probe(void) __naked
{
    __asm
	exx
	ex	af,af
	push	af		;Protect the alternate and index
	push	bc		; registers, which the driver may
	push	de		; use freely.
	push	hl
	push	ix
	push	iy
	ex	af,af
	exx
	xor	a		;A = 0: get current mode
	ld	d,#0x11		;D = Kanji driver EXTBIO device
	ld	e,#0		;E = function 0
	call	#0xFFCA		;EXTBIO; if there is no Kanji driver
	exx			; A stays 0 (= ANK mode = English)
	ex	af,af
	pop	iy
	pop	ix
	pop	hl
	pop	de
	pop	bc
	pop	af
	ex	af,af
	exx
	ld	(_kanji_flag),a
	ret
    __endasm;
}

/* check_ver: require MSX-DOS 2 or better, as the original did. */
void check_ver(void)
{
    DosCall(_DOSVER, &regs, REGS_NONE, REGS_MAIN);
    if (regs.Bytes.A != 0 || regs.Bytes.B < 2)
        error(E_WRONG_VER);
}


/*   E R R O R S   A N D   T E R M I N A T I O N   */

/* do_terminate: _TERM with the code in term_code.  On MSX-DOS 1 (only
   possible for the wrong-version error) _TERM returns, and _TERM0
   finishes the job (same sequence as the crt0).  _TERM also runs the
   abort routine, which restores the verify flag and deletes any
   half-written destination file. */
void do_terminate(void) __naked
{
    __asm
	ld	a,(_term_code)
	ld	b,a
	ld	c,#0x62		;_TERM
	call	5
	ld	c,#0		;_TERM0 (MSX-DOS 1 fallback)
	jp	5
    __endasm;
}

void terminate(byte code)
{
    term_code = code;
    do_terminate();
}

/* error: print the message for an internal error number (DOS error
   codes have no internal message: COMMAND prints theirs) and exit. */
void error(byte num)
{
    if (num == E_WRONG_VER)
        put_msg(M_WVER);
    newline();
    terminate(num);
}

/* warning: print a warning message and continue */
void warning_opt(void)
{
    put_msg(M_OPT);		/* "Option selected more than once" */
    newline();
}


/*   T H E   A B O R T   R O U T I N E   */

/* abort_handler: entered (through _DEFAB) whenever the program
   terminates, normally or not: delete any half-written destination
   file and restore the original verify flag setting.  It can be
   entered in the middle of a BDOS call (CTRL-STOP, aborted disk
   error), where the AsmCall machinery must not be re-entered, so it is
   written in plain assembler over direct BDOS calls. */
void abort_handler(void) __naked
{
    __asm
	push	af		;The error code, returned to the system
	push	bc
	push	de
	push	hl
	push	ix
	push	iy
	ld	a,(_file_not_ensured)
	or	a		;A destination file half written?
	jr	z,ab_no_del
	ld	b,a		;Delete it (its handle); a zero first
	xor	a		; protects against recursive aborts
	ld	(_file_not_ensured),a
	ld	c,#0x52		;_HDELETE (errors ignored)
	call	5
ab_no_del:
	ld	a,(_verify_flag)
	ld	e,a		;Restore the original verify setting
	ld	c,#0x2E		;_VERIFY
	call	5
	pop	iy
	pop	ix
	pop	hl
	pop	de
	pop	bc
	pop	af
	ret
    __endasm;
}

void set_abort_routine(void)
{
    regs.UWords.DE = (uint)abort_handler;
    DosCall(_DEFAB, &regs, REGS_MAIN, REGS_NONE);
}


/*   D O S   C A L L   W R A P P E R S   */

/* first: find first entry.  'search' is a drive/path/file string, or a
   FIB describing the directory to search (and then 'filename' is the
   pattern to match). */
byte first(void* search, char* filename, byte* fib, byte attr)
{
    regs.UWords.DE = (uint)search;
    regs.UWords.HL = (uint)filename;
    regs.UWords.IX = (uint)fib;
    regs.Bytes.B = attr;
    DosCall(_FFIRST, &regs, REGS_ALL, REGS_AF);
    return regs.Bytes.A;
}

/* next: find the next entry matching the search that filled the FIB */
byte next(byte* fib)
{
    regs.UWords.IX = (uint)fib;
    DosCall(_FNEXT, &regs, REGS_ALL, REGS_AF);
    return regs.Bytes.A;
}

/* find_new: create a new entry (a file, or a directory if the
   attributes say so) in the directory given by 'fib' (a string or a
   FIB, as in first()).  An ambiguous filename is resolved against the
   template name already present in 'new_fib'. */
byte find_new(void* fib, char* filename, byte* new_fib, byte attr)
{
    regs.UWords.DE = (uint)fib;
    regs.UWords.HL = (uint)filename;
    regs.UWords.IX = (uint)new_fib;
    regs.Bytes.B = attr;
    DosCall(_FNEW, &regs, REGS_ALL, REGS_AF);
    return regs.Bytes.A;
}

/* do_parse: _PARSE the given string; the results are stored in the
   parse_* globals.  Returns the error code. */
byte do_parse(char* ptr)
{
    regs.Bytes.B = 0;
    regs.UWords.DE = (uint)ptr;
    DosCall(_PARSE, &regs, REGS_MAIN, REGS_MAIN);
    parse_last = (char*)regs.UWords.HL;
    parse_end = (char*)regs.UWords.DE;
    parse_pflags = regs.Bytes.B;
    return regs.Bytes.A;
}

/* get_w_path: the whole path (from the root, no drive, no leading
   backslash) of the entry located by the last search, into the buffer;
   parse_last is set to the start of its last item. */
byte get_w_path(char* buffer_)
{
    regs.UWords.DE = (uint)buffer_;
    DosCall(_WPATH, &regs, REGS_MAIN, REGS_MAIN);
    parse_last = (char*)regs.UWords.HL;
    return regs.Bytes.A;
}

/* do_open: open a file handle (read/write) on a FIB or path string;
   the error lands in open_err (0 = success). */
byte do_open(void* fib, byte* handle)
{
    regs.Bytes.A = 0;			/* open mode: read/write */
    regs.UWords.DE = (uint)fib;
    DosCall(_OPEN, &regs, REGS_MAIN, REGS_MAIN);
    *handle = regs.Bytes.B;
    open_err = regs.Bytes.A;
    return open_err;
}

/* open_file: as do_open, but any error is fatal */
void open_file(void* fib, byte* handle)
{
    if (do_open(fib, handle))
        error(open_err);
}

/* close_file/ensure: fatal on error, as the original's MYSTOP */
void close_file(byte handle)
{
    regs.Bytes.B = handle;
    DosCall(_CLOSE, &regs, REGS_MAIN, REGS_AF);
    if (regs.Bytes.A) terminate(regs.Bytes.A);
}

void ensure(byte handle)
{
    regs.Bytes.B = handle;
    DosCall(_ENSURE, &regs, REGS_MAIN, REGS_AF);
    if (regs.Bytes.A) terminate(regs.Bytes.A);
}

/* do_read: read from a handle; end of file is not an error (returns
   the bytes actually read, 0 at the end), anything else is fatal. */
uint do_read(byte* buf, uint length, byte handle)
{
    regs.Bytes.B = handle;
    regs.UWords.DE = (uint)buf;
    regs.UWords.HL = length;
    DosCall(_READ, &regs, REGS_MAIN, REGS_MAIN);
    if (regs.Bytes.A == _EOF) return regs.UWords.HL;
    if (regs.Bytes.A) terminate(regs.Bytes.A);
    return regs.UWords.HL;
}

/* do_write: write to a handle; writing zero bytes is a no-op; any
   error is fatal. */
void do_write(byte* buf, uint length, byte handle)
{
    if (length == 0) return;
    regs.Bytes.B = handle;
    regs.UWords.DE = (uint)buf;
    regs.UWords.HL = length;
    DosCall(_WRITE, &regs, REGS_MAIN, REGS_AF);
    if (regs.Bytes.A) terminate(regs.Bytes.A);
}

/* fork_process/join_process: the handle-lifetime scheme (see the file
   header).  join closes every handle opened since the matching fork. */
void fork_process(void)
{
    DosCall(_FORK, &regs, REGS_NONE, REGS_MAIN);
    if (regs.Bytes.A) terminate(regs.Bytes.A);
    process_id = regs.Bytes.B;
}

void join_process(byte pid)
{
    regs.Bytes.B = pid;
    DosCall(_JOIN, &regs, REGS_MAIN, REGS_AF);
    if (regs.Bytes.A) terminate(regs.Bytes.A);
}

/* try_delete: delete a directory if (and only if) it is empty; "not
   empty" is fine, anything else is fatal. */
void try_delete(byte* fib)
{
    regs.UWords.DE = (uint)fib;
    DosCall(_DELETE, &regs, REGS_MAIN, REGS_AF);
    if (regs.Bytes.A != 0 && regs.Bytes.A != _DIRNE)
        terminate(regs.Bytes.A);
}

/* set_file_attr: set the attributes of an open file handle */
byte set_file_attr(byte handle, byte attribs)
{
    regs.Bytes.A = 1;			/* set */
    regs.Bytes.B = handle;
    regs.Bytes.L = attribs;
    DosCall(_HATTR, &regs, REGS_MAIN, REGS_AF);
    return regs.Bytes.A;
}

/* set_time: give the destination handle the source FIB's date and time */
void set_time(byte* src_file, byte handle)
{
    regs.Bytes.A = 1;			/* set */
    regs.Bytes.B = handle;
    regs.UWords.IX = src_file[FIB_TIME] | (src_file[FIB_TIME + 1] << 8);
    regs.UWords.HL = src_file[FIB_DATE] | (src_file[FIB_DATE + 1] << 8);
    DosCall(_HFTIME, &regs, REGS_ALL, REGS_AF);
}

/* do_flush: flush all disk buffers, so that no access is inadvertently
   delayed across a disk swap (inherited from the original) */
void do_flush(void)
{
    regs.Bytes.B = 0xFF;		/* all drives */
    regs.Bytes.D = 0;			/* flush, don't invalidate */
    DosCall(_FLUSH, &regs, REGS_MAIN, REGS_NONE);
}

/* log2phy: the physical drive behind a logical drive */
byte log2phy(byte drive)
{
    regs.Bytes.B = drive;
    regs.Bytes.D = 0xFF;		/* inquire only */
    DosCall(_ASSIGN, &regs, REGS_MAIN, REGS_MAIN);
    return regs.Bytes.D;
}

byte get_ver_flag(void)
{
    DosCall(_GETVFY, &regs, REGS_NONE, REGS_MAIN);
    return regs.Bytes.B;
}

void set_verify_flag(void)
{
    regs.Bytes.E = 0xFF;
    DosCall(_VERIFY, &regs, REGS_MAIN, REGS_NONE);
}


/*   G E T   D R I V E S   */

/* set_true: set a flag, warning if it was already set */
void set_true(bool* flag)
{
    if (*flag) warning_opt();
    *flag = true;
}

/* set_dup_mode: the second character of a /Dx option */
void set_dup_mode(char c)
{
    if (dup_mode_set) warning_opt();
    dup_mode_set = true;
    switch (c) {
        case 'W': dup_mode = DUP_OVERWRITE; break;
        case 'K': dup_mode = DUP_SKIP; break;
        case 'N': dup_mode = DUP_NEWER; break;
        case 'O': dup_mode = DUP_OLDER; break;
        case 'S': dup_mode = DUP_SMALLER; break;
        case 'B': dup_mode = DUP_BIGGER; break;
        case 'D': dup_mode = DUP_DIFF; break;
        case 'P': dup_mode = DUP_PROMPT; break;
        case 0  : error(_IPARM);
        default : error(_IOPT);
    }
}

/* parse_flags: parses a set of flags of the form "/s/e /p"; the first
   character pointed to must be non-blank. */
void parse_flags(char* ptr)
{
    while (*ptr) {
        if (*ptr != '/') error(_IPARM);
        switch (upper(*(++ptr))) {
            case 'S': set_true(&subdirectory_flag); break;
            case 'H': set_true(&hidden_flag); break;
            case 'T': set_true(&time_flag); break;
            case 'E': set_true(&empty_flag); break;
            case 'W': set_true(&wait_flag); break;
            case 'P': set_true(&prompt_flag); break;
            case 'V': set_verify_flag(); break;
            case 'A': archive_flag = true; break;
            case 'M': archive_flag = true;
                      set_true(&update_arc_flag);
                      break;
            case 'D': set_dup_mode(upper(*(++ptr))); break;
            case 0  : error(_IPARM);
            default : error(_IOPT);
        }
        while (*(++ptr) == ' ') ;
    }
}

/* path_parse: parse one path off the command line: the path (with the
   filename) is copied to 'path', the filename part alone to 'fname',
   and 'stfile' is set to where the filename starts within 'path'.
   Returns the parse flags; advances the command line pointer. */
uint path_parse(char** cmd, char* path, char* fname, char** stfile)
{
    byte err;
    char* ptr;

    ptr = *cmd;
    if ((err = do_parse(ptr)) != 0) error(err);
    *stfile = path + (parse_last - ptr);

    while (ptr < parse_end) *path++ = *ptr++;	/* path + filename */
    *path = '\0';
    ptr = parse_last;
    if (parse_end - ptr > MAX_FIL_LEN - 1)
        error(_IFNM);			/* longer than any valid filename
					   (and fname cannot hold it) */
    while (ptr < parse_end) *fname++ = *ptr++;	/* filename alone */
    *fname = '\0';

    *cmd = ptr;
    return parse_pflags;
}

/* get_drives: parse the source and target paths and the flags from the
   command line.  With no arguments, or with only switches, the usage
   text is displayed and the program exits with no error (see the file
   header).  The original worked on the raw command line at 0x80; the
   crt0 has already tokenized that area, so an equivalent line is
   rebuilt from the argv items, and the original parsing code then
   works on it unchanged. */
void get_drives(char** argv, int argc)
{
    char *p, *cmd;
    int i;
    uint flags;

    cmd = cmd_line;
    for (i = 0; i < argc; i++) {
        if (i > 0) *cmd++ = ' ';
        for (p = argv[i]; *p; p++)
            if (cmd < cmd_line + MAX_CMD_LEN - 1)
                *cmd++ = *p;
    }
    *cmd = '\0';

    s_path[0] = '\0';
    t_path[0] = '\0';

    cmd = cmd_line;
    while (*cmd == ' ') cmd++;		/* strip leading blanks */
    if (*cmd == '\0' || *cmd == '/') {
        /* No arguments, or a switch without a source: show the usage
           text and exit successfully */
        put_msg(M_USAG);
        terminate(0);
    }

    flags = path_parse(&cmd, s_path, sf_name, &s_st_file);
    s_ambig_flag = (flags & 0x20) || (flags & 0x18) == 0;

    while (*cmd == ' ') cmd++;

    flags = path_parse(&cmd, t_path, tf_name, &t_st_file);
    t_ambig_flag = (flags & 0x20) || (flags & 0x18) == 0;

    if (*cmd) {
        while (*cmd == ' ') cmd++;
        if (*cmd == '/') parse_flags(cmd);
        else if (*cmd) error(_IPARM);
    }
}


/*   S O U R C E   A N D   T A R G E T   R E S O L U T I O N   */

/* modify_path: resolves a path against a file name: "." leaves it
   alone, ".." removes the last component, anything else is appended.
   Returns true (an error) if the result exceeds MAX_PATH_LEN. */
bool modify_path(char* path, char* fname)
{
    char* ptr;
    byte count;

    ptr = path;
    count = 0;
    while (*ptr) { ptr++; count++; }

    if (str_eq(fname, "..")) {
        if (!(path[0] == '\\' && path[1] == '\0')) {
            ptr--;
            while (*ptr != '\\') ptr--;
            *ptr = '\0';
            if (path[0] == '\0') {
                path[0] = '\\';
                path[1] = '\0';
            }
        }
    } else if (!str_eq(fname, ".")) {
        if (*(ptr - 1) != '\\') { *ptr++ = '\\'; count++; }
        do { *ptr++ = *fname; count++; }
        while (*fname++ && count <= MAX_PATH_LEN);
    }
    return count > MAX_PATH_LEN;
}

/* get_file_name: copy the filename part of one FIB to another (used to
   set up the rename template for find_new) */
void get_file_name(byte* from_fib, byte* to_fib)
{
    memcpy(to_fib + FIB_FILE_NAME, from_fib + FIB_FILE_NAME, 13);
}

/* src_parse: resolve the source specification: a directory (its FIB is
   returned and the files inside it are the sources), or a file/pattern
   (the path string itself is returned for the searches).  Also
   initializes ws_path with the whole path of the source directory. */
byte* src_parse(void)
{
    byte error_flag, err;
    char* ptr;

    error_flag = first(s_path, null_file, sp_fib, 0x16);

    ws_path[0] = '\\';
    if ((err = get_w_path(ws_path + 1)) != 0) error(err);
    ptr = parse_last;
    if (ptr == ws_path + 1) *ptr = '\0'; else *(ptr - 1) = '\0';

    if (!s_ambig_flag) {
        switch (error_flag) {
            case 0:
                if (sp_fib[FIB_ATTRIBUTES] & MASK_SUB_DIR) {
                    /* the source is a directory: copy its contents */
                    if (modify_path(ws_path, (char*)sp_fib + FIB_FILE_NAME))
                        error(_PLONG);
                    sf_name[0] = '\0';
                    return sp_fib;
                }
                break;			/* a plain file: use the string */
            case _NOFIL:
                break;			/* nothing yet: use the string */
            default:
                error(error_flag);
        }
    }
    return (byte*)s_path;
}

/* dst_parse: resolve the target specification: an existing directory
   (its FIB is returned, no renaming) or a path with an optional rename
   pattern (the path string is returned; tf_name keeps the pattern).
   Also records the target directory's physical drive and start cluster
   for the recursion guard (don't descend into the target itself), and
   refuses devices. */
byte* dst_parse(void)
{
    byte error_flag;
    static char dirnam[MAX_CMD_LEN];
    char *p, *d;

    error_flag = first(t_path, tf_name, dp_fib, 0x16);
    if (error_flag && error_flag != _NOFIL) error(error_flag);
    if ((dp_fib[FIB_ATTRIBUTES] & MASK_DEVICE) && error_flag == 0)
        error(_IDEV);

    t_drive = log2phy(dp_fib[FIB_DRIVE]);

    if ((dp_fib[FIB_ATTRIBUTES] & MASK_SUB_DIR) && !t_ambig_flag
            && error_flag == 0) {
        tf_name[0] = '\0';		/* no renaming */
        tl_cluster = dp_fib[FIB_CLUSTER];
        th_cluster = dp_fib[FIB_CLUSTER + 1];
        return dp_fib;
    }

    /* Not an existing directory: find the directory PART of the target
       path, to locate the directory entry for the recursion guard */
    if ((error_flag = do_parse(t_path)) != 0) error(error_flag);
    p = t_path;
    d = dirnam;
    while (p < parse_last) *d++ = *p++;
    *d = '\0';
    if (*t_path && *(d - 1) == '\\') *(d - 1) = '\0';
    first(dirnam, tf_name, dp_tempfib, 0x16);
    tl_cluster = dp_tempfib[FIB_CLUSTER];
    th_cluster = dp_tempfib[FIB_CLUSTER + 1];
    return (byte*)t_path;
}


/*   T H E   C O P Y   */

/* tell_attributes: print " (hidden,read only)" as applicable */
void tell_attributes(byte attributes)
{
    byte attr_count;

    attr_count = 0;
    if (attributes & MASK_HIDDEN) {
        put_string(" (");
        attr_count++;
        put_msg(M_HID);
    }
    if (attributes & MASK_R_ONLY) {
        put_string(attr_count == 0 ? " (" : ",");
        attr_count++;
        put_msg(M_RD_ONLY);
    }
    if (attr_count != 0) put_char(')');
}

/* put_file_line: the indented file name plus its attributes (the line
   printed for each file processed) */
void put_file_line(byte level, byte* fib)
{
    put_spaces(level * 3);
    put_string((char*)fib + FIB_FILE_NAME);
    tell_attributes(fib[FIB_ATTRIBUTES]);
}


/*   D U P L I C A T E   F I L E   H A N D L I N G   */

/* fib_size: a FIB's file size, as a 32 bit number */
ulong fib_size(byte* fib)
{
    return (ulong)fib[FIB_SIZE]
         | ((ulong)fib[FIB_SIZE + 1] << 8)
         | ((ulong)fib[FIB_SIZE + 2] << 16)
         | ((ulong)fib[FIB_SIZE + 3] << 24);
}

/* fib_stamp: a FIB's date and time, as one comparable 32 bit number
   (newer file => bigger number) */
ulong fib_stamp(byte* fib)
{
    return ((ulong)(fib[FIB_DATE] | (fib[FIB_DATE + 1] << 8)) << 16)
         | (uint)(fib[FIB_TIME] | (fib[FIB_TIME + 1] << 8));
}

/* put_stamp: print a FIB's date and time as "2026/04/21 12:34:56" */
void put_stamp(byte* fib)
{
    uint d, t;

    d = fib[FIB_DATE] | (fib[FIB_DATE + 1] << 8);
    t = fib[FIB_TIME] | (fib[FIB_TIME + 1] << 8);
    put_unsigned((d >> 9) + 1980);
    put_char('/');
    put_2d((d >> 5) & 0x0F);
    put_char('/');
    put_2d(d & 0x1F);
    put_char(' ');
    put_2d(t >> 11);
    put_char(':');
    put_2d((t >> 5) & 0x3F);
    put_char(':');
    put_2d((t & 0x1F) << 1);
}

/* put_dup_line: one file of the /DP dialog: name, date/time, size */
void put_dup_line(byte* fib)
{
    put_string((char*)fib + FIB_FILE_NAME);
    put_string(", ");
    put_stamp(fib);
    put_string(", ");
    put_u32(fib_size(fib));
    put_msg(M_BYTES);			/* " bytes" */
    newline();
}

/* to_fcb_name: a "NAME.EXT" string to the 11 character FCB form,
   uppercased, with '*' expanded to '?'s */
void to_fcb_name(const char* name, char* fcb)
{
    byte i;

    for (i = 0; i < 11; i++) fcb[i] = ' ';
    i = 0;
    while (*name && *name != '.') {
        if (*name == '*') { while (i < 8) fcb[i++] = '?'; }
        else if (i < 8) fcb[i++] = upper(*name);
        name++;
    }
    if (*name == '.') name++;
    i = 8;
    while (*name) {
        if (*name == '*') { while (i < 11) fcb[i++] = '?'; }
        else if (i < 11) fcb[i++] = upper(*name);
        name++;
    }
}

/* expand_name: the destination filename for a source filename: the
   rename pattern with its wildcards filled from the source name (the
   same expansion the "find new" DOS call performs), back as a
   "NAME.EXT" string */
void expand_name(const char* src, const char* pattern, char* out)
{
    char s_fcb[11], p_fcb[11];
    byte i;
    char c;
    bool has_ext;

    if (pattern[0] == '\0') {		/* no renaming */
        while ((*out++ = *src++) != '\0') ;
        return;
    }
    to_fcb_name(src, s_fcb);
    to_fcb_name(pattern, p_fcb);
    for (i = 0; i < 8; i++) {
        c = p_fcb[i] == '?' ? s_fcb[i] : p_fcb[i];
        if (c != ' ') *out++ = c;
    }
    has_ext = false;
    for (i = 8; i < 11; i++) {
        c = p_fcb[i] == '?' ? s_fcb[i] : p_fcb[i];
        if (c != ' ') {
            if (!has_ext) { *out++ = '.'; has_ext = true; }
            *out++ = c;
        }
    }
    *out = '\0';
}

/* check_duplicate: does the destination file for this source file
   already exist?  The destination name is the source name passed
   through the rename pattern; the directory to look in is t_fib (a
   FIB, or the target path string at the top level).  Fills dup_fib
   with the existing entry.  An existing entry that is not a plain file
   (a subdirectory, a system file, a device) is not a duplicate: the
   normal copy path reports those with the proper error message. */
bool check_duplicate(byte* t_fib, byte* src_file)
{
    char dup_name[MAX_FIL_LEN];
    char *p, *d;

    expand_name((char*)src_file + FIB_FILE_NAME, tf_name, dup_name);
    if (*t_fib == 0xFF) {		/* a FIB: search inside it */
        if (first(t_fib, dup_name, dup_fib, 0x16)) return false;
    } else {				/* the path string: replace its
					   filename part with the name */
        p = (char*)t_fib;
        d = dup_path;
        while (p < t_st_file) *d++ = *p++;
        p = dup_name;
        while ((*d++ = *p++) != '\0') ;
        if (first(dup_path, null_file, dup_fib, 0x16)) return false;
    }
    return !(dup_fib[FIB_ATTRIBUTES]
             & (MASK_SUB_DIR | MASK_SYSTEM | MASK_DEVICE));
}

/* dup_decision: apply an automatic /Dx mode to the duplicate found by
   check_duplicate(): true = overwrite it.  Ties (same date and time,
   same size) keep the existing destination. */
bool dup_decision(byte* src_file)
{
    ulong s, t;

    if (dup_mode == DUP_OVERWRITE) return true;
    if (dup_mode == DUP_SKIP) return false;
    if (dup_mode == DUP_NEWER || dup_mode == DUP_OLDER) {
        s = fib_stamp(src_file);
        t = fib_stamp(dup_fib);
        return dup_mode == DUP_NEWER ? s > t : s < t;
    }
    s = fib_size(src_file);
    t = fib_size(dup_fib);
    if (dup_mode == DUP_DIFF) return s != t;
    return dup_mode == DUP_SMALLER ? s < t : s > t;
}

/* dup_prompt: the /DP dialog for one duplicate file: show both files
   and ask.  Returns true to overwrite; the "all" answers also change
   dup_mode for the remaining files; Cancel prints the copied files
   summary and terminates the program. */
bool dup_prompt(byte level, byte* src_file)
{
    char ans;

    newline();			/* a blank line before the dialog */
    put_spaces(level * 3);
    put_msg(M_SRC);			/* "Source: " */
    put_dup_line(src_file);
    put_spaces(level * 3);
    put_msg(M_TGT);			/* "Target: " */
    put_dup_line(dup_fib);
    newline();
    for (;;) {
        put_spaces(level * 3);
        put_msg(M_DUPQ);	/* "(O)verwrite, (S)kip, ...? " */
        clr_in();
        ans = upper(get_char());
        newline();
        if (ans == 'W') dup_mode = DUP_OVERWRITE;
        if (ans == 'K') dup_mode = DUP_SKIP;
        if (ans == 'O' || ans == 'W' || ans == 'S' || ans == 'K'
                || ans == 'C')
            break;
        /* invalid answer: ask again */
    }
    newline();				/* the extra blank line */
    if (ans == 'C') {			/* Cancel: stop the whole run */
        put_summary();
        terminate(0);
    }
    return ans == 'O' || ans == 'W';
}


/* pr_err: print " -- " plus a message, and close the (source) handle */
void pr_err(byte msg, byte handle)
{
    put_string(" -- ");
    put_msg(msg);
    close_file(handle);
}

/* chk_and_open: open the source file plus a probe handle to "NUL" (to
   check that a handle and the memory for the coming destination file
   are available).  On failure, a join/fork closes all the handles
   accumulated for the current directory and everything is retried; a
   second failure is fatal. */
void chk_and_open(byte* fib, byte* handle)
{
    byte temp_handle;

    do_open(fib, handle);
    if (!open_err) do_open("NUL", &temp_handle);
    if (open_err) {
        join_process(process_id);
        fork_process();
        do_open(fib, handle);
        if (!open_err) do_open("NUL", &temp_handle);
        if (open_err) error(open_err);
    }
    close_file(temp_handle);
}

/* copy_data: the actual transfer of one file.  The destination is
   created only after the first buffer is read (so a source that cannot
   even be read does not clobber its destination), written batch by
   batch, then ensured, attribute-fixed and time-stamped. */
void copy_data(byte* src_file, byte* dst_dir, byte* dst_file, byte attr)
{
    byte src_handle, dst_handle;
    byte error_flag, err2_flag;
    uint num_read;

    chk_and_open(src_file, &src_handle);

    get_file_name(src_file, dst_file);	/* the rename template */
    dst_handle = 0xFF;			/* not yet created */
    error_flag = 0;
    file_not_ensured = 0;

    do {
        do_flush();
        num_read = do_read(buffer, buf_len, src_handle);

        if (dst_handle == 0xFF) {
            error_flag = find_new(dst_dir, tf_name, dst_file, attr & 0x7E);
            if (!error_flag) {
                open_file(dst_file, &dst_handle);
                file_not_ensured = dst_handle;
            }
        }

        if (!error_flag) do_write(buffer, num_read, dst_handle);
    } while (num_read == buf_len && !error_flag);

    switch (error_flag) {
        case 0:
            ensure(dst_handle);
            file_not_ensured = 0;
            /* a read only source makes a read only (and archive-set)
               destination */
            if (attr & MASK_R_ONLY)
                if ((err2_flag = set_file_attr(dst_handle,
                                               attr | MASK_ARCHIVE)) != 0)
                    error(err2_flag);
            do_flush();
            if (!time_flag) set_time(src_file, dst_handle);
            if (update_arc_flag)	/* /M: clear the source archive bit */
                if ((err2_flag = set_file_attr(src_handle,
                                               attr & ~MASK_ARCHIVE)) != 0)
                    error(err2_flag);
            if (dup_existed)		/* an existing destination file
                                           was replaced */
                put_msg(M_OVER);	/* " - Overwritten" */
            f_count++;
            break;

        case _DIRX:      pr_err(M_T_DE, src_handle); break;
        case _SYSX:      pr_err(M_T_SE, src_handle); break;
        case _FILRO:     pr_err(M_T_RO, src_handle); break;
        case _IFNM:      pr_err(M_T_IN, src_handle); break;
        case _FOPEN_ERR: pr_err(M_T_FO, src_handle); break;

        default:
            newline();
            error(error_flag);
    }
}

/* xcopy: the heart of the program.  For the directory described by
   s_fib (a FIB or a path string), copy all the matching files into
   t_fib (idem), then recurse into the subdirectories when /S is given.
   File handles are kept open per directory (see the file header), so a
   fork/join pair brackets the file phase of each level. */
void xcopy(byte level, byte* s_fib, byte* t_fib)
{
    /* These three MUST be one-per-level (the FIBs are passed down to
       the recursive call, and d_err_flag decides the try_delete after
       it returns); the original marked them 'auto' for the same
       reason. */
    byte s_next_fib[FIB_LENGTH];
    byte t_next_fib[FIB_LENGTH];
    byte d_err_flag;

    byte err_flag, attributes;
    char ans;
    bool do_it, name_printed;

    fork_process();

    err_flag = first(s_fib, sf_name, s_next_fib, 0x16);

    while (err_flag == 0) {
        attributes = s_next_fib[FIB_ATTRIBUTES];
        /* Note: no device filter here, like the original: an exact
           device name as the source ("XCOPY CON FOO.TXT") copies from
           the device (console input until CTRL-Z, etc.) */
        if (!(attributes & (MASK_SUB_DIR | MASK_SYSTEM))
                && (hidden_flag || !(attributes & MASK_HIDDEN))
                && ((attributes & MASK_ARCHIVE) || !archive_flag)) {
            /* a matching file to copy */
            do_it = true;
            name_printed = false;
            if (prompt_flag) {
                for (;;) {
                    put_file_line(level, s_next_fib);
                    put_msg(M_PROMPT);	/* " - Copy (Y/N)? " */
                    clr_in();
                    ans = get_char();
                    if (no(ans)) {
                        newline();
                        do_it = false;
                        break;
                    }
                    if (yes(ans)) {
                        name_printed = true;  /* the prompt line doubles
                                                 as the file line */
                        break;
                    }
                    newline();		/* invalid answer: ask again */
                }
            }
            dup_existed = false;
            if (do_it && check_duplicate(t_fib, s_next_fib)) {
                dup_existed = true;
                if (dup_mode == DUP_PROMPT) {
                    if (name_printed) {	/* finish the /P prompt line */
                        newline();
                        name_printed = false;
                    }
                    do_it = dup_prompt(level, s_next_fib);
                } else
                    do_it = dup_decision(s_next_fib);
                if (!do_it) {		/* skipped by the /Dx option */
                    if (!name_printed) put_file_line(level, s_next_fib);
                    put_msg(M_SKIP);	/* " - Skipped" */
                    newline();
                }
            }
            if (do_it) {
                if (!name_printed) put_file_line(level, s_next_fib);
                copy_data(s_next_fib, t_fib, t_next_fib, attributes);
                newline();
            }
        }
        err_flag = next(s_next_fib);
    }
    if (err_flag != _NOFIL) error(err_flag);

    join_process(process_id);		/* close all this level's files */

    if (subdirectory_flag) {
        if (level == 0) {
            *s_st_file = '\0';		/* strip the filename parts off */
            *t_st_file = '\0';		/* the path strings */
        }
        err_flag = first(s_fib, null_file, s_next_fib, 0x16);
        while (err_flag == 0) {
            attributes = s_next_fib[FIB_ATTRIBUTES];
            if ((attributes & MASK_SUB_DIR)
                    && s_next_fib[FIB_FILE_NAME] != '.'
                    && (hidden_flag || !(attributes & MASK_HIDDEN))
                    && !(attributes & MASK_SYSTEM)
                    && (t_drive != log2phy(s_next_fib[FIB_DRIVE])
                        || tl_cluster != s_next_fib[FIB_CLUSTER]
                        || th_cluster != s_next_fib[FIB_CLUSTER + 1])) {
                /* a subdirectory to copy (and not the target directory
                   itself: that would recurse forever) */
                put_spaces(level * 3);
                if (modify_path(ws_path, (char*)s_next_fib + FIB_FILE_NAME))
                    error(_PLONG);
                put_string(ws_path);
                tell_attributes(attributes);

                get_file_name(s_next_fib, t_next_fib);
                d_err_flag = find_new(t_fib, null_file, t_next_fib,
                                      attributes & 0x7E);
                switch (d_err_flag) {
                    case 0:
                    case _DIRX:		/* created, or it already existed
                                           (the FIB then describes the
                                           existing directory) */
                        newline();
                        xcopy(level + 1, s_next_fib, t_next_fib);
                        if (d_err_flag == 0 && !empty_flag)
                            try_delete(t_next_fib);
                        break;

                    case _FILEX:	/* non-fatal errors */
                    case _IFNM:
                    case _FOPEN_ERR:
                        put_msg(M_CCSD);
                        break;

                    default:
                        newline();
                        error(d_err_flag);
                }
                modify_path(ws_path, "..");
            }
            err_flag = next(s_next_fib);
        }
        if (err_flag != _NOFIL) error(err_flag);
    }
}


/*   T H E   C O P Y   B U F F E R   */

void read_sp(void) __naked
{
    __asm
	ld	(_stack_top_raw),sp
	ret
    __endasm;
}

/* alloc_buffer: the copy buffer takes all the memory between the end
   of the program and the stack, minus room for the deepest recursion,
   512-byte aligned. */
void alloc_buffer(void)
{
    uint high_mem, start;

    read_sp();
    high_mem = (stack_top_raw - STACK_SLACK) & 0xFE00;
    start = ((uint)&HEAP_start + 511) & 0xFE00;
    if (start >= high_mem) error(_NORAM);
    buffer = (byte*)start;
    buf_len = high_mem - start;
}


/*   M A I N   */

int main(char** argv, int argc)
{
    /* The crt0 does not zero plain globals, so initialize them here */
    kanji_flag = 0;
    prompt_flag = empty_flag = wait_flag = false;
    subdirectory_flag = archive_flag = update_arc_flag = false;
    time_flag = hidden_flag = false;
    s_ambig_flag = t_ambig_flag = false;
    dup_mode = DUP_OVERWRITE;
    dup_mode_set = false;
    dup_existed = false;
    f_count = 0;
    file_not_ensured = 0;
    null_file[0] = '\0';

    kanji_probe();
    check_ver();

    newline();

    verify_flag = get_ver_flag();	/* read before registering the */
    set_abort_routine();		/* routine that restores it */

    get_drives(argv, argc);		/* may show the usage and exit */

    if (wait_flag) {
        put_msg(M_WAIT);		/* "Press any key to continue" */
        clr_in();
        get_char();
        newline();
        newline();
    }

    alloc_buffer();

    src_fib = src_parse();
    dst_fib = dst_parse();

    xcopy(0, src_fib, dst_fib);

    put_summary();			/* " N file(s) copied" */
    return 0;
}
