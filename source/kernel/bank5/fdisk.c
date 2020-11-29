//FDISK - Disk partitionner for Nextor
//This is the main program. There is also an extra functions file (fdisk2.c)
//that is placed in the bank immediately following.
//To call functions on that bank, use CallFunctionInExtraBank.

// Compilation command line:
//
// sdcc --code-loc 0x4150 --data-loc 0x8000 -mz80 --disable-warning 196 --disable-warning 84 --no-std-crt0 fdisk_crt0.rel msxchar.lib asm.lib fdisk.c
// hex2bin -e dat fdisk.ihx
//
// Once compiled, embed the first 16000 bytes of fdisk.dat at position 82176 of the appropriate Nextor ROM file:
// dd if=fdisk.dat of=nextor.rom bs=1 count=16000 seek=82176

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef MAKEBUILD
#include <system.h>
#include "dos.h"
#include "types.h"
#include "AsmCall.h"
#include "drivercall.h"
#include "partit.h"
#else
#include "../../tools/C/system.h"
#include "../../tools/C/dos.h"
#include "../../tools/C/types.h"
#include "../../tools/C/asmcall.h"
#include "drivercall.h"
#include "../../tools/C/partit.h"
#endif

#include "fdisk.h"

//#define FAKE_DEVICE_INFO
//#define FAKE_DRIVER_INFO
//#define FAKE_PARTITION_COUNT 256

/*
If FAKE_DEVICE_SIZE is defined,
simulated device size is provided by user:
_FDISK(sizeInK)
_FDISK(sizeInK, sizeInM)
_FDISK(sizeInK, sizeInM, sizeInG)
All the provided values are added up to get the simulated value.
Maximum value for each parameter is 32767.
*/

//#define FAKE_DEVICE_SIZE
//#define TEST_FAT_PARAMETERS

#define MESSAGE_ROW 9
#define PARTITIONS_PER_PAGE 15

#define CMD_FDISK 1

typedef struct {
    byte screenMode;
    byte screenWidth;
    bool functionKeysVisible;
} ScreenConfiguration;

char buffer[1000];
driverInfo drivers[MAX_INSTALLED_DRIVERS];
driverInfo* selectedDriver;
char selectedDriverName[50];
lunInfo luns[MAX_LUNS_PER_DEVICE];
lunInfo* selectedLun;
deviceInfo devices[MAX_DEVICES_PER_DRIVER];
deviceInfo* currentDevice;
byte selectedDeviceIndex;
byte selectedLunIndex;
byte installedDriversCount;
bool availableDevicesCount;
byte availableLunsCount;
Z80_registers regs;
byte ASMRUT[4];
byte OUT_FLAGS;
ScreenConfiguration originalScreenConfig;
ScreenConfiguration currentScreenConfig;
bool is80ColumnsDisplay;
byte screenLinesCount;
partitionInfo partitions[MAX_PARTITIONS_TO_HANDLE];
int partitionsCount;
bool partitionsExistInDisk;
ulong unpartitionnedSpaceInSectors;
bool canCreatePartitions;
bool canDoDirectFormat;
ulong autoPartitionSizeInK;
ulong dividend, divisor;
bool dos1;
#ifdef FAKE_DEVICE_SIZE
ulong fakeDeviceSizeInK;
#endif


#define HideCursor() print("\x1Bx5")
#define DisplayCursor() print("\x1By5")
#define CursorDown() chput('\x1F')
#define CursorUp() chput('\x1E')
#define ClearScreen() chput('\x0C')
#define HomeCursor() print("\x0D\x1BK")
#define DeleteToEndOfLine() print("\x1BK")
#define DeleteToEndOfLineAndCursorDown() print("\x1BK\x1F");
#define NewLine() print("\x0A\x0D");


void DoFdisk();
void GoDriverSelectionScreen();
void ShowDriverSelectionScreen();
void ComposeSlotString(byte slot, char* destination);
void GoDeviceSelectionScreen(byte driverIndex);
void ShowDeviceSelectionScreen();
void GetDevicesInformation();
void EnsureMaximumStringLength(char* string, int maxLength);
void GoLunSelectionScreen(byte deviceIndex);
void InitializePartitioningVariables(byte lunIndex);
void ShowLunSelectionScreen();
void PrintSize(ulong sizeInK);
byte GetRemainingBy1024String(ulong value, char* destination);
void GetLunsInformation();
void PrintDeviceInfoWithIndex();
void GoPartitioningMainMenuScreen();
bool GetYesOrNo();
byte GetDiskPartitionsInfo();
void ShowPartitions();
void TogglePartitionActive(byte partitionIndex);
void PrintOnePartitionInfo(partitionInfo* info);
void DeleteAllPartitions();
void RecalculateAutoPartitionSize(bool setToAllSpaceAvailable);
void AddPartition();
void AddAutoPartition();
void UndoAddPartition();
void TestDeviceAccess();
void InitializeScreenForTestDeviceAccess(char* message);
void PrintDosErrorMessage(byte code, char* header);
bool FormatWithoutPartitions();
byte CreateFatFileSystem(ulong firstDeviceSector, ulong fileSystemSizeInK);
#ifdef TEST_FAT_PARAMETERS
void CalculateFatFileSystemParameters(ulong fileSystemSizeInK, dosFilesystemParameters* parameters);
#endif
bool WritePartitionTable();
void PreparePartitioningProcess();
byte CreatePartition(int index);
byte ToggleStatusBit(byte partitionTableEntryIndex, ulong partitionTablesector);
bool ConfirmDataDestroy(char* action);
void ClearInformationArea();
void GetDriversInformation();
void TerminateRightPaddedStringWithZero(char* string, byte length);
byte WaitKey();
byte GetKey();
void SaveOriginalScreenConfiguration();
void ComposeWorkScreenConfiguration();
void SetScreenConfiguration(ScreenConfiguration* screenConfig);
void InitializeWorkingScreen(char* header);
void PrintRuler();
void Locate(byte x, byte y);
void LocateX(byte x);
void PrintCentered(char* string);
void PrintStateMessage(char* string);
void chput(char ch);
void print(char* string);
int CallFunctionInExtraBank(int functionNumber, void* parametersBuffer);

