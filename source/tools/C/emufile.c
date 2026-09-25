/* DSK emulation configuration file creation tool for Nextor v1.1
   By Konamiman 3/2019

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196 --no-std-crt0 -I../../../sdk/C/includes -I../../../sdk/C/code
          crt0_msxdos.rel asmcall.rel printf.rel print_msxdos.rel strcmpi.rel emufile.c
   hex2bin -e com emufile.ihx
*/

/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "asmcall.h"
#include "types.h"
#include "data_structures.h"
#include "dos_functions.h"
#include "dos_errors.h"
#include "drivers.h"
#include "driver_routines.h"
#include "msx_bios.h"
#include "msx_workarea.h"
#include "partit.h"
#include "persistent_storage.h"
#include "strcmpi.h"

	/* Typedefs */

typedef struct {
    char signature[16];
    byte numberOfEntriesInImagesTable;
    byte indexOfImageToMountAtBoot;
    uint workAreaAddress;
    byte flags;
    byte reserved[3];
} GeneratedFileHeader;

typedef struct {
    byte deviceIndex;
    byte logicalUnitNumber;
    ulong firstFileSector;
    uint fileSizeInSector;
} GeneratedFileTableEntry;
    
	/* Defines */

#define IS_NEXTOR (1 << 7)
#define IS_DEVICE_BASED (1)

#define EMU_FLAG_50HZ (1 << 1)
#define EMU_FLAG_60HZ (1 << 2)
#define EMU_FLAG_INVERT_CTRL (1 << 3)
#define EMU_FLAG_INVERT_SHIFT (1 << 4)
#define EMU_FLAG_R800 (1 << 5)
#define EMU_FLAG_SKIP_FIRMWARE (1 << 6)

#define PARSE_FLAG_HAS_FILENAME (1 << 3)
#define PARSE_FLAG_HAS_EXTENSION (1 << 4)
#define PARSE_FLAG_IS_AMBIGUOUS (1 << 5)

#define MallocBase 0x8000

#define MaxFilesToProcess 32

#define SetupRAMAddress ((byte*)0xA000)



/* Strings */

const char* strTitle=
    "Disk image emulation tool for Nextor v3.0\r\n"
    "By Konamiman, 9/2026\r\n"
    "\r\n";

const char* strUsage=
    "Usage: emufile [<options>] <output file> <files> [<files> ...]\r\n"
    "       emufile set <data file> [o|p] [-5|-6] [-c] [-s] [-8] [-f] [-x]\r\n"
    "       emufile k\r\n"
    "       emufile ?\r\n";

const char* strHelp=
    "* To create an emulation data file:\r\n"
    "\r\n"
    "emufile [<options>] <output file> <files> [<files> ...]\r\n"
    "\r\n"
    "<output file>: Path and name of emulation data file to create.\r\n"
    "               Default extension is .EMU\r\n"
    "\r\n"
    "<options>:\r\n"
    "\r\n"
    "-b <number>: The index of the image file to mount at boot time.\r\n"
    "             Must be 1-9 or A-W. Default is 1.\r\n"
    "-a <address>: Page 3 address for the 16 byte work area.\r\n"
    "              Must be a hexadecimal number between C000 and FFEF.\r\n"
    "              If missing or 0, the work area is allocated at boot time.\r\n"
    "-p : Print filenames and associated keys after creating the data file.\r\n"
    "-5 : Force the screen to 50Hz when the emulation session starts.\r\n"
    "-6 : Force the screen to 60Hz when the emulation session starts.\r\n"
    "     These two have no effect on MSX1 computers.\r\n"
    "-c : Free memory for the game by disabling the ghost floppy drive when\r\n"
    "     the emulation session starts.\r\n"
    "-s : Free more memory by disabling the MSX-DOS kernels (e.g. the floppy\r\n"
    "     disk drive) when the emulation session starts.\r\n"
    "-8 : Boot the emulation session in R800 mode (turbo R only).\r\n"
    "-f : Disable the built-in firmware of the computer (like pressing\r\n"
    "     the 7 key at boot time).\r\n"
    "     All of these work for both the one-time and persistent variants.\r\n"
    "\r\n"
    "TYPE /B the generated file to see the names of the registered files.\r\n"
    "\r\n"
    "\r\n"
    "* To setup an existing emulation data file for booting:\r\n"
    "\r\n"
    "emufile set <data file> [o|p[<device index>[<LUN index>]]] [-5|-6] [-c] [-s] [-8] [-f] [-x]\r\n"
    "\r\n"
    "Default extension for <data file> is EMU. A directory can be specified instead,\r\n"
    "in that case, a file with the same name and .EMU extension inside the directory\r\n"
    "will be used.\r\n"
    "\r\n"
    "o: one-time emulation (store emulation data file pointer in RAM).\r\n"
    "This is the default if only <data file> is specified.\r\n"
    "\r\n"
    "p: persistent emulation (store emulation data file pointer in the\r\n"
    "persistent storage of the primary controller; this fails if there's none).\r\n"
    "\r\n"
    "-5 or -6: force the screen to 50Hz or 60Hz for this emulation session\r\n"
    "(this overrides the setting stored in the emulation data file).\r\n"
    "\r\n"
    "-c: disable the ghost floppy drive to free memory for the game.\r\n"
    "-s: disable the MSX-DOS kernels (e.g. the floppy drive) to free even more.\r\n"
    "-8: boot the emulation session in R800 mode (turbo R only). Like -5/-6,\r\n"
    "this works for both variants and can be stored in the data file.\r\n"
    "-f: disable the built-in firmware of the computer (like the 7 key).\r\n"
    "\r\n"
    "-x: ignore all the flags stored in the data file, apply only the flags\r\n"
    "passed in this command line. The order of the arguments doesn't matter,\r\n"
    "so \"-5 -s -x\" is the same as \"-x -5 -s\".\r\n"
    "\r\n"
    "The computer will reset after successfully finishing the setup.\r\n"
    "\r\n"
    "\r\n"
    "* To disable the persistent emulation:\r\n"
    "\r\n"
    "emufile k\r\n"
    "\r\n"
    "Boot with the 0 key pressed to skip the emulation once, then run this.\r\n"
    "CALL EMUKILL in BASIC does the same.\r\n";

