/* DISKCOPY - Copy full disks sector by sector
 *
 * This is a port to SDCC C of the original MSX-DOS 2 DISKCOPY program by
 * IS Systems / ASCII Corporation (which lived in source/command/diskcopy
 * in older revisions of this repository, written in c80 C), adapted to
 * the Nextor 3 tools conventions (a single binary with both message
 * languages, Kanji mode probed once at startup; see also the banner in
 * crt0_diskcopy.asm, which TYPE DISKCOPY.COM displays).
 *
 * Syntax: DISKCOPY src: [tgt:] [/X] [/S]
 *
 *   When the target drive is omitted, the current drive is used.
 *   Source and target may be the same drive: the copy is then done in
 *   passes through the memory buffer, prompting to swap the disks
 *   ("(pass X of Y)" on the source prompts; the first one carries no
 *   count, the total is only known once the source disk parameters
 *   have been read).
 *   /X suppresses all the prompts (the "insert disks + press a key" wait
 *   and the "copy more disks" loop) except the swap prompts of a
 *   same-drive copy, which are the copy mechanism itself; /S copies the
 *   boot sector from the source disk instead of keeping the target's
 *   one.
 *
 * Differences from the original program:
 *
 * - The source drive is mandatory: with no arguments (or with just a
 *   switch) the usage text is displayed and the program exits with no
 *   error, following the convention of the other ported tools (the
 *   original prompted for the missing drives instead).
 * - The same drive may be given for source and target (the original
 *   refused it): the swap-based single-drive copy above.  The "can't
 *   copy disk onto itself" error remains for two different drive
 *   letters mapped to the same physical drive.
 * - The drives are gated (both of them, same rule as KMODE /S and
 *   FIXDISK): on plain MSX-DOS 2 every drive is allowed (the original
 *   program's scope); under Nextor only drives mapped to MSX-DOS
 *   drivers are allowed, plus (on Nextor 3 or later) drives mapped to
 *   Nextor devices flagged as floppy disk drives by their driver
 *   (DEVICE_QUERY "get device parameters", flags bit 2) and ghost
 *   drives (whose main drive is always a floppy disk device).
 *   Whole-disk raw copying makes no sense on the other drives (hard
 *   disk partitions, mounted files, the RAM disk), whose layout is
 *   managed by Nextor.
 * - Sector I/O uses _RDABS/_WRABS on plain MSX-DOS 2, and
 *   _RDDRV/_WRDRV under Nextor (any version: they exist since Nextor
 *   2.0, where _RDABS/_WRABS are deprecated and refuse FAT16 drives).
 * - The volume id check ("Target disk is not an MSX-DOS 2 disk", given
 *   when the source has a volume id but the target does not) now tests
 *   the volume id reported by _DPARM against "no id" (FFh in its first
 *   byte).  The original tested "first byte <= 127", which would reject
 *   disks with standard (non MSX-DOS 2.20) boot sectors whose serial
 *   number begins with 80h-FEh; Nextor reports those serials as the
 *   volume id and they must be accepted.
 * - When the target keeps its own boot sector (no /S), the "dirty disk"
 *   flag patched into it is written at the offset of the target's boot
 *   sector style: 26h for MSX-DOS 2.20 "VOL_ID" boot sectors (the only
 *   style the original knew), 25h for standard boot sectors with the
 *   extended block.
 *
 * Everything else works as the original: same syntax, same prompts, same
 * compatibility checks, same error handling (recoverable disk errors are
 * reported, counted and ignored, so that as much of the disk as possible
 * is copied), disk check (DSKCHK) disabled during the copy and restored
 * on any exit through the _DEFAB abort routine.
 *
 * Exit codes: 0 = success, then the original internal codes 1 = disks
 * incompatible, 2 = wrong sector size, 3 = same drive, 4 = target has no
 * volume id, 5 = wrong MSX-DOS version, plus the new 6 = drive not
 * supported (the gate above).  DOS error codes (invalid parameter etc.)
 * are returned as such.
 */

