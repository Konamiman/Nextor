/* DRVROP - RAM driver operation tool for Nextor v1.0
   By Konamiman 4/2026

   Compilation command line:

   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196
          --no-std-crt0 crt0_msxdos_advanced.rel drvrop.c
   hex2bin -e com drvrop.ihx
*/


/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "asmcall.h"
#include "types.h"
#include "dos.h"
#include "system.h"


/* Defines */

#define BUFFER ((byte*)0x8000)
#define MAPDATA ((byte*)0x8200)
#define REGBUF ((int*)0x8100)

#define MAX_FILE_SIZE 0x3F00
#define MAX_INIT_DATA 255

/* Function codes not in dos.h */
#define _DRVRO  0x7F
#define _MAPDRV 0x7C

/* _DRVRO operations */
#define DRVRO_INIT      1
#define DRVRO_SHUTDOWN  2

/* Mapper support routine table offsets */
#define MAP_ALL_SEG  0x00
#define MAP_FRE_SEG  0x03
#define MAP_PUT_P1   0x1E
#define MAP_GET_P1   0x21

/* Handy macros */
#define DoDosCall(f) DosCall(f, &regs, REGS_ALL, REGS_ALL)


/* Global variables */

Z80_registers regs;
byte ASMRUT[4];
byte OUT_FLAGS;

uint mapperTable;
byte allocSlot;
byte allocSegment;
byte savedP1Segment;

byte initData[MAX_INIT_DATA];
byte initDataLength;

bool silent;
bool autoMap;

/* Print routine machine code for driver init/shutdown messages.
   Preserves HL, DE, BC, IX as required by the driver callback contract.

   Uses BIOS CHPUT (00A2h) via an inter-slot call (CALSLT at 001Ch)
   to the BIOS slot (from EXPTBL at FCC1h). This avoids any BDOS
   re-entrancy since CHPUT is a direct VDP-level BIOS routine.

   F_DRVRO switches pages 0/2 to the caller's TPA before invoking
   the driver, so this routine is directly reachable at its original
   address and CALSLT at 001Ch has a working stub in TPA page 0.
   The only constraint is: must NOT be in page 1 (4000h-7FFFh)
   since page 1 holds the driver segment during the callback. */