const char* strInvParam = "Invalid parameter";
const char* strCRLF = "\r\n";

const char* emuDataSignature = "NEXTOR_EMU_DATA";

/* Global variables */

Z80_registers regs;
void* mallocPointer;
char* outputFileName;
int bootFileIndex;
bool printFilenames;
uint workAreaAddress;
fileInfoBlock* fib;
int totalFilesProcessed;
driveLetterInfo* driveInfo;
byte* driveParameters;
byte* fileContentsBase;
byte* fileNamesBase;
byte* fileNamesAppendAddress;
byte fileHandle;
bool setupAsPersistent;
byte* psBuffer;
persistentStorageInfo psInfo;
byte psSectors;
byte hzFlags;
bool invertCtrl;
bool invertShift;
bool bootR800;
bool skipFirmware;
bool resetFileFlags;
byte fileFlags;

/* Some handy code defines */

#define PrintNewLine() print(strCRLF)
#define InvalidParameter() Terminate(strInvParam)
#define PrimaryControllerSlot() (*(byte*)0xF348)

/* Function prototypes */

void CheckPreconditions();
void CheckPrimaryControllerIsNextor();
void Initialize();
bool ProcessArguments(char** argv, int argc);
void ProcessCreateFileArguments(char** argv, int argc);
void ProcessSetupFileArguments(char** argv, int argc);
void ProcessSetupVariantArgument(char* arg);
int ProcessOption(char optionLetter, char* optionValue);
bool IsHzOption(char optionLetter);
void ProcessHzOption(char optionLetter);
char* HzFlagsToString();
byte EmuFlagsByte();
byte EffectiveHzFlags();
bool EffectiveInvertCtrl();
bool EffectiveInvertShift();
bool EffectiveBootR800();
bool EffectiveSkipFirmware();
byte PointerFlagsByte();
void ProcessFilename(char* fileName);
void TooManyFiles();
void StartSearchingFiles(char* fileName);
bool DirectoryExists(char* dirName);
void ProcessFileFound();
void GetDriveInfoForFileInFib();
void CheckControllerForFileInFib();
void CheckConsecutiveClustersForFileInFib();
ulong GetFirstFileSectorForFileInFib();
void AddFileInFibToFilesTable(ulong sector);
void SetDeviceIndexesOfFilesTableToZeroIfAllInSameDeviceAsDataFile();
void AddFileInFibToFilenamesInfo();
void GenerateFile();
void ConvertDirectoryToFilename(char* fileOrDirectoryName);
void AddFileExtension(char* fileName);
void ProcessBootIndexOption(char* optionValue);
void ProcessWorkAreaAddressOption(char* optionValue);
void ProcessPrintFilenamesOption();
void Terminate(const char* errorMessage);
void TerminateWithDosError(byte errorCode);
void print(char* s);
void CheckDosVersion();
void* malloc(int size);
uint ParseHex(char* hexString);
void DoDosCall(byte functionCode);
void SetupFile();
void PsLoad();
void PsSave();
void KillPersistentEmulation();
void VerifyDataFileSignature(byte* sectorBuffer);
void ResetComputer();

	/* MAIN */
	