#include <string.h>
#include "asmcall.h"
#include "types.h"
#include "dos_functions.h"
#include "dos_errors.h"
#include "driver_routines.h"
#include "driver_device_queries.h"


/*   C O N S T A N T S   */

#define MAX_CMD_LEN	129	/* maximum command line length (+1) */

#define SECT_LENGTH	512	/* length of a sector in bytes */

/* Disk parameter (_DPARM) offsets */
#define DP_DRIVE	0	/* physical drive number */
#define DP_DDF		19	/* dirty disk flag */
#define DP_VOL_ID	20	/* volume id (FFh in byte 0 = no id) */

/* Offsets in the boot sector */
#define BS_VOLID_STR	0x20	/* "VOL_ID" mark of DOS 2.20 boot sectors */
#define BS_DIRT_VOLID	0x26	/* dirty flag, "VOL_ID" style */
#define BS_EBS_FLAG	0x26	/* extended block signature, standard style */
#define BS_DIRT_EBS	0x25	/* dirty flag, standard style */
#define BS_ID		0x27	/* volume id / serial (same in both styles) */

/* Errors reported to the _DEFER routine that are to be reported,
   counted, and ignored. */
#define ERR_WR		0xFE	/* write protected */
#define ERR_VR		0xFB	/* verify error */
#define ERR_DATA	0xFA	/* data (CRC) error */
#define ERR_RNF		0xF9	/* sector not found */
#define ERR_SEEK	0xF3	/* seek error */

/* Internal diskcopy errors: unique integers below 32 (so COMMAND.COM
   will not print messages for them). */
#define E_INCOM		1
#define E_WR_SSIZ	2
#define E_SAM_DRIVE	3
#define E_NO_ID		4
#define E_WRONG_VER	5
#define E_NSUP		6	/* new: drive not supported */

#define STACK_SLACK	512	/* room left for the stack above the buffer */

/* Nextor driver slot byte flag: bits 6-4 = 001, the "Nextor 3 driver
   structure awareness" mark required by _CDRVR. */
#define CDRVR_SLOT_FLAG	0x10


/*   M E S S A G E S   */

/* The message texts live in diskcopy_msgs.mac (assembled with Nestor80
   in SDCC relocatable mode and linked in; the Japanese table is
   converted to Shift-JIS at assembly time by its .strenc directive).
   Each table is a sequence of zero-terminated strings; the message
   numbers below are ordinal positions in the tables, so THE ORDER OF
   THIS ENUM AND OF THE TABLES IN diskcopy_msgs.mac MUST MATCH EXACTLY
   (the count parity between the two tables is checked at assembly
   time). */

enum {
    M_WARN, M_ERS, M_FIN, M_FROM, M_TO, M_KEY,
    M_WER, M_RER, M_VER, M_RNF, M_SEEK,
    M_E_INCOM, M_E_WR_SSIZ, M_E_SAM_DR, M_E_NO_ID, M_E_VER,
    M_MORE,
    M_NSUP1, M_NSUP2, M_USAG,
    M_PASS1, M_PASS2, M_PASS3
};

extern const byte msgs_en[];	/* defined in diskcopy_msgs.mac */
extern const byte msgs_ja[];

static const char yes_chars[] = "Yy";
static const char no_chars[]  = "Nn";


/*   G L O B A L   V A R I A B L E S   */

Z80_registers regs;

byte kanji_flag;		/* non-zero => print Japanese messages */
byte nextor_ver;		/* 0 = plain MSX-DOS 2, else Nextor major */

byte source_drive;		/* logical source drive (1 = A: etc) */
byte target_drive;		/* logical target drive */

byte o_chk;			/* DSKCHK value to restore on exit */
bool prompt_flag;		/* true => give warnings and loop */
bool cpboot_flag;		/* true => copy the boot sector (/S) */
bool single_mode;		/* true => source and target are the
				   same drive (swap-based copy) */

uint e_count;			/* errors detected during the copy */
uint err_sec;			/* sector number of the current error */
byte err_num;			/* error code of the current error */
byte ret_action;		/* action returned to the error routine */

