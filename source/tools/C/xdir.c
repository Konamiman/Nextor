/* XDIR - List the contents of a directory and all its subdirectories
 *
 * This is a port to SDCC C of the original MSX-DOS 2 XDIR program by
 * IS Systems / ASCII Corporation (which lived in source/command/xdir in
 * older revisions of this repository, written in c80 C), adapted to the
 * Nextor 3 tools conventions (a single binary with both message
 * languages, Kanji mode probed once at startup; see also the banner in
 * crt0_xdir.mac, which TYPE XDIR.COM displays).
 *
 * Syntax: XDIR [filespec] [/H] [/B]
 *
 *   Lists the files matching the filespec (default: everything) in the
 *   given directory (default: the current one) and, recursively, in all
 *   its subdirectories, with their attributes and sizes, followed by
 *   the total size, the file count and the free space on the drive.
 *   /H includes hidden files and directories in the listing.
 *   /B displays all the sizes in bytes.
 *
 * Differences from the original program:
 *
 * - File sizes and totals are no longer limited to 8 digits, and the
 *   free space on the drive is no longer computed in 16 bits: a file
 *   over 99 999 999 bytes or a drive with more than 65535 free K (both
 *   possible on the FAT16 volumes supported since Nextor 2.0) are now
 *   displayed correctly.
 *
 * - Sizes from a threshold up are displayed in K (rounded to the
 *   nearest), following the same rules as the DIR command of
 *   COMMAND3.COM: the threshold is 10K by default and can be changed
 *   with the DIRK environment item (a number of K from 1 to 65535,
 *   optionally followed by one or two letters that replace the "K"
 *   suffix of the figures; 0 or OFF keeps the file sizes in bytes,
 *   with the totals in K from 1K up as the original did, and 0 can
 *   take the suffix letters too, for the totals; any other value
 *   means the default), and the /B switch displays every figure
 *   in bytes regardless of DIRK, like the DIRB command does. The
 *   total and free space figures follow the same rules (the original
 *   showed them in K, truncated, from 1K up).
 *
 * Everything else works as the original: same syntax, same output
 * format, same messages (English and Japanese).  Like the original, the
 * program runs on any MSX-DOS 2 or later system, Nextor included; it
 * performs no writes at all.
 *
 * Exit codes: 0 = success, 1 = wrong MSX-DOS version; DOS error codes
 * (invalid drive, invalid option...) are returned as such and COMMAND
 * prints their messages.
 */

#include <string.h>
#include "asmcall.h"
#include "types.h"
#include "dos_functions.h"
#include "dos_errors.h"
#include "strcmpi.h"


/*   C O N S T A N T S   */

#define MAX_CMD_LEN	129	/* maximum command line length (+1) */

#define E_WRONG_VER	1	/* internal error code */

/* Offsets into file information blocks (FIBs) */
#define FIB_LENGTH	64
#define FIB_NAME	1
#define FIB_ATTRIBUTES	14
#define FIB_SIZE	21
#define FIB_DRIVE	25

#define FIL_NAME_LEN	13

/* Bits in the attributes byte */
#define MASK_READ_ONLY	0x01
#define MASK_HIDDEN	0x02
#define MASK_SYSTEM	0x04
#define MASK_SUB_DIR	0x10
#define MASK_DEVICE	0x80


/*   M E S S A G E S   */

/* The message texts live in xdir_msgs.mac (assembled with Nestor80 in
   SDCC relocatable mode and linked in; the Japanese table is converted
   to Shift-JIS at assembly time by its .strenc directive).  Each table
   is a sequence of zero-terminated strings; the message numbers below
   are ordinal positions in the tables, so THE ORDER OF THIS ENUM AND OF
   THE TABLES IN xdir_msgs.mac MUST MATCH EXACTLY (the count parity
   between the two tables is checked at assembly time). */

enum {
    M_SIZ1, M_SIZ2, M_IN, M_KIN, M_FIL1, M_FIL2, M_KFREE,
    M_VOL1, M_VOL2, M_VOL3, M_XDIR, M_WVER, M_BFREE
};

extern const byte msgs_en[];	/* defined in xdir_msgs.mac */
extern const byte msgs_ja[];


/*   G L O B A L   V A R I A B L E S   */

Z80_registers regs;

byte kanji_flag;		/* non-zero => print Japanese messages */

byte source_drive;		/* logical source drive (1 = A: etc) */
bool ambig_flag;		/* true => the filespec is ambiguous */
bool hidden_flag;		/* true => list hidden files too (/H) */
bool bytes_flag;		/* true => sizes always in bytes (/B) */

