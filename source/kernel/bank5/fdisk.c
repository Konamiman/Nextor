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
#include "../../tools/C/system.h"
#include "../../tools/C/dos.h"
#include "../../tools/C/types.h"
#include "../../tools/C/asmcall.h"
#include "drivercall.h"
#include "../../tools/C/partit.h"
#include "fdisk.h"

#define MESSAGE_ROW 9
#define PARTITIONS_PER_PAGE 15

#define CMD_FDISK 1

#define DEVICES_PER_PAGE 4
#define MAX_DEVICE_PAGES 16
#define MAX_DEVICES_PER_DRIVER (DEVICES_PER_PAGE * MAX_DEVICE_PAGES)

typedef struct {
    byte screenMode;
    byte screenWidth;
    bool functionKeysVisible;
} ScreenConfiguration;

char buffer[1000];
driverInfo drivers[MAX_INSTALLED_DRIVERS];
driverInfo* selectedDriver;
char selectedDriverName[50];
deviceInfo* selectedDevice;
deviceParams* selectedDeviceParams;
deviceInfo devices[MAX_DEVICES_PER_DRIVER];
deviceInfo* currentDevice;
byte selectedDeviceIndex;
byte installedDriversCount;
bool availableDevicesCount;
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
byte currentDevicesPage;
byte devicesPageCount;
byte sectorBuffer[512];
byte sectorBufferBackup[512];
byte ASMRUT[4];
byte OUT_FLAGS;
Z80_registers regs;
ulong nextDeviceSector;
ulong mainExtendedPartitionSectorCount;
ulong mainExtendedPartitionFirstSector;

#define HideCursor() print("\x1Bx5")
#define DisplayCursor() print("\x1By5")
#define CursorDown() chput('\x1F')
#define CursorUp() chput('\x1E')
#define ClearScreen() chput('\x0C')
#define HomeCursor() print("\x0D\x1BK")
#define DeleteToEndOfLine() print("\x1BK")
#define DeleteToEndOfLineAndCursorDown() print("\x1BK\x1F");
#define NewLine() print("\x0A\x0D");


void GoDriverSelectionScreen();
void ShowDriverSelectionScreen();
void ComposeSlotString(byte slot, char* destination);
void GoDeviceSelectionScreen(byte driverIndex);
void ShowDeviceSelectionScreen();
void GetDevicesInformation();
void EnsureMaximumStringLength(char* string, int maxLength);
void ReplaceLastCharsWithDots(char* string, int length);
void InitializePartitioningVariables(byte deviceIndex);
void PrintSize(ulong sizeInK);
byte GetRemainingBy1024String(ulong value, char* destination);
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
void PrintDosErrorMessage(byte code, char* header);
bool FormatWithoutPartitions();
#ifdef TEST_FAT_PARAMETERS
void CalculateFatFileSystemParameters(ulong fileSystemSizeInK, dosFilesystemParameters* parameters);
#endif
bool WritePartitionTable();
void PreparePartitioningProcess();
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

#define Clear(address, len) memset(address, 0, len)
#define ReadSectorFromDevice(firstDeviceSector) DeviceSectorRW(firstDeviceSector, 0)
#define WriteSectorToDevice(firstDeviceSector) DeviceSectorRW(firstDeviceSector, 1)

byte CreateFatFileSystem(ulong firstDeviceSector, ulong fileSystemSizeInK);
void CreateFatBootSector(dosFilesystemParameters* parameters);
ulong GetNewSerialNumber();
void ClearSectorBuffer();
void SectorBootCode();
void CalculateFatFileSystemParameters(ulong fileSystemSizeInK, dosFilesystemParameters* parameters);
int CalculateFatFileSystemParametersFat12(ulong fileSystemSizeInK, dosFilesystemParameters* parameters);
int CalculateFatFileSystemParametersFat16(ulong fileSystemSizeInK, dosFilesystemParameters* parameters);
byte DeviceSectorRW(ulong firstDeviceSector, byte write);
int CreatePartition(int index);
int ToggleStatusBit(byte partitionTableEntryIndex, ulong partitonTablesector);


