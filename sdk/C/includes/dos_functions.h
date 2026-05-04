/*
 * Nextor SDK - DOS function call codes and standard file handles.
 *
 * The names mirror the asm SDK file sdk/asm/constants/dos_calls.inc;
 * each function code is named with an underscore prefix (e.g. _TERM0,
 * _STROUT). Standard file handles (STDIN..STDLST) come from the asm SDK
 * file sdk/asm/constants/dos_file_handles.inc.
 */

#ifndef __DOS_FUNCTIONS_H
#define __DOS_FUNCTIONS_H

/* ----------------------------------------------------------------- */
/*  DOS function codes                                              */
/* ----------------------------------------------------------------- */

#define _TERM0     0x00  /* Terminate program                       */
#define _CONIN     0x01  /* Console input                           */
#define _CONOUT    0x02  /* Console output                          */
#define _AUXIN     0x03  /* Auxilliary input                        */
#define _AUXOUT    0x04  /* Auxilliary output                       */
#define _LSTOUT    0x05  /* List output                             */
#define _DIRIO     0x06  /* Direct console I/O                      */
#define _DIRIN     0x07  /* Direct console input, no echo           */
#define _INNOE     0x08  /* Console input, no echo                  */
#define _STROUT    0x09  /* String output                           */
#define _BUFIN     0x0A  /* Buffered line input                     */
#define _CONST     0x0B  /* Console status                          */

#define _CPMVER    0x0C  /* Return CP/M version number              */
#define _DSKRST    0x0D  /* Disk reset                              */
#define _SELDSK    0x0E  /* Select disk                             */

#define _FOPEN     0x0F  /* Open file (FCB)                         */
#define _FCLOSE    0x10  /* Close file (FCB)                        */
#define _SFIRST    0x11  /* Search for first (FCB)                  */
#define _SNEXT     0x12  /* Search for next (FCB)                   */
#define _FDEL      0x13  /* Delete file (FCB)                       */
#define _RDSEQ     0x14  /* Read sequential (FCB)                   */
#define _WRSEQ     0x15  /* Write sequential (FCB)                  */
#define _FMAKE     0x16  /* Create file (FCB)                       */
#define _FREN      0x17  /* Rename file (FCB)                       */

#define _LOGIN     0x18  /* Get login vector                        */
#define _CURDRV    0x19  /* Get current drive                       */
#define _SETDTA    0x1A  /* Set disk transfer address               */
#define _ALLOC     0x1B  /* Get allocation information              */

#define _RDRND     0x21  /* Read random (FCB)                       */
#define _WRRND     0x22  /* Write random (FCB)                      */
#define _FSIZE     0x23  /* Get file size (FCB)                     */
#define _SETRND    0x24  /* Set random record (FCB)                 */
#define _WRBLK     0x26  /* Write random block (FCB)                */
#define _RDBLK     0x27  /* Read random block (FCB)                 */
#define _WRZER     0x28  /* Write random with zero fill (FCB)       */

#define _GDATE     0x2A  /* Get date                                */
#define _SDATE     0x2B  /* Set date                                */
#define _GTIME     0x2C  /* Get time                                */
#define _STIME     0x2D  /* Set time                                */
#define _VERIFY    0x2E  /* Set/reset verify flag                   */

#define _RDABS     0x2F  /* Absolute sector read                    */
#define _WRABS     0x30  /* Absolute sector write                   */
#define _DPARM     0x31  /* Get disk parameters                     */

#define _FFIRST    0x40  /* Find first entry                        */
#define _FNEXT     0x41  /* Find next entry                         */
#define _FNEW      0x42  /* Find new entry                          */

#define _OPEN      0x43  /* Open file handle                        */
#define _CREATE    0x44  /* Create file and open handle             */
#define _CLOSE     0x45  /* Close file handle                       */
#define _ENSURE    0x46  /* Ensure file handle                      */
#define _DUP       0x47  /* Duplicate file handle                   */
#define _READ      0x48  /* Read from file handle                   */
#define _WRITE     0x49  /* Write to file handle                    */
#define _SEEK      0x4A  /* Seek (position file pointer)            */
#define _IOCTL     0x4B  /* I/O control for devices                 */
#define _HTEST     0x4C  /* Test file handle                        */