int main(char** argv, int argc)
{
    bool isSetupFile;

	print(strTitle);

    CheckPreconditions();
	Initialize();
	isSetupFile = ProcessArguments(argv, argc);

    if(isSetupFile) {
        SetupFile();
        printf("Done. Resetting computer...");
        ResetComputer();
    }
    if(totalFilesProcessed > 0) {
        SetDeviceIndexesOfFilesTableToZeroIfAllInSameDeviceAsDataFile();
        GenerateFile();
        printf(
            "%s%s successfully generated!\r\n%i disk image file(s) registered\r\n",
            printFilenames ? "\r\n" : "", outputFileName, totalFilesProcessed);
        if(hzFlags != 0) {
            printf("Screen mode forced to %s\r\n", HzFlagsToString());
        }
        if(invertCtrl) {
            print("Ghost floppy drive will be disabled\r\n");
        }
        if(invertShift) {
            print("MSX-DOS kernels will be disabled\r\n");
        }
        if(bootR800) {
            print("Will boot in R800 mode (turbo R only)\r\n");
        }
        if(skipFirmware) {
            print("Built-in firmware will be disabled\r\n");
        }
    } else {
        print(strUsage);
    }
    
	Terminate(null);
	return 0;
}

/* Functions */

void CheckPreconditions()
{
    CheckDosVersion();
    CheckPrimaryControllerIsNextor();
}

void CheckPrimaryControllerIsNextor()
{
    byte flags;
    
    regs.Bytes.A = 0;
    regs.Bytes.D = PrimaryControllerSlot();
    regs.Bytes.E = 0xFF;
    regs.Words.HL = (int)MallocBase;
    
    DoDosCall(_GDRVR);
    
    flags = ((byte*)MallocBase)[4];
    if((flags & (IS_NEXTOR | IS_DEVICE_BASED)) != (IS_NEXTOR | IS_DEVICE_BASED)) {
        Terminate("The primary controller is not a Nextor kernel.");
    }
}

void Initialize()
{
	mallocPointer = (void*)MallocBase;
	
	outputFileName = malloc(128);
    *outputFileName = (char)0;
    
    fib = malloc(sizeof(fileInfoBlock));
    driveInfo = malloc(sizeof(driveLetterInfo));
    driveParameters = malloc(32);
	fileContentsBase = malloc(
        sizeof(GeneratedFileHeader) +
        (sizeof(GeneratedFileTableEntry) * MaxFilesToProcess));
    fileNamesBase = malloc(19 * MaxFilesToProcess);
    fileNamesAppendAddress = fileNamesBase;
    
    fileHandle = 0;
	bootFileIndex = 1;
    printFilenames = false;
    workAreaAddress = 0;
    totalFilesProcessed = 0;
    hzFlags = 0;
    invertCtrl = false;
    invertShift = false;
    bootR800 = false;
    skipFirmware = false;
    resetFileFlags = false;
    fileFlags = 0;
}

bool ProcessArguments(char** argv, int argc) 
{
    if(argc > 0 && argv[0][0] == '?') {
        print(strHelp);
        Terminate(null);
    }

    if(argc == 1 && (argv[0][0] | 32) == 'k' && argv[0][1] == '\0') {
        KillPersistentEmulation();
        Terminate(null);
    }

	if(argc < 2) {
        print(strUsage);
        Terminate(null);
    }

    if(strcmpi(argv[0], "set") == 0) {
        ProcessSetupFileArguments(argv, argc);
        return true;
    }

    ProcessCreateFileArguments(argv, argc);
    return false;
}

void ProcessCreateFileArguments(char** argv, int argc)
{
    int i;
	char* currentArg;
    bool processingOptions;

    processingOptions = true;

    for(i=0; i<argc; i++) {
	    currentArg = argv[i];
		if(currentArg[0] == '-') {
            if(processingOptions) {
		        i += ProcessOption(currentArg[1], argv[i+1]);
            } else {
                Terminate("Can't process more options after filenames");
            }
        } else if(*outputFileName == null) {
            processingOptions = false;
            AddFileExtension(currentArg);
		} else if(totalFilesProcessed < MaxFilesToProcess) {
		    ProcessFilename(currentArg);
		} else {
            TooManyFiles();
        }
	}

    if(*outputFileName == null) {
        Terminate("No output file name specified");
    }

    if(totalFilesProcessed == 0) {
        Terminate("No disk image files to emulate specified");
    }
}