void main(int bc, int hl)
{
    ASMRUT[0] = 0xC3;   //Code for JP
	dos1 = (*((byte*)0xF313) == 0);

	installedDriversCount = 0;

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
    char rev[5];
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
			sprintf(rev, ".%i", revByte);
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
	byte driverNameLength;
	byte deviceInfoIndex;

	selectedDriver = &drivers[driverIndex - 1];
	ComposeSlotString(selectedDriver->slot, slot);
	strcpy(selectedDriverName, selectedDriver->driverName);
	driverNameLength = strlen(selectedDriverName);
	sprintf(selectedDriverName + driverNameLength,
		"%son slot %s\r\n",
		is80ColumnsDisplay || (driverNameLength < DRIVER_NAME_LENGTH_40 - 12) ? " " : "\r\n",
		slot);

	GetDevicesInformation();
	currentDevicesPage = 0;
	devicesPageCount = availableDevicesCount / DEVICES_PER_PAGE;
	if((availableDevicesCount % DEVICES_PER_PAGE) != 0) {
		devicesPageCount++;
	}

    while(true) {
        ShowDeviceSelectionScreen();
        if(availableDevicesCount == 0) {
            return;
        }
        
        while(true) {
            key = WaitKey();
			if(key == CURSOR_RIGHT) {
				if(currentDevicesPage < devicesPageCount - 1) {
					currentDevicesPage++;
					break;
				}
			}
			else if(key == CURSOR_LEFT) {
				if(currentDevicesPage != 0) {
					currentDevicesPage--;
					break;
				}
			}
            else if(key == ESC) {
                return;
            } else {
                key -= '0';
				if(key < 1 || key > DEVICES_PER_PAGE) {
					continue;
				}
				deviceInfoIndex = (currentDevicesPage * DEVICES_PER_PAGE) + (key - 1);
                if(deviceInfoIndex < availableDevicesCount && devices[deviceInfoIndex].canCreatePartitions) {
					InitializePartitioningVariables(deviceInfoIndex);
                    GoPartitioningMainMenuScreen();
                    break;
                }
            }
        }
    }
}


void ShowDeviceSelectionScreen()
{
	byte i;
	byte baseDeviceIndex = currentDevicesPage * DEVICES_PER_PAGE;

	ClearInformationArea();
	Locate(0,3);
	print(selectedDriverName);
	CursorDown();
	CursorDown();

    if(availableDevicesCount == 0) {
    	Locate(0, 9);
    	PrintCentered("There are no suitable devices");
    	CursorDown();
    	PrintCentered("attached to the driver");
    	PrintStateMessage("Press any key to go back...");
    	WaitKey();
    	return;
	}

	for(i = 0; i < DEVICES_PER_PAGE; i++) {
		currentDevice = &devices[baseDeviceIndex + i];
		if(baseDeviceIndex + i >= availableDevicesCount) {
			break;
		}
		currentDevice->canCreatePartitions = false;
		printf("\x1BK%i. %s\r\n",
			i + 1,
			currentDevice->deviceName);
		if(!currentDevice->isValid) {
			printf("(error getting device info)");
		}
		else if(!currentDevice->isOnline) {
			printf("(device is offline)");
		}
		else if(currentDevice->params.mediumType != DEV_TYPE_BLOCK) {
			printf("(not a block device)");
		}
		else if((currentDevice->params.flags & DEV_FLAG_FLOPPY) != 0) {
			printf("(floppy disk device)");
		}
		else if((currentDevice->params.flags & DEV_FLAG_READ_ONLY) != 0) {
			printf("(read-only device)");
		}
		else if(currentDevice->params.sectorSize != 512) {
			printf("(unsupported sector size)");
		}
		else if(currentDevice->params.sectorCount == 0) {
			printf("(unknown device size)");
		}
		else {
			printf("Size: ");
			PrintSize(currentDevice->params.sectorCount / 2);
			currentDevice->canCreatePartitions = true;
		}
		printf(currentDevice->canCreatePartitions || is80ColumnsDisplay ? " (Device number: %i)" : " (Dev: %i)", currentDevice->deviceNumber);

		if(!currentDevice->canCreatePartitions) {
			//Erase the number displayed before the device name
			//to give a visual clue that it can't be selected.
			//The sequence is:
			//\r - carriage return (go to beginning of line)
			//\x1E - cursor up (point to the number)
			//two spaces (remove number and dot)
			//\x1F - cursor down (back to the error message line)
			printf("\r\x1E  \x1F");
		}

		NewLine();
		NewLine();
	}

	if(availableDevicesCount < MAX_DEVICES_PER_DRIVER) {
		NewLine();
	}

    PrintStateMessage("Select the device");
	Locate(0, screenLinesCount - 5);
	print("ESC. Go back to driver selection screen\r\n");

	if(devicesPageCount > 1) {
		if(currentDevicesPage == 0) {
			print("RIGHT. Next page");
		}
		else if(currentDevicesPage == devicesPageCount - 1) {
			print("LEFT. Previous page");
		}
		else {
			print("LEFT/RIGHT. Previous/Next page");
		}
		Locate(is80ColumnsDisplay ? 80-13 : 40-13, screenLinesCount - 1);
		printf("Page %i / %i", currentDevicesPage + 1, devicesPageCount);
	}
}