void main(int bc, int hl)
{
    ASMRUT[0] = 0xC3;   //Code for JP
	dos1 = (*((byte*)0xF313) == 0);

#ifdef FAKE_DEVICE_SIZE
	fakeDeviceSizeInK = 0;
	if(*((char*)hl) == '(') {
		hl++;
		regs.Words.HL = hl;
		regs.Words.IX = FRMEVL;
		SwitchSystemBankThenCall(CALBAS, REGS_MAIN);
		fakeDeviceSizeInK = *(int*)(DAC+2);
		hl = regs.Words.HL;
		if(*((char*)hl++) == ',') {
			regs.Words.HL = hl;
			regs.Words.IX = FRMEVL;
			SwitchSystemBankThenCall(CALBAS, REGS_MAIN);
			fakeDeviceSizeInK += (ulong)(*(int*)(DAC+2)) * (ulong)1024;
			hl = regs.Words.HL;
			if(*((char*)hl++) == ',') {
				regs.Words.HL = hl;
				regs.Words.IX = FRMEVL;
				SwitchSystemBankThenCall(CALBAS, REGS_MAIN);
				fakeDeviceSizeInK += (ulong)(*(int*)(DAC+2)) * (ulong)1024 * (ulong)1024;
				hl = regs.Words.HL;
			}
		}
	}
#endif
#ifdef TEST_FAT_PARAMETERS
	CalculateFatFileSystemParameters(fakeDeviceSizeInK, (dosFilesystemParameters*)buffer);
	printf("Total sectors: "); _ultoa(((dosFilesystemParameters*)buffer)->totalSectors, buffer+80, 10); printf("%s\r\n", buffer+80);
	printf("Data sectors:  "); _ultoa(((dosFilesystemParameters*)buffer)->dataSectors, buffer+80, 10); printf("%s\r\n", buffer+80);
	printf("Cluster count: %u\r\n", ((dosFilesystemParameters*)buffer)->clusterCount);
	printf("Sectors per FAT:     %u\r\n", ((dosFilesystemParameters*)buffer)->sectorsPerFat);
	printf("Sectors per cluster: %u\r\n", ((dosFilesystemParameters*)buffer)->sectorsPerCluster);
	printf("Sectors per root dir: %u\r\n", ((dosFilesystemParameters*)buffer)->sectorsPerRootDirectory);
	printf(((dosFilesystemParameters*)buffer)->isFat16 ? "FAT16\r\n" : "FAT12\r\n");
#else
    DoFdisk();
#endif
}


void DoFdisk()
{
	installedDriversCount = 0;
	selectedDeviceIndex = 0;
	selectedLunIndex = 0;
	availableLunsCount = 0;

    SaveOriginalScreenConfiguration();
    ComposeWorkScreenConfiguration();
    SetScreenConfiguration(&currentScreenConfig);
    InitializeWorkingScreen("Nextor disk partitioning tool");

	GoDriverSelectionScreen();

	SetScreenConfiguration(&originalScreenConfig);
}


void GoDriverSelectionScreen()
{
    byte key;

    while(true) {
        ShowDriverSelectionScreen();
        if(installedDriversCount == 0) {
            return;
        }

        while(true) {
            key = WaitKey();
            if(key == ESC) {
                return;
            } else {
                key -= '0';
                if(key >= 1 && key <= installedDriversCount) {
                    GoDeviceSelectionScreen(key);
                    break;
                }
            }
        }
    }
}


void ShowDriverSelectionScreen()
{
    byte i;
    char slot[4];
    char rev[3];
    driverInfo* currentDriver;
    byte slotByte;
    byte revByte;
    char* driverName;

    ClearInformationArea();
    
    if(installedDriversCount == 0) {
        GetDriversInformation();
    }
    
    if(installedDriversCount == 0) {
    	Locate(0, 7);
    	PrintCentered("There are no device-based drivers");
    	CursorDown();
    	PrintCentered("available in the system");
    	PrintStateMessage("Press any key to exit...");
    	WaitKey();
    	return;
	}

	currentDriver = &drivers[0];
	Locate(0,3);
	for(i = 0; i < installedDriversCount; i++) {
    	ComposeSlotString(currentDriver->slot, slot);
	    
	    revByte = currentDriver->versionRev;
	    if(revByte == 0) {
	        rev[0] = '\0';
	    } else {
	        rev[0] = '.';
	        rev[1] = revByte + '0';
	        rev[2] = '\0';
	    }
	    
	    driverName = currentDriver->driverName;
	    
	    printf("\x1BK%i. %s%sv%i.%i%s on slot %s",
	        i + 1,
	        driverName,
	        is80ColumnsDisplay ? " " : "\x0D\x0A   ",
	        currentDriver->versionMain,
	        currentDriver->versionSec,
	        rev,
	        slot);
	    
	    NewLine();
	    if(is80ColumnsDisplay || installedDriversCount <= 4) {
	        NewLine();
	    }
	    
	    currentDriver++;
	}
	
	NewLine();
	print("ESC. Exit");

    PrintStateMessage("Select the device driver");
}


void ComposeSlotString(byte slot, char* destination)
{
	if((slot & 0x80) == 0) {
	    destination[0] = slot + '0';
	    destination[1] = '\0';
	} else {
	    destination[0] = (slot & 3) + '0';
	    destination[1] = '-';
	    destination[2] = ((slot >> 2) & 3) + '0';
	    destination[3] = '\0';
	}
}


void GoDeviceSelectionScreen(byte driverIndex)
{
	char slot[4];
	int i;
	byte key;

	selectedDriver = &drivers[driverIndex - 1];
	ComposeSlotString(selectedDriver->slot, slot);
	strcpy(selectedDriverName, selectedDriver->driverName);
	if(!is80ColumnsDisplay) {
		EnsureMaximumStringLength(selectedDriverName, MAX_LINLEN_MSX1 - 12);
	}
	sprintf(selectedDriverName + strlen(selectedDriverName),
		" on slot %s\r\n",
		slot);

	availableDevicesCount = 0;

    while(true) {
        ShowDeviceSelectionScreen();
        if(availableDevicesCount == 0) {
            return;
        }
        
        while(true) {
            key = WaitKey();
            if(key == ESC) {
                return;
            } else {
                key -= '0';
                if(key >= 1 && key <= MAX_DEVICES_PER_DRIVER && devices[key - 1].lunCount != 0) {
                    GoLunSelectionScreen(key);
                    break;
                }
            }
        }
    }
}


void ShowDeviceSelectionScreen()
{
	deviceInfo* currentDevice;
	byte i;

	ClearInformationArea();
	Locate(0,3);
	print(selectedDriverName);
	CursorDown();
	CursorDown();

	if(availableDevicesCount == 0) {
        GetDevicesInformation();
    }
    
    if(availableDevicesCount == 0) {
    	Locate(0, 9);
    	PrintCentered("There are no suitable devices");
    	CursorDown();
    	PrintCentered("attached to the driver");
    	PrintStateMessage("Press any key to go back...");
    	WaitKey();
    	return;
	}

	currentDevice = &devices[0];
	for(i = 0; i < MAX_DEVICES_PER_DRIVER; i++) {
		if(currentDevice->lunCount > 0) {
			printf("\x1BK%i. %s\r\n\r\n",
				i + 1,
				currentDevice->deviceName);
		}

		currentDevice++;
	}

	if(availableDevicesCount < 7) {
		NewLine();
	}
	print("ESC. Go back to driver selection screen");

    PrintStateMessage("Select the device");
}