void ProcessSetupFileArguments(char** argv, int argc)
{
    int i;
    bool variantSpecified;

    setupAsPersistent = false;
    variantSpecified = false;

    strcpy(outputFileName, argv[1]);

    for(i=2; i<argc; i++) {
        if(argv[i][0] == '-') {
            if(IsHzOption(argv[i][1])) {
                ProcessHzOption(argv[i][1]);
            } else if((argv[i][1] | 32) == 'c') {
                invertCtrl = true;
            } else if((argv[i][1] | 32) == 's') {
                invertShift = true;
            } else if(argv[i][1] == '8') {
                bootR800 = true;
            } else if((argv[i][1] | 32) == 'f') {
                skipFirmware = true;
            } else if((argv[i][1] | 32) == 'x') {
                resetFileFlags = true;
            } else {
                InvalidParameter();
            }
        } else if(variantSpecified) {
            InvalidParameter();
        } else {
            ProcessSetupVariantArgument(argv[i]);
            variantSpecified = true;
        }
    }
}

void ProcessSetupVariantArgument(char* arg)
{
    byte paramFirstChar;

    paramFirstChar = arg[0] | 32;

    if(paramFirstChar == 'o') {
        return;
    }
    else if(paramFirstChar != 'p') {
        InvalidParameter();
    }

    setupAsPersistent = true;

    if(arg[1] != '\0') {
        Terminate("The emulation data pointer goes to the persistent storage:\r\nuse just 'p', with no device index");
    }
}

int ProcessOption(char optionLetter, char* optionValue)
{
    optionLetter |= 32;

	if(optionLetter == 'b') {
	    ProcessBootIndexOption(optionValue);
		return 1;
	}

	if(optionLetter == 'a') {
	    ProcessWorkAreaAddressOption(optionValue);
		return 1;
	}

    if(optionLetter == 'p') {
	    ProcessPrintFilenamesOption();
		return 0;
	}

    if(IsHzOption(optionLetter)) {
        ProcessHzOption(optionLetter);
        return 0;
    }

    if(optionLetter == 'c') {
        invertCtrl = true;
        return 0;
    }

    if(optionLetter == 's') {
        invertShift = true;
        return 0;
    }

    if(optionLetter == '8') {
        bootR800 = true;
        return 0;
    }

    if(optionLetter == 'f') {
        skipFirmware = true;
        return 0;
    }

	InvalidParameter();
	return 0;
}

/* Only the character right after the "-" matters, so that
   "-50", "-50Hz", "-60hz" etc. are accepted too. */

bool IsHzOption(char optionLetter)
{
    return optionLetter == '5' || optionLetter == '6';
}

void ProcessHzOption(char optionLetter)
{
    byte flag;

    flag = optionLetter == '5' ? EMU_FLAG_50HZ : EMU_FLAG_60HZ;
    if(hzFlags != 0 && hzFlags != flag) {
        Terminate("Can't force both 50Hz and 60Hz");
    }
    hzFlags = flag;
}

char* HzFlagsToString()
{
    if(hzFlags == EMU_FLAG_50HZ) {
        return "50Hz";
    }
    if(hzFlags == EMU_FLAG_60HZ) {
        return "60Hz";
    }
    return "none";
}

/* The emulation flags byte stored in the data file header. The kernel doesn't
   read it: 'emufile set' reads it back and combines it with the command line
   flags into the flags of the emulation data pointer (see PointerFlagsByte). */
byte EmuFlagsByte()
{
    return hzFlags |
        (invertCtrl ? EMU_FLAG_INVERT_CTRL : 0) |
        (invertShift ? EMU_FLAG_INVERT_SHIFT : 0) |
        (bootR800 ? EMU_FLAG_R800 : 0) |
        (skipFirmware ? EMU_FLAG_SKIP_FIRMWARE : 0);
}

/* Effective flags applied by 'emufile set': the command line ones combined
   with those stored in the data file, unless -x was given (then the stored
   ones are ignored and only the command line ones apply). The kernel reads
   the Hz bits from the pointer only, so the resolved value is written there. */

byte EffectiveHzFlags()
{
    if(hzFlags != 0) {
        return hzFlags;
    }
    return resetFileFlags ? 0 : (fileFlags & (EMU_FLAG_50HZ | EMU_FLAG_60HZ));
}

bool EffectiveInvertCtrl()
{
    return invertCtrl || (!resetFileFlags && (fileFlags & EMU_FLAG_INVERT_CTRL) != 0);
}