/* How the sizes are displayed (see set_size_mode) */
enum {
    SM_BYTES,		/* every figure in bytes (/B) */
    SM_TOTALS,		/* file sizes in bytes, totals in rounded K from
			   1K up (DIRK off) */
    SM_THRESHOLD	/* every figure in rounded K from k_threshold up,
			   bytes below */
};
byte size_mode;
ulong k_threshold;		/* the threshold, in bytes */
char k_suffix[3];		/* the suffix of the figures in K */

ulong total_sizes;		/* accumulated sum of the file sizes */
uint f_count;			/* count of files found */

char s_path[MAX_CMD_LEN];	/* the path being searched */
char w_path[MAX_CMD_LEN];	/* the accumulated whole path */
char f_name[FIL_NAME_LEN + 1];	/* the filename part of the filespec */
char null_file[1];		/* an empty filename (= "*.*") */

char* st_file;			/* start of the filename within s_path */

/* Results of the last _PARSE / _WPATH call */
char* parse_last;		/* HL: start of the last item */
char* parse_end;		/* DE: the termination character */
byte parse_pflags;		/* B: parse flags */

byte term_code;			/* exit code for do_terminate() */

static byte root_fib[FIB_LENGTH];  /* level-0 "search in this directory"
				      case (used once, before recursing) */
static char cmd_line[MAX_CMD_LEN];


/*   P R O T O T Y P E S   */

void kanji_probe(void);
void put_char(char c);
void do_terminate(void);
void error(byte num);
void xdir(byte level, byte* search_fib);


/*   B A S I C   P R E D I C A T E S   */

bool islowercase(char c) { return c >= 'a' && c <= 'z'; }
char upper(char c) { return islowercase(c) ? c - 'a' + 'A' : c; }

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

/* format_u32: format a 32 bit number into the buffer, right aligned in
   a field of the given width whose leading positions are filled with
   the given character; a zero leader suppresses them (the field
   shrinks).  Numbers wider than the field (possible on FAT16 volumes)
   widen it instead of being garbled (the original was limited to 8).
   Returns a pointer to the terminating null. */
char* format_u32(char leader, byte width, ulong value, char* buffer)
{
    char digits[10];
    byte n, i;

    n = 0;
    do {
        digits[n++] = (char)(value % 10) + '0';
        value /= 10;
    } while (value != 0);
    if (leader)
        for (i = n; i < width; i++) *buffer++ = leader;
    while (n) *buffer++ = digits[--n];
    *buffer = '\0';
    return buffer;
}

void put_u32(ulong value)
{
    char buffer[12];

    format_u32(0, 0, value, buffer);
    put_string(buffer);
}

/* in_k: whether a total is to be displayed in rounded K: bytes are
   not forced and the count reaches the threshold. */
bool in_k(ulong bytes)
{
    return size_mode != SM_BYTES && bytes >= k_threshold;
}

/* to_k: a byte count in K, rounded to the nearest (halves up). */
ulong to_k(ulong bytes)
{
    return (bytes >> 10) + ((bytes & 0x200) ? 1 : 0);
}

/* format_size: a file size into the buffer as a right aligned 8
   column field: the rounded K count followed by the suffix when in_k
   says so in the threshold mode, the bytes otherwise. */
void format_size(ulong bytes, char* buffer)
{
    const char* s;

    if (size_mode == SM_THRESHOLD && in_k(bytes)) {
        buffer = format_u32(' ', 8 - strlen(k_suffix), to_k(bytes), buffer);
        for (s = k_suffix; *s; ) *buffer++ = *s++;
        *buffer = '\0';
    } else
        format_u32(' ', 8, bytes, buffer);
}

/* put_total: a total (no padding) followed by the message for its
   unit: the rounded K count, the suffix and k_msg when in_k says so;
   else the bytes and the singular or plural bytes message. */
void put_total(ulong bytes, byte k_msg, byte byte_msg, byte bytes_msg)
{
    if (in_k(bytes)) {
        put_u32(to_k(bytes));
        put_string(k_suffix);
        put_msg(k_msg);
    } else {
        put_u32(bytes);
        put_msg(bytes == 1 ? byte_msg : bytes_msg);
    }
}


/*   S T A R T U P   */

/* kanji_probe: ask the Kanji driver (if present) for the current screen
   mode via EXTBIO, so all messages are printed in Japanese when a Kanji
   mode is active.  The Kanji driver answers with inter-slot calls that
   switch pages 0-2 out, so the stack must be in page 3 here: the crt0
   put the whole stack at the top of the TPA (see crt0_xdir.mac), so no
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

/* path_parse: _PARSE the given string; the results are stored in the
   parse_* globals.  Returns the error code. */