void GetDevicesInformation()
{
    byte error = 0;
    byte deviceIndex = 1;
    deviceInfo* currentDevice = &devices[0];
	char* currentDeviceName;

#ifdef FAKE_DEVICE_INFO
	availableDevicesCount = 7;
	devices[0].lunCount = 1;
	strcpy(devices[0].deviceName, "Un dispositivo.");
	devices[2].lunCount = 1;
	strcpy(devices[2].deviceName, "Otro dispositivo con nombre mas larguillo, de 64 caracteres tal.");
	EnsureMaximumStringLength(devices[2].deviceName, MAX_LINLEN_MSX1 - 4);
	devices[1].lunCount = 1;
	devices[3].lunCount = 2;
	devices[4].lunCount = 3;
	devices[5].lunCount = 4;
	devices[6].lunCount = 5;

	*devices[1].deviceName = '\0';
	*devices[3].deviceName = '\0';
	*devices[4].deviceName = '\0';
	*devices[5].deviceName = '\0';
	*devices[6].deviceName = '\0';

	return;
#else
    availableDevicesCount = 0;

	while(deviceIndex <= MAX_DEVICES_PER_DRIVER) {
		currentDeviceName = currentDevice->deviceName;
		regs.Bytes.A = deviceIndex;
		regs.Bytes.B = 0;
		regs.Words.HL = (int)currentDevice;
		DriverCall(selectedDriver->slot, DEV_INFO);
		if(regs.Bytes.A == 0) {
			availableDevicesCount++;
			regs.Bytes.A = deviceIndex;
			regs.Bytes.B = 2;
			regs.Words.HL = (int)currentDeviceName;
			DriverCall(selectedDriver->slot, DEV_INFO);
			//memcpy(currentDeviceName, "Device name with 40 characters lorem ips", 40); //!!!
			if(regs.Bytes.A == 0) {
				TerminateRightPaddedStringWithZero(currentDeviceName, MAX_INFO_LENGTH);
			} else {
				sprintf(currentDeviceName, "(Unnamed device, ID=%i)", deviceIndex);
			}
			if(!is80ColumnsDisplay) {
				EnsureMaximumStringLength(currentDeviceName, MAX_LINLEN_MSX1 - 4);
			}
		} else {
			currentDevice->lunCount = 0;
		}

		deviceIndex++;
		currentDevice++;
	}
#endif
}


void EnsureMaximumStringLength(char* string, int maxLength)
{
	int len = strlen(string);
	if(len > maxLength) {
		string += maxLength - 3;
		*string++ = '.';
		*string++ = '.';
		*string++ = '.';
		*string = '\0';
	}
}


void GoLunSelectionScreen(byte deviceIndex)
{
	int i;
	byte key;

	currentDevice = &devices[deviceIndex - 1];
	selectedDeviceIndex = deviceIndex;

	availableLunsCount = 0;

    while(true) {
        ShowLunSelectionScreen();
        if(availableLunsCount == 0) {
            return;
        }
        
        while(true) {
            key = WaitKey();
            if(key == ESC) {
                return;
            } else {
                key -= '0';
                if(key >= 1 && key <= MAX_LUNS_PER_DEVICE && luns[key - 1].suitableForPartitioning) {
					InitializePartitioningVariables(key);
                    GoPartitioningMainMenuScreen();
                    break;
                }
            }
        }
    }
}


void InitializePartitioningVariables(byte lunIndex)
{
	selectedLunIndex = lunIndex - 1;
	selectedLun = &luns[selectedLunIndex];
	partitionsCount = 0;
	partitionsExistInDisk = true;
	canCreatePartitions = (selectedLun->sectorCount >= (MIN_DEVICE_SIZE_FOR_PARTITIONS_IN_K * 2));
	canDoDirectFormat = (selectedLun->sectorCount <= MAX_DEVICE_SIZE_FOR_DIRECT_FORMAT_IN_K * 2);
	unpartitionnedSpaceInSectors = selectedLun->sectorCount;
	RecalculateAutoPartitionSize(true);
}


void ShowLunSelectionScreen()
{
	byte i;
	lunInfo* currentLun;

	ClearInformationArea();
    Locate(0,3);
	print(selectedDriverName);
	print(currentDevice->deviceName);
	PrintDeviceInfoWithIndex();
	NewLine();
	NewLine();
	NewLine();

	if(availableLunsCount == 0) {
        GetLunsInformation();
    }
    
    if(availableLunsCount == 0) {
    	Locate(0, 9);
    	PrintCentered("There are no suitable logical units");
    	CursorDown();
    	PrintCentered("available in the device");
    	PrintStateMessage("Press any key to go back...");
    	WaitKey();
    	return;
	}

	currentLun = &luns[0];
	for(i = 0; i < MAX_LUNS_PER_DEVICE; i++) {
		if(currentLun->suitableForPartitioning) {
			printf("\x1BK%i. Size: ", i + 1);
			PrintSize(currentLun->sectorCount / 2);
			NewLine();
		}

		i++;
		currentLun++;
	}

	NewLine();
	NewLine();
	print("ESC. Go back to device selection screen");

    PrintStateMessage("Select the logical unit");
}


void PrintSize(ulong sizeInK)
{
	byte remaining;
	char buf[3];
	ulong dividedSize;
	char* remString;

	if(sizeInK < (ulong)(10 * 1024)) {
		printf("%iK", sizeInK);
		return;
	}
	
	dividedSize = sizeInK >> 10;
	if(dividedSize < (ulong)(10 * 1024)) {
		printf("%i", dividedSize + GetRemainingBy1024String(sizeInK, buf));
		printf("%sM", buf);
	} else {
		sizeInK >>= 10;
		dividedSize = sizeInK >> 10;
		printf("%i", dividedSize + GetRemainingBy1024String(sizeInK, buf));
		printf("%sG", buf);
	}
}


byte GetRemainingBy1024String(ulong value, char* destination)
{
	byte remaining2;
	char remainingDigit;

	int remaining = value & 0x3FF;
	if(remaining >= 950) {
		*destination = '\0';
		return 1;
	}
	remaining2 = remaining % 100;
	remainingDigit = (remaining / 100) + '0';
	if(remaining2 >= 50) {
		remainingDigit++;
	}

	if(remainingDigit == '0') {
		*destination = '\0';
	} else {
		destination[0] = '.';
		destination[1] = remainingDigit;
		destination[2] = '\0';
	}

	return 0;
}


