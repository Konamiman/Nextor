/* MKNEXROM - Make a Nextor kernel ROM
   By Konamiman, 3/2019

   Usage:
   mknexrom <basefile> [<newfile>] [/d:<driverfile>] [/m:<mapperfile>] [/e:<extrafile>] [/8:<8K bank select address>] [/k:<boot keys inverter>]

   This program creates a Nextor kernel ROM from a base file and a driver file,
   as per the recipe specified in the driver development guide. It also allows modifying
   an existing kernel ROM file, by changing the mapper code and/or adding extra content
   in the free 1K areas present in the DOS 1 and DOS 2 main kernel banks.


   <basefile> can be:
   - A kernel base file, that is, a file containing the code for the five kernel ROM banks and no driver; or
   - A complete ROM file consisting of the kernel bank with the driver bank(s) already appended.


   <driverfile> is the file containing the driver code. It must be a valid driver according
   to the driver development guide. The contents of this file is expected to be as follows:

   1. 256 dummy bytes.
   2. The driver signature (see the driver development guide)
   3. The driver jump table
   4. The driver code itself

   And optionally, if the driver is more than 16K long, for each additional 16K block:

   5. 256 dummy bytes.
   6. The additional driver code or data.
   7. Dummy space up to 16K.

   The last block does not need to be 16K long.

   Specifying a driver file is mandarory if a base file without driver is specified in <basefile>,
   and prohibited if a complete ROM file is specified.


   <mapperfile> is the file containing the bank switching code. The following is required for this code:

   - When called, switches ing page 1 the ROM bank passed in A.
   - Up to 48 bytes long.
   - Runs at address 0x7FD0.
   - May corrupt AF, must preserve all other registers.

   If no mapper file is specified, the mapper code from the base file itself is appended to the driver code.


   <extrafile> is the file containing the extra code or data for the resulting ROM file. This extra data
   will be placed at DOS 1 and DOS 2 kernel main banks at address 0x3BD0, this means that this code or data
   will be visible to applications via standard inter-slot calls (such as RDSLT or CALSLT) to the kernel
   slot, at address 0x7BD0.

   The maximum size for the extra file is 1K.


   <8K bank select address> must be specified if the ROM maps two 8K banks instead of one single 16K bank
   in Z80 page 1. The value must be the hexadecimal address where the bank number must be written
   in order to make it visible in the first half of the page (4000h-6000h). This will cause a small
   portion of the boot code to be overwritten with a direct write of a 0 byte to that address, where the
   original code is a call to 7FD0h (that will not contain any meaningful code at boot time in the case
   of mappers that use 8K banks).

   As an alternative to using the /8 parameter, the same can be achieved (a patched ROM file) if a
   mapper file is specified that has a special header, consisting of a FFh byte plus the bank select address
   in little endian. So for example the code for a ASCII8 mapper file with that header would be:

   db	0FFh
   dw	6000h
   rlca
   ld	(6000h),a
   inc	a
   ld	(6800h),a
   ret


   <boot keys inverter> is a hexadecimal number that contains the value of the boot key invert bytes,
   LSB is byte 0 and MSX is byte 1. For example 1002 to invert keys SHIFT and 1.
*/