#define _DELETE    0x4D  /* Delete file or subdirectory             */
#define _RENAME    0x4E  /* Rename file or subdirectory             */
#define _MOVE      0x4F  /* Move file or subdirectory               */
#define _ATTR      0x50  /* Change file or subdirectory attributes  */
#define _FTIME     0x51  /* Get/set file date and time              */

#define _HDELETE   0x52  /* Delete file handle                      */
#define _HRENAME   0x53  /* Rename file handle                      */
#define _HMOVE     0x54  /* Move file handle                        */
#define _HATTR     0x55  /* Change file handle attributes           */
#define _HFTIME    0x56  /* Get/set file handle date and time       */

#define _GETDTA    0x57  /* Get disk transfer address               */
#define _GETVFY    0x58  /* Get verify flag setting                 */
#define _GETCD     0x59  /* Get current directory                   */
#define _CHDIR     0x5A  /* Change directory                        */
#define _PARSE     0x5B  /* Parse pathname                          */
#define _PFILE     0x5C  /* Parse filename                          */
#define _CHKCHR    0x5D  /* Check character                         */
#define _WPATH     0x5E  /* Get whole path string                   */
#define _FLUSH     0x5F  /* Flush disk buffers                      */

#define _FORK      0x60  /* Fork a child process                    */
#define _JOIN      0x61  /* Return to parent process                */
#define _TERM      0x62  /* Terminate with error code               */
#define _DEFAB     0x63  /* Define abort exit routine               */
#define _DEFER     0x64  /* Define critical error handle routine    */
#define _ERROR     0x65  /* Get previous error code                 */
#define _EXPLAIN   0x66  /* Explain error code                      */

#define _FORMAT    0x67  /* Format disk                             */
#define _RAMD      0x68  /* Create or destroy RAMdisk               */
#define _BUFFER    0x69  /* Allocate sector buffers                 */
#define _ASSIGN    0x6A  /* Logical drive assignment                */

#define _GENV      0x6B  /* Get environment item                    */
#define _SENV      0x6C  /* Set environment item                    */
#define _FENV      0x6D  /* Find environment item                   */

#define _DSKCHK    0x6E  /* Get/set disk check status               */
#define _DOSVER    0x6F  /* Get MSX-DOS version number              */
#define _REDIR     0x70  /* Get/set redirection flags               */

/* Function calls introduced in Nextor */

#define _FOUT      0x71  /* Get/set fast STROUT mode                */
#define _ZSTROUT   0x72  /* Zero-terminated string output           */

#define _RDDRV     0x73  /* Absolute drive sector read              */
#define _WRDRV     0x74  /* Absolute drive sector write             */

#define _RALLOC    0x75  /* Reduced allocation information vector   */
#define _DSPACE    0x76  /* Get disk space information              */

#define _LOCK      0x77  /* Lock/unlock drive                       */

#define _GDRVR     0x78  /* Get information about disk driver       */
#define _GDLI      0x79  /* Get information about drive letter      */
#define _GPART     0x7A  /* Get information about disk partition    */
#define _CDRVR     0x7B  /* Call a routine in a disk driver         */
#define _MAPDRV    0x7C  /* Map a drive letter to a drive/device    */

#define _Z80MODE   0x7D  /* Enable/disable Z80 access mode          */
#define _GETCLUS   0x7E  /* Get information about a FAT cluster     */

#define _DRVRO     0x7F  /* Operate on a RAM-loaded driver          */

/* ----------------------------------------------------------------- */
/*  Standard file handles                                            */
/* ----------------------------------------------------------------- */

#define STDIN      0
#define STDOUT     1
#define STDERR     2
#define STDAUX     3
#define STDLST     4

#endif /* __DOS_FUNCTIONS_H */
