/* Nextor temporary boot keys configuration tool
   By Konamiman 3/2019

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196 --no-std-crt0 -I../../../sdk/C/includes -I../../../sdk/C/code
        crt0_msxdos.rel asmcall.rel printf.rel print_msxdos.rel
        nexboot.c
   hex2bin -e com nexboot.ihx
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "msx_bios.h"      /* CALSLT */
#include "msx_workarea.h"  /* EXPTBL */
#include "dos_functions.h"
#include "dos_errors.h"
#include "persistent_storage.h"
#include "asmcall.h"

/* Defines */

#define RamKeysAddress ((byte*)0xA100)
#define DisableAllKernelsKeyOffset 2
#define DisableAllKernelsBitMask 0x80

/* Strings */

const char* strTitle=
    "Boot keys configuration tool for Nextor v3.0\r\n"
    "By Konamiman, 9/2026\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: nexboot <keys>|. [*|<slot> [<slot> ...]]\r\n"
    "       nexboot /p <keys>|.\r\n"
    "       nexboot /k\r\n"
    "       nexboot /i\r\n"
    "\r\n"
    "<keys>: keys considered as pressed in next boot, all of them in ONE\r\n"
    "single argument: the numbers 1 to 9, C (for CTRL), S (for SHIFT); e.g.\r\n"
    "1C. Or '.' for no key at all (if you only want to disable kernels).\r\n"
    "\r\n"
    "<slot>s: slot numbers of the Nextor kernels to disable in the next boot.\r\n"
    "They can be followed by a subslot number, e.g. 13 for slot 1, subslot 3.\r\n"
    "Or * to disable all the Nextor kernels (being v2.1 or newer).\r\n"
    "The computer will reset after successfully setting these keys.\r\n"
    "\r\n"
    "/p <keys>: store in the persistent storage the keys whose meaning is\r\n"
    "INVERTED at every boot: an inverted key acts as pressed when you don't\r\n"
    "press it, and as not pressed when you do. Only 1 to 6, C and S can be\r\n"
    "inverted, and as above they all go in one single argument, e.g. /p C6\r\n"
    "(there are no kernels to disable here). '.' stores 'no key inverted'.\r\n"
    "/k: remove the stored keys, so the ones in the kernel ROM are used.\r\n"
    "/i: show the stored keys and the kind of persistent storage in use.\r\n"
    "\r\n"
    "/p, /k and /i don't reset the computer, they take effect on the next\r\n"
    "boot, and they don't affect the persistent disk emulation mode.\r\n";

const char* strInvParam = "Invalid parameter";
const char* strInvSlot = "Invalid slot specification";
const char* strCRLF = "\r\n";

/* Global variables */

Z80_registers regs;
byte keyFlags[5];

byte psBuffer[PSD_MAX_SIZE];
persistentStorageInfo psInfo;
byte psSectors;


//First value: byte offset in keyFlags
//Second value: mask to set in the byte in keyFlags
byte keysBySlot[16*2] = {
    3, 0b01000000,	//0-0: U
	3, 0b00000100,	//1-0: Q
	1, 0b00000100,	//2-0: A
	4, 0b00001000,	//3-0: Z

	2, 0b00000100,	//0-1: I
    4, 0b00000001,	//1-1: W
	3, 0b00010000,	//2-1: S
	4, 0b00000010,	//3-1: X

	3, 0b00000001,	//0-2: O
	1, 0b01000000,	//1-2: E
	1, 0b00100000,	//2-2: D
	1, 0b00010000,	//3-2: C

	3, 0b00000010,	//0-3: P
	3, 0b00001000,	//1-3: R
	1, 0b10000000,	//2-3: F 
	3, 0b10000000,	//3-3: V
};

/* Function prototypes */

void GetSlotDisableKeys(int count, char** slots);
void GetNumericBits(char* keys);
void SetKeysInRam();
void Terminate(const char* errorMessage);
void TerminateWithDosError(byte errorCode);
void InvalidParameter();
void ResetComputer();
void CheckDosVersion();
void GetPersistentBits(char* keys);
void SetPersistentKeys();
void RemovePersistentKeys();
void ShowPersistentInfo();
void PrintStoredKeys();
void ProcessOption(char** argv, int argc);
void print(char* s);
void PsLoad();
void PsSave();

/* MAIN */

