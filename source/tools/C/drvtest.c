/* Driver test tool for Nextor v1.0
   By Konamiman 9/2023

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196
          --no-std-crt0 crt0_msxdos_advanced.rel drvtest.c
   hex2bin -e com drvtest.ihx
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
#define REGS_BUFFER ((int*)0x8100)
#define STRING_BUFFER_ADDRESS 0x8200
#define STRING_BUFFER ((byte*)STRING_BUFFER_ADDRESS)
#define DEV_PARAMS ((deviceInfo*)STRING_BUFFER_ADDRESS);

__at STRING_BUFFER_ADDRESS deviceParams* deviceInfoBuffer;

/* Some handy code defines */

#define PrintNewLine() print(strCRLF)
#define InvalidParameter() Terminate(strInvParam)
#define DoDosCall(functionCode) DosCall(functionCode, &regs, REGS_ALL, REGS_ALL)
#define DriverQuery(queryIndex) ((regs.Bytes.A = queryIndex), CallDriver(DRIVER_QUERY))
#define DeviceQuery(queryIndex) (regs.Bytes.A = queryIndex, regs.Bytes.C = deviceNumber, CallDriver(DEVICE_QUERY))


/* Global variables */

Z80_registers regs;
byte ASMRUT[4];
byte OUT_FLAGS;
byte slotNumber;
byte maxStringLength;
byte deviceNumber;
bool success;
byte lastDriverError;
int bufferIndex;
bool invokeInit;
byte relativeDriveNumber;
bool invokeGetDeviceStatus;
int spaceAvailableInPage3;


/* Function prototypes */

void Terminate(const char* errorMessage);
void TerminateWithDosError(byte errorCode);
void print(char* s);
void CheckDosVersion();
void ParseSlotNumber(char* arg);
void VerifySlotNumber();
void ParseArguments(char** argv, int argc);
bool CallDriver(int routineAddress);
void PrintStringFromDriver(char* title, char* address);
void DoDriverQueries();
void GetDriverName(byte nameIndex);
void DoGetDriveBootConfigQuery(bool dos1Mode, bool reducedDriveCount);
void PrintChar(char theChar);
void PrintCharCore(char theChar);
void DoGetDriverInitParamsQuery(bool reducedDriveCount);
void MaybePrintStringBuffer();
void DoDriverInitQuery(bool reducedDriveCount);
void DoGetNumberOfBootDrivesQuery(bool dos1Mode, bool reducedDriveCount);
void DoDeviceQueries();
char* YesOrNo(bool condition);


/* Strings */

const char* strTitle=
    "Nextor driver test tool v1.0\r\n"
    "(tests the DRIVER_QUERY and DEVICE_QUERY routines)\r\n"
    "By Konamiman, 9/2023\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: drvtest <slot>[-<subslot>] [-l <string length>]  [-a <space>] [-i]\r\n"
    "               [-n <drive number>] [-d <device number>] [-t]\r\n"
    "       drvtest ?\r\n";

const char* strHelp=
    "\r\n"
    "<slot>[-<subslot>]: Slot of the driver being tested\r\n"
    "<string length>: Maximum string length to request (max 255)\r\n"
    "\r\n"
    "To test DRIVER_QUERY: don't use -d, or use -d 0\r\n"
    "\r\n"
    "  -i: Invoke the driver initialization query too\r\n"
    "  <space>: Value to pass in HL to \"Get driver init params\" and \"Init driver\"\r\n"
    "           (default: 1024)\r\n"
    "  <drive number>: Relative drive number to pass to the\r\n"
    "                  \"Get drive configuration at boot time\" query\r\n"
    "                  (default: 0)\r\n"
    "\r\n"
    "To test DEVICE_QUERY: pass the <device number> with -d\r\n"
    "\r\n"
    "  -t: Invoke the \"Get device status\" routine too";

const char* strInvParam = "Invalid parameter";
const char* strCRLF = "\r\n";
const char* strNoNextor = "Not a Nextor driver";


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
    ParseSlotNumber(argv[0]);
    VerifySlotNumber();
    ParseArguments(argv, argc);

    if(deviceNumber == 0) {
        DoDriverQueries();
    }
    else {
        DoDeviceQueries();
    }

	return 0;
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


