/*****************************************************************************/
/*                                                                           */
/*                   MSX-DOS 2 XCOPY Utility, Header file                    */
/*                                                                           */
/*                     Copyright (c) IS Systems Ltd 1986                     */
/*                                                                           */
/*****************************************************************************/
        
		     /*   C O N S T A N T S   */

    
#define FALSE           0    
#define TRUE            -1

#define DEBUG           FALSE

#define CR              13

/* maximum length of CP/M command line (+1), and its absolute address */
#define MAX_CMD_LEN     129
#define CMD_ADDRESS     0x0080

/* length of a sector in bytes */
#define SECT_LENGTH     512

/* maximum length of a filename */
#define MAX_FIL_LEN     8+1+3+1         /* Name, ., Ext, 0. */

/* maximum length of a path string */
#define MAX_PATH_LEN    63

/* MSX-DOS Error codes */
#define FE_FILE_EXISTS  0xCB
#define FE_DIR_EXISTS   0xCC
#define FE_SYS_EXISTS   0xCD
#define FE_R_ONLY       0xD1
#define FE_NO_FILE      0xD7
#define FE_IFNM         0xDA
#define FE_FOPEN        0xCA
#define FE_PLONG        0xD8
#define FE_IOPT         0x88
#define FE_IPARM        0x8B
#define FE_NORAM        0xDE
#define FE_IDEV         0xC1
#define FE_WRONG_VER    0x01            /* internally generated */
        
/* offsets into FIBs */
#define FIB_LENGTH      64
#define FIB_FILE_NAME   1
#define FIB_ATTRIBUTES  14
#define FIB_CLUSTER     19
#define FIB_FSIZE       21
#define FIB_DRIVE       25

/* offsets in directory entries */
#define DIR_ATTRIBUTES  11
#define DIR_TIME        22
            
/* internal xcopy warning messages */

#define W_MULT_DEF_OPT  1



#define FNAME_LEN       20

	/* Bits in the attributes byte */

#define MASK_R_ONLY     0x01
#define MASK_HIDDEN     0x02
#define MASK_SYSTEM     0x04
#define MASK_SUB_DIR    0x10
#define MASK_ARCHIVE    0x20
#define MASK_DEVICE     0x80



	   /*   E X T E R N A L   T E X T   */

extern int M_WAIT;      
extern int M_FIL1;
extern int M_FIL2;
extern int M_COP;
extern int M_HID;
extern int M_RD_ONLY;
extern int M_T_DE;
extern int M_T_SE;
extern int M_T_RO;
extern int M_T_IN;
extern int M_T_FO;
extern int M_TPTL;
extern int M_CCSD;
extern int M_OPT;
extern int M_PROMPT;
extern int M_WVER;


#ifndef GLOBAL
#define GLOBAL extern           /* define as external of static */
#endif

	       /*   G L O B A L   V A R I A B L E S   */


GLOBAL unsigned reg_bc, reg_de, reg_hl, reg_ix;
GLOBAL char *s_st_file, *t_st_file;
GLOBAL char *src_fib;
GLOBAL char *dst_fib;

GLOBAL int open_err;            /* gets set if an error whilst opening       */

GLOBAL int prompt_flag;         /* true if prompt before each file.          */
GLOBAL int empty_flag;          /* true if can leave empty sub-directories   */
GLOBAL int wait_flag;           /* true if wait before starting              */
GLOBAL int subdirectory_flag;   /* true if we recurse through subdirectories */
GLOBAL int archive_flag;        /* true if we only copy 'archive bit' files  */
GLOBAL int update_arc_flag;     /* true if we update the archive bit         */
GLOBAL int time_flag;           /* true if we should write new time & dates  */
GLOBAL int hidden_flag;         /* true if we should copy hidden files etc.  */
GLOBAL int s_ambig_flag;        /* true if ambiguous filename given for src  */
GLOBAL int t_ambig_flag;        /* true if ambiguous filename given for trg  */

GLOBAL int verify_flag;         /* storage of the original setting           */

GLOBAL int file_not_ensured;

GLOBAL char s_path [MAX_CMD_LEN];       /* Source path */
GLOBAL char t_path [MAX_CMD_LEN];       /* Target path */
GLOBAL char ws_path [MAX_PATH_LEN+6];   /* Whole source path */
    
GLOBAL char sf_name [MAX_FIL_LEN];
GLOBAL char tf_name [MAX_FIL_LEN];
GLOBAL char null_file [1];

GLOBAL int f_count;             /* count of files copied                     */

#asm
	.z80
MSX_DOS         EQU     5

fib_attr        EQU     14      ;Offset in a fib for attributes.
archive_bit     EQU     5       ;Archive bit in attributes byte.
read_bit        EQU     0       ;Read Only bit in attribute byte.

date_low_fib    EQU     17      ;Low Byte  of Modification Date in FIB
date_high_fib   EQU     18      ;High Byte of Modification Date in FIB
time_low_fib    EQU     15      ;Low Byte  of Modification Time in FIB
time_high_fib   EQU     16      ;High Byte of Modification Time in FIB

	.8080
#endasm