bool EffectiveInvertShift()
{
    return invertShift || (!resetFileFlags && (fileFlags & EMU_FLAG_INVERT_SHIFT) != 0);
}

bool EffectiveBootR800()
{
    return bootR800 || (!resetFileFlags && (fileFlags & EMU_FLAG_R800) != 0);
}

bool EffectiveSkipFirmware()
{
    return skipFirmware || (!resetFileFlags && (fileFlags & EMU_FLAG_SKIP_FIRMWARE) != 0);
}

/* The flags the kernel reads from the emulation data pointer: the frequency
   bits, the R800 bit, and the three that force the CTRL, SHIFT and 7 boot
   keys.
   The kernel acts on all of them in both the one-time and the persistent
   emulation variants. */
byte PointerFlagsByte()
{
    byte flags;

    flags = EffectiveHzFlags() | (EffectiveBootR800() ? EMU_FLAG_R800 : 0);

    if(EffectiveInvertCtrl()) flags |= EMU_FLAG_INVERT_CTRL;
    if(EffectiveInvertShift()) flags |= EMU_FLAG_INVERT_SHIFT;
    if(EffectiveSkipFirmware()) flags |= EMU_FLAG_SKIP_FIRMWARE;

    return flags;
}

void ConvertDirectoryToFilename(char* fileOrDirectoryName)
{
    int dirNameLength;

    if(!DirectoryExists(fileOrDirectoryName))
        return;

    regs.Bytes.B = 0;
    regs.Words.DE = (int)fileOrDirectoryName;
    DoDosCall(_PARSE);

    dirNameLength = strlen((char*)regs.Words.HL);

    strcat(fileOrDirectoryName, "\\");
    strncat(fileOrDirectoryName, (char*)regs.Words.HL, dirNameLength);
    strcat(fileOrDirectoryName, ".EMU");
}

void AddFileExtension(char* fileName)
{
    strcpy(outputFileName, fileName);

    regs.Bytes.B = 0;
    regs.Words.DE = (int)outputFileName;
    DoDosCall(_PARSE);

    if(!(regs.Bytes.B & PARSE_FLAG_HAS_EXTENSION)) {
        strcpy((char*)regs.Words.DE, ".EMU");
    }
}

void ProcessBootIndexOption(char* optionValue)
{
    char index;
    
	if(optionValue[1] != 0) {
	    InvalidParameter();
	}
	
	index = *optionValue | 32;
	
	if(index >= '1' && index <= '9') {
        bootFileIndex = index - '0';
	} else if(index >= 'a' && index <= 'w') {
        bootFileIndex = index - 'a' + 10;
    } else {
        InvalidParameter();
    }
}

void ProcessWorkAreaAddressOption(char* optionValue)
{
	workAreaAddress = ParseHex(optionValue);
    
    if(workAreaAddress != 0 && (workAreaAddress < 0xC000 || workAreaAddress > 0xFFEF)) {
        InvalidParameter();
    }
}

void ProcessPrintFilenamesOption()
{
    printFilenames = true;
}

void ProcessFilename(char* fileName) 
{ 
    StartSearchingFiles(fileName);
    
    while(regs.Bytes.A == 0 && totalFilesProcessed < MaxFilesToProcess) {
        ProcessFileFound();
        DoDosCall(_FNEXT);
    } 
    
    if(regs.Bytes.A == 0 && totalFilesProcessed == MaxFilesToProcess) {
        TooManyFiles();
    }
 }

void TooManyFiles()
{
    printf("*** Too many files specified, maximum is %i\r\n", MaxFilesToProcess);
    Terminate(null);
}

void StartSearchingFiles(char* fileName)
{
    regs.Words.DE = (int)fileName;
    regs.Bytes.B = 0;
    regs.Words.IX = (int)fib;
    
    DoDosCall(_FFIRST);
}

bool DirectoryExists(char* dirName)
{
    regs.Words.DE = (int)dirName;
    regs.Bytes.B = ATTR_SUB_DIR;
    regs.Words.IX = (int)fib;
    
    DosCall(_FFIRST, &regs, REGS_ALL, REGS_ALL);
    if(regs.Bytes.A == _NOFIL)
        return false;
    if(regs.Bytes.A != 0)
        TerminateWithDosError(regs.Bytes.A);

    return (fib->attributes & ATTR_SUB_DIR) != 0;
}