void GetLunsInformation()
{
    byte error = 0;
    byte lunIndex = 1;
    lunInfo* currentLun = &luns[0];
	char* currentDeviceName;

	while(lunIndex <= MAX_LUNS_PER_DEVICE) {
		regs.Bytes.A = selectedDeviceIndex;
		regs.Bytes.B = lunIndex;
		regs.Words.HL = (int)currentLun;
		DriverCall(selectedDriver->slot, LUN_INFO);
#ifdef FAKE_DEVICE_SIZE
		if(fakeDeviceSizeInK != 0) {
			currentLun->sectorCount = fakeDeviceSizeInK * 2;
		}
#endif
		currentLun->suitableForPartitioning =
			(regs.Bytes.A == 0) &&
			(currentLun->mediumType == BLOCK_DEVICE) &&
			(currentLun->sectorSize == 512) &&
			(currentLun->sectorCount >= MIN_DEVICE_SIZE_IN_K * 2) &&
			((currentLun->flags & (READ_ONLY_LUN | FLOPPY_DISK_LUN)) == 0);
		if(currentLun->suitableForPartitioning) {
			availableLunsCount++;
		}

		if(currentLun->sectorsPerTrack == 0 || currentLun->sectorsPerTrack > EXTRA_PARTITION_SECTORS) {
			currentLun->sectorsPerTrack = EXTRA_PARTITION_SECTORS;
		}

		lunIndex++;
		currentLun++;
	}
}


void PrintDeviceInfoWithIndex()
{
	printf(is80ColumnsDisplay ? " (Id = %i)" : " (%i)", selectedDeviceIndex);
}


void PrintTargetInfo()
{
	Locate(0,3);
	print(selectedDriverName);
	print(currentDevice->deviceName);
	PrintDeviceInfoWithIndex();
	NewLine();
	printf("Logical unit %i, size: ", selectedLunIndex + 1);
	PrintSize(selectedLun->sectorCount / 2);
	NewLine();
}


void GoPartitioningMainMenuScreen()
{
	char key;
	byte error;
	bool canAddPartitionsNow;
	bool mustRetrievePartitionInfo = true;

	while(true) {
		if(mustRetrievePartitionInfo) {
			ClearInformationArea();
			PrintTargetInfo();

			if(canCreatePartitions) {
				Locate(0, MESSAGE_ROW);
				PrintCentered("Searching partitions...");
				PrintStateMessage("Please wait...");
				error = GetDiskPartitionsInfo();
				if(error != 0) {
					PrintDosErrorMessage(error, "Error when searching partitions:");
					PrintStateMessage("Manage device anyway? (y/n) ");
					if(!GetYesOrNo()) {
						return;
					}
				}
				partitionsExistInDisk = (partitionsCount > 0);
			}
			mustRetrievePartitionInfo = false;
		}

		ClearInformationArea();
		PrintTargetInfo();
		if(!partitionsExistInDisk) {
			print("Unpartitionned space available: ");
			PrintSize(unpartitionnedSpaceInSectors / 2);
			NewLine();
		}
		NewLine();

		printf("Changes are not committed%suntil W is pressed.\r\n"
		   "\r\n", is80ColumnsDisplay ? " " : "\r\n");

		if(partitionsCount > 0) {
			printf("S. Show partitions (%i %s)\r\n"
				  "D. Delete all partitions\r\n",
				  partitionsCount,
				  partitionsExistInDisk ? "found" : "defined");
		} else if(canCreatePartitions) {
			print("(No partitions found or defined)\r\n");
		}
		canAddPartitionsNow = 
			!partitionsExistInDisk && 
			canCreatePartitions && 
			unpartitionnedSpaceInSectors >= (MIN_REMAINING_SIZE_FOR_NEW_PARTITIONS_IN_K * 2) + (EXTRA_PARTITION_SECTORS) &&
			partitionsCount < MAX_PARTITIONS_TO_HANDLE;
		if(canAddPartitionsNow) {
			print("A. Add one ");
			PrintSize(autoPartitionSizeInK);
			print(" partition\r\n");
			print("P. Add partition...\r\n");
		}
		if(!partitionsExistInDisk && partitionsCount > 0) {
			print("U. Undo add ");
			PrintSize(partitions[partitionsCount - 1].sizeInK);
			print(" partition\r\n");
		}
		NewLine();
		if(canDoDirectFormat) {
			print("F. Format device without partitions\r\n\r\n");
		}
		if(!partitionsExistInDisk && partitionsCount > 0) {
			print("W. Write partitions to disk\r\n\r\n");
		}
		print("T. Test device access\r\n");

		PrintStateMessage("Select an option or press ESC to return");

		while((key = WaitKey()) == 0);
		if(key == ESC) {
			if(partitionsExistInDisk || partitionsCount == 0) {
				return;
			}
			PrintStateMessage("Discard changes and return? (y/n) ");
			if(GetYesOrNo()) {
				return;
			} else {
				continue;
			}
		}
		key |= 32;
		if(key == 's' && partitionsCount > 0) {
			ShowPartitions();
		} else if(key == 'd' && partitionsCount > 0) {
			DeleteAllPartitions();
		} else if(key == 'p' && canAddPartitionsNow > 0) {
			AddPartition();
		} else if(key == 'a' && canAddPartitionsNow > 0) {
			AddAutoPartition();
		} else if(key == 'u' && !partitionsExistInDisk && partitionsCount > 0) {
			UndoAddPartition();
		}else if(key == 't') {
			TestDeviceAccess();
		} else if(key == 'f' && canDoDirectFormat) {
			if(FormatWithoutPartitions()) {
				mustRetrievePartitionInfo = true;
			}
		}else if(key == 'w' && !partitionsExistInDisk && partitionsCount > 0) {
			if(WritePartitionTable()) {
				mustRetrievePartitionInfo = true;
			}
		}
	}
}


bool GetYesOrNo()
{
	char key;

	DisplayCursor();
	key = WaitKey() | 32;
	HideCursor();
	return key == 'y';
}


byte GetDiskPartitionsInfo()
{
	byte primaryIndex = 1;
	byte extendedIndex = 0;
	int i;
	byte error;
	partitionInfo* currentPartition = &partitions[0];

#ifdef FAKE_PARTITION_COUNT
	partitionsCount = FAKE_PARTITION_COUNT;
	for(i = 1; i <= FAKE_PARTITION_COUNT; i++) {
		currentPartition->primaryIndex = 1;
		currentPartition->extendedIndex = i;
		currentPartition->partitionType = i & 0xFF;
		currentPartition->sizeInK = (ulong)i * (ulong)1024 * (ulong)1024;
		currentPartition++;
	}
	return 0;
#else
	partitionsCount = 0;

	do {
		regs.Bytes.A = selectedDriver->slot;
		regs.Bytes.B = 0xFF;
		regs.Bytes.D = selectedDeviceIndex;
		regs.Bytes.E = selectedLunIndex + 1;
		regs.Bytes.H = primaryIndex;
		regs.Bytes.L = extendedIndex;
		DosCallFromRom(_GPART, REGS_ALL);
		error = regs.Bytes.A;
		if(error == 0) {
			if(regs.Bytes.B == PARTYPE_EXTENDED) {
				extendedIndex = 1;
			} else {
				currentPartition->primaryIndex = primaryIndex;
				currentPartition->extendedIndex = extendedIndex;
				currentPartition->partitionType = regs.Bytes.B;
                currentPartition->status = regs.Bytes.C;
				((uint*)&(currentPartition->sizeInK))[0] = regs.UWords.IY;
				((uint*)&(currentPartition->sizeInK))[1] = regs.UWords.IX;
				currentPartition->sizeInK /= 2;
				partitionsCount++;
				currentPartition++;
				extendedIndex++;
			}
		} else if(error == _IPART) {
			primaryIndex++;
			extendedIndex = 0;
		} else {
			return error;
		}
	} while(primaryIndex <= 4 && partitionsCount < MAX_PARTITIONS_TO_HANDLE);

	return 0;
#endif
}