void ParseSlotNumber(char* arg)
{
    if(arg[0] < '0' || arg[0] > '3') {
        Terminate(strInvParam);
    }

    slotNumber = arg[0] - '0';

    if(arg[1] == '\0') {
        return;
    }

    if(arg[2] < '0' || arg[2] > '3') {
        Terminate(strInvParam);
    }

    slotNumber += ((arg[2] - '0') << 2) + 0x80;
}


void VerifySlotNumber()
{
    regs.Bytes.A = 0x80;
    regs.Bytes.D = slotNumber;
    regs.Bytes.E = 0xFF;
    regs.Words.HL = (int)BUFFER;
    DoDosCall(_GDRVR);

    if(regs.Bytes.A != 0) {
        TerminateWithDosError(regs.Bytes.A);
    }

    if((BUFFER[4] & 0x80) == 0) {
        Terminate(strNoNextor);
    }
}


void ParseArguments(char** argv, int argc)
{
    byte i;
    char arg;

    maxStringLength = 255;
    deviceNumber = 0;
    invokeInit = false;
    relativeDriveNumber = 0;
    invokeGetDeviceStatus = false;
    spaceAvailableInPage3 = 1024;

    for(i = 1; i < argc; i++) {
        arg = argv[i][1] | 0x20;
        if(arg == 'l' ) {
            maxStringLength = (byte)atoi(argv[i+1]);
            if(maxStringLength != 255) {
                maxStringLength++;
            }
            i++;
        }
        else if(arg == 'a') {
            spaceAvailableInPage3 = atoi(argv[i+1]);
            i++;
        }
        else if(arg == 'd') {
            deviceNumber = (byte)atoi(argv[i+1]);
            i++;
        }
        else if(arg == 'n') {
            relativeDriveNumber = (byte)atoi(argv[i+1]);
            i++;
        }
        else if(arg == 'i') {
            invokeInit = true;
        }
        else if(arg == 't') {
            invokeGetDeviceStatus = true;
        }
        else {
            Terminate(strInvParam);
        }
    }
}


bool CallDriver(int routineAddress)
{
    REGS_BUFFER[0] = regs.Words.AF;
    REGS_BUFFER[1] = regs.Words.BC;
    REGS_BUFFER[2] = regs.Words.DE;
    REGS_BUFFER[3] = regs.Words.HL;

    regs.Bytes.A = slotNumber | CDRVR_NEXTOR_3_FLAG;
    regs.Bytes.B = 0xFF;
    regs.Words.DE = routineAddress;
    regs.Words.HL = (int)REGS_BUFFER;

    DoDosCall(_CDRVR);
    if(regs.Bytes.A != 0) {
        TerminateWithDosError(regs.Bytes.A);
    }

    regs.Words.AF = regs.Words.IX;
    lastDriverError = regs.Bytes.A;

    if(lastDriverError == ERR_QUERY_INVALID_DEVICE) {
        print("  *** Error: Invalid device number\r\n");
        return false;
    }
    else if(lastDriverError == ERR_QUERY_NOT_IMPLEMENTED) {
        print("  *** Error: Query not implemented\r\n");
        return false;
    }
    else if(lastDriverError != ERR_QUERY_OK && regs.Bytes.A != ERR_QUERY_TRUNCATED_STRING) {
        printf("  *** Error: %d\r\n", regs.Bytes.A);
        return false;
    }

    return true;
}


void PrintStringFromDriver(char* title, char* address)
{
    if(lastDriverError == ERR_QUERY_TRUNCATED_STRING) {
        printf("  %s (truncated): %s\r\n", title, address);
    }
    else {
        printf("  %s: %s\r\n", title, address);
    }
}