void ProcessFileFound()
{
    char key;
	ulong sector;

	GetDriveInfoForFileInFib();
    CheckControllerForFileInFib();
    CheckConsecutiveClustersForFileInFib();
    
    if(fib->fileSize < 512) {
        printf("*** %s is too small (< 512 bytes) or empty - skipped\r\n", fib->filename);
        return;
    }
    
    if(fib->fileSize >= (32768 * 1024)) {
        printf("*** %s is too big (> 32 MBytes) - skipped\r\n", fib->filename);
        return;
    }
    
	sector = driveInfo->firstSectorNumber + GetFirstFileSectorForFileInFib();
	AddFileInFibToFilesTable(sector);
	AddFileInFibToFilenamesInfo();
	
    totalFilesProcessed++;

    if(printFilenames) {
        key = totalFilesProcessed < 10 ? totalFilesProcessed + '0' : totalFilesProcessed - 10 + 'A';
        printf("%c -> %s\r\n", key, fib->filename);
    }
}

void GetDriveInfoForFileInFib()
{
	regs.Bytes.A = fib->logicalDrive - 1;
    regs.Words.HL = (int)driveInfo;
    DoDosCall(_GDLI);
}

void CheckControllerForFileInFib()
{
    if(driveInfo->driveStatus != DRIVE_STATUS_ASSIGNED_TO_DEVICE || driveInfo->driverSlotNumber != PrimaryControllerSlot()) {
        printf("*** Drive %c: is not controlled by the primary Nextor kernel\r\n", fib->logicalDrive - 1 + 'A');
        Terminate(null);
    }
}

void CheckConsecutiveClustersForFileInFib()
{
    uint currentCluster;
    clusterInfo* ci;

    currentCluster = fib->startCluster;
    ci = malloc(sizeof(clusterInfo));

    printf("Checking FAT chain for %s... ", fib->filename);
    while(true)
    {
        regs.Bytes.A = fib->logicalDrive;
        regs.Words.HL = (int)ci;
        regs.UWords.DE = currentCluster;
        DoDosCall(_GETCLUS);

        if(ci->flags.isLastClusterOfFile)
        {
            print("Ok!\r\n");
            return;
        }

        if(ci->fatEntryValue != currentCluster + 1)
        {
            print("Error!\r\n*** The file is not stored across consecutive sectors in disk");
            Terminate(null);
        }

        currentCluster++;
    }
}

ulong GetFirstFileSectorForFileInFib()
{
    ulong firstDataSector;
    byte sectorsPerCluster;

    regs.Words.DE = (int)driveParameters;
    regs.Bytes.L = fib->logicalDrive;
    DoDosCall(_DPARM);
    firstDataSector = *(uint*)(driveParameters+15);
    sectorsPerCluster = *(byte*)(driveParameters+3);
    
    return
        firstDataSector +
        ((ulong)(fib->startCluster - 2) * (ulong)sectorsPerCluster);
}

void AddFileInFibToFilesTable(ulong sector)
{
    GeneratedFileTableEntry* tableEntry;
    
    tableEntry =
        (GeneratedFileTableEntry*)
            (fileContentsBase + 
            sizeof(GeneratedFileHeader) + 
            (sizeof(GeneratedFileTableEntry) * totalFilesProcessed));
            
    tableEntry->deviceIndex = driveInfo->deviceIndex;
    tableEntry->logicalUnitNumber = driveInfo->logicalUnitNumber;
    tableEntry->firstFileSector = sector;
    tableEntry->fileSizeInSector = (uint)(fib->fileSize >> 9);
}

void AddFileInFibToFilenamesInfo()
{
    int fileIndex = totalFilesProcessed + 1;
    
    sprintf(fileNamesAppendAddress, "%c -> ", fileIndex <= 9 ? fileIndex + '0' : fileIndex - 10 + 'A');
    fileNamesAppendAddress += 5;
    strcpy(fileNamesAppendAddress, fib->filename);
    fileNamesAppendAddress += strlen(fib->filename);
    *fileNamesAppendAddress++ = '\r';
    *fileNamesAppendAddress++ = '\n';
}

void SetDeviceIndexesOfFilesTableToZeroIfAllInSameDeviceAsDataFile()
{
    GeneratedFileTableEntry* tableEntry;
    int i;

    regs.Bytes.B = 0;
    regs.Words.DE = (int)outputFileName;
    DoDosCall(_PARSE);
    regs.Bytes.A = regs.Bytes.C - 1;    //drive number of the data file
    regs.Words.HL = (int)driveInfo;
    DoDosCall(_GDLI);

    tableEntry = (GeneratedFileTableEntry*)(fileContentsBase + sizeof(GeneratedFileHeader));

    for(i=0; i<totalFilesProcessed; i++)
    {
        if(tableEntry->deviceIndex != driveInfo->deviceIndex || tableEntry->logicalUnitNumber != driveInfo->logicalUnitNumber) {
            return;
        }
        tableEntry++;
    }

    tableEntry = (GeneratedFileTableEntry*)(fileContentsBase + sizeof(GeneratedFileHeader));

    for(i=0; i<totalFilesProcessed; i++)
    {
        tableEntry->deviceIndex = 0;
        tableEntry->logicalUnitNumber = 0;
        tableEntry++;
    }
}