byte path_parse(byte vol_flag, char* path)
{
    regs.Bytes.B = vol_flag;
    regs.UWords.DE = (uint)path;
    DosCall(_PARSE, &regs, REGS_MAIN, REGS_MAIN);
    parse_last = (char*)regs.UWords.HL;
    parse_end = (char*)regs.UWords.DE;
    parse_pflags = regs.Bytes.B;
    return regs.Bytes.A;
}

/* get_w_path: the whole path (from the root, no drive, no leading
   backslash) of the entry located by the last search, into the buffer;
   parse_last is set to the start of its last item. */
byte get_w_path(char* buffer)
{
    regs.UWords.DE = (uint)buffer;
    DosCall(_WPATH, &regs, REGS_MAIN, REGS_MAIN);
    parse_last = (char*)regs.UWords.HL;
    return regs.Bytes.A;
}

/* get_bytes_free: free space on the given logical drive, in bytes
   (free clusters * sectors per cluster * 512, sectors being always
   512 bytes; fits in 32 bits for any FAT12/FAT16 volume) */
ulong get_bytes_free(byte drive)
{
    regs.Bytes.E = drive;
    DosCall(_ALLOC, &regs, REGS_MAIN, REGS_MAIN);
    return ((ulong)regs.UWords.HL * regs.Bytes.A) << 9;
}

/* get_env: the value of the environment item into the buffer (which
   must hold 256 bytes); an empty string when it does not exist. */
void get_env(const char* name, char* buffer)
{
    regs.UWords.HL = (uint)name;
    regs.UWords.DE = (uint)buffer;
    regs.Bytes.B = 255;
    DosCall(_GENV, &regs, REGS_MAIN, REGS_AF);
    if (regs.Bytes.A != 0) *buffer = '\0';
}

