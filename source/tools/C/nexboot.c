/* Nextor temporary boot keys configuration tool
   By Konamiman 3/2019

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196
        --no-std-crt0 crt0_msxdos_advanced.rel msxchar.rel
        nexboot.c
   hex2bin -e com emufile.ihx
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "system.h"
#include "dos.h"
#include "asmcall.h"

/* Defines */

#define RamKeysAddress ((byte*)0xA100)
#define DisableAllKernelsKeyOffset 2
#define DisableAllKernelsBitMask 0x80

/* Strings */

const char* strTitle=
    "One-time boot keys configuration tool for Nextor v1.1\r\n"
    "By Konamiman, 6/2020\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: nexboot <keys>|. [*|<slot> [<slot> ...]]\r\n"
    "\r\n"
    "<keys>: keys that will be considered as pressed in next boot.\r\n"
    "<keys> can include the numbers 1 to 9, C (for CTRL) and S (for SHIFT)\r\n,"
    "or be '.' to not specify any key (if you only want to disable kernels).\r\n"
    "\r\n"
    "<slot>s: slot numbers of the Nextor kernels to disable in the next boot.\r\n"
    "They can be followed by a subslot number, e.g. 13 for slot 1, subslot 3.\r\n"
    "Or * to disable all the Nextor kernels (being v2.1 or newer)."
    "\r\n"
    "The computer will reset after successfully setting the keys.\r\n";

const char* strInvParam = "Invalid parameter";
const char* strInvSlot = "Invalid slot specification";
const char* strCRLF = "\r\n";

/* Global variables */

Z80_registers regs;
byte ASMRUT[4];
byte OUT_FLAGS;
byte keyFlags[5];

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
void InvalidParameter();
void ResetComputer();

/* MAIN */

int main(char** argv, int argc)
{
    ASMRUT[0] = 0xC3;
    memset(keyFlags, 0, 5);

    printf(strTitle);

    if(argc == 0) {
        printf(strUsage);
        Terminate(null);
    }

    if(argv[0][0] != '.') {
        GetNumericBits(argv[0]);
    }

    if(argc > 1) {
        GetSlotDisableKeys(argc-1, argv+1);
    }

    SetKeysInRam();

    printf("Done. Resetting computer...");
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

#define COM_FILE
#include "print_msxdos.c"
#include "printf.c"
#include "asmcall.c"