void ShowPartitions()
{
	int i;
	int firstShownPartitionIndex = 1;
	int lastPartitionIndexToShow;
	bool isLastPage;
	bool isFirstPage;
    bool allPartitionsArePrimary;
	byte key;
	partitionInfo* currentPartition;

    if(partitionsExistInDisk) {
        allPartitionsArePrimary = true;
        for(i=0; i<partitionsCount; i++) {
            currentPartition = &partitions[i];
            if(currentPartition->extendedIndex != 0) {
                allPartitionsArePrimary = false;
                break;
            }
        }
    } else {
        allPartitionsArePrimary = false;
    }

	while(true) {
		isFirstPage = (firstShownPartitionIndex == 1);
		isLastPage = (firstShownPartitionIndex + PARTITIONS_PER_PAGE) > partitionsCount;
		lastPartitionIndexToShow = isLastPage ? partitionsCount : firstShownPartitionIndex + PARTITIONS_PER_PAGE - 1;

    	Locate(0, screenLinesCount-1);
    	DeleteToEndOfLine();
        if(isFirstPage) {
            sprintf(buffer, partitionsCount == 1 ? "1" : partitionsCount > 9 ? "1-9" : "1-%i", partitionsCount);
            if(isLastPage) {
                sprintf(buffer+4, "ESC = return, %s = toggle active (*)", buffer);
            } else {
                sprintf(buffer+4, "ESC=back, %s=toggle active (*)", buffer);
            }
            PrintCentered(buffer+4);
        } else {
	        PrintCentered("Press ESC to return");
        }

        if(!(isFirstPage && isLastPage)) {
            Locate(0, screenLinesCount-1);
            print(isFirstPage ? "   " : "<--");

            Locate(currentScreenConfig.screenWidth - 4, screenLinesCount-1);
            print(isLastPage ? "   " : "-->");
        }

		ClearInformationArea();
		Locate(0, 3);
		if(partitionsCount == 1) {
			PrintCentered(partitionsExistInDisk ? "One partition found on device" : "One new partition defined");
		} else {
            if(allPartitionsArePrimary) {
                sprintf(buffer, partitionsExistInDisk ? "%i primary partitions found on device" : "%i new primary partitions defined", partitionsCount);
            } else {
			    sprintf(buffer, partitionsExistInDisk ? "%i partitions found on device" : "%i new partitions defined", partitionsCount);
            }
			PrintCentered(buffer);
		}
		NewLine();
		if(partitionsCount > PARTITIONS_PER_PAGE) {
			sprintf(buffer, "Displaying partitions %i - %i", 
				firstShownPartitionIndex, 
				lastPartitionIndexToShow);
			PrintCentered(buffer);
			NewLine();
		}
		NewLine();

		currentPartition = &partitions[firstShownPartitionIndex - 1];

		for(i = firstShownPartitionIndex; i <= lastPartitionIndexToShow; i++) {
			PrintOnePartitionInfo(currentPartition);
			currentPartition++;
		}

		while(true) {
			key = WaitKey();
			if(key == ESC) {
				return;
			} else if(key == CURSOR_LEFT && !isFirstPage) {
				firstShownPartitionIndex -= PARTITIONS_PER_PAGE;
				break;
			} else if(key == CURSOR_RIGHT && !isLastPage) {
				firstShownPartitionIndex += PARTITIONS_PER_PAGE;
				break;
			} else if(isFirstPage && key>=KEY_1 && key<KEY_1+partitionsCount && key<KEY_1+9) {
                TogglePartitionActive(key-KEY_1);
                break;
            }
		}
	}
}

void TogglePartitionActive(byte partitionIndex)
{
    byte status, primaryIndex, extendedIndex;
    partitionInfo* partition;
    ulong partitionTableEntrySector;
    int error;

    partition = &partitions[partitionIndex];

    if(!partitionsExistInDisk) {
        partition->status ^= 0x80;
        return;
    }

    status = partition->status;
    primaryIndex = partition->primaryIndex;
    extendedIndex = partition->extendedIndex;

    sprintf(buffer, "%set active bit of partition %i? (y/n) ", status & 0x80 ? "Res" : "S", partitionIndex + 1);
	PrintStateMessage(buffer);
	if(!GetYesOrNo()) {
		return;
	}

    regs.Bytes.A = selectedDriver->slot;
	regs.Bytes.B = 0xFF;
	regs.Bytes.D = selectedDeviceIndex;
	regs.Bytes.E = selectedLunIndex + 1;
	regs.Bytes.H = partition->primaryIndex | 0x80;
	regs.Bytes.L = partition->extendedIndex;
	DosCallFromRom(_GPART, REGS_ALL);
	if(regs.Bytes.A != 0) {
        return;
    }

	((uint*)&(partitionTableEntrySector))[0] = regs.UWords.DE;
	((uint*)&(partitionTableEntrySector))[1] = regs.UWords.HL;

    PreparePartitioningProcess();  //Needed to set up driver slot, device index, etc
    error = ToggleStatusBit(extendedIndex == 0 ? primaryIndex-1 : 0, partitionTableEntrySector);
    if(error == 0) {
        partition->status ^= 0x80;
    } else {
        sprintf(buffer, "Error when accessing device: %i", error);
        ClearInformationArea();
        Locate(0,7);
        PrintCentered(buffer);
        PrintStateMessage("Press any key...");
        WaitKey();
    }

    return;
}

void PrintOnePartitionInfo(partitionInfo* info)
{
	printf("%c %i: ", info->status & 0x80 ? '*' : ' ', info->extendedIndex == 0 ? info->primaryIndex : info->extendedIndex + 1);

	if(info->partitionType == PARTYPE_FAT12) {
		print("FAT12");
	} else if(info->partitionType == PARTYPE_FAT16 || info->partitionType == PARTYPE_FAT16_SMALL || info->partitionType == PARTYPE_FAT16_LBA) {
		print("FAT16");
	} else if(info->partitionType == 0xB || info->partitionType == 0xC) {
		print("FAT32");
	} else if(info->partitionType == 7) {
		print("NTFS");
	} else {
		printf("Type #%x", info->partitionType);
	}
	print(", ");
	PrintSize(info->sizeInK);
	NewLine();
}