bool isletter(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

/* set_size_mode: decide how sizes are displayed, from the /B switch
   and the DIRK environment item, with the rules of COMMAND3's DIR:
   /B => every figure in bytes; else DIRK = "0" or "OFF" (any case) =>
   file sizes in bytes and totals in K from 1K up, DIRK = 1 to 65535
   => that many K is the threshold from which every figure is shown in
   K. The number, 0 included, may be followed by one or two letters
   that replace the "K" suffix of the figures shown in K (case
   preserved); anything else (no DIRK included) => the default
   threshold of 10K with the "K" suffix. */
void set_size_mode(void)
{
    static char value[256];
    char suffix[3];
    char* p;
    ulong n;
    byte i;

    size_mode = bytes_flag ? SM_BYTES : SM_THRESHOLD;
    k_threshold = 10UL << 10;
    k_suffix[0] = 'K';
    k_suffix[1] = '\0';
    if (bytes_flag) return;

    get_env("DIRK", value);
    if (strcmpi(value, "OFF") == 0) {
        size_mode = SM_TOTALS;
        k_threshold = 1024;
        return;
    }

    n = 0;
    for (p = value; *p >= '0' && *p <= '9'; p++) {
        n = n * 10 + (*p - '0');
        if (n > 65535) return;
    }
    if (p == value) return;		/* no number: default */

    for (i = 0; i < 2 && isletter(*p); i++) suffix[i] = *p++;
    suffix[i] = '\0';
    if (*p) return;			/* something else follows: default */

    if (i) strcpy(k_suffix, suffix);
    if (n == 0) {
        size_mode = SM_TOTALS;		/* with the suffix, if any */
        k_threshold = 1024;
    } else
        k_threshold = n << 10;
}


/*   E R R O R S   A N D   T E R M I N A T I O N   */

/* do_terminate: _TERM with the code in term_code.  On MSX-DOS 1 (only
   possible for the wrong-version error) _TERM returns, and _TERM0
   finishes the job (same sequence as the crt0). */
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


/*   G E T   P A T H   */

/* parse_flags: parses a set of flags of the form "/h /h"; the first
   character pointed to must be non-blank. */
void parse_flags(char* ptr)
{
    while (*ptr) {
        if (*ptr != '/') error(_IOPT);
        switch (upper(*(++ptr))) {
            case 0  : error(_IOPT);
            case 'H': hidden_flag = true;
                      break;
            case 'B': bytes_flag = true;
                      break;
            default : error(_IOPT);
        }
        while (*(++ptr) == ' ') ;
    }
}

/* get_fname: copy a FIB filename to the given buffer */
void get_fname(char* ptr, char* buffer)
{
    byte i;

    for (i = 0; i < 12; i++) *buffer++ = *ptr++;
    *buffer = '\0';
}

/* get_vol_name: the volume name of the drive given in the path (empty
   string if it has none); also sets source_drive from the search. */
void get_vol_name(char* buffer, char* path)
{
    byte temp_fib[FIB_LENGTH];
    byte err_flag;

    err_flag = first(path, null_file, temp_fib, 0x08);

    *buffer = '\0';
    source_drive = temp_fib[FIB_DRIVE];	/* filled in even on "no files" */

    if (err_flag == 0)
        get_fname((char*)temp_fib + FIB_NAME, buffer);
    else if (err_flag != _NOFIL)
        error(err_flag);
}

/* get_path: find out the path to list by parsing the command line;
   parses any flags; prints the volume name header.  The original
   worked on the raw command line at 0x80; the crt0 has already
   tokenized that area, so an equivalent line is rebuilt from the argv
   items, and the original parsing code then works on it unchanged. */
void get_path(char** argv, int argc)
{
    static char vol_id[13];
    char *p, *cmd, *ptr, *path;
    byte err_flag;
    int i;

    cmd = cmd_line;
    for (i = 0; i < argc; i++) {
        if (i > 0) *cmd++ = ' ';
        for (p = argv[i]; *p; p++)
            if (cmd < cmd_line + MAX_CMD_LEN - 1)
                *cmd++ = *p;
    }
    *cmd = '\0';

    cmd = cmd_line;
    while (*cmd == ' ') cmd++;		/* strip leading blanks */

    if (*cmd == '/') {			/* only flags: list everything */
        parse_flags(cmd);
        *cmd = '\0';
    }

    ptr = cmd;
    path = s_path;
    do *path++ = *ptr; while (*ptr++);

    if ((err_flag = path_parse(0, s_path)) != 0)
        error(err_flag);

    st_file = parse_last;		/* start of the filename */

    cmd = st_file;
    if (parse_end - cmd > FIL_NAME_LEN - 1)
        error(_IFNM);			/* longer than any valid filename
					   (and f_name cannot hold it) */
    for (i = 0; i < FIL_NAME_LEN + 1; i++) f_name[i] = '\0';
    for (i = 0; cmd < parse_end; i++) f_name[i] = *cmd++;
    ptr = cmd;				/* end of the filename */

    while (*cmd == ' ') cmd++;
    if (*cmd == '/')
        parse_flags(cmd);
    else if (*cmd)
        error(_IPARM);

    *ptr = '\0';			/* cut the flags off s_path */

    /* No filename, or a wildcard in it: the search is ambiguous */
    ambig_flag = (parse_pflags & 0x20) || (parse_pflags & 0x18) == 0;

    put_msg(M_VOL1);			/* "Volume in drive " */
    get_vol_name(vol_id, s_path);
    put_char(source_drive - 1 + 'A');
    if (*vol_id) {
        put_msg(M_VOL2);		/* ": is " */
        put_string(vol_id);
    } else {
        put_msg(M_VOL3);		/* ": has no name" */
    }
    newline();
}


/*   T H E   X D I R   M O D U L E   */

/* modify_path: resolves w_path against a file name: "." leaves it
   alone, ".." removes the last component, anything else is appended.
   eg:  w_path before       fname      w_path after
          \one\two            .          \one\two
          \one\two            ..         \one
          \one                ..         \
          \                   fred       \fred
          \fred               joe        \fred\joe                   */
void modify_path(char* fname)
{
    char* ptr;

    ptr = w_path;
    while (*ptr) ptr++;

    if (str_eq(fname, "..")) {
        if (!(w_path[0] == '\\' && w_path[1] == '\0')) {
            ptr--;
            while (*ptr != '\\') ptr--;
            *ptr = '\0';
            if (w_path[0] == '\0') {
                w_path[0] = '\\';
                w_path[1] = '\0';
            }
        }
    } else if (!str_eq(fname, ".")) {
        if (*(ptr - 1) != '\\') *ptr++ = '\\';
        do *ptr++ = *fname; while (*fname++);
    }
}

/* put_file: print one file line: 13-char name field, 'h'/'r' attribute
   flags, 8-column right-aligned size (in bytes or K, see format_size);
   accumulate the size. */
void put_file(byte* fib)
{
    char buffer[FIL_NAME_LEN + 1 + 2 + 11 + 1];
    char *ptr, *bufptr;
    byte i;
    ulong size;

    for (i = 0; i < 22; i++) buffer[i] = ' ';

    bufptr = buffer;
    ptr = (char*)fib + FIB_NAME;
    while (*ptr) *bufptr++ = *ptr++;

    bufptr = buffer + FIL_NAME_LEN + 2;
    if (fib[FIB_ATTRIBUTES] & MASK_READ_ONLY) *bufptr-- = 'r';
    if (fib[FIB_ATTRIBUTES] & MASK_HIDDEN) *bufptr = 'h';

    size = *(ulong*)(fib + FIB_SIZE);
    format_size(size, buffer + FIL_NAME_LEN + 3);
    put_string(buffer);
    newline();

    total_sizes += size;
}

/* xdir: the heart of the program: a recursive descent through the
   directory structure, at each directory listing the files matching
   the pattern, then recursing into each subdirectory.  There is a
   fiddle for the first search: if the path given on the command line
   was unambiguous and matches a directory, the xdir is done IN that
   directory ("XDIR DIR1" lists "X-Directory of A:\DIR1..." rather than
   finding DIR1 itself in the root). */
void xdir(byte level, byte* search_fib)
{
    byte next_fib[FIB_LENGTH];		/* one FIB per recursion level */
    char fname[20];
    byte err_flag, attributes;
    char* ptr;

    err_flag = first(search_fib, f_name, next_fib, 0x16);

    if (level == 0) {
        byte err2_flag;

        if (err_flag && err_flag != _NOFIL) error(err_flag);

        w_path[0] = '\\';
        if ((err2_flag = get_w_path(w_path + 1)) != 0) error(err2_flag);
        ptr = parse_last;
        if (ptr == w_path + 1) *ptr = '\0'; else *(ptr - 1) = '\0';

        if (err_flag == 0 && !ambig_flag
                && (next_fib[FIB_ATTRIBUTES] & MASK_SUB_DIR)) {
            /* The path names a directory: list its contents */
            get_fname((char*)next_fib + FIB_NAME, fname);
            modify_path(fname);
            memcpy(root_fib, next_fib, FIB_LENGTH);
            search_fib = root_fib;
            f_name[0] = '\0';			/* search for *.* */
            err_flag = first(search_fib, f_name, next_fib, 0x16);
        }

        put_msg(M_XDIR);		/* "X-Directory of " */
        put_char(source_drive - 1 + 'A');
        put_char(':');
        put_string(w_path);
        newline();
        newline();

        *st_file = '\0';	/* remove the filename from s_path */
    }

    while (err_flag == 0) {
        attributes = next_fib[FIB_ATTRIBUTES];
        if (!(attributes & (MASK_SUB_DIR | MASK_SYSTEM | MASK_DEVICE))
                && (hidden_flag || !(attributes & MASK_HIDDEN))) {
            /* a matching file to display */
            put_spaces(level * 3);
            put_file(next_fib);
            f_count++;
        }
        err_flag = next(next_fib);
    }
    if (err_flag != _NOFIL) error(err_flag);

    /* Now look for subdirectories to search through */
    err_flag = first(search_fib, null_file, next_fib, 0x16);

    while (err_flag == 0) {
        attributes = next_fib[FIB_ATTRIBUTES];
        if ((attributes & MASK_SUB_DIR)
                && next_fib[FIB_NAME] != '.'
                && (hidden_flag || !(attributes & MASK_HIDDEN))) {
            put_spaces(level * 3);
            get_fname((char*)next_fib + FIB_NAME, fname);
            modify_path(fname);
            put_string(w_path);
            if (attributes & MASK_HIDDEN) put_string("        h");
            newline();

            /* recurse to xdir the files in this directory */
            xdir(level + 1, next_fib);
            modify_path("..");
        }
        err_flag = next(next_fib);
    }
    if (err_flag != _NOFIL) error(err_flag);
}


/*   M A I N   */

int main(char** argv, int argc)
{
    /* The crt0 does not zero plain globals, so initialize them here */
    kanji_flag = 0;
    hidden_flag = false;
    bytes_flag = false;
    ambig_flag = false;
    f_count = 0;
    total_sizes = 0;
    null_file[0] = '\0';

    kanji_probe();
    check_ver();

    get_path(argv, argc);
    set_size_mode();

    xdir(0, (byte*)s_path);

    /* "<n>K in " or "<n> byte(s) in " */
    put_total(total_sizes, M_KIN, M_SIZ1, M_SIZ2);
    if (!in_k(total_sizes)) put_msg(M_IN);

    put_unsigned(f_count);
    put_msg(f_count == 1 ? M_FIL1 : M_FIL2);		/* " file(s)" */

    /* "<n>K free" or "<n> bytes free" */
    put_total(get_bytes_free(source_drive), M_KFREE, M_BFREE, M_BFREE);
    return 0;
}