void GenerateFile() 
{
    GeneratedFileHeader* header;
    byte fileHandle;
    char* fileNamesHeader;
    char* bootFileIndexString;
          
    if(bootFileIndex > totalFilesProcessed) {
        bootFileIndex = totalFilesProcessed;
        printf("\r\n*** Warning: boot file index is greater than number of files processed.\r\n    Set to %c instead in the generated file.\r\n",
            bootFileIndex <= 9 ? bootFileIndex + '0' : bootFileIndex - 10 + 'A');
    }
    
    header = (GeneratedFileHeader*)fileContentsBase;
    strcpy(header->signature, emuDataSignature);
    header->numberOfEntriesInImagesTable = totalFilesProcessed;
    header->indexOfImageToMountAtBoot = bootFileIndex;
    header->workAreaAddress = workAreaAddress;
    header->flags = EmuFlagsByte();
    memset(header->reserved, 0, 3);
    
    regs.Words.DE = (int)outputFileName;
    regs.Bytes.A = 0;
    regs.Bytes.B = 0;
    DoDosCall(_CREATE);
    fileHandle = regs.Bytes.B;
    
    regs.Words.DE = (int)fileContentsBase;
    regs.Words.HL = 
            (uint)
            (sizeof(GeneratedFileHeader) + 
            (sizeof(GeneratedFileTableEntry) * totalFilesProcessed));
    DoDosCall(_WRITE);
        
    fileNamesHeader = "\fDisk image files registered:\r\n\r\n";
    regs.Bytes.B = fileHandle;
    regs.Words.DE = (int)fileNamesHeader;
    regs.Words.HL = strlen(fileNamesHeader);
    DoDosCall(_WRITE);
    
    regs.Bytes.B = fileHandle;
    regs.Words.DE = (int)fileNamesBase;
    regs.Words.HL = (int)(fileNamesAppendAddress - fileNamesBase);
    DoDosCall(_WRITE);

    bootFileIndexString = malloc(128);
    sprintf(bootFileIndexString, "\r\nBoot file index: %i\r\n", bootFileIndex);
    if(hzFlags == EMU_FLAG_50HZ) {
        strcat(bootFileIndexString, "50Hz mode forced\r\n");
    } else if(hzFlags == EMU_FLAG_60HZ) {
        strcat(bootFileIndexString, "60Hz mode forced\r\n");
    }
    if(invertCtrl) {
        strcat(bootFileIndexString, "CTRL inverted\r\n");
    }
    if(invertShift) {
        strcat(bootFileIndexString, "SHIFT inverted\r\n");
    }
    if(bootR800) {
        strcat(bootFileIndexString, "Boot in R800 mode\r\n");
    }
    regs.Bytes.B = fileHandle;
    regs.Words.DE = (int)bootFileIndexString;
    regs.Words.HL = strlen(bootFileIndexString);
    DoDosCall(_WRITE);
     
    regs.Bytes.B = fileHandle;
    DoDosCall(_CLOSE);
}

void SetupFile()
{
	ulong sector;
    masterBootRecord* sectorBuffer;
    partitionTableEntry* partition;
    byte error;

    ConvertDirectoryToFilename(outputFileName);
    AddFileExtension(outputFileName);
    StartSearchingFiles(outputFileName);
	GetDriveInfoForFileInFib();
    CheckControllerForFileInFib();
  
    if(fib->fileSize == 0) {
        Terminate("*** The emulation data file is empty");
    }

    sectorBuffer = malloc(sizeof(masterBootRecord));

    VerifyDataFileSignature((byte*)sectorBuffer);

	sector = driveInfo->firstSectorNumber + GetFirstFileSectorForFileInFib();
	
    if(setupAsPersistent) {
        PsLoad();
        psBuffer[PSD_EMU_DEVICE] = driveInfo->deviceIndex;
        psBuffer[PSD_EMU_SECTOR] = ((byte*)&sector)[0];   //LSB
        psBuffer[PSD_EMU_SECTOR + 1] = ((byte*)&sector)[1];
        psBuffer[PSD_EMU_SECTOR + 2] = ((byte*)&sector)[2];
        psBuffer[PSD_EMU_SECTOR + 3] = ((byte*)&sector)[3];   //MSB
        psBuffer[PSD_EMU_FLAGS] = PointerFlagsByte();
        PsSave();
        return;
    }

    strcpy(SetupRAMAddress, emuDataSignature);
    SetupRAMAddress[0x10] = driveInfo->deviceIndex;
    SetupRAMAddress[0x11] = PointerFlagsByte();
    SetupRAMAddress[0x12] = ((byte*)&sector)[0];   //LSB
    SetupRAMAddress[0x13] = ((byte*)&sector)[1];
    SetupRAMAddress[0x14] = ((byte*)&sector)[2];
    SetupRAMAddress[0x15] = ((byte*)&sector)[3];   //MSB
}