uint num_sects;			/* number of sectors to copy */
uint s_size;			/* sector size of the source drive */

byte* buf_adr;			/* start of the copy buffer */
uint num_buffers;		/* how many sectors fit in it */

uint stack_top_raw;		/* SP as sampled by read_sp() */
byte term_code;			/* exit code for do_terminate() */

byte src_dr_info[32];		/* _DPARM data of the source drive */
byte trg_dr_info[32];		/* _DPARM data of the target drive */
byte boot_sector[SECT_LENGTH];

byte gdli_buf[64];		/* _GDLI drive information */
byte dev_buf[12];		/* DEV_QUERY device parameters */
byte reg_buf[8];		/* _CDRVR register buffer (F,A,C,B,E,D,L,H) */

static char cmd_line[MAX_CMD_LEN];

extern byte HEAP_start;		/* first free byte after the program */


/*   P R O T O T Y P E S   */

void kanji_probe(void);
void put_char(char c);
void read_sp(void);
void defer_handler(void);
void abort_handler(void);
void do_terminate(void);

void do_err_c(void);
void restore_dchk_c(void);
void error(byte num);
void terminate(byte code);
byte disk_rw(bool writing, uint sector, uint count, byte drive, byte* buffer);

#define read_sectors(s, n, d, b)  disk_rw(false, (s), (n), (d), (b))
#define write_sectors(s, n, d, b) disk_rw(true, (s), (n), (d), (b))


/*   B A S I C   P R E D I C A T E S   */

bool islowercase(char c) { return c >= 'a' && c <= 'z'; }
char upper(char c) { return islowercase(c) ? c - 'a' + 'A' : c; }
bool isdrive_ch(char c) { return c >= 'A' && c <= 'Z'; }

bool in_set(char c, const char* set)
{
    while (*set)
        if (*set++ == c) return true;
    return false;
}

bool yes(char c) { return in_set(c, yes_chars); }
bool is_yes_or_no(char c) { return yes(c) || in_set(c, no_chars); }

/* is_VOL_ID: true if the _DPARM data reports a volume id.  "No id" is
   FFh in the first byte (the original tested <= 127, see the header). */
bool is_VOL_ID(byte* dr_info) { return dr_info[DP_VOL_ID] != 0xFF; }


/*   M E S S A G E   A N D   C O N S O L E   O U T P U T   */

/* put_char goes straight to the BDOS console output function, without
   the AsmCall machinery: it is also called from do_err_c(), which runs
   inside a disk BDOS call (through the _DEFER routine), and a nested
   AsmCall would corrupt the outer call's OUT_FLAGS state. */
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