void DoDriverQueries()
{
    print("Driver query: get version number\r\n");
    success = DriverQuery(DRVQ_GET_VERSION);
    if(success) {
        printf("  Version: %d.%d.%d\r\n", regs.Bytes.B, regs.Bytes.C, regs.Bytes.D);
    }

    print("\r\nDriver query: get driver name\r\n");
    GetDriverName(1);

    print("\r\nDriver query: get driver author name\r\n");
    GetDriverName(2);

    print("\r\nDriver query: get hardware name\r\n");
    GetDriverName(3);

    print("\r\nDriver query: get hardware author name\r\n");
    GetDriverName(4);

    print("\r\nDriver query: get serial number\r\n");
    GetDriverName(5);

    print("\r\nDriver query: get driver init parameters\r\n");
    DoGetDriverInitParamsQuery(false);

    print("\r\nDriver query: get driver init parameters (reduced drive count)\r\n");
    DoGetDriverInitParamsQuery(true);

    if(invokeInit) {
        print("\r\nDriver query: init driver\r\n");
        DoDriverInitQuery(false);

        print("\r\nDriver query: init driver (reduced drive count)\r\n");
        DoDriverInitQuery(true);
    }

    print("\r\nDriver query: get boot drives count\r\n");
    DoGetNumberOfBootDrivesQuery(false, false);

    print("\r\nDriver query: get boot drives count (DOS 1 mode)\r\n");
    DoGetNumberOfBootDrivesQuery(true, false);

    print("\r\nDriver query: get boot drives count (reduced drive count)\r\n");
    DoGetNumberOfBootDrivesQuery(false, true);

    print("\r\nDriver query: get boot drives count (DOS 1 mode, reduced drive count)\r\n");
    DoGetNumberOfBootDrivesQuery(true, true);

    printf("\r\nDriver query: get boot config for drive %d\r\n", relativeDriveNumber);
    DoGetDriveBootConfigQuery(false, false);

    printf("\r\nDriver query: get boot config for drive %d (DOS 1 mode)\r\n", relativeDriveNumber);
    DoGetDriveBootConfigQuery(true, false);

    printf("\r\nDriver query: get boot config for drive %d (reduced drive count)\r\n", relativeDriveNumber);
    DoGetDriveBootConfigQuery(false, true);

    printf("\r\nDriver query: get boot config for drive %d (DOS 1 mode, reduced drive count)\r\n", relativeDriveNumber);
    DoGetDriveBootConfigQuery(true, true);
}


void DoGetDriverInitParamsQuery(bool reducedDriveCount)
{
    regs.Words.HL = spaceAvailableInPage3;
    regs.Bytes.B = reducedDriveCount ? 0x20 : 0;
    regs.Words.DE = (int)&PrintChar;
    bufferIndex = 0;
    success = DriverQuery(DRVQ_GET_INIT_PARAMS);
    MaybePrintStringBuffer();
    if(success) {
        printf("  Hook timer interrupt: %s\r\n", regs.Bytes.B & 1 ? "YES" : "NO");
        printf("  Space required in page 3: %d bytes\r\n", regs.Words.HL);
    }
}


void MaybePrintStringBuffer()
{
    if(bufferIndex != 0) {
        STRING_BUFFER[bufferIndex] = '\0';
        print("----------\r\n");
        print(STRING_BUFFER);
        if(STRING_BUFFER[bufferIndex-1] != '\n') {
            print(strCRLF);
        }
        print("----------\r\n");
    }
}


void DoDriverInitQuery(bool reducedDriveCount)
{
    regs.Words.HL = spaceAvailableInPage3;
    regs.Bytes.C = reducedDriveCount ? 0x20 : 0;
    regs.Words.DE = (int)&PrintChar;
    bufferIndex = 0;
    success = DriverQuery(DRVQ_INIT_DRIVER);
    MaybePrintStringBuffer();

    if(success && bufferIndex == 0) {
        print("  Ok!\r\n");
    }
}


void DoGetNumberOfBootDrivesQuery(bool dos1Mode, bool reducedDriveCount)
{
    regs.Bytes.C = (dos1Mode ? 1 : 0) | (reducedDriveCount ? 0x20 : 0);
    success = DriverQuery(DRVQ_GET_BOOT_DRIVES_COUNT);
    if(success) {
        printf("  Drives count: %d\r\n", regs.Bytes.B);
    }
}


void DoGetDriveBootConfigQuery(bool dos1Mode, bool reducedDriveCount)
{
    regs.Bytes.B = relativeDriveNumber;
    regs.Bytes.C = (dos1Mode ? 1 : 0) | (reducedDriveCount ? 0x20 : 0);
    success = DriverQuery(DRVQ_GET_DRIVE_BOOT_CONFIG);
    if(success) {
        if(regs.Bytes.B == 0) {
            print("  Device number: 0 (automatic)\r\n");
        }
        else {
            printf("  Device number: %d\r\n", regs.Bytes.B);
        }
    }
}


