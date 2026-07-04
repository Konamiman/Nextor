;-----------------------------------------------------------------------------
;
; DOS function call codes.
;
;-----------------------------------------------------------------------------

	include ../macros/const.inc

    ; Function calls inherited from MSX-DOS 1

	const	_TERM0,   00h	;Terminate program
	const	_CONIN,   01h	;Console input
	const	_CONOUT,  02h	;Console output
	const	_AUXIN,   03h	;Auxilliary input
	const	_AUXOUT,  04h	;Auxilliary output
	const	_LSTOUT,  05h	;List output
	const	_DIRIO,   06h	;Direct console I/O
	const	_DIRIN,   07h	;Direct console input, no echo
	const	_INNOE,   08h	;Console input, no echo
	const	_STROUT,  09h	;String output
	const	_BUFIN,   0Ah	;Buffered line input
	const	_CONST,   0Bh	;Console status
;
	const	_CPMVER,  0Ch	;Return CP/M version number
	const	_DSKRST,  0Dh	;Disk reset
	const	_SELDSK,  0Eh	;Select disk
;
	const	_FOPEN,   0Fh	;Open file (FCB)
	const	_FCLOSE,  10h	;Close file (FCB)
	const	_SFIRST,  11h	;Search for first (FCB)
	const	_SNEXT,   12h	;Search for next (FCB)
	const	_FDEL,    13h	;Delete file (FCB)
	const	_RDSEQ,   14h	;Read sequential (FCB)
	const	_WRSEQ,   15h	;Write sequential (FCB)
	const	_FMAKE,   16h	;Create file (FCB)
	const	_FREN,    17h	;Rename file (FCB)
;
	const	_LOGIN,   18h	;Get login vector
	const	_CURDRV,  19h	;Get current drive
	const	_SETDTA,  1Ah	;Set disk transfer address
	const	_ALLOC,   1Bh	;Get allocation information
;
	const	_RDRND,   21h	;Read random (FCB)
	const	_WRRND,   22h	;Write random (FCB)
	const	_FSIZE,   23h	;Get file size (FCB)
	const	_SETRND,  24h	;Set random record (FCB)
	const	_WRBLK,   26h	;Write random block (FCB)
	const	_RDBLK,   27h	;Read random block (FCB)
	const	_WRZER,   28h	;Write random with zero fill (FCB)
;
	const	_GDATE,   2Ah	;Get date
	const	_SDATE,   2Bh	;Set date
	const	_GTIME,   2Ch	;Get time
	const	_STIME,   2Dh	;Set time
	const	_VERIFY,  2Eh	;Set/reset verify flag
;
	const	_RDABS,   2Fh	;Absolute sector read
	const	_WRABS,   30h	;Absolute sector write

	; Function calls inherited from MSX-DOS 2

	const	_DPARM,   31h	;Get disk parameters
;
	const	_FFIRST,  40h	;Find first entry
	const	_FNEXT,   41h	;Find next entry
	const	_FNEW,    42h	;Find new entry
;
	const	_OPEN,    43h	;Open file handle
	const	_CREATE,  44h	;Create file and open handle
	const	_CLOSE,   45h	;Close file handle
	const	_ENSURE,  46h	;Ensure file handle
	const	_DUP,     47h	;Duplicate file handle
	const	_READ,    48h	;Read from file handle
	const	_WRITE,   49h	;Write to file handle
	const	_SEEK,    4Ah	;Seek (position file pointer)
	const	_IOCTL,   4Bh	;I/O control for devices
	const	_HTEST,   4Ch	;Test file handle
;
	const	_DELETE,  4Dh	;Delete file or subdirectory
	const	_RENAME,  4Eh	;Rename file or subdirectory
	const	_MOVE,    4Fh	;Move file or subdirectory
	const	_ATTR,    50h	;Change file or subdirectory attributes
	const	_FTIME,   51h	;Get/set file date and time
;
	const	_HDELETE, 52h	;Delete file handle
	const	_HRENAME, 53h	;Rename file handle
	const	_HMOVE,   54h	;Move file handle
	const	_HATTR,   55h	;Change file handle attributes
	const	_HFTIME,  56h	;Get/set file handle date and time
;
	const	_GETDTA,  57h	;Get disk transfer address
	const	_GETVFY,  58h	;Get verify flag setting
	const	_GETCD,   59h	;Get current directory
	const	_CHDIR,   5Ah	;Change directory
	const	_PARSE,   5Bh	;Parse pathname
	const	_PFILE,   5Ch	;Parse filename
	const	_CHKCHR,  5Dh	;Check character
	const	_WPATH,   5Eh	;Get whole path string
	const	_FLUSH,   5Fh	;Flush disk buffers
;
	const	_FORK,    60h	;Fork a child process
	const	_JOIN,    61h	;Return to parent process
	const	_TERM,    62h	;Terminate with error code
	const	_DEFAB,   63h	;Define abort exit routine
	const	_DEFER,   64h	;Define critical error handle routine
	const	_ERROR,   65h	;Get previous error code
	const	_EXPLAIN, 66h	;Explain error code
;
	const	_FORMAT,  67h	;Format disk
	const	_RAMD,    68h	;Create or destroy RAMdisk
	const	_BUFFER,  69h	;Allocate sector buffers
	const	_ASSIGN,  6Ah	;Logical drive assignment
;
	const	_GENV,    6Bh	;Get environment item
	const	_SENV,    6Ch	;Set environment item
	const	_FENV,    6Dh	;Find environment item
;
	const	_DSKCHK,  6Eh	;Get/set disk check status
	const	_DOSVER,  6Fh	;Get MSX-DOS version number
	const	_REDIR,   70h	;Get/set redirection flags

    ; Function calls introduced in Nextor

	const	_FOUT,	  71h   ;Get/set fast STROUT mode
    const	_ZSTROUT, 72h	;Zero-terminated string output

	const	_RDDRV,   73h	;Absolute drive sector read
	const	_WRDRV,   74h	;Absolute drive sector write

	const	_RALLOC,  75h	;Reduced allocation information mode vector
	const	_DSPACE,  76h	;Get disk space information

	const	_LOCK,   77h	;Lock/unlock drive

	const	_GDRVR,	  78h	;Get information about disk driver
	const	_GDLI,    79h	;Get information about drive letter
	const	_GPART,   7Ah	;Get information about disk partition
	const	_CDRVR,	  7Bh	;Call a routine in a disk driver
	const	_MAPDRV,  7Ch   ;Map a drive letter to a drive or device/partition

	const	_Z80MODE, 7Dh	;Enable or disable the Z80 access mode for a driver
    const   _GETCLUS, 7Eh   ;Get information about a cluster on a FAT drive

	const	_DRVRO,   7Fh	;Operate on a RAM-loaded driver