void put_unsigned(uint i)
{
    if (i >= 10)
        put_unsigned(i / 10);
    put_char((char)(i % 10) + '0');
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
   put the whole stack at the top of the TPA (see crt0_diskcopy.asm),
   so no stack switching is needed (or allowed: moving SP up to (0006h)
   from a routine would overwrite the live stack data below it). */
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

/* check_ver: require MSX-DOS 2 or better (as the original), and find
   out whether this is Nextor, and which major version, for the drive
   gate and the sector I/O function choice. */
void check_ver(void)
{
    regs.Bytes.A = 0;
    regs.Bytes.B = 0x5A;	/* magic numbers that make Nextor */
    regs.UWords.HL = 0x1234;	/* return its version in IX/IY */
    regs.UWords.DE = 0xABCD;
    regs.UWords.IX = 0;
    regs.UWords.IY = 0;
    DosCall(_DOSVER, &regs, REGS_ALL, REGS_ALL);
    if (regs.Bytes.A != 0 || regs.Bytes.B < 2)
        error(E_WRONG_VER);
    if (regs.Bytes.IXh == 1)
        nextor_ver = regs.Bytes.IXl;	/* the major version */
}


/*   T H E   D R I V E   G A T E   */

/* device_is_floppy: ask the Nextor driver of the drive described in
   gdli_buf whether its device is flagged as a floppy disk drive
   (DEV_QUERY "get device parameters", flags byte bit 2).  Any result
   other than OK (including "not implemented", which stands for default
   parameters with no flags) means "not a floppy". */
bool device_is_floppy(void)
{
    reg_buf[1] = DEVICE_QUERY_GET_PARAMS;	/* A = query code */
    reg_buf[2] = gdli_buf[4];			/* C = device index */
    reg_buf[6] = (byte)((uint)dev_buf & 0xFF);	/* L,H = parameters buffer */
    reg_buf[7] = (byte)((uint)dev_buf >> 8);
    regs.Bytes.A = gdli_buf[1] | CDRVR_SLOT_FLAG;	/* driver slot */
    regs.Bytes.B = gdli_buf[2];			/* driver segment */
    regs.UWords.DE = DRIVER_DEVICE_QUERY_ENTRY;
    regs.UWords.HL = (uint)reg_buf;
    DosCall(_CDRVR, &regs, REGS_MAIN, REGS_ALL);
    if (regs.Bytes.A != 0) return false;
    if (regs.Bytes.IXh != 0) return false;	/* the A of the driver routine */
    return (dev_buf[7] & 0x04) != 0;
}

/* check_drive_supported: on plain MSX-DOS 2 every drive is allowed (the
   original program's scope); under Nextor the drive must be mapped to an
   MSX-DOS driver, or (on Nextor 3 or later) to a Nextor device flagged
   as a floppy disk drive, or be a ghost drive (whose main drive is
   always a floppy disk device); see the file header.  Terminates with a
   message and exit code 6 otherwise. */
void check_drive_supported(byte drive)
{
    if (nextor_ver == 0)		/* plain MSX-DOS 2: no gate */
        return;				/* (and no _GDLI to ask) */

    regs.Bytes.A = drive - 1;
    regs.UWords.HL = (uint)gdli_buf;
    DosCall(_GDLI, &regs, REGS_MAIN, REGS_AF);
    if (regs.Bytes.A != 0)
        terminate(regs.Bytes.A);	/* e.g. "Invalid drive" */

    if (gdli_buf[0] == 5)		/* ghost drive */
        return;
    if (gdli_buf[0] == 1) {		/* assigned to a storage device */
        if (gdli_buf[3] != 0xFF)	/* relative drive: a real number */
            return;			/* means an MSX-DOS driver */
        if (nextor_ver >= 3 && device_is_floppy())
            return;			/* Nextor devices need Nextor 3+ */
    }
    /* Unassigned, mounted file, RAM disk, non-floppy Nextor device, or
       any Nextor device below Nextor 3 */
    put_msg(M_NSUP1);
    put_char(drive + 'A' - 1);
    put_msg(M_NSUP2);
    terminate(E_NSUP);
}


/*   G E T   D R I V E S   */

/* parse_flags: parses a set of flags of the form "/x/s /x"; the first
   character pointed to must be non-blank. */
void parse_flags(char* ptr)
{
    while (*ptr) {
        if (*ptr != '/') error(_IPARM);
        switch (*(++ptr)) {
            case 'X': prompt_flag = false;
                      break;
            case 'S': cpboot_flag = true;
                      break;
            case 0  : error(_IPARM);
            default : error(_IOPT);
        }
        while (*(++ptr) == ' ') ;
    }
}

byte get_default_drive(void)
{
    DosCall(_CURDRV, &regs, REGS_MAIN, REGS_AF);
    return regs.Bytes.A + 1;
}

/* get_drives: find out the source and target drives by parsing the
   command line; parses any flags.  With no arguments, or with only
   switches, the usage text is displayed and the program exits with no
   error (the original prompted for the drives instead; this is the
   convention of the other ported tools).  The original worked on the
   raw command line at 0x80; the crt0 has already tokenized that area,
   so an equivalent line is rebuilt from the argv items (upper cased,
   single spaces), and the original parsing code then works on it
   unchanged. */
void get_drives(char** argv, int argc)
{
    char *p, *cmd;
    int i;

    cmd = cmd_line;
    for (i = 0; i < argc; i++) {
        if (i > 0) *cmd++ = ' ';
        for (p = argv[i]; *p; p++)
            if (cmd < cmd_line + MAX_CMD_LEN - 1)
                *cmd++ = upper(*p);
    }
    *cmd = '\0';

    cmd = cmd_line;
    while (*cmd == ' ') cmd++;		/* strip leading blanks */
    if (*cmd == '\0' || *cmd == '/') {
        /* No arguments, or a switch without a drive: show the usage
           text and exit successfully */
        put_msg(M_USAG);
        terminate(0);
    }
    if (!isdrive_ch(*cmd)) error(_IPARM);

    /* First parameter is a drive name.  Check for ':' */
    source_drive = (*cmd) - 'A' + 1;
    if (*(++cmd) != ':') error(_IPARM);
    do cmd++; while (*cmd == ' ');	/* strip blanks */
    if (isdrive_ch(*cmd)) {
        /* We have the target drive specified as well */
        target_drive = (*cmd) - 'A' + 1;
        if (*(++cmd) != ':') error(_IPARM);
        /* strip blanks: anything left must be flags */
        do ++cmd; while (*cmd == ' ');
        if (*cmd) parse_flags(cmd);
    } else if (*cmd) {
        /* Only the source is specified: pick up the flags and let
           the target default to the current drive */
        parse_flags(cmd);
        target_drive = get_default_drive();
    } else {
        /* Only one drive given, and no flags, so the target
           defaults to the current drive */
        target_drive = get_default_drive();
    }

    /* The same drive for source and target selects the swap-based
       single-drive copy (the original refused this) */
    single_mode = source_drive == target_drive;
}


/*   C H E C K   D R I V E S   */

/* get_info: _DPARM into the given buffer; sets s_size, returns the
   total number of sectors.  Terminates on any error (e.g. not ready),
   letting COMMAND print the message, as the original did. */
uint get_info(byte drive, byte* info)
{
    regs.Bytes.L = drive;
    regs.UWords.DE = (uint)info;
    DosCall(_DPARM, &regs, REGS_MAIN, REGS_AF);
    if (regs.Bytes.A != 0)
        terminate(regs.Bytes.A);
    s_size = info[1] | (info[2] << 8);
    return info[9] | (info[10] << 8);
}

/* check_compatible: the two disks must have identical geometry (bytes
   1..12 of _DPARM) and 512-byte sectors. */
void check_compatible(void)
{
    byte i;

    for (i = 1; i < 13; i++)
        if (src_dr_info[i] != trg_dr_info[i]) error(E_INCOM);
    if (s_size != SECT_LENGTH) error(E_WR_SSIZ);
}

/* chk_drives: read both drives' parameters (fresh on every pass of the
   copy loop: the disks may have been swapped), check that the disks are
   compatible, and set num_sects. */
void chk_drives(void)
{
    get_info(source_drive, src_dr_info);	/* also sets s_size */
    num_sects = get_info(target_drive, trg_dr_info);

    check_compatible();

    if (src_dr_info[DP_DRIVE] == trg_dr_info[DP_DRIVE])
        error(E_SAM_DRIVE);	/* both on the same physical drive */

    if (is_VOL_ID(src_dr_info) && !is_VOL_ID(trg_dr_info))
        error(E_NO_ID);		/* volume id on source but not target */
}


/*   C O P Y   D A T A   */

/* read_sectors/write_sectors (through disk_rw): transfer sectors with
   _RDDRV/_WRDRV under Nextor (they exist since Nextor 2.0), with
   _RDABS/_WRABS on plain MSX-DOS 2; returns the error code (0 =
   success). */
byte disk_rw(bool writing, uint sector, uint count, byte drive, byte* buffer)
{
    regs.UWords.DE = (uint)buffer;
    DosCall(_SETDTA, &regs, REGS_MAIN, REGS_NONE);
    if (nextor_ver != 0) {
        regs.Bytes.A = drive - 1;
        regs.Bytes.B = (byte)count;
        regs.UWords.DE = sector;	/* 32 bit sector number in DE:HL, */
        regs.UWords.HL = 0;		/* always < 65536 here */
        DosCall(writing ? _WRDRV : _RDDRV, &regs, REGS_MAIN, REGS_AF);
    } else {
        regs.Bytes.L = drive - 1;
        regs.Bytes.H = (byte)count;
        regs.UWords.DE = sector;
        DosCall(writing ? _WRABS : _RDABS, &regs, REGS_MAIN, REGS_AF);
    }
    return regs.Bytes.A;
}

void read_sp(void) __naked
{
    __asm
	ld	(_stack_top_raw),sp
	ret
    __endasm;
}

/* alloc_buffers: the copy buffer takes all the memory between the end
   of the program and the stack, 512-byte aligned (so no sector crosses
   a 16K page boundary). */
uint alloc_buffers(void)
{
    uint high_mem, start, count;

    read_sp();
    high_mem = (stack_top_raw - STACK_SLACK) & 0xFE00;
    start = ((uint)&HEAP_start + 511) & 0xFE00;
    if (start >= high_mem) error(_NORAM);
    buf_adr = (byte*)start;
    count = (high_mem - start) >> 9;
    if (count > 255) count = 255;	/* sector count fits in a byte */
    return count;
}

/* patch_boot: adjust the boot sector image in boot_sector before it is
   written to the target.  Without /S the buffer holds the target's own
   boot sector and gets the "dirty disk" flag from the source, at the
   offset of the target's boot sector style (see the file header); with
   /S it holds the source's boot sector and gets the target's old
   volume id back (both styles keep the id at 27h). */
void patch_boot(void)
{
    byte i;

    if (!is_VOL_ID(src_dr_info))
        return;
    if (!cpboot_flag) {
        if (memcmp(boot_sector + BS_VOLID_STR, "VOL_ID", 6) == 0)
            boot_sector[BS_DIRT_VOLID] = src_dr_info[DP_DDF];
        else if ((boot_sector[BS_EBS_FLAG] & 0x28) == 0x28)
            boot_sector[BS_DIRT_EBS] = src_dr_info[DP_DDF];
    } else {
        for (i = 0; i < 4; i++)
            boot_sector[BS_ID + i] = trg_dr_info[DP_VOL_ID + i];
    }
}

/* copy_data: copy the entire disk in buffer-sized batches (source and
   target on different drives).  The boot sector (sector 0) is special:
   without /S the target keeps its own (read before the copy, written
   back after the first batch is read); with /S it comes from the
   source; see patch_boot. */
void copy_data(void)
{
    uint sect, num_now;
    byte err;

    num_buffers = alloc_buffers();

    if ((err = read_sectors(0, 1, cpboot_flag ? source_drive : target_drive,
                            boot_sector)) != 0)
        error(err);
    patch_boot();

    sect = 1;
    while (sect < num_sects) {
        num_now = num_sects - sect;
        if (num_now > num_buffers) num_now = num_buffers;
        if ((err = read_sectors(sect, num_now, source_drive, buf_adr)) != 0)
            error(err);
        if (sect == 1)
            if ((err = write_sectors(0, 1, target_drive, boot_sector)) != 0)
                error(err);
        if ((err = write_sectors(sect, num_now, target_drive, buf_adr)) != 0)
            error(err);
        sect += num_now;
    }
}

/* swap_prompt: the "insert the source/target disk + press a key" wait
   of the single-drive copy.  A non-zero pass number appends the
   "(pass X of Y)" suffix. */
void swap_prompt(byte msg, uint pass, uint total)
{
    newline();
    put_msg(msg);
    put_char(source_drive + 'A' - 1);
    put_char(':');
    if (pass != 0) {
        put_msg(M_PASS1);
        put_unsigned(pass);
        put_msg(M_PASS2);
        put_unsigned(total);
        put_msg(M_PASS3);
    }
    newline();
    put_msg(M_KEY);
    clr_in();
    get_char();
    newline();
}

/* copy_single: the same-drive copy: the disk is copied in passes
   through the memory buffer, prompting for the source and the target
   disks alternately (always, even with /X: the swaps are the copy
   mechanism).  The compatibility and volume id checks run when the
   target disk is first inserted, before anything is written.  The
   first source prompt carries no pass count: the total is only known
   once the source disk parameters have been read. */
void copy_single(void)
{
    uint sect, num_now, pass, total_passes;
    byte err;

    num_buffers = alloc_buffers();

    /* Pass 1, source phase */
    swap_prompt(M_FROM, 0, 0);
    num_sects = get_info(source_drive, src_dr_info);
    if (s_size != SECT_LENGTH) error(E_WR_SSIZ);
    total_passes = (num_sects - 1) / num_buffers;
    if ((num_sects - 1) % num_buffers != 0) total_passes++;

    if (cpboot_flag)
        if ((err = read_sectors(0, 1, source_drive, boot_sector)) != 0)
            error(err);
    num_now = num_sects - 1;
    if (num_now > num_buffers) num_now = num_buffers;
    if ((err = read_sectors(1, num_now, source_drive, buf_adr)) != 0)
        error(err);

    /* Pass 1, target phase: checks first (nothing written yet) */
    swap_prompt(M_TO, 0, 0);
    get_info(target_drive, trg_dr_info);
    check_compatible();
    if (is_VOL_ID(src_dr_info) && !is_VOL_ID(trg_dr_info))
        error(E_NO_ID);

    if (!cpboot_flag)
        if ((err = read_sectors(0, 1, target_drive, boot_sector)) != 0)
            error(err);
    patch_boot();
    if ((err = write_sectors(0, 1, target_drive, boot_sector)) != 0)
        error(err);
    if ((err = write_sectors(1, num_now, target_drive, buf_adr)) != 0)
        error(err);
    sect = 1 + num_now;

    /* The remaining passes */
    pass = 2;
    while (sect < num_sects) {
        swap_prompt(M_FROM, pass, total_passes);
        num_now = num_sects - sect;
        if (num_now > num_buffers) num_now = num_buffers;
        if ((err = read_sectors(sect, num_now, source_drive, buf_adr)) != 0)
            error(err);
        swap_prompt(M_TO, 0, 0);
        if ((err = write_sectors(sect, num_now, target_drive, buf_adr)) != 0)
            error(err);
        sect += num_now;
        pass++;
    }
}


/*   D I S K   E R R O R   H A N D L I N G   */

/* defer_handler: entered by the kernel (through _DEFER) when a disk
   error occurs during the copy.  A = error code, C = flags (bit 3 set
   when DE = sector number is valid), B = physical drive.  Returns in A
   the action: 3 = ignore, 0 = use the normal system error routine. */
void defer_handler(void) __naked
{
    __asm
	bit	3,c		;Sector number valid?
	jr	z,dh_normal	;If not, do the normal error.
	ld	(_err_num),a
	ld	(_err_sec),de
	push	ix
	push	iy
	call	_do_err_c	;Report and decide in C.
	pop	iy
	pop	ix
	ld	a,(_ret_action)
	ret
dh_normal:
	xor	a
	ret
    __endasm;
}

/* do_err_c: report recoverable errors (with the sector number) and
   ignore them, counting them; let the system handle anything else.
   Runs inside a disk BDOS call: console output only (put_char goes
   straight to the BDOS entry, see above). */
void do_err_c(void)
{
    ret_action = 3;		/* ignore the error if possible */
    switch (err_num) {
        case ERR_WR:   put_msg(M_WER);  break;
        case ERR_VR:   put_msg(M_VER);  break;
        case ERR_DATA: put_msg(M_RER);  break;
        case ERR_RNF:  put_msg(M_RNF);  break;
        case ERR_SEEK: put_msg(M_SEEK); break;
        default: ret_action = 0; return;	/* normal error routine */
    }
    put_unsigned(err_sec);
    newline();
    e_count++;
}

void set_error_routine(void)
{
    regs.UWords.DE = (uint)defer_handler;
    DosCall(_DEFER, &regs, REGS_MAIN, REGS_NONE);
}

void unset_error_routine(void)
{
    regs.UWords.DE = 0;
    DosCall(_DEFER, &regs, REGS_MAIN, REGS_NONE);
}


/*   D S K C H K   A N D   T H E   A B O R T   R O U T I N E   */

byte get_dchk(void)
{
    regs.Bytes.A = 0;		/* get */
    regs.Bytes.B = 0;
    DosCall(_DSKCHK, &regs, REGS_MAIN, REGS_MAIN);
    return regs.Bytes.B;
}

void set_chk(byte value)
{
    regs.Bytes.A = 1;		/* set */
    regs.Bytes.B = value;
    DosCall(_DSKCHK, &regs, REGS_MAIN, REGS_AF);
}

/* abort_handler: entered (through _DEFAB) whenever the program
   terminates, normally or not: restore the disk check state. */
void abort_handler(void) __naked
{
    __asm
	push	af
	push	bc
	push	de
	push	hl
	push	ix
	push	iy
	call	_restore_dchk_c
	pop	iy
	pop	ix
	pop	hl
	pop	de
	pop	bc
	pop	af
	ret
    __endasm;
}

void restore_dchk_c(void)
{
    set_chk(o_chk);
}

void set_abort_routine(void)
{
    regs.UWords.DE = (uint)abort_handler;
    DosCall(_DEFAB, &regs, REGS_MAIN, REGS_NONE);
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
    switch (num) {
        case E_INCOM:     put_msg(M_E_INCOM);   break;
        case E_WR_SSIZ:   put_msg(M_E_WR_SSIZ); break;
        case E_SAM_DRIVE: put_msg(M_E_SAM_DR);  break;
        case E_NO_ID:     put_msg(M_E_NO_ID);   break;
        case E_WRONG_VER: put_msg(M_E_VER);     break;
    }
    newline();
    terminate(num);
}


/*   U S E R   I N T E R A C T I O N   */

/* yes_ans: print a message and wait for a Y/N answer */
bool yes_ans(byte mes)
{
    char ans;

    do {
        put_msg(mes);
        clr_in();
        ans = get_char();
        newline();
    } while (!is_yes_or_no(ans));
    return yes(ans);
}

/* wait_for_ready: say what is about to happen, wait for a key */
void wait_for_ready(void)
{
    newline();
    put_msg(M_FROM); put_char(source_drive + 'A' - 1); put_char(':'); newline();
    put_msg(M_TO);   put_char(target_drive + 'A' - 1); put_char(':'); newline();
    put_msg(M_KEY);  clr_in(); get_char(); newline();
}


/*   M A I N   */

int main(char** argv, int argc)
{
    prompt_flag = true;		/* by default we warn the user */
    cpboot_flag = false;	/* by default we don't copy the boot */
    nextor_ver = 0;		/* (the crt0 does not zero plain
				   globals, so set it here) */
    kanji_probe();
    check_ver();

    get_drives(argv, argc);

    check_drive_supported(source_drive);
    check_drive_supported(target_drive);

    o_chk = get_dchk();		/* read before registering the abort */
    set_abort_routine();	/* routine that restores it */
    set_chk(0xFF);		/* disable disk checking for the copy */

    do {
        e_count = 0;

        if (single_mode) {
            set_error_routine();
            copy_single();	/* prompting and checks are inside */
            unset_error_routine();
        } else {
            if (prompt_flag) wait_for_ready();

            chk_drives();

            set_error_routine();
            copy_data();
            unset_error_routine();
        }

        if (e_count != 0) {
            put_msg(M_WARN);
            put_unsigned(e_count);
            put_msg(M_ERS);
        } else {
            put_msg(M_FIN);
        }
        newline();
    } while (prompt_flag && yes_ans(M_MORE));
    newline();
    return 0;
}