void DeleteAllPartitions()
{
	sprintf(buffer, "Discard all %s partitions? (y/n) ", partitionsExistInDisk ? "existing" : "defined");
	PrintStateMessage(buffer);
	if(!GetYesOrNo()) {
		return;
	}

	partitionsCount = 0;
	partitionsExistInDisk = false;
	unpartitionnedSpaceInSectors = selectedLun->sectorCount;
	RecalculateAutoPartitionSize(true);
}


void RecalculateAutoPartitionSize(bool setToAllSpaceAvailable)
{
	ulong maxAbsolutePartitionSizeInK = (unpartitionnedSpaceInSectors - EXTRA_PARTITION_SECTORS) / 2;

	if(setToAllSpaceAvailable) {
		autoPartitionSizeInK = maxAbsolutePartitionSizeInK;
	}

	if(autoPartitionSizeInK > MAX_FAT16_PARTITION_SIZE_IN_K) {
		autoPartitionSizeInK = MAX_FAT16_PARTITION_SIZE_IN_K;
	} else if(!setToAllSpaceAvailable && autoPartitionSizeInK > maxAbsolutePartitionSizeInK) {
		autoPartitionSizeInK = maxAbsolutePartitionSizeInK;
	}

	if(autoPartitionSizeInK < MIN_PARTITION_SIZE_IN_K) {
		autoPartitionSizeInK = MIN_PARTITION_SIZE_IN_K;
	} else if(autoPartitionSizeInK > maxAbsolutePartitionSizeInK) {
		autoPartitionSizeInK = maxAbsolutePartitionSizeInK;
	}

	if(dos1 && autoPartitionSizeInK > 16*1024) {
		autoPartitionSizeInK = 16*1024;
	}
}


void AddPartition()
{
	uint maxPartitionSizeInM;
	uint maxPartitionSizeInK;
	byte lineLength;
	char* pointer;
	char ch;
	bool validNumberEntered = false;
	ulong enteredSizeInK;
	ulong temp;
	bool lessThan1MAvailable;
	bool sizeInKSpecified;
	ulong unpartitionnedSpaceExceptAlignmentInK = (unpartitionnedSpaceInSectors - EXTRA_PARTITION_SECTORS) / 2;

	maxPartitionSizeInM = (uint)((unpartitionnedSpaceInSectors / 2) >> 10);
	maxPartitionSizeInK = unpartitionnedSpaceExceptAlignmentInK > (ulong)32767 ? (uint)32767 : unpartitionnedSpaceExceptAlignmentInK;

	lessThan1MAvailable = (maxPartitionSizeInM == 0);

	if(maxPartitionSizeInM > (ulong)MAX_FAT16_PARTITION_SIZE_IN_M) {
		maxPartitionSizeInM = MAX_FAT16_PARTITION_SIZE_IN_M;
	}

	PrintStateMessage("Enter size or press ENTER to cancel");

	while(!validNumberEntered) {
		sizeInKSpecified = true;
		ClearInformationArea();
		PrintTargetInfo();
		NewLine();
		print("Add new partition\r\n\r\n");

		if(dos1) {
			printf("WARNING: only partitions of 16M or less%scan be used in DOS 1 mode\r\n\r\n",
				is80ColumnsDisplay ? " " : "\r\n");
		}

		if(lessThan1MAvailable) {
			print("Enter");
		} else {
			printf("Enter partition size in MB (1-%i)\r\nor",
				maxPartitionSizeInM);
		}
		printf(" partition size in KB followed by%s\"K\" (%i-%i): ", 
			is80ColumnsDisplay ? " " : "\r\n",
			MIN_PARTITION_SIZE_IN_K,
			maxPartitionSizeInK);

		buffer[0] = 6;
		regs.Words.DE = (int)buffer;
		DosCallFromRom(_BUFIN, REGS_NONE);
		lineLength = buffer[1];
		if(lineLength == 0) {
			return;
		}

		pointer = buffer + 2;
		pointer[lineLength] = '\0';
		enteredSizeInK = 0;
		while(true) {
			ch = (*pointer++) | 32;
			if(ch == 'k') {
				validNumberEntered = true;
				break;
			} else if(ch == '\0' || ch == 13 || ch == 'm') {
				validNumberEntered = true;
				enteredSizeInK <<= 10;
				sizeInKSpecified = false;
				break;
			} else if(ch < '0' || ch > '9') {
				break;
			}
			//This should be: enteredSizeInK = (enteredSizeInK * 10) + (ch - '0'),
			//but thew computer crashes. Looks like the compiler is doing something wrong
			//when linking the longs handling library.
			temp = enteredSizeInK;
			enteredSizeInK = (enteredSizeInK << 3) + temp + temp  + (ch - '0');
			lineLength--;
			if(lineLength == 0) {
				validNumberEntered = true;
				enteredSizeInK *= 1024;
				sizeInKSpecified = false;
				break;
			}
		}

		if(validNumberEntered &&
			(sizeInKSpecified && (enteredSizeInK > maxPartitionSizeInK) || (enteredSizeInK < MIN_PARTITION_SIZE_IN_K)) ||
			(!sizeInKSpecified && (enteredSizeInK > ((ulong)maxPartitionSizeInM  * 1024)))
			) {
				validNumberEntered = false;
		}
	}

	autoPartitionSizeInK = enteredSizeInK > unpartitionnedSpaceExceptAlignmentInK ? unpartitionnedSpaceExceptAlignmentInK : enteredSizeInK;
	AddAutoPartition();
	unpartitionnedSpaceExceptAlignmentInK = (unpartitionnedSpaceInSectors - EXTRA_PARTITION_SECTORS) / 2;
	autoPartitionSizeInK = enteredSizeInK > unpartitionnedSpaceExceptAlignmentInK ? unpartitionnedSpaceExceptAlignmentInK : enteredSizeInK;
	RecalculateAutoPartitionSize(false);
}


void AddAutoPartition()
{
	partitionInfo* partition = &partitions[partitionsCount];

    partition->status = partitionsCount == 0 ? 0x80 : 0;
	partition->sizeInK = autoPartitionSizeInK;
	partition->partitionType = 
		partition->sizeInK > MAX_FAT12_PARTITION_SIZE_IN_K ? PARTYPE_FAT16_LBA : PARTYPE_FAT12;
	if(partitionsCount == 0) {
		partition->primaryIndex = 1;
		partition->extendedIndex = 0;
	} else {
		partition->primaryIndex = 2;
		partition->extendedIndex = partitionsCount;
	}

	unpartitionnedSpaceInSectors -= (autoPartitionSizeInK * 2);
	unpartitionnedSpaceInSectors -= EXTRA_PARTITION_SECTORS;
	partitionsCount++;
	RecalculateAutoPartitionSize(false);
}


void UndoAddPartition()
{
	partitionInfo* partition = &partitions[partitionsCount - 1];
	autoPartitionSizeInK = partition->sizeInK;
	unpartitionnedSpaceInSectors += (partition->sizeInK * 2);
	unpartitionnedSpaceInSectors += EXTRA_PARTITION_SECTORS;
	partitionsCount--;
	RecalculateAutoPartitionSize(false);
}