void GetDevicesInformation()
{
    byte error = 0;
    byte deviceIndex;
	byte deviceNumber;
    deviceInfo* currentDevice = &devices[0];
	char* currentDeviceName;
	byte maxDeviceNumber;
	byte maxDeviceNameLength = (is80ColumnsDisplay ? DRIVER_NAME_LENGTH_80 : DRIVER_NAME_LENGTH_40) + 1;

	regs.Bytes.A = DRVQ_GET_MAX_DEVICE_NUMBER;
	DriverCall(selectedDriver->slot, DRIVER_QUERY);
	maxDeviceNumber = regs.Bytes.A == 0 ? regs.Bytes.B : DEFAULT_MAX_DEVICE_NUMBER;

    availableDevicesCount = 0;
	deviceIndex = 0;
	deviceNumber = 1;

	while(deviceIndex < MAX_DEVICES_PER_DRIVER && deviceNumber <= maxDeviceNumber) {
		currentDevice->deviceNumber = deviceNumber;
		currentDeviceName = currentDevice->deviceName;

		regs.Bytes.A = DEVQ_GET_STRING;
		regs.Bytes.B = STRING_DEVICE_NAME;
		regs.Bytes.C = deviceNumber;
		regs.Bytes.D = maxDeviceNameLength;
		regs.Words.HL = (int)currentDeviceName;
		DriverCall(selectedDriver->slot, DEVICE_QUERY);

		deviceNumber++;

		if(regs.Bytes.A == ERR_QUERY_NOT_IMPLEMENTED) {
			sprintf(currentDeviceName, "(Unnamed device)");
		}
		else if(regs.Bytes.A == ERR_QUERY_TRUNCATED_STRING) {
			ReplaceLastCharsWithDots(currentDeviceName, 0);
		}
		else if(regs.Bytes.A != 0) {
			continue;
		}

		currentDevice->isValid = true;
		availableDevicesCount++;

		deviceIndex++;
		currentDevice = &devices[deviceIndex];
	}

	if(availableDevicesCount == 0) {
		return;
	}

	deviceIndex = 0;

	while(deviceIndex < availableDevicesCount) {
		currentDevice = &devices[deviceIndex];
		currentDevice->isOnline = true;

		regs.Bytes.A = DEVQ_GET_AVAILABILITY;
		regs.Bytes.C = currentDevice->deviceNumber;
		DriverCall(selectedDriver->slot, DEVICE_QUERY);

		deviceIndex++;

		if(regs.Bytes.A == ERR_QUERY_NOT_IMPLEMENTED) {
			continue;
		}

		if(regs.Bytes.A != 0) {
			//Should never happen if the driver is consistent
			//(for a given device number it either returns "Invalid device" error for
			//all the queries invoked through the DEVICE_QUERY routine,
			//or for none of them)
			currentDevice->isValid = false;
		}
		else if(regs.Bytes.B == 0) {
			currentDevice->isOnline = false;
		}
	}

	deviceIndex = 0;

	while(deviceIndex < availableDevicesCount) {
		currentDevice = &devices[deviceIndex];
		if(currentDevice->isValid && currentDevice->isOnline) {
			regs.Bytes.A = DEVQ_GET_PARAMS;
			regs.Bytes.C = currentDevice->deviceNumber;
			regs.Words.HL = (int)currentDevice->params;
			DriverCall(selectedDriver->slot, DEVICE_QUERY);

			if(regs.Bytes.A == ERR_QUERY_NOT_IMPLEMENTED) {
				currentDevice->params.mediumType = DEV_TYPE_BLOCK;
				currentDevice->params.sectorSize = 512;
				currentDevice->params.sectorCount = 0;
				currentDevice->params.flags = 0;
			}
			else if(regs.Bytes.A != 0) {
				//Should never happen if the driver is consistent
				//(for a given device number it either returns "Invalid device" error for
				//all the queries invoked through the DEVICE_QUERY routine,
				//or for none of them)
				currentDevice->isValid = false;
			}
		}

		deviceIndex++;
	}
}