byte printRoutine() __naked
{
    __asm
        
    push hl
    push de
    push bc
    push ix
    ld iy,(#0xFCC0)  ; IYh=BIOS slot
    ld ix,#0x00A2    ; CHPUT address
    call #0x001C     ; CALSLT
    pop ix
    pop bc
    pop de
    pop hl
    ret
    __endasm;
}

/* Function prototypes */

void Terminate(const char* errorMessage);
void TerminateWithDosError(byte errorCode);
void print(char* s);
void CheckDosVersion();
void GetMapperTable();
void DoInstall(char** argv, int argc);
void DoUninstall(char** argv, int argc);
void DoAutoMap();
void ParseSlotNumber(char* arg, byte* slot);
void ParseInitData(char* str);
void ComposeSlotString(byte slot, byte segment, char* dest);
void FreeAllocatedSegment();


/* Strings */

const char* strTitle =
    "DRVROP - RAM driver operation tool v1.0\r\n"
    "By Konamiman, 4/2026\r\n"
    "\r\n";

const char* strUsage =
    "Usage: DRVROP i <file> [/s] [/m] [/d <data>[,<data>...]]\r\n"
    "       DRVROP u <slot>[-<subslot>] <segment> [/s]\r\n"
    "       DRVROP ?\r\n";

const char* strHelp =
    "\r\n"
    "i: Install a RAM driver from a file\r\n"
    "u: Uninstall a RAM driver\r\n"
    "\r\n"
    "/s: Silent mode (don't print driver messages)\r\n"
    "/m: Auto-map first available drive (install only)\r\n"
    "/d: Initialization data bytes, comma separated\r\n"
    "    (decimal by default, prefix with # for hex)\r\n"
    "\r\n"
    "For install, the driver file is loaded into an allocated\r\n"
    "RAM segment and initialized via the _DRVRO function call.\r\n"
    "\r\n"
    "For uninstall, specify the slot and segment as shown\r\n"
    "by the DRIVERS command or CALL DRIVERS.\r\n";

const char* strInvParam = "Invalid parameter";
const char* strCRLF = "\r\n";


	/* MAIN */


int main(char** argv, int argc)
{
    ASMRUT[0] = 0xC3;
    print(strTitle);

    if(argc == 0) {
        print(strUsage);
        return 0;
    }

    if(argv[0][0] == '?') {
        print(strUsage);
        print(strHelp);
        return 0;
    }

    CheckDosVersion();
    GetMapperTable();

    if((argv[0][0] | 0x20) == 'i') {
        DoInstall(argv, argc);
    }
    else if((argv[0][0] | 0x20) == 'u') {
        DoUninstall(argv, argc);
    }
    else {
        Terminate(strInvParam);
    }

    //TerminateWithDosError(0);
    return 0;
}


void CheckDosVersion()
{
    regs.Bytes.B = 0x5A;
    regs.Words.HL = 0x1234;
    regs.Words.DE = (int)0xABCD;
    regs.Words.IX = 0;
    DosCall(_DOSVER, &regs, REGS_ALL, REGS_ALL);

    if(regs.Bytes.B < 2 || regs.Bytes.IXh != 1 || regs.Bytes.IXl < 3) {
        Terminate("This program is for Nextor 3 only.");
    }
}


void GetMapperTable()
{
    regs.Bytes.D = 4;
    regs.Bytes.E = 2; /* "Get mapper support routine address" (DOS2-PIS §5.2) */
    AsmCall(0xFFCA, &regs, REGS_MAIN, REGS_MAIN);
    mapperTable = regs.UWords.HL;
}


void ParseSlotNumber(char* arg, byte* slot)
{
    if(arg[0] < '0' || arg[0] > '3') {
        Terminate(strInvParam);
    }

    *slot = arg[0] - '0';

    if(arg[1] == '\0') {
        return;
    }

    if(arg[1] != '-' || arg[2] < '0' || arg[2] > '3') {
        Terminate(strInvParam);
    }

    *slot += ((arg[2] - '0') << 2) + 0x80;
}


void ParseInitData(char* str)
{
    while(*str) {
        int val;

        if(*str == '#') {
            str++;
            val = 0;
            while((*str >= '0' && *str <= '9') ||
                  ((*str | 0x20) >= 'a' && (*str | 0x20) <= 'f')) {
                if(*str >= '0' && *str <= '9')
                    val = val * 16 + (*str - '0');
                else
                    val = val * 16 + ((*str | 0x20) - 'a' + 10);
                str++;
            }
        }
        else {
            val = atoi(str);
            while(*str && *str != ',') str++;
        }

        if(initDataLength < MAX_INIT_DATA) {
            if((val & 0xFF00) == 0 || (val & 0xFF00) == 0xFF00) {
                initData[initDataLength++] = (byte)val;
            } else {
                initData[initDataLength++] = (byte)(val & 0xFF);
                if(initDataLength < MAX_INIT_DATA) {
                    initData[initDataLength++] = (byte)((val >> 8) & 0xFF);
                }
            }
        }

        if(*str == ',') str++;
    }
}


void ComposeSlotString(byte slot, byte segment, char* dest)
{
    *dest++ = (slot & 3) + '0';
    if(slot & 0x80) {
        *dest++ = '-';
        *dest++ = ((slot >> 2) & 3) + '0';
    }
    *dest++ = ':';
    if(segment >= 100) {
        *dest++ = (segment / 100) + '0';
    }
    if(segment >= 10) {
        *dest++ = ((segment / 10) % 10) + '0';
    }
    *dest++ = (segment % 10) + '0';
    *dest = '\0';
}


void FreeAllocatedSegment()
{
    regs.Bytes.A = allocSegment;
    regs.Bytes.B = allocSlot;
    AsmCall(mapperTable + MAP_FRE_SEG, &regs, REGS_MAIN, REGS_NONE);
}


void DoInstall(char** argv, int argc)
{
    byte i, fileHandle;
    uint bytesRead;
    int printAddr;
    char slotStr[8];
    char* fileName;
    byte err;

    if(argc < 2) {
        Terminate(strInvParam);
    }

    fileName = argv[1];
    silent = false;
    autoMap = false;
    initDataLength = 0;

    /* Parse options */
    for(i = 2; i < argc; i++) {
        if(argv[i][0] != '/' && argv[i][0] != '-') {
            Terminate(strInvParam);
        }

        switch(argv[i][1] | 0x20) {
            case 's':
                silent = true;
                break;
            case 'm':
                autoMap = true;
                break;
            case 'd':
                if(i + 1 >= argc) Terminate(strInvParam);
                i++;
                ParseInitData(argv[i]);
                break;
            default:
                Terminate(strInvParam);
        }
    }

    /* 1. Allocate segment (system mode, prefer non-primary mapper).
       We pass B!=0 so the returned B holds the real slot address of the
       allocated mapper (with B=0 ALL_SEG returns B=0 as a sentinel, which
       we can't pass to F_DRVRO/RD_MAP later). 00110000b = "try non-primary
       mappers first, fall back to primary". */
    regs.Bytes.A = 0x01;
    regs.Bytes.B = 0x30;
    AsmCall(mapperTable + MAP_ALL_SEG, &regs, REGS_MAIN, REGS_MAIN);
    if(regs.Flags.C) {
        Terminate("Not enough memory");
    }
    allocSegment = regs.Bytes.A;
    allocSlot = regs.Bytes.B;

    /* 2. Open file */
    regs.Bytes.A = 0;
    regs.Words.DE = (int)fileName;
    DoDosCall(_OPEN);
    if(regs.Bytes.A != 0) {
        err = regs.Bytes.A;
        FreeAllocatedSegment();
        TerminateWithDosError(err);
    }
    fileHandle = regs.Bytes.B;

    /* 3. Read file into buffer at 0x8000 */
    regs.Bytes.B = fileHandle;
    regs.Words.DE = (int)BUFFER;
    regs.Words.HL = MAX_FILE_SIZE;
    DoDosCall(_READ);
    bytesRead = regs.Words.HL;

    if(regs.Bytes.A != 0) {
        err = regs.Bytes.A;
        regs.Bytes.B = fileHandle;
        DoDosCall(_CLOSE);
        FreeAllocatedSegment();
        TerminateWithDosError(err);
    }

    regs.Bytes.B = fileHandle;
    DoDosCall(_CLOSE);

    /* 4. Switch page 1 to our segment and load data */

    /* Save current page 1 segment */
    AsmCall(mapperTable + MAP_GET_P1, &regs, REGS_NONE, REGS_AF);
    savedP1Segment = regs.Bytes.A;

    /* Switch to allocated segment */
    regs.Bytes.A = allocSegment;
    AsmCall(mapperTable + MAP_PUT_P1, &regs, REGS_AF, REGS_NONE);

    /* Zero first 256 bytes (4000h-40FFh) */
    memset((byte*)0x4000, 0, 256);

    /* Write init data: byte 0 = length, bytes 1+ = data */
    *((byte*)0x4000) = initDataLength;
    if(initDataLength > 0) {
        memcpy((byte*)0x4001, initData, initDataLength);
    }

    /* Copy driver code to 4100h */
    memcpy((byte*)0x4100, BUFFER, bytesRead);

    /* Restore page 1 */
    regs.Bytes.A = savedP1Segment;
    AsmCall(mapperTable + MAP_PUT_P1, &regs, REGS_AF, REGS_NONE);

    /* 5. Call _DRVRO to initialize driver */
    printAddr = silent ? 0 : (int)printRoutine;
    regs.Bytes.A = allocSlot;
    regs.Bytes.B = allocSegment;
    regs.Words.DE = printAddr;
    regs.Bytes.H = DRVRO_INIT;
    DoDosCall(_DRVRO);

    if(regs.Bytes.A != 0) {
        err = regs.Bytes.A;
        FreeAllocatedSegment();
        TerminateWithDosError(err);
    }

    /* 6. Print result */
    if(!silent) print(strCRLF);
    ComposeSlotString(allocSlot, allocSegment, slotStr);
    printf("Driver installed in %s\r\n", slotStr);

    /* 7. Auto-map if requested */
    if(autoMap) {
        DoAutoMap();
    }
}


void DoUninstall(char** argv, int argc)
{
    byte slot, segment;
    int printAddr;
    char slotStr[8];
    byte i;

    if(argc < 3) {
        Terminate(strInvParam);
    }

    ParseSlotNumber(argv[1], &slot);
    segment = (byte)atoi(argv[2]);

    silent = false;
    for(i = 3; i < argc; i++) {
        if((argv[i][0] == '/' || argv[i][0] == '-') &&
           ((argv[i][1] | 0x20) == 's')) {
            silent = true;
        }
        else {
            Terminate(strInvParam);
        }
    }

    /* Call _DRVRO to shut down */
    printAddr = silent ? 0 : (int)printRoutine;
    regs.Bytes.A = slot;
    regs.Bytes.B = segment;
    regs.Words.DE = printAddr;
    regs.Bytes.H = DRVRO_SHUTDOWN;
    DoDosCall(_DRVRO);

    if(regs.Bytes.A != 0) {
        TerminateWithDosError(regs.Bytes.A);
    }

    /* Free segment */
    regs.Bytes.A = segment;
    regs.Bytes.B = slot;
    AsmCall(mapperTable + MAP_FRE_SEG, &regs, REGS_MAIN, REGS_NONE);

    if(!silent) print(strCRLF);
    ComposeSlotString(slot, segment, slotStr);
    printf("Driver in %s uninstalled\r\n", slotStr);
}


void DoAutoMap()
{
    byte maxDevices;
    byte device;
    byte foundDevice;
    byte freeDrive;
    byte i;

    /* 1. Get max device number from driver using _CDRVR */
    REGBUF[0] = DRVQ_GET_MAX_DEVICE_NUMBER << 8; /* AF: A=query, F=0 */
    REGBUF[1] = 0; /* BC */
    REGBUF[2] = 0; /* DE */
    REGBUF[3] = 0; /* HL */

    regs.Bytes.A = allocSlot | CDRVR_NEXTOR_3_FLAG;
    regs.Bytes.B = allocSegment;
    regs.Words.DE = DRIVER_QUERY;
    regs.Words.HL = (int)REGBUF;
    DoDosCall(_CDRVR);

    if(regs.Bytes.A != 0) {
        print("  Warning: could not query driver for devices\r\n");
        return;
    }

    maxDevices = regs.Bytes.B;
    if(regs.Bytes.IXh != 0 && regs.Bytes.IXh != 0xFF) {
        maxDevices = DEFAULT_MAX_DEVICE_NUMBER;
    }
    if(maxDevices == 0) {
        maxDevices = DEFAULT_MAX_DEVICE_NUMBER;
    }

    /* 2. Find first suitable block device with 512-byte sectors
       (mirrors IDRV_AUTOMAP in idrvauto.mac: query DEVQ_GET_PARAMS,
       check byte 0 = 0 (block) and bytes 1-2 = 0x0200 (512)) */
    foundDevice = 0;
    /* device != 0 guards against wrap when maxDevices == 255 */
    for(device = 1; device != 0 && device <= maxDevices; device++) {
        REGBUF[0] = DEVQ_GET_PARAMS << 8; /* AF: A=query */
        REGBUF[1] = device;               /* BC: C=device */
        REGBUF[2] = 0;                    /* DE */
        REGBUF[3] = (int)MAPDATA;         /* HL = info buffer */

        regs.Bytes.A = allocSlot | CDRVR_NEXTOR_3_FLAG;
        regs.Bytes.B = allocSegment;
        regs.Words.DE = DEVICE_QUERY;
        regs.Words.HL = (int)REGBUF;
        DoDosCall(_CDRVR);

        if(regs.Bytes.A != 0) continue;
        if(regs.Bytes.IXh != 0) continue; /* driver returned error */

        /* MAPDATA now has device info: [0]=type, [1-2]=sector size, ... */
        if(MAPDATA[0] != 0) continue;     /* not a block device */
        if(MAPDATA[1] != 0 || MAPDATA[2] != 2) continue; /* not 512-byte sectors */

        foundDevice = device;
        break;
    }

    if(!foundDevice) {
        print("  No suitable devices found\r\n");
        return;
    }

    /* 3. Find first free drive letter using _GDLI */
    freeDrive = 0xFF;
    for(i = 0; i < 26; i++) {
        regs.Bytes.A = i;
        regs.Words.HL = (int)MAPDATA;
        DoDosCall(_GDLI);

        if(regs.Bytes.A == 0 && MAPDATA[0] == 0) {
            freeDrive = i;
            break;
        }
    }

    if(freeDrive == 0xFF) {
        print("  No free drive letters available\r\n");
        return;
    }

    /* 4. Map drive at sector 0 via _MAPDRV (same as IDRV_AUTOMAP).
       For unpartitioned media the filesystem is at sector 0;
       for partitioned media Nextor discovers the partition table. */
    MAPDATA[0] = allocSlot;
    MAPDATA[1] = allocSegment;
    MAPDATA[2] = foundDevice;
    MAPDATA[3] = 0;
    MAPDATA[4] = 0; /* start sector = 0 */
    MAPDATA[5] = 0;
    MAPDATA[6] = 0;
    MAPDATA[7] = 0;

    regs.Bytes.A = freeDrive;
    regs.Bytes.B = 2; /* action: map to specific device */
    regs.Words.HL = (int)MAPDATA;
    DoDosCall(_MAPDRV);

    if(regs.Bytes.A != 0) {
        printf("  Warning: could not map drive (%d)\r\n", regs.Bytes.A);
        return;
    }

    printf("Drive %c: mapped to device %d\r\n", freeDrive + 'A', foundDevice);
}


void Terminate(const char* errorMessage)
{
    if(errorMessage != NULL) {
        printf("\r\x1BK*** %s\r\n", errorMessage);
    }

    regs.Bytes.B = (errorMessage == NULL ? 0 : 1);
    DoDosCall(_TERM);
    DoDosCall(_TERM0);
}


void TerminateWithDosError(byte errorCode)
{
    regs.Bytes.B = errorCode;
    DoDosCall(_TERM);
}


#define COM_FILE
#define SUPPORT_LONG
#include "print_msxdos.c"
#include "printf.c"
#include "asmcall.c"
