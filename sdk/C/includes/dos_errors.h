/*
 * Nextor SDK - DOS error codes.
 *
 * The names mirror the asm SDK file sdk/asm/constants/dos_errors.inc;
 * each error code uses an underscore prefix in place of the asm-side
 * dot prefix (asm `.NCOMP` -> C `_NCOMP`).
 */

#ifndef __DOS_ERRORS_H
#define __DOS_ERRORS_H

/* ----------------------------------------------------------------- */
/*  Disk error codes (returned to a disk error routine)              */
/* ----------------------------------------------------------------- */

#define _NCOMP     0xFF  /* Incompatible disk                       */
#define _WRERR     0xFE  /* Write error                             */
#define _DISK      0xFD  /* Disk error                              */
#define _NRDY      0xFC  /* Not ready                               */
#define _VERFY     0xFB  /* Verify error                            */
#define _DATA      0xFA  /* Data error                              */
#define _RNF       0xF9  /* Sector not found                        */
#define _WPROT     0xF8  /* Write protected disk                    */
#define _UFORM     0xF7  /* Unformatted disk                        */
#define _NDOS      0xF6  /* Not a DOS disk                          */
#define _WDISK     0xF5  /* Wrong disk                              */
#define _WFILE     0xF4  /* Wrong disk for file                     */
#define _SEEK_ERR  0xF3  /* Seek error (renamed from _SEEK to avoid clash with the DOS function code) */
#define _IFAT      0xF2  /* Bad file allocation table               */
#define _NOUPB     0xF1  /* (forces re-validation of disk)          */
#define _IFORM     0xF0  /* Cannot format this drive                */

/* ----------------------------------------------------------------- */
/*  KBDOS error codes                                                */
/* ----------------------------------------------------------------- */

#define _INTER     0xDF  /* Internal error                          */
#define _NORAM     0xDE  /* Not enough memory                       */
#define _IBDOS     0xDC  /* Invalid DOS call                       */

#define _IDRV      0xDB  /* Invalid drive                           */
#define _IFNM      0xDA  /* Invalid filename                        */
#define _IPATH     0xD9  /* Invalid pathname                        */
#define _PLONG     0xD8  /* Pathname too long                       */

#define _NOFIL     0xD7  /* File not found                          */
#define _NODIR     0xD6  /* Directory not found                     */
#define _DRFUL     0xD5  /* Root directory full                     */
#define _DKFUL     0xD4  /* Disk full                               */
#define _DUPF      0xD3  /* Duplicate filename                      */
#define _DIRE      0xD2  /* Invalid directory move                  */
#define _FILRO     0xD1  /* Read only file                          */
#define _DIRNE     0xD0  /* Directory not empty                     */
#define _IATTR     0xCF  /* Invalid attributes                      */
#define _DOT       0xCE  /* Invalid . or .. operation               */
#define _SYSX      0xCD  /* System file exists                      */
#define _DIRX      0xCC  /* Directory exists                        */
#define _FILEX     0xCB  /* File exists                             */
#define _FOPEN_ERR 0xCA  /* File is already in use                  */

#define _OV64K     0xC9  /* Cannot transfer above 64k               */
#define _FILE      0xC8  /* File allocation error                   */
#define _EOF       0xC7  /* End of file                             */
#define _ACCV      0xC6  /* File access violation                   */

#define _IPROC     0xC5  /* Invalid process id                      */
#define _NHAND     0xC4  /* No spare file handles                   */
#define _IHAND     0xC3  /* Invalid file handle                     */
#define _NOPEN     0xC2  /* File handle not open                    */
#define _IDEV      0xC1  /* Invalid device operation                */

#define _IENV      0xC0  /* Invalid environment string              */
#define _ELONG     0xBF  /* Environment string too long             */

#define _IDATE     0xBE  /* Invalid date                            */
#define _ITIME     0xBD  /* Invalid time                            */

#define _RAMDX     0xBC  /* RAM disk already exists                 */
#define _NRAMD     0xBB  /* RAM disk does not exist                 */

#define _HDEAD     0xBA  /* File handle has been deleted            */
#define _EOL       0xB9  /* End of line (internal error)            */
#define _ISBFN     0xB8  /* Invalid sub-function number             */
#define _IFCB      0xB7  /* Invalid FCB                             */

/* Error codes introduced in Nextor */

#define _IDRVR     0xB6  /* Invalid device driver                   */
#define _IDEVN     0xB5  /* Invalid device number                   */
#define _IPART     0xB4  /* Invalid partition number                */
#define _PUSED     0xB3  /* Partition is already in use             */
#define _FMNT      0xB2  /* File is mounted                         */
#define _BFSZ      0xB1  /* Bad file size                           */
#define _ICLUS     0xB0  /* Invalid cluster number                  */
#define _INITE     0xAF  /* Initialization error                    */

/* ----------------------------------------------------------------- */
/*  Abort-routine error codes (passed to user ABORT routine)         */
/* ----------------------------------------------------------------- */

#define _STOP      0x9F  /* Ctrl-STOP pressed                       */
#define _CTRLC     0x9E  /* Ctrl-C pressed                          */
#define _ABORT     0x9D  /* Disk operation aborted                  */
#define _OUTERR    0x9C  /* Error on standard output                */
#define _INERR     0x9B  /* Error on standard input                 */

/* ----------------------------------------------------------------- */
/*  COMMAND2.COM error codes (not returned by KBDOS)                 */
/* ----------------------------------------------------------------- */

#define _BADCOM    0x8F  /* Wrong version of COMMAND2.COM           */
#define _BADCMD    0x8E  /* Unrecognized command                    */
#define _BUFUL     0x8D  /* Command too long                        */
#define _OKCMD     0x8C  /* Command executed correctly              */

#define _IPARM     0x8B  /* Invalid parameter                       */
#define _INP       0x8A  /* Too many parameters                     */
#define _NOPAR     0x89  /* Missing parameter                       */
#define _IOPT      0x88  /* Invalid option                          */
#define _BADNO     0x87  /* Invalid number                          */

#define _NOHELP    0x86  /* File for HELP not found                 */
#define _BADVER    0x85  /* Wrong version of system                 */

#define _NOCAT     0x84  /* Cannot concatenate destination file     */
#define _BADEST    0x83  /* Cannot create destination file          */
#define _COPY      0x82  /* File cannot be copied onto itself       */
#define _OVDEST    0x81  /* Cannot overwrite previous destination   */
#define _BATEND    0x80  /* Batch file has ended                    */

#define _INSDSK    0x7F  /* Insert MSX-DOS 2 disk in drive x:       */
#define _PRAK      0x7E  /* Press any key to continue...            */

#endif /* __DOS_ERRORS_H */
