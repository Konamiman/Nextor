/* DSK emulation configuration file creation tool for Nextor v1.1
   By Konamiman 3/2019

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196
          --no-std-crt0 crt0_msxdos_advanced.rel emufile.c
   hex2bin -e com emufile.ihx
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
#include "partit.h"
#include "strcmpi.h"

	/* Typedefs */

typedef struct {
    char signature[16];
    byte numberOfEntriesInImagesTable;
    byte indexOfImageToMountAtBoot;
    uint workAreaAddress;
    byte reserved[4];
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

#define PARSE_FLAG_HAS_FILENAME (1 << 3)
#define PARSE_FLAG_HAS_EXTENSION (1 << 4)
#define PARSE_FLAG_IS_AMBIGUOUS (1 << 5)

#define MallocBase 0x8000

#define MaxFilesToProcess 32

#define SetupRAMAddress ((byte*)0xA000)


/* Strings */

const char* strTitle=
    "Disk image emulation tool for Nextor v1.21\r\n"
    "By Konamiman, 7/2020\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: emufile [<options>] <output file> <files> [<files> ...]\r\n"
    "       emufile set <data file> [o|p[<device index>[<LUN index>]]]\r\n"
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
    "\r\n"
    "TYPE /B the generated file to see the names of the registered files.\r\n"
    "\r\n"
    "\r\n"
    "* To setup an existing emulation data file for booting:\r\n"
    "\r\n"
    "emufile set <data file> [o|p[<device index>[<LUN index>]]]\r\n"
    "\r\n"
    "Default extension for <data file> is EMU. A directory can be specified instead,\r\n"
    "in that case, a file with the same name and .EMU extension inside the directory\r\n"
    "will be used.\r\n"
    "\r\n"
    "o: one-time emulation (store emulation data file pointer in RAM).\r\n"
    "This is the default if only <data file> is specified.\r\n"
    "\r\n"
    "p: persistent emulation (store emulation data file pointer in partition table).\r\n"
    "Use <device index> and <LUN index> to specify the device whose\r\n"
    "partition table will be written. Default device (if only 'p' is specified)\r\n"
    "is the device where the emulation data file is located.\r\n"
    "Default <LUN index> (if only 'p<device index>' is specified) is 1.\r\n"
    "\r\n"
    "The computer will reset after successfully finishing the setup.\r\n";

const char* strInvParam = "Invalid parameter";
const char* strCRLF = "\r\n";

const char* emuDataSignature = "NEXTOR_EMU_DATA";

/* Global variables */

Z80_registers regs;
byte ASMRUT[4];
byte OUT_FLAGS;
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
int setupPartitionDeviceIndex;
int setupPartitionLunIndex;
bool setupAsPersistent;

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
int ProcessOption(char optionLetter, char* optionValue);
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
void DriverCall(byte slot, uint routineAddress);
byte DeviceSectorRW(byte driverSlot, byte deviceIndex, byte lunIndex, ulong sectorNumber, byte* buffer, bool write);
void VerifyDataFileSignature(byte* sectorBuffer);
void ResetComputer();

#define ReadDeviceSector(driverSlot, deviceIndex, lunIndex, sectorNumber, buffer) DeviceSectorRW(driverSlot, deviceIndex, lunIndex, sectorNumber, buffer, false)
#define WriteDeviceSector(driverSlot, deviceIndex, lunIndex, sectorNumber, buffer) DeviceSectorRW(driverSlot, deviceIndex, lunIndex, sectorNumber, buffer, true)

	/* MAIN */
	
int main(char** argv, int argc)
{
    bool isSetupFile;

    ASMRUT[0] = 0xC3;
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
        Terminate("The primary controller is not a Nextor kernel with a device-based driver.");
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
}

bool ProcessArguments(char** argv, int argc) 
{
    if(argc > 0 && argv[0][0] == '?') {
        print(strHelp);
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
    byte paramFirstChar;

    setupAsPersistent = false;
    setupPartitionDeviceIndex = 0;
    setupPartitionLunIndex = 0;

    strcpy(outputFileName, argv[1]);

    if(argc == 2) {
        return;
    }

    paramFirstChar = argv[2][0] | 32;

    if(paramFirstChar == 'o') {
        return;
    }
    else if(paramFirstChar != 'p') {
        InvalidParameter();
    }

    setupAsPersistent = true;

    if(argv[2][1] == '\0') {
        return;
    }

    setupPartitionDeviceIndex = argv[2][1] - '0';
    if(setupPartitionDeviceIndex < 1 || setupPartitionDeviceIndex > 9) {
        Terminate("Invalid device index");
    }

    if(argv[2][2] == '\0') {
        setupPartitionLunIndex = 1;
        return;
    }

    setupPartitionLunIndex = argv[2][2] - '0';
    if(setupPartitionLunIndex < 1 || setupPartitionLunIndex > 9) {
        Terminate("Invalid LUN index");
    }

    if(argv[2][3] != '\0') {
        InvalidParameter();
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

	InvalidParameter();
	return 0;
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
    regs.Bytes.B = FILEATTR_DIRECTORY;
    regs.Words.IX = (int)fib;
    
    DosCall(_FFIRST, &regs, REGS_ALL, REGS_ALL);
    if(regs.Bytes.A == _NOFIL)
        return false;
    if(regs.Bytes.A != 0)
        TerminateWithDosError(regs.Bytes.A);

    return (fib->attributes & FILEATTR_DIRECTORY) != 0;
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
    memset(header->reserved, 0, 4);
    
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

    bootFileIndexString = malloc(32);
    sprintf(bootFileIndexString, "\r\nBoot file index: %i\r\n", bootFileIndex);
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
	
    if(setupPartitionDeviceIndex == 0) {
        setupPartitionDeviceIndex = driveInfo->deviceIndex;
        setupPartitionLunIndex = driveInfo->logicalUnitNumber;
    }

    sectorBuffer = malloc(sizeof(masterBootRecord));
    error = ReadDeviceSector(driveInfo->driverSlotNumber, setupPartitionDeviceIndex, setupPartitionLunIndex, 0, (byte*)sectorBuffer);
    if(error != 0) {
        print("*** Error when reading MBR of device:");
        TerminateWithDosError(error);
    }

    if(setupAsPersistent) {
        partition = &(sectorBuffer->primaryPartitions[0]);
        partition->status |= 1;
        partition->chsOfFirstSector[0] = driveInfo->deviceIndex;
        partition->chsOfFirstSector[1] = driveInfo->logicalUnitNumber;
        partition->chsOfFirstSector[2] = ((byte*)&sector)[3];   //MSB
        partition->chsOfLastSector[0] = ((byte*)&sector)[2];
        partition->chsOfLastSector[1] = ((byte*)&sector)[1];
        partition->chsOfLastSector[2] = ((byte*)&sector)[0];    //LSB

        error = WriteDeviceSector(driveInfo->driverSlotNumber, setupPartitionDeviceIndex, setupPartitionLunIndex, 0, (byte*)sectorBuffer);
        if(error != 0) {
            print("*** Error when writing MBR of device:");
            TerminateWithDosError(error);
        }
    } else {
        strcpy(SetupRAMAddress, emuDataSignature);
        SetupRAMAddress[0x10] = driveInfo->deviceIndex;
        SetupRAMAddress[0x11] = driveInfo->logicalUnitNumber;
        SetupRAMAddress[0x12] = ((byte*)&sector)[0];   //LSB
        SetupRAMAddress[0x13] = ((byte*)&sector)[1];
        SetupRAMAddress[0x14] = ((byte*)&sector)[2];
        SetupRAMAddress[0x15] = ((byte*)&sector)[3];   //MSB
    }
}

void VerifyDataFileSignature(byte* sectorBuffer)
{
    regs.Words.DE = (int)outputFileName;
    regs.Bytes.A = 1;    //read-only
    DoDosCall(_OPEN);
    fileHandle = regs.Bytes.B;

    regs.Words.DE = (int)sectorBuffer;
    regs.Words.HL = 16;
    DoDosCall(_READ);

    if(regs.Words.HL < 16 || strcmpi((char*)sectorBuffer, emuDataSignature) != 0) {
        Terminate("Invalid emulation data file");
    }
}

byte DeviceSectorRW(byte driverSlot, byte deviceIndex, byte lunIndex, ulong sectorNumber, byte* buffer, bool write)
{
	regs.Flags.C = write;
	regs.Bytes.A = deviceIndex;
	regs.Bytes.B = 1;
	regs.Bytes.C = lunIndex;
	regs.Words.HL = (int)buffer;
	regs.Words.DE = (int)&sectorNumber;

	DriverCall(driverSlot, DEV_RW);
	return regs.Bytes.A;
}

void DriverCall(byte slot, uint routineAddress)
{
	byte registerData[8];
	int i;

	memcpy(registerData, &regs, 8);

	regs.Bytes.A = slot;
	regs.Bytes.B = 0xFF;
	regs.UWords.DE = routineAddress;
	regs.Words.HL = (int)registerData;

	DoDosCall(_CDRVR);

    regs.Words.AF = regs.Words.IX;
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

#define COM_FILE
#include "print_msxdos.c"
#include "printf.c"
#include "asmcall.c"
#include "strcmpi.c"