void EnsureMaximumStringLength(char* string, int maxLength)
{
	int len = strlen(string);
	if(len > maxLength) {
		ReplaceLastCharsWithDots(string, maxLength);
	}
}


void ReplaceLastCharsWithDots(char* string, int length) {
	if(length == 0) length = strlen(string);
	string += length - 3;
	*string++ = '.';
	*string++ = '.';
	*string++ = '.';
	*string = '\0';
}


void InitializePartitioningVariables(byte deviceIndex)
{
	selectedDevice = &devices[deviceIndex];
	selectedDeviceParams = &selectedDevice->params;
	partitionsCount = 0;
	partitionsExistInDisk = true;
	canCreatePartitions = (selectedDeviceParams->sectorCount >= (MIN_DEVICE_SIZE_FOR_PARTITIONS_IN_K * 2));
	canDoDirectFormat = (selectedDeviceParams->sectorCount <= MAX_DEVICE_SIZE_FOR_DIRECT_FORMAT_IN_K * 2);
	unpartitionnedSpaceInSectors = selectedDeviceParams->sectorCount;
	RecalculateAutoPartitionSize(true);
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


void PrintTargetInfo()
{
	Locate(0,3);
	print(selectedDriverName);
	printf("%s\r\nSize: ", selectedDevice->deviceName);
	PrintSize(selectedDeviceParams->sectorCount / 2);
	printf(" (Device number: %i)\r\n", selectedDevice->deviceNumber);
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

	partitionsCount = 0;

	do {
		regs.Bytes.A = selectedDriver->slot;
		regs.Bytes.B = 0xFF;
		regs.Bytes.D = selectedDevice->deviceNumber;
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
	regs.Bytes.D = selectedDevice->deviceNumber;
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
	unpartitionnedSpaceInSectors = selectedDeviceParams->sectorCount;
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
	CalculateFatFileSystemParameters(selectedDeviceParams->sectorCount / 2, &parameters);
	WaitKey();
	return 0;
#else
	error = CreateFatFileSystem(0, selectedDeviceParams->sectorCount / 2);
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
	int i;

	nextDeviceSector = 0;
	mainExtendedPartitionSectorCount = 0;
	mainExtendedPartitionFirstSector = 0;

	for(i = 1; i < partitionsCount; i++) {
		mainExtendedPartitionSectorCount += ((&partitions[i])->sizeInK * 2) + 1;	//+1 for the MBR
	}
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
    
   
    while(error == 0 && driverIndex <= MAX_INSTALLED_DRIVERS) {
        regs.Bytes.A = driverIndex | GDRVR_EXTENDED_DRIVER_NAME_FLAG;
        regs.Words.HL = (int)currentDriver;
        DosCallFromRom(_GDRVR, REGS_AF);
        if((error = regs.Bytes.A) == 0 && (currentDriver->flags & DRIVER_IS_NEXTOR == DRIVER_IS_NEXTOR)) {
            installedDriversCount++;
			EnsureMaximumStringLength(currentDriver->driverName, is80ColumnsDisplay ? DRIVER_NAME_LENGTH_80 : DRIVER_NAME_LENGTH_40);
			currentDriver++;
        }
        driverIndex++;
    }
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
	byte width;
	
	// "Hack" for korean MSX computers that do weird things
	// when printing a character at the last column of the screen
	if(*((byte*)H_CHPH) != 0xC9) {
		width = currentScreenConfig.screenWidth - 1;
	}
	else {
		width = currentScreenConfig.screenWidth;
	}

	HomeCursor();
	for(i = 0; i < width; i++) {
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
	;A = ch

	jp CHPUT
    __endasm;
}


void print(char* string) __naked
{
	 __asm

	 ;HL = string
	 
PRLOOP:
	ld	a,(hl)
	or	a
	jr	z,PREND
    call CHPUT
	inc	hl
	jr	PRLOOP
PREND:
    ret
    __endasm;
}


byte CreateFatFileSystem(ulong firstDeviceSector, ulong fileSystemSizeInK)
{
	dosFilesystemParameters parameters;
	byte error;
	ulong sectorNumber;
	uint zeroSectorsToWrite;
	uint i;

	CalculateFatFileSystemParameters(fileSystemSizeInK, &parameters);
	
	//* Boot sector

	CreateFatBootSector(&parameters);

	if((error = WriteSectorToDevice(firstDeviceSector)) != 0) {
		return error;
	}

	//* FAT (except 1st sector) and root directory sectors

	ClearSectorBuffer();
	zeroSectorsToWrite = (parameters.sectorsPerFat * FAT_COPIES) + (parameters.sectorsPerRootDirectory) - 1;
	sectorNumber = firstDeviceSector + 2;
	for(i = 0; i < zeroSectorsToWrite; i++) {
		if((error = WriteSectorToDevice(sectorNumber)) != 0) {
			return error;
		}
		sectorNumber++;
	}

	//* First sector of each FAT

	sectorBuffer[0] = 0xF0;
	sectorBuffer[1] = 0xFF;
	sectorBuffer[2] = 0xFF;
	if(parameters.isFat16) {
		sectorBuffer[3] = 0xFF;
	}
	if((error = WriteSectorToDevice(firstDeviceSector + 1)) != 0) {
		return error;
	}
	if((error = WriteSectorToDevice(firstDeviceSector + 1 + parameters.sectorsPerFat)) != 0) {
		return error;
	}

	//* Done

	return 0;
}


void CreateFatBootSector(dosFilesystemParameters* parameters)
{
	fatBootSector* sector = (fatBootSector*)sectorBuffer;

	ClearSectorBuffer();

	sector->jumpInstruction[0] = 0xEB;
	sector->jumpInstruction[1] = 0xFE;
	sector->jumpInstruction[2] = 0x90;
	strcpy(sector->oemNameString, "NEXTOR20");
	sector->sectorSize = 512;
	sector->sectorsPerCluster = parameters->sectorsPerCluster;
	sector->reservedSectors = 1;
	sector->numberOfFats = FAT_COPIES;
	sector->rootDirectoryEntries = parameters->sectorsPerRootDirectory * DIR_ENTRIES_PER_SECTOR;
	if((parameters->totalSectors & 0xFFFF0000) == 0) {
		sector->smallSectorCount = parameters->totalSectors;
	}
	sector->mediaId = 0xF0;
	sector->sectorsPerFat = parameters->sectorsPerFat;
	strcpy(sector->params.standard.volumeLabelString, "NEXTOR 2.0 "); //it is same for DOS 2.20 format
	sector->params.standard.serialNumber = GetNewSerialNumber(); //it is same for DOS 2.20 format
	sector->mbrSignature = 0xAA55;

	if(parameters->isFat16) {
		sector->params.standard.bigSectorCount = parameters->totalSectors;
		sector->params.standard.extendedBlockSignature = 0x29;
		strcpy(sector->params.standard.fatTypeString, "FAT16   ");
	} else {
		sector->params.DOS220.z80JumpInstruction[0] = 0x18;
		sector->params.DOS220.z80JumpInstruction[1] = 0x1E;
		strcpy(sector->params.DOS220.volIdString, "VOL_ID");
		strcpy(sector->params.DOS220.fatTypeString, "FAT12   ");
		memcpy(&(sector->params.DOS220.z80BootCode), SectorBootCode, (uint)0xC090 - (uint)0xC03E);
	}
}


ulong GetNewSerialNumber() __naked
{
	__asm

		ld a,r
		xor	b
		ld e,a
		or #128
		ld b,a
gnsn_1:
		nop
		djnz gnsn_1

		ld	a,r
		xor	e
		ld d,a
		or #64
		ld b,a
gnsn_2:
		nop
		nop
		djnz gnsn_2

		ld	a,r
		xor	d
		ld l,a
		or #32
		ld b,a
gnsn_3:
		nop
		nop
		nop
		djnz gnsn_3

		ld	a,r
		xor l
		ld	h,a

		ret

	__endasm;
}


void ClearSectorBuffer() __naked
{
	__asm

		ld hl,#_sectorBuffer
		ld de,#_sectorBuffer
		inc de
		ld bc,#512-1
		ld (hl),#0
		ldir
		ret

	__endasm;
}


void SectorBootCode() __naked
{
	__asm

		ret nc
		ld (#0xc07b),de
		ld de,#0xc078
		ld (hl),e
		inc hl
		ld (hl),d
		ld de,#0xc080
		ld c,#0x0f
		call #0xf37d
		inc a
		jp z,#0x4022
		ld de,#0x100
		ld c,#0x1a
		call #0xf37d
		ld hl,#1
		ld (#0xc08e),hl
		ld hl,#0x3f00
		ld de,#0xc080
		ld c,#0x27
		push de
		call #0xf37d
		pop de
		ld c,#0x10
		call #0xf37d
		jp 0x0100
		ld l,b
		ret nz
		call #0
		jp 0x4022
		nop
		.ascii "MSXDOS  SYS"
		nop
		nop
		nop
		nop

	__endasm;
}


void CalculateFatFileSystemParameters(ulong fileSystemSizeInK, dosFilesystemParameters* parameters)
{
	if(fileSystemSizeInK > MAX_FAT12_PARTITION_SIZE_IN_K) {
		CalculateFatFileSystemParametersFat16(fileSystemSizeInK, parameters);
	} else {
		CalculateFatFileSystemParametersFat12(fileSystemSizeInK, parameters);
	}
}


int CalculateFatFileSystemParametersFat12(ulong fileSystemSizeInK, dosFilesystemParameters* parameters)
{
	//Note: Partitions <=16M are defined to have at most 3 sectors per FAT,
	//so that they can boot DOS 1. This limits the cluster count to 1021.

	uint sectorsPerCluster;
	uint sectorsPerFat;
	uint clusterCount;
	ulong dataSectorsCount;
	uint difference;
	uint sectorsPerClusterPower;
	uint maxClusterCount = MAX_FAT12_CLUSTER_COUNT;
	uint maxSectorsPerFat = 12;

	if(fileSystemSizeInK <= (2 * (ulong)1024)) {
		sectorsPerClusterPower = 0;
		sectorsPerCluster = 1;
	} else if(fileSystemSizeInK <= (4 * (ulong)1024)) {
		sectorsPerClusterPower = 1;
		sectorsPerCluster = 2;
	} else if(fileSystemSizeInK <= (8 * (ulong)1024)) {
		sectorsPerClusterPower = 2;
		sectorsPerCluster = 4;
	} else if(fileSystemSizeInK <= (16 * (ulong)1024)) {
		sectorsPerClusterPower = 3;
		sectorsPerCluster = 8;
	} else {
		sectorsPerClusterPower = 4;
		sectorsPerCluster = 16;
	}

    if(fileSystemSizeInK <= (16 * (ulong)1024)) {
        maxClusterCount = 1021;
		maxSectorsPerFat = 3;
        sectorsPerCluster *= 4;
        sectorsPerClusterPower += 2;
    }

	dataSectorsCount = (fileSystemSizeInK * 2) - (FAT12_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR) - 1;

	clusterCount = dataSectorsCount >> sectorsPerClusterPower;
	sectorsPerFat = ((uint)clusterCount + 2) * 3;

	if((sectorsPerFat & 0x3FF) == 0) {
		sectorsPerFat >>= 10;
	} else {
		sectorsPerFat >>= 10;
		sectorsPerFat++;
	}
	
	clusterCount = (dataSectorsCount - FAT_COPIES * sectorsPerFat) >> sectorsPerClusterPower;
	dataSectorsCount = (uint)clusterCount * (uint)sectorsPerCluster;

	if(clusterCount > maxClusterCount) {
		difference = clusterCount - maxClusterCount;
		clusterCount = maxClusterCount;
		sectorsPerFat = maxSectorsPerFat;
		dataSectorsCount -= difference * sectorsPerCluster;
	}

	parameters->totalSectors = dataSectorsCount + 1 + (sectorsPerFat * FAT_COPIES) + (FAT12_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	parameters->dataSectors = dataSectorsCount;
	parameters->clusterCount = clusterCount;
	parameters->sectorsPerFat = sectorsPerFat;
	parameters->sectorsPerCluster = sectorsPerCluster;
	parameters->sectorsPerRootDirectory = (FAT12_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	parameters->isFat16 = false;

	return 0;
}


int CalculateFatFileSystemParametersFat16(ulong fileSystemSizeInK, dosFilesystemParameters* parameters)
{
	byte sectorsPerCluster;
	uint sectorsPerFat;
	ulong clusterCount;
	ulong dataSectorsCount;
	uint sectorsPerClusterPower;
	ulong fileSystemSizeInM = fileSystemSizeInK >> 10;
	ulong difference;

	if(fileSystemSizeInM <= (ulong)128) {
		sectorsPerClusterPower = 2;
		sectorsPerCluster = 4;
	} else if(fileSystemSizeInM <= (ulong)256) {
		sectorsPerClusterPower = 3;
		sectorsPerCluster = 8;
	} else if(fileSystemSizeInM <= (ulong)512) {
		sectorsPerClusterPower = 4;
		sectorsPerCluster = 16;
	} else if(fileSystemSizeInM <= (ulong)1024) {
		sectorsPerClusterPower = 5;
		sectorsPerCluster = 32;
	} else if(fileSystemSizeInM <= (ulong)2048) {
		sectorsPerClusterPower = 6;
		sectorsPerCluster = 64;
	} else {
		sectorsPerClusterPower = 7;
		sectorsPerCluster = 128;
	}

	dataSectorsCount = (fileSystemSizeInK * 2) - (FAT16_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR) - 1;
	clusterCount = dataSectorsCount >> sectorsPerClusterPower;
	sectorsPerFat = (clusterCount + 2) >> 8;

	if(((clusterCount + 2) & 0x3FF) != 0) {
		sectorsPerFat++;
	}

	clusterCount = (dataSectorsCount - FAT_COPIES * sectorsPerFat);
	clusterCount >>= sectorsPerClusterPower;
    dataSectorsCount = clusterCount << sectorsPerClusterPower;

	if(clusterCount > MAX_FAT16_CLUSTER_COUNT) {
		difference = clusterCount - MAX_FAT16_CLUSTER_COUNT;
		clusterCount = MAX_FAT16_CLUSTER_COUNT;
		sectorsPerFat = 256;
		dataSectorsCount -= difference << sectorsPerClusterPower;
	}

	parameters->totalSectors = dataSectorsCount + 1 + (sectorsPerFat * FAT_COPIES) + (FAT16_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	parameters->dataSectors = dataSectorsCount;
	parameters->clusterCount = clusterCount;
	parameters->sectorsPerFat = sectorsPerFat;
	parameters->sectorsPerCluster = sectorsPerCluster;
	parameters->sectorsPerRootDirectory = (FAT16_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	parameters->isFat16 = true;

	return 0;
}


byte DeviceSectorRW(ulong firstDeviceSector, byte write)
{
	regs.Flags.C = write;
	regs.Bytes.A = selectedDevice->deviceNumber;
	regs.Bytes.B = 1;
	regs.Words.HL = (int)sectorBuffer;
	regs.Words.DE = (int)&firstDeviceSector;

	DriverCall(selectedDriver->slot, READ_WRITE);
	return regs.Bytes.A;
}


int CreatePartition(int index)
{
	byte error;
	masterBootRecord* mbr = (masterBootRecord*)sectorBuffer;
	partitionInfo* partition = &partitions[index];
	ulong mbrSector;
	uint paddingSectors;
	ulong firstFileSystemSector;
	ulong extendedPartitionFirstAbsoluteSector;
	partitionTableEntry* tableEntry;
	ulong x;

    mbrSector = nextDeviceSector;
	tableEntry = &(mbr->primaryPartitions[0]);
	ClearSectorBuffer();
	tableEntry->firstAbsoluteSector = 1;

    tableEntry->status = partition->status;
	tableEntry->partitionType = partition->partitionType;
	tableEntry->sectorCount = partition->sizeInK * 2;

	firstFileSystemSector = mbrSector + tableEntry->firstAbsoluteSector;

	nextDeviceSector += tableEntry->firstAbsoluteSector + tableEntry->sectorCount;

	if(index != (partitionsCount - 1)) {
		tableEntry++;
		tableEntry->partitionType = PARTYPE_EXTENDED;
		tableEntry->firstAbsoluteSector = nextDeviceSector;
		if(index == 0) {
			mainExtendedPartitionFirstSector = nextDeviceSector;
			tableEntry->sectorCount = mainExtendedPartitionSectorCount;
		} else {
			tableEntry->firstAbsoluteSector -= mainExtendedPartitionFirstSector;
			tableEntry->sectorCount = (((partitionInfo*)(partition + 1))->sizeInK * 2);
		}
	}

    if(index == 0) {
        mbr->jumpInstruction[0] = 0xEB;
    	mbr->jumpInstruction[1] = 0xFE;
    	mbr->jumpInstruction[2] = 0x90;
    	strcpy(mbr->oemNameString, "NEXTO30");
    }

	mbr->mbrSignature = 0xAA55;

	x = tableEntry->firstAbsoluteSector;	//Without this, firstAbsoluteSector is written to disk as 0. WTF???
	tableEntry->firstAbsoluteSector = x;

	memcpy(sectorBufferBackup, sectorBuffer, 512);

	if((error = WriteSectorToDevice(mbrSector)) != 0) {
		return error;
	}

	return CreateFatFileSystem(firstFileSystemSector, partition->sizeInK);
}


int ToggleStatusBit(byte partitionTableEntryIndex, ulong partitonTablesector)
{
    int error;
    masterBootRecord* mbr = (masterBootRecord*)sectorBuffer;
    partitionTableEntry* entry;

    error = ReadSectorFromDevice(partitonTablesector);
    if(error != 0)
        return error;

    entry =&mbr->primaryPartitions[partitionTableEntryIndex];

    entry->status ^= 0x80;

    return WriteSectorToDevice(partitonTablesector);
}


#include "../../tools/C/printf.c"
#include "../../tools/C/asmcall.c"
#include "drivercall.c"