void PrintChar(char theChar) __naked
{
    // This routine must preserve HL, DE and BC,
    // thus an assembler wrapper for PUSH/POPs is needed.

    __asm

    push hl
    push de
    push bc

    call #_PrintCharCore

    pop bc
    pop de
    pop hl

    ret

    __endasm;
}


void PrintCharCore(char theChar)
{
    if(theChar != 0) {
        STRING_BUFFER[bufferIndex++] = theChar;
    }
}


void GetDriverName(byte nameIndex)
{
    regs.Bytes.B = nameIndex;
    regs.Bytes.D = maxStringLength;
    regs.Words.HL = (int)BUFFER;
    success = DriverQuery(DRVQ_GET_INFO_STRING);
    if(success) {
        PrintStringFromDriver("Name", BUFFER);
    }
}


void GetDeviceInfo(byte infoIndex)
{
    regs.Bytes.B = infoIndex;
    regs.Bytes.D = maxStringLength;
    regs.Words.HL = (int)BUFFER;
    success = DeviceQuery(DEVQ_GET_STRING);
    if(success) {
        PrintStringFromDriver("Value", BUFFER);
    }
}


void DoDeviceQueries()
{
    byte flags;

    print("Device query: get manufacturer name\r\n");
    GetDeviceInfo(1);

    print("\r\nDevice query: get device name\r\n");
    GetDeviceInfo(2);

    print("\r\nDevice query: get serial number\r\n");
    GetDeviceInfo(3);

    print("\r\nDevice query: get device parameters\r\n");
    regs.Words.HL = (int)deviceInfoBuffer;
    success = DeviceQuery(DEVQ_GET_PARAMS);
    if(success) {
        print("  Medium type:  ");
        switch(deviceInfoBuffer->mediumType) {
            case 0:
                print("Block device\r\n");
                break;
            case 1:
                print("CD/DVD\r\n");
                break;
            case 2:
                print("Other\r\n");
                break;
            default:
                printf("Unknown (%u)\r\n", deviceInfoBuffer->mediumType);
        }
        printf("  Sector size:  %u (0x%x)\r\n", deviceInfoBuffer->sectorSize, deviceInfoBuffer->sectorSize);
        printf("  Sector count: %lu (0x%lx)\r\n", deviceInfoBuffer->sectorCount, deviceInfoBuffer->sectorSize);

        flags = deviceInfoBuffer->flags;
        print("  Flags:\r\n");
        printf("    Removable: %s\r\n", YesOrNo(flags & 1));
        printf("    Read-only: %s\r\n", YesOrNo(flags & 2));
        printf("    Floppy:    %s\r\n", YesOrNo(flags & 4));
        printf("    Suitable for automapping: %s\r\n", YesOrNo(!(flags & 8)));
        printf("  Cylinders:     %u\r\n", deviceInfoBuffer->cylinders);
        printf("  Heads:         %u\r\n", deviceInfoBuffer->heads);
        printf("  Sectors/track: %u\r\n", deviceInfoBuffer->sectorsPerTrack);
    }

    if(invokeGetDeviceStatus) {
        print("\r\nDevice query: get device status\r\n");
        success = DeviceQuery(DEVQ_GET_STATUS);
        if(success) {
            printf("  Status: ");
            switch(regs.Bytes.B) {
                case 0:
                  print("Unavailable\r\n");
                  break;
                case 1:
                  print("Not changed\r\n");
                  break;
                case 2:
                  print("Changed\r\n");
                  break;
                case 3:
                  print("Unknown change status\r\n");
                  break;
                default:
                  printf("Unknown (%u)\r\n", regs.Bytes.B);
            }
        }
    }

    print("\r\nDevice query: get device availability\r\n");
    success = DeviceQuery(DEVQ_GET_AVAILABILITY);
    if(success) {
        printf("  Available: ");
        switch(regs.Bytes.B) {
            case 0:
                print("NO\r\n");
                break;
            case 1:
                print("YES\r\n");
                break;
            default:
                printf("Unknown (%u)\r\n", regs.Bytes.B);
        }
    }
}


char* YesOrNo(bool condition)
{
    return condition ? "YES" : "NO";
}

#define COM_FILE
#define SUPPORT_LONG
#include "print_msxdos.c"
#include "printf.c"
#include "asmcall.c"