int main(char** argv, int argc)
{
    memset(keyFlags, 0, 5);

    printf(strTitle);

    if(argc == 0) {
        printf(strUsage);
        Terminate(null);
    }

    CheckDosVersion();

    if(argv[0][0] == '/') {
        ProcessOption(argv, argc);
        Terminate(null);   /* these options never reset the computer */
    }

    if(argv[0][0] != '.') {
        GetNumericBits(argv[0]);
    }

    if(argc > 1) {
        GetSlotDisableKeys(argc-1, argv+1);
    }

    SetKeysInRam();

    print("Done. Resetting computer...");
    ResetComputer();

    return 0;
}

void GetNumericBits(char* keys)
{
    byte keysCount;
    byte currentKey;
    byte i;

    keysCount = strlen(keys);
    for(i=0; i<keysCount; i++) {
        currentKey = keys[i] | 32;

        if(currentKey >= '1' && currentKey <= '7') {
            keyFlags[0] |= (1 << currentKey-'0');
        }
        else if(currentKey == '8' || currentKey == '9') {
            keyFlags[1] |= (1 << currentKey-'8');
        }
        else if(currentKey == 's') {
            keyFlags[4] |= 0x10;
        }
        else if(currentKey == 'c') {
            keyFlags[4] |= 0x20;
        }
        else {
            Terminate("Invalid key specification");
        }
    }
}

void GetSlotDisableKeys(int count, char** slots)
{
    int i;
    char* slot;
    byte slotNumber;

    for(i=0; i<count; i++) {
        slot = slots[i];

        if(slot[0] == '*') {
            keyFlags[DisableAllKernelsKeyOffset] |= DisableAllKernelsBitMask;
            continue;
        }

        if(slot[0] < '0' || slot[0] > '3') {
            Terminate(strInvSlot);
        }

        slotNumber = slot[0] - '0';

        if(slot[1] != '\0') {
            if(slot[1] < '0' || slot[1] > '3') {
               Terminate(strInvSlot);
            }

            slotNumber |= (slot[1] - '0') << 2;
        }

        keyFlags[keysBySlot[slotNumber*2]] |= keysBySlot[(slotNumber*2)+1];
    }
}

void SetKeysInRam()
{
    strcpy(RamKeysAddress, "NEXTOR_BOOT_KEYS");
    memcpy(RamKeysAddress+0x11, keyFlags, 5);
}

/* Dispatch of the options that work on the persistent storage. */

void ProcessOption(char** argv, int argc)
{
    byte option;

    option = argv[0][1];
    if(option == '\0' || argv[0][2] != '\0') {
        InvalidParameter();   /* before |32, since '\0'|32 isn't zero */
    }
    option |= 32;

    if(option == 'p') {
        if(argc == 1) {
            Terminate("/p needs the keys to store, or '.' for none");
        }
        if(argc > 2) {
            Terminate("All the keys to invert go in one argument, e.g. /p C6");
        }
        if(argv[1][0] != '.' || argv[1][1] != '\0') {
            GetPersistentBits(argv[1]);
        }
        SetPersistentKeys();
    }
    else if(option == 'k') {
        if(argc > 1) {
            InvalidParameter();
        }
        RemovePersistentKeys();
    }
    else if(option == 'i') {
        if(argc > 1) {
            InvalidParameter();
        }
        ShowPersistentInfo();
    }
    else {
        InvalidParameter();
    }
}

/* Like GetNumericBits, but only for the keys that can be inverted. */

void GetPersistentBits(char* keys)
{
    byte keysCount;
    byte currentKey;
    byte i;

    keysCount = strlen(keys);
    for(i=0; i<keysCount; i++) {
        currentKey = keys[i] | 32;

        if(currentKey >= '1' && currentKey <= '6') {
            keyFlags[0] |= (1 << (currentKey-'0'));
        }
        else if(currentKey == 's') {
            keyFlags[4] |= 0x10;
        }
        else if(currentKey == 'c') {
            keyFlags[4] |= 0x20;
        }
        else if(currentKey >= '7' && currentKey <= '9') {
            Terminate("Only the keys 1 to 6, C and S can be inverted");
        }
        else {
            Terminate("Invalid key specification");
        }
    }
}