/* Persistent storage access. The format handling itself lives in the SDK
   (sdk/C/code/persistent_storage.c); these two just add the error policy
   of this tool, which is to give up with a message. */

void PsLoad()
{
    byte error;

    psBuffer = malloc(PSD_MAX_SIZE);
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

void KillPersistentEmulation()
{
    byte device;
    int i;

    PsLoad();
    device = psBuffer[PSD_EMU_DEVICE];
    if(device == 0 || device == 0xFF) {
        print("Persistent emulation mode is not set\r\n");
        return;
    }

    for(i = PSD_EMU_DEVICE; i <= PSD_EMU_FLAGS; i++) psBuffer[i] = 0;
    PsSave();
    print("Persistent emulation mode removed\r\n");
}

/* For one-time emulation, place the one-time boot keys block in RAM, next to
   the emulation pointer, so the kernel applies these keys at the emulation
   boot as if they were physically held. CTRL disables the ghost floppy drive
   and SHIFT disables the MSX-DOS kernels, freeing the 1.5K MSX-DOS 1 FAT copy
   each would use. These must act before the drives are set up, which is earlier
   than disk emulation mode is entered, so unlike the frequency and R800 options
   they can't be driven by the emulation flags byte; hence the boot keys, and
   hence they only work for one-time emulation. Whether to do each comes from
   the -c / -s switches or the equivalent flags in the data file (see the
   Effective... helpers). */
void VerifyDataFileSignature(byte* sectorBuffer)
{
    regs.Words.DE = (int)outputFileName;
    regs.Bytes.A = 1;    //read-only
    DoDosCall(_OPEN);
    fileHandle = regs.Bytes.B;

    regs.Words.DE = (int)sectorBuffer;
    regs.Words.HL = 24;    //signature (16) + rest of the header, including the flags byte at offset 20
    DoDosCall(_READ);

    if(regs.Words.HL < 16 || strcmpi((char*)sectorBuffer, emuDataSignature) != 0) {
        Terminate("Invalid emulation data file");
    }

    fileFlags = regs.Words.HL >= 21 ? sectorBuffer[20] : 0;
}

void ResetComputer()
{
    regs.Bytes.IYh = *(byte*)EXPTBL;
    regs.Words.IX = 0;
    AsmCall(CALSLT, &regs, REGS_ALL, REGS_NONE);

    //Just in case, but we should never reach here
    Terminate(null);
}

void Terminate(const char* errorMessage)
{
    if(fileHandle != 0) {
        regs.Bytes.B = fileHandle;
        DoDosCall(_CLOSE);
    }

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
	
    if(regs.Bytes.B < 2 || regs.Bytes.IXh != 1) {
        Terminate("This program is for Nextor only.");
    }

    if(regs.Bytes.IXl < 3) {
        Terminate("This program needs Nextor 3 or newer.\r\nUse an older version of EMUFILE for Nextor 2.");
    }
}

void* malloc(int size)
{
	void* value = mallocPointer;
	mallocPointer = (void*)(((int)mallocPointer) + size);
	return value;
}

uint ParseHex(char* hexString)
{
    uint result;
    char digit;
    
    result = 0;
    while((digit = *hexString) != 0) {
        digit |= 32;
        result *= 16;
        if(digit >= '0' && digit <= '9') {
            result += digit - '0';
        }
        else if(digit >= 'a' && digit <='f') {
            result += digit - 'a' + 10;
        }
        else {
            InvalidParameter();
        }
        hexString++;
    }
    
    return result;
}

void DoDosCall(byte functionCode)
{
    DosCall(functionCode, &regs, REGS_ALL, REGS_ALL);
    if(regs.Bytes.A != 0 && !(functionCode == _FNEXT && regs.Bytes.A == _NOFIL)) {
        TerminateWithDosError(regs.Bytes.A);
    }
}