/* v1.01 (4/2011):
   BASE_BANK_COUNT changed from 5 to 6

   v1.02 (4/2011):
   BASE_BANK_COUNT changed from 6 to 7

   v1.03 (4/2011):
   BASE_BANK_COUNT no longer used, the value is read from the file (at position 0xFE) instead.
   Driver signature searched changed to NEXTOR_DRIVER.
   
   v1.04 (8/2011):
   The address for the mapper code embedded in the initialization code is no longer
   calculated from the startup address. Instead, it is now at a fixed position
   (MAPPER_INIT_CODE_ADDRESS).

   v1.05 (4/2014):
   Added the <8K bank select address> parameter.

   v1.06 (3/2019):
   Added the <boot keys inverter> parameter.
   Existing full ROM files can now be updated, <newfile> is optional for these.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BANK_SIZE 16384					//Size of each ROM bank
//#define BASE_BANK_COUNT 7				//Number of kernel banks
#define BASE_BANK_COUNT_OFFSET 0xFE		//Offset in the base file of number of kernel banks
#define DRIVER_BANK baseBankCount		//Index of the first driver bank
#define MAPPER_CODE_SIZE 48				//Size of the bank change code
#define EXTRA_CODE_SIZE 1024			//Size of the extra code for banks 0 and 3
#define EXTRA_ADDRESS 0x3BD0			//Address of the extra code
#define DRIVER_MIN_SIZE 0x172			//Minimum size of the disk driver
#define PAGE0_SIZE 256					//Size of the common page 0 code
#define DOS2_EXTRA_BANK 0				//Bank for the extra code in DOS 2 mode
#define DOS1_EXTRA_BANK 3				//Bank for the extra code in DOS 2 mode
#define DATABUFFER_SIZE sizeof(dataBuffer)
#define MAPPER_INIT_CODE_ADDRESS 0x07DC
#define _8K_INIT_PATCH_ADDRESS 0x00F7	//File position where the patch for 8K bank mapper must be written
#define LD_XXXX_A_OPCODE 0x32
#define MAPPER_CODE_HEADER_SIZE 3
#define BOOT_KEYS_INVERTER_OFFSET 0x200

#define safeClose(file) {if(file!=NULL) {fclose(file); file=NULL;}}

void DisplayInfo();
int GetFileSize(FILE* file);
int IsParam(char* arg, char paramLetteR);
void DoExit(int code);

FILE* baseFile=NULL;
FILE* newFile=NULL;
FILE* driverFile=NULL;
FILE* mapperFile=NULL;
FILE* extraFile=NULL;

int baseBankCount;

int main(int argc, char* argv[])
{
	int hasDriver;
	int baseFileSize;
	int readCount;
	int writeCount;
	int signatureLength;
	int i;
	unsigned short position;
	int First8KMappingAddress=0;
    int hasBootKeys=0;
    int bootKeys=0;
    char byteBuffer;
    int modifyOriginalFile;
    int firstModifierArgumentIndex;

	char* baseFilename=NULL;
	char* newFilename=NULL;
	char* driverFilename=NULL;
	char* mapperFilename=NULL;
	char* extraFilename=NULL;
	char* _First8KMappingAddress=NULL;

	char* mapperCode;
	char mapperCodeBuffer[MAPPER_CODE_SIZE + MAPPER_CODE_HEADER_SIZE];
	char* extraCode[EXTRA_CODE_SIZE];
	char* dataBuffer[1024];

	char* driverSignature="NEXTOR_DRIVER";
	signatureLength=strlen(driverSignature);
	mapperCode = mapperCodeBuffer;
	
	//* Get command line parameters

	printf("\r\n");
	if(argc<3) {
		DisplayInfo();
		DoExit(0);
	}

	baseFilename=argv[1];

    if(argv[2][0]=='/') {
        modifyOriginalFile=1;
        firstModifierArgumentIndex=2;
    } else {
        modifyOriginalFile=0;
        firstModifierArgumentIndex=3;
        newFilename=argv[2];
    }

	for(i=firstModifierArgumentIndex; i<argc; i++) {
		if(IsParam(argv[i], 'd')) {
			driverFilename=argv[i]+3;
		} else if(IsParam(argv[i], 'm')) {
			mapperFilename=argv[i]+3;
		} else if(IsParam(argv[i], 'e')) {
			extraFilename=argv[i]+3;
		} else if (IsParam(argv[i], '8')) {
			_First8KMappingAddress=argv[i]+3;
		} else if (IsParam(argv[i], 'k')) {
            hasBootKeys = 1;
            sscanf(argv[i]+3, "%4x", &bootKeys);
            if(bootKeys == 0) {
                printf("--- WARNING: the value of the boot keys inverter is 0. If that's what you intended, fine; otherwise please check the value and try again.\r\n");
            }
        } else {
			DisplayInfo();
			DoExit(0);
		}
	}

	if (_First8KMappingAddress != NULL) {
		sscanf(_First8KMappingAddress, "%4x", &First8KMappingAddress);
		if (First8KMappingAddress < 0x4000 || First8KMappingAddress >= 0x8000) {
			printf("*** Invalid value for the /8 parameter, must be a hexadecimal address in the range 4000-7FFF");
			DoExit(1);
		}
	}

	//* Open the base file and check its size

	baseFile=fopen(baseFilename, "rb");
	if(baseFile==NULL) {
		printf("*** Can't open base file: %s\r\n", baseFilename);
		DoExit(1);
	}

	fseek(baseFile, BASE_BANK_COUNT_OFFSET, SEEK_SET);
	baseBankCount=0;
	fread(&baseBankCount, 1, 1, baseFile);
	fseek(baseFile, 0, SEEK_SET);

	baseFileSize=GetFileSize(baseFile);
	if(baseFileSize==baseBankCount * BANK_SIZE) {
		hasDriver=0;
	} else if(baseFileSize>=(baseBankCount+1) * BANK_SIZE) {
		hasDriver=1;
	} else {
		printf("*** The base file has not the expected length. Expected either %iK (for base file without driver) or >=%iK (for file with driver).\r\n",
			(baseBankCount * BANK_SIZE)/1024,
			((baseBankCount+1) * BANK_SIZE)/1024);
		DoExit(1);
	}

	//* Check for the presence or absence of driver file, this depends on the base file size

	if(driverFilename!=NULL && hasDriver) {
		printf("*** A driver file has been specified, but the base file appears to have a driver already.\r\n");
		DoExit(1);
	} else if(driverFilename==NULL && !hasDriver) {
		printf("*** No driver file has been specified, but the base file does not have driver.\r\n");
		DoExit(1);
	} if(!hasDriver && modifyOriginalFile) {
        printf("*** No new file to create specified, but the base file does not have driver.\r\n");
        DoExit(1);
    }


	//* Read the mapper code file, if specified;
	//  otherwise get the mapper code from the base file itself

	if(mapperFilename==NULL) {
		fseek(baseFile, BANK_SIZE-MAPPER_CODE_SIZE, SEEK_SET);
		readCount=fread(mapperCode, 1, MAPPER_CODE_SIZE, baseFile);
		if(readCount!=MAPPER_CODE_SIZE) {
			printf("*** Can't read the mapper code from the base file\r\n");
			DoExit(1);
		}
		fseek(baseFile, 0, SEEK_SET);
	} else {
		mapperFile=fopen(mapperFilename, "rb");
		if(mapperFile==NULL) {
			printf("*** Can't open mapper code file: %s\r\n", mapperFilename);
			DoExit(1);
		}
		int fileSize = GetFileSize(mapperFile);
		if (fileSize > (MAPPER_CODE_SIZE + MAPPER_CODE_HEADER_SIZE)) {
			printf("*** The mapper code file '%s', has not the expected size (%i bytes, or %i bytes if it has a header).  Size is: %d\r\n", mapperFilename, MAPPER_CODE_SIZE, MAPPER_CODE_SIZE + MAPPER_CODE_HEADER_SIZE, fileSize);
			DoExit(1);
		}
		readCount = fread(mapperCode, 1, MAPPER_CODE_SIZE + MAPPER_CODE_HEADER_SIZE, mapperFile);
		if(readCount==0) {
			printf("*** Can't read the mapper code file: %s\r\n", mapperFilename);
			DoExit(1);
		}

		if(mapperCode[0] == (char)0xFF) {
			if(First8KMappingAddress == 0) {
				First8KMappingAddress = (int)mapperCode[1] + (((int)mapperCode[2]) << 8);
			}
			mapperCode = &(mapperCodeBuffer[MAPPER_CODE_HEADER_SIZE]);
		}

		safeClose(mapperFile);
	}


	//* Read the extra code file, if specified

	if(extraFilename!=NULL) {
		extraFile=fopen(extraFilename, "rb");
		if(extraFile==NULL) {
			printf("*** Can't open extra code file: %s\r\n", mapperFilename);
			DoExit(1);
		}
		if(GetFileSize(extraFile)>EXTRA_CODE_SIZE) {
			printf("*** The extra code file is too big, maximum allowed size is %i bytes\r\n", EXTRA_CODE_SIZE);
			DoExit(1);
		}
		readCount=fread(extraCode, 1, EXTRA_CODE_SIZE, extraFile);
		if(readCount==0) {
			printf("*** Can't read the extra code file: %s", extraFilename);
			DoExit(1);
		}
		safeClose(extraFile);
	}


	//* Open the driver file, if necessary, and check its signature

	if(driverFilename!=NULL) {
		driverFile=fopen(driverFilename, "rb");
		if(driverFile==NULL) {
			printf("*** Can't open driver file: %s\r\n", driverFilename);
			DoExit(1);
		}
		if(GetFileSize(driverFile)<DRIVER_MIN_SIZE) {
			printf("*** The driver file is too small. '%s'", driverFilename);
			DoExit(1);
		}
		fseek(driverFile, PAGE0_SIZE, SEEK_SET);
		fread(dataBuffer, 1, signatureLength, driverFile);
		if(strncmp((char*)dataBuffer, driverSignature, signatureLength)!=0) {
			printf("*** The driver file is invalid. Driver signature not found at position %i.", PAGE0_SIZE);
			DoExit(1);
		}

		fseek(driverFile, 0, SEEK_SET);
	}


	//* Create the new ROM file, initially as a copy of the base file;
    //  or reopen the file to modify, this time with write access

    if(modifyOriginalFile) {
        safeClose(baseFile);
        newFile=fopen(baseFilename, "rb+");
    } else {
	    newFile=fopen(newFilename, "wb+");
    }
	if(newFile==NULL) {
		printf("*** Can't create the ROM file: %s\r\n", newFilename);
		DoExit(1);
	}

    if(!modifyOriginalFile) {
        while(!feof(baseFile)) {
            readCount=fread(dataBuffer, 1, DATABUFFER_SIZE, baseFile);
            writeCount=fwrite(dataBuffer, 1, readCount, newFile);
            if(writeCount!=readCount) {
                printf("*** Can't write to the ROM file: %s\r\n", newFilename);
                DoExit(1);
            }
        }
        safeClose(baseFile);
    }


	//* Copy the driver contents at the end of the new ROM file, if necessary

	if(driverFilename!=NULL) {
		while(!feof(driverFile)) {
			readCount=fread(dataBuffer, 1, DATABUFFER_SIZE, driverFile);
			writeCount=fwrite(dataBuffer, 1, readCount, newFile);
			if(writeCount!=readCount) {
				printf("*** Can't append the driver to the ROM file: %s\r\n", newFilename);
				DoExit(1);
			}
		}	
	}


	//* Copy page 0 at the start of each bank of the driver

	if(driverFilename!=NULL) {
		fseek(newFile, 0, SEEK_SET);
		readCount=fread(dataBuffer, 1, PAGE0_SIZE-1, newFile);
		if(readCount!=PAGE0_SIZE-1) {
			printf("*** Can't read page 0 data from the ROM file: %s\r\n", newFilename);
			DoExit(1);
		}

		fseek(newFile, BANK_SIZE*DRIVER_BANK, SEEK_SET);
		i=DRIVER_BANK;

		while(!feof(newFile)) {
			fseek(newFile, 0, SEEK_CUR);	//Necessary for fwrite after fgetc
			writeCount=fwrite(dataBuffer, 1, PAGE0_SIZE-1, newFile);
			if(writeCount!=PAGE0_SIZE-1) {
				printf("*** Can't write page 0 data to the ROM file: %s\r\n", newFilename);
				DoExit(1);
			}
			//Write the bank number too
			writeCount=fputc(i, newFile);
			if(writeCount==EOF) {
				printf("*** Can't write the bank number (%i) to the ROM file: %s\r\n", i, newFilename);
				DoExit(1);
			}
			i++;

			fseek(newFile, BANK_SIZE-PAGE0_SIZE-1, SEEK_CUR);
			fgetc(newFile);		//Necessary for feof (inside "while") to work
		}
	}


	//* Set the mapper code on all the necessary banks

	if(mapperFilename!=NULL || driverFilename!=NULL) {
		if(mapperFilename==NULL) {
			fseek(newFile, BANK_SIZE*DRIVER_BANK+1, SEEK_SET);	//The "+1" is for the "-1" of the fseek at the start of the loop
		} else {
			fseek(newFile, 1, SEEK_SET);						//Same as above for the "1"
		}

		do {
			fseek(newFile, BANK_SIZE-MAPPER_CODE_SIZE-1, SEEK_CUR);	//The "-1" is for the fgetc inside the "while"

			writeCount=fwrite(mapperCode, 1, MAPPER_CODE_SIZE, newFile);
			if(writeCount!=MAPPER_CODE_SIZE) {
				printf("*** Can't write mapper code to the ROM file: %s\r\n", newFilename);
				DoExit(1);
			}

			fseek(newFile, 0, SEEK_CUR);	//Necessary for fgetc after fwrite
			fgetc(newFile);					//Necessary for feof to work
		} while(!feof(newFile));
	}


	//* Write the patch for 8K bank based ROM mapper, if necessary

	if (First8KMappingAddress != 0) {
		char PatchFor8KMapper[3];
		PatchFor8KMapper[0] = LD_XXXX_A_OPCODE;
		PatchFor8KMapper[1] = (char)(First8KMappingAddress & 0xFF);
		PatchFor8KMapper[2] = (char)((First8KMappingAddress >> 8) & 0xFF);

		fseek(newFile, _8K_INIT_PATCH_ADDRESS, SEEK_SET);

		do {
			fseek(newFile, 0, SEEK_CUR);
			writeCount = fwrite(PatchFor8KMapper, 1, sizeof(PatchFor8KMapper), newFile);
			if (writeCount != sizeof(PatchFor8KMapper)) {
				printf("*** Can't write patch for 8K mapper initialization to the ROM file: %s\r\n", newFilename);
				DoExit(1);
			}

			fseek(newFile, BANK_SIZE - sizeof(PatchFor8KMapper) - 1, SEEK_CUR);
			fgetc(newFile);
		} while (!feof(newFile));
	}

	//* Set the mapper code on the intitialization block

	if(mapperFilename!=NULL) {
	    position = MAPPER_INIT_CODE_ADDRESS;
		fseek(newFile, position, SEEK_SET);

		writeCount=fwrite(mapperCode, 1, MAPPER_CODE_SIZE, newFile);
		if(writeCount!=MAPPER_CODE_SIZE) {
			printf("*** Can't write mapper code to the initialization block on the ROM file: %s\r\n", newFilename);
			DoExit(1);
		}
	}


	//* Append the extra data if necessary

	if(extraFilename!=NULL) {
		fseek(newFile, (DOS2_EXTRA_BANK*BANK_SIZE)+EXTRA_ADDRESS, SEEK_SET);
		writeCount=fwrite(extraCode, 1, EXTRA_CODE_SIZE, newFile);
		if(writeCount!=EXTRA_CODE_SIZE) {
			printf("*** Can't write extra code (DOS 2 segment) to the ROM file: %s\r\n", newFilename);
			DoExit(1);
		}

		fseek(newFile, (DOS1_EXTRA_BANK*BANK_SIZE)+EXTRA_ADDRESS, SEEK_SET);
		writeCount=fwrite(extraCode, 1, EXTRA_CODE_SIZE, newFile);
		if(writeCount!=EXTRA_CODE_SIZE) {
			printf("*** Can't write extra code (DOS 1 segment) to the ROM file: %s\r\n", newFilename);
			DoExit(1);
		}
	}

    //* Set the boot keys inverter if necessary

    if(hasBootKeys) {
        fseek(newFile, BOOT_KEYS_INVERTER_OFFSET, SEEK_SET);

        byteBuffer=(char)(bootKeys & 0xFF);
        writeCount=fwrite(&byteBuffer, 1, 1, newFile);
        byteBuffer=(char)((bootKeys & 0xFF00) >> 8);
        writeCount+=fwrite(&byteBuffer, 1, 1, newFile);

        if(writeCount!=2) {
			printf("*** Can't write boot keys inverter bytes to the ROM file: %s\r\n", newFilename);
			DoExit(1);
		}
    }

	//* Done

    if(modifyOriginalFile) {
        printf("ROM file %s updated successfully.", baseFilename);
    } else {
    	printf("ROM file %s created successfully.", newFilename);
    }

	DoExit(0);
}


//* Auxiliary methods

void DisplayInfo()
{
#if defined(__linux__)
	printf("MKNEXROM v1.07 - Make a Nextor kernel ROM\r\n"
		   "By Konamiman, 3/2019\r\n"
		   "\r\n"
		   "Usage:\r\n"
		   "mknexrom <basefile> <newfile> [-d:<driverfile>] [-m:<mapperfile>]\r\n"
		   "         [-e:<extrafile>] [-8:<8K bank select address>]\r\n"
		   );
#else
	printf("MKNEXROM v1.07 - Make a Nextor kernel ROM\r\n"
		   "By Konamiman, 3/2019\r\n"
		   "\r\n"
		   "Usage:\r\n"
		   "mknexrom <basefile> [<newfile>] [/d:<driverfile>] [/m:<mapperfile>]\r\n"
		   "         [/e:<extrafile>] [/8:<8K bank select address>] [/k:<boot keys inverter>]\r\n"
		   );
#endif
}

int GetFileSize(FILE* file)
{
	int pos;
	int end;

	pos=ftell(file);
	fseek(file, 0, SEEK_END);
	end=ftell(file);
	fseek(file,pos,SEEK_SET);

	return end;
}

#if defined(__linux__)
#define SWITCH_CHAR '-'
#else
#define SWITCH_CHAR '/'
#endif

int IsParam(char* arg, char paramLetter)
{
	return arg[0] == SWITCH_CHAR && ((arg[1] | 0x20) == (paramLetter | 0x20)) && arg[2]==':';
}

void DoExit(int code)
{
	safeClose(baseFile);
	safeClose(newFile);
	safeClose(driverFile);
	safeClose(mapperFile);
	safeClose(extraFile);
	printf("\r\n");
	exit(code);
}