void SetPersistentKeys()
{
    PsLoad();
    psBuffer[PSD_KEYS] = keyFlags[0] & PSD_KEYSM_LOW;
    psBuffer[PSD_KEYS + 1] = keyFlags[4] & PSD_KEYSM_HIGH;
    PsSave();

    print("Stored, will take effect on the next boot.\r\n");
    PrintStoredKeys();
}

void RemovePersistentKeys()
{
    PsLoad();

    if(psBuffer[PSD_KEYS] == 0xFF && psBuffer[PSD_KEYS + 1] == 0xFF) {
        print("No boot keys were stored.\r\n");
        return;
    }

    psBuffer[PSD_KEYS] = 0xFF;
    psBuffer[PSD_KEYS + 1] = 0xFF;
    PsSave();

    print("Removed. From the next boot the keys inverted in the\r\n");
    print("kernel ROM will be used again.\r\n");
}

void ShowPersistentInfo()
{
    PsLoad();

    /* Device 0 isn't a device: it's what a future version of Nextor would
       report for a storage that isn't a file in one of them. */
    if(psInfo.deviceIndex == 0) {
        print("Persistent storage: in the controller\r\n");
    }
    else {
        printf("Persistent storage: file in device %i\r\n", psInfo.deviceIndex);
    }

    PrintStoredKeys();
}

void PrintStoredKeys()
{
    byte low;
    byte high;
    byte i;
    bool any;

    low = psBuffer[PSD_KEYS];
    high = psBuffer[PSD_KEYS + 1];

    if(low == 0xFF && high == 0xFF) {
        print("Inverted boot keys: not configured\r\n");
        print("(the ones in the kernel ROM are used)\r\n");
        return;
    }

    print("Inverted boot keys: ");

    any = false;
    for(i=1; i<=6; i++) {
        if(low & (1 << i)) {
            printf("%i ", i);
            any = true;
        }
    }
    if(high & 0x20) {
        print("CTRL ");
        any = true;
    }
    if(high & 0x10) {
        print("SHIFT ");
        any = true;
    }
    if(!any) {
        print("none");
    }
    print("\r\n");
}

/* Persistent storage access. The format handling itself lives in the SDK
   (sdk/C/code/persistent_storage.c); these two just add the error policy
   of this tool, which is to give up with a message. */

void PsLoad()
{
    byte error;

    error = PsLoadData(psBuffer, PSD_MAX_SIZE, &psInfo, &psSectors);

    if(error == _IBDOS) {
        Terminate("This version of Nextor doesn't have persistent storage");
    }
    if(error == _IDRVR) {
        Terminate("No persistent storage available");
    }
    if(error != 0 && error != _NOFIL) {
        TerminateWithDosError(error);
    }
}

void PsSave()
{
    byte error;

    error = PsSaveData(psBuffer, psSectors);
    if(error != 0) {
        TerminateWithDosError(error);
    }
}

void CheckDosVersion()
{
    regs.Bytes.B = 0x5A;
    regs.Words.HL = 0x1234;
    regs.Words.DE = (int)0xABCD;
    regs.Words.IX = 0;
    DosCall(_DOSVER, &regs, REGS_ALL, REGS_ALL);

    if(regs.Bytes.B < 2 || regs.Bytes.IXh != 1) {
        Terminate("This program is for Nextor only.");
    }

    if(regs.Bytes.IXl < 3) {
        Terminate("This program needs Nextor 3 or newer.\r\nUse an older version of NEXBOOT for Nextor 2.");
    }
}

void InvalidParameter()
{
    Terminate(strInvParam);
}

void TerminateWithDosError(byte errorCode)
{
    regs.Bytes.B = errorCode;
    DosCall(_TERM, &regs, REGS_MAIN, REGS_NONE);
    DosCall(_TERM0, &regs, REGS_MAIN, REGS_NONE);
}

void Terminate(const char* errorMessage)
{
    if(errorMessage != NULL) {
        printf("\r\x1BK*** %s\r\n", errorMessage);
    }
    
    regs.Bytes.B = (errorMessage == NULL ? 0 : 1);
    DosCall(_TERM, &regs, REGS_MAIN, REGS_NONE);
    DosCall(_TERM0, &regs, REGS_MAIN, REGS_NONE);
}

void ResetComputer()
{
    regs.Bytes.IYh = *(byte*)EXPTBL;
    regs.Words.IX = 0;
    AsmCall(CALSLT, &regs, REGS_ALL, REGS_NONE);

    //Just in case, but we should never reach here
    Terminate(null);
}