void TestDeviceAccess()
{
	ulong sectorNumber = 0;
	char* message = "Now reading device sector ";
	byte messageLen = strlen(message);
	byte error;
	char* errorMessageHeader = "Error when reading sector ";

	InitializeScreenForTestDeviceAccess(message);

	while(GetKey() == 0) {
		sprintf(buffer, "%u", sectorNumber);
		//_ultoa(sectorNumber, buffer, 10);
		Locate(messageLen, MESSAGE_ROW);
		print(buffer);
		print(" ...\x1BK");

		regs.Flags.C = 0;
		regs.Bytes.A = selectedDeviceIndex;
		regs.Bytes.B = 1;
		regs.Bytes.C = selectedLunIndex + 1;
		regs.Words.HL = (int)buffer;
		regs.Words.DE = (int)&sectorNumber;
		DriverCall(selectedDriver->slot, DEV_RW);

		if((error = regs.Bytes.A) != 0) {
			strcpy(buffer, errorMessageHeader);
			sprintf(buffer + strlen(errorMessageHeader), "%u", sectorNumber);
			strcpy(buffer + strlen(buffer), ":");
			PrintDosErrorMessage(error, buffer);
			PrintStateMessage("Continue reading sectors? (y/n) ");
			if(!GetYesOrNo()) {
				return;
			}
			InitializeScreenForTestDeviceAccess(message);
		}

		sectorNumber++;
		if(sectorNumber >= selectedLun->sectorCount) {
			sectorNumber = 0;
		}
	}
}


void InitializeScreenForTestDeviceAccess(char* message)
{
	ClearInformationArea();
	PrintTargetInfo();
	Locate(0, MESSAGE_ROW);
	print(message);
	PrintStateMessage("Press any key to stop...");
}


void PrintDosErrorMessage(byte code, char* header)
{
	Locate(0, MESSAGE_ROW);
	PrintCentered(header);
	NewLine();

	regs.Bytes.B = code;
	regs.Words.DE = (int)buffer;
	DosCallFromRom(_EXPLAIN, REGS_NONE);
	if(strlen(buffer) > currentScreenConfig.screenWidth) {
		print(buffer);
	} else {
		PrintCentered(buffer);
	}

	PrintStateMessage("Press any key to return...");
}


void PrintDone()
{
	PrintCentered("Done!");
	print("\x0A\x0D\x0A\x0A\x0A");
	PrintCentered("If this device had drives mapped to,");
	NewLine();
	PrintCentered("please reset the computer.");
}

bool FormatWithoutPartitions()
{
	dosFilesystemParameters parameters;
	byte error;

	if(!ConfirmDataDestroy("Format device without partitions")) {
		return false;
	}

	ClearInformationArea();
	PrintTargetInfo();
	Locate(0, MESSAGE_ROW);
	PrintCentered("Formatting the device...");
	PrintStateMessage("Please wait...");

#ifdef TEST_FAT_PARAMETERS
	CalculateFatFileSystemParameters(selectedLun->sectorCount / 2, &parameters);
	WaitKey();
	return 0;
#else
	error = CreateFatFileSystem(0, selectedLun->sectorCount / 2);
	if(error == 0) {
		Locate(0, MESSAGE_ROW + 2);
		PrintDone();
		PrintStateMessage("Press any key to return...");
	} else {
		PrintDosErrorMessage(error, "Error when formatting device:");
	}
	WaitKey();
	return (error == 0);
#endif
}


byte CreateFatFileSystem(ulong firstDeviceSector, ulong fileSystemSizeInK)
{
	byte* remoteCallParams = buffer;

	remoteCallParams[0] = selectedDriver->slot;
	remoteCallParams[1] = selectedDeviceIndex;
	remoteCallParams[2] = selectedLunIndex + 1;
	*((ulong*)&remoteCallParams[3]) = 0;
	*((ulong*)&remoteCallParams[7]) = selectedLun->sectorCount / 2;
	
	return (byte)CallFunctionInExtraBank(f_CreateFatFileSystem, remoteCallParams);
}


#ifdef TEST_FAT_PARAMETERS
void CalculateFatFileSystemParameters(ulong fileSystemSizeInK, dosFilesystemParameters* parameters)
{
	byte remoteCallParams[6];
	*((ulong*)&remoteCallParams[0]) = fileSystemSizeInK;
	*((int*)&remoteCallParams[4]) = (int)parameters;
	CallFunctionInExtraBank(f_CalculateFatFileSystemParameters, remoteCallParams);
}
#endif


bool WritePartitionTable()
{
	//http://www.rayknights.org/pc_boot/ext_tbls.htm

	int i;
	//masterBootRecord* mbr = (masterBootRecord*)buffer + 80;
	byte error;

	sprintf(buffer, "Create %i partitions on device", partitionsCount);

	if(!ConfirmDataDestroy(buffer)) {
		return false;
	}

	ClearInformationArea();
	PrintTargetInfo();
	PrintStateMessage("Please wait...");

	Locate(0, MESSAGE_ROW);
	PrintCentered("Preparing partitioning process...");
	PreparePartitioningProcess();

	for(i = 0; i < partitionsCount; i++) {
		Locate(0, MESSAGE_ROW);
		sprintf(buffer, "Creating partition %i of %i ...", i + 1, partitionsCount);
		PrintCentered(buffer);

		error = CreatePartition(i);
		if(error != 0) {
			sprintf(buffer, "Error when creating partition %i :", i + 1);
			PrintDosErrorMessage(error, buffer);
			WaitKey();
			return false;
		}

		
	}
	
	Locate(0, MESSAGE_ROW + 2);
	PrintDone();
	PrintStateMessage("Press any key to return...");
	WaitKey();
	return true;
}


void PreparePartitioningProcess()
{
	byte* remoteCallParams = buffer;

	remoteCallParams[0] = selectedDriver->slot;
	remoteCallParams[1] = selectedDeviceIndex;
	remoteCallParams[2] = selectedLunIndex + 1;
	*((uint*)&remoteCallParams[3]) = partitionsCount;
	*((partitionInfo**)&remoteCallParams[5]) = &partitions[0];
	*((uint*)&remoteCallParams[7]) = luns[selectedLunIndex].sectorsPerTrack;

	CallFunctionInExtraBank(f_PreparePartitioningProcess, remoteCallParams);
}


byte CreatePartition(int index)
{
	byte* remoteCallParams = buffer;

	*((uint*)&remoteCallParams[0]) = index;
	
	return (byte)CallFunctionInExtraBank(f_CreatePartition, remoteCallParams);
}

byte ToggleStatusBit(byte partitionTableEntryIndex, ulong partitionTablesector)
{
    byte* remoteCallParams = buffer;

	remoteCallParams[0] = partitionTableEntryIndex;
    *((ulong*)&remoteCallParams[1]) = partitionTablesector;
	
	return (byte)CallFunctionInExtraBank(f_ToggleStatusBit, remoteCallParams);
}

bool ConfirmDataDestroy(char* action)
{
	char* spaceOrNewLine = is80ColumnsDisplay ? " " : "\r\n";

	PrintStateMessage("");
	ClearInformationArea();
	PrintTargetInfo();
	Locate(0, MESSAGE_ROW);

	printf("%s\r\n"
		   "\r\n"
		   "THIS WILL DESTROY%sALL DATA ON THE DEVICE!!\r\n"
		   "This action can't be cancelled%sand can't be undone\r\n"
		   "\r\n"
		   "Are you sure? (y/n) ",
		   action,
		   spaceOrNewLine,
		   spaceOrNewLine);

	return GetYesOrNo();
}


void ClearInformationArea()
{
    int i;
    
    Locate(0, 2);
    for(i = 0; i < screenLinesCount - 4; i++) {
        DeleteToEndOfLineAndCursorDown();
    }
}


void GetDriversInformation()
{
    byte error = 0;
    byte driverIndex = 1;
    driverInfo* currentDriver = &drivers[0];

    installedDriversCount = 0;
    
#ifdef FAKE_DRIVER_INFO
    strcpy(currentDriver->driverName, "Un drivercillo de nada ");
    currentDriver->slot = 1;
    currentDriver->flags = 0xFF;
    currentDriver->versionMain = 2;
    currentDriver->versionSec = 34;
    currentDriver->versionRev = 5;
    
    currentDriver++;
    strcpy(currentDriver->driverName, "Otro driver, este de 32 caracter");
    currentDriver->slot = 0x8E;
    currentDriver->flags = 0xFF;
    currentDriver->versionMain = 2;
    currentDriver->versionSec = 34;
    currentDriver->versionRev = 0;
    
    installedDriversCount = 2;
    return;
#else
    
    while(error == 0 && driverIndex <= MAX_INSTALLED_DRIVERS) {
        regs.Bytes.A = driverIndex;
        regs.Words.HL = (int)currentDriver;
        DosCallFromRom(_GDRVR, REGS_AF);
        if((error = regs.Bytes.A) == 0 && (currentDriver->flags & (DRIVER_IS_DOS250 | DRIVER_IS_DEVICE_BASED) == (DRIVER_IS_DOS250 | DRIVER_IS_DEVICE_BASED))) {
            installedDriversCount++;
            TerminateRightPaddedStringWithZero(currentDriver->driverName, DRIVER_NAME_LENGTH);
			currentDriver++;
        }
        driverIndex++;
    }
#endif
}


void TerminateRightPaddedStringWithZero(char* string, byte length)
{
    char* pointer = string + length - 1;
    while(*pointer == ' ' && length > 0) {
        pointer--;
        length--;
    }
    pointer[1] = '\0';
}


byte WaitKey()
{
	byte key;

	while((key = GetKey()) == 0);
	return key;
}


byte GetKey()
{
	regs.Bytes.E = 0xFF;
	DosCallFromRom(_DIRIO, REGS_AF);
	return regs.Bytes.A;
}


void SaveOriginalScreenConfiguration()
{
	originalScreenConfig.screenMode = *(byte*)SCRMOD;
	originalScreenConfig.screenWidth = *(byte*)LINLEN;
	originalScreenConfig.functionKeysVisible = (*(byte*)CNSDFG != 0);
}


void ComposeWorkScreenConfiguration()
{
	currentScreenConfig.screenMode = 0;
	currentScreenConfig.screenWidth = (*(byte*)LINLEN <= MAX_LINLEN_MSX1 ? MAX_LINLEN_MSX1 : MAX_LINLEN_MSX2);
	currentScreenConfig.functionKeysVisible = false;
	is80ColumnsDisplay = (currentScreenConfig.screenWidth == MAX_LINLEN_MSX2);
	screenLinesCount = *(byte*)CRTCNT;
}


void SetScreenConfiguration(ScreenConfiguration* screenConfig)
{
	if(screenConfig->screenMode == 0) {
		*((byte*)LINL40) = screenConfig->screenWidth;
		AsmCall(INITXT, &regs, REGS_NONE, REGS_NONE);
	} else {
		*((byte*)LINL32) = screenConfig->screenWidth;
		AsmCall(INIT32, &regs, REGS_NONE, REGS_NONE);
	}

	AsmCall(screenConfig->functionKeysVisible ? DSPFNK : ERAFNK, &regs, REGS_NONE, REGS_NONE);
}


void InitializeWorkingScreen(char* header)
{
	ClearScreen();
	PrintCentered(header);
	CursorDown();
	PrintRuler();
	Locate(0, screenLinesCount - 2);
	PrintRuler();
}


void PrintRuler()
{
	int i;

	HomeCursor();
	for(i = 0; i < currentScreenConfig.screenWidth; i++) {
		chput('-');
	}
}


void Locate(byte x, byte y)
{
	regs.Bytes.H = x + 1;
	regs.Bytes.L = y + 1;
	AsmCall(POSIT, &regs, REGS_MAIN, REGS_NONE);
}


void LocateX(byte x)
{
	regs.Bytes.H = x + 1;
	regs.Bytes.L = *(byte*)CSRY;
	AsmCall(POSIT, &regs, REGS_MAIN, REGS_NONE);
}


void PrintCentered(char* string)
{
	byte pos = (currentScreenConfig.screenWidth - strlen(string)) / 2;
	HomeCursor();
	LocateX(pos);
	print(string);
}


void PrintStateMessage(char* string)
{
	Locate(0, screenLinesCount-1);
	DeleteToEndOfLine();
	print(string);
}


void chput(char ch) __naked
{
    __asm
    push    ix
    ld      ix,#4
    add     ix,sp
    ld  a,(ix)
    call CHPUT
    pop ix
    ret
    __endasm;
}


void print(char* string) __naked
{
	 __asm
    push    ix
    ld      ix,#4
    add     ix,sp
    ld  l,(ix)
	ld	h,1(ix)
PRLOOP:
	ld	a,(hl)
	or	a
	jr	z,PREND
    call CHPUT
	inc	hl
	jr	PRLOOP
PREND:
    pop ix
    ret
    __endasm;
}


// function number is defined in fdisk.h
int CallFunctionInExtraBank(int functionNumber, void* parametersBuffer)
{
	regs.Bytes.A = (*((byte*)0x40ff)) + 1;	//Extra functions bank = our bank + 1
	regs.Words.BC = functionNumber;
	regs.Words.HL = (int)parametersBuffer;
	regs.Words.IX = (int)0x4100;	//Address of "main" for extra functions program
	AsmCall(CALBNK, &regs, REGS_ALL, REGS_MAIN);
	return regs.Words.HL;
}

#ifdef MAKEBUILD
#include "printf.c"
#include "AsmCall.c"
#else
#include "../../tools/C/printf.c"
#include "../../tools/C/asmcall.c"
#endif
#include "drivercall.c"
