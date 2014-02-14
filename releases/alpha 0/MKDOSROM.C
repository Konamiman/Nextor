/* MKDOSROM - Make a MSX-DOS 2.50 kernel ROM
   By Konamiman, 7/2009

   Usage:
   mkdosrom <basefile> <newfile> [/d:<driverfile>] [/m:<mapperfile>] [/e:<extrafile>]

   This program creates a MSXDOS2 kernel ROM from a base file and a driver file,
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

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BANK_SIZE 16384					//Size of each ROM bank
#define BASE_BANK_COUNT 5				//Number of kernel banks
#define DRIVER_BANK (BASE_BANK_COUNT)	//Index of the first driver bank
#define MAPPER_CODE_SIZE 48				//Size of the bank change code
#define EXTRA_CODE_SIZE 1024			//Size of the extra code for banks 0 and 3
#define EXTRA_ADDRESS 0x3BD0			//Address of the extra code
#define DRIVER_MIN_SIZE 0x172			//Minimum size of the disk driver
#define PAGE0_SIZE 256					//Size of the common page 0 code
#define DOS2_EXTRA_BANK 0				//Bank for the extra code in DOS 2 mode
#define DOS1_EXTRA_BANK 3				//Bank for the extra code in DOS 2 mode
#define DATABUFFER_SIZE sizeof(dataBuffer)

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

int main(int argc, char* argv[])
{
	int hasDriver;
	int baseFileSize;
	int readCount;
	int writeCount;
	int signatureLength;
	int i;

	char* baseFilename=NULL;
	char* newFilename=NULL;
	char* driverFilename=NULL;
	char* mapperFilename=NULL;
	char* extraFilename=NULL;

	char* mapperCode[MAPPER_CODE_SIZE];
	char* extraCode[EXTRA_CODE_SIZE];
	char* dataBuffer[1024];

	char* driverSignature="MSXDOS_DRIVER";
	signatureLength=strlen(driverSignature);

	
	//* Get command line parameters

	printf("\r\n");
	if(argc<4) {
		DisplayInfo();
		DoExit(0);
	}

	baseFilename=argv[1];
	newFilename=argv[2];
	for(i=3; i<argc; i++) {
		if(IsParam(argv[i], 'd')) {
			driverFilename=argv[i]+3;
		} else if(IsParam(argv[i], 'm')) {
			mapperFilename=argv[i]+3;
		} else if(IsParam(argv[i], 'e')) {
			extraFilename=argv[i]+3;
		} else {
			DisplayInfo();
			DoExit(0);
		}
	}


	//* Open the base file and check its size

	baseFile=fopen(baseFilename, "rb");
	if(baseFile==NULL) {
		printf("*** Can't open base file: %s\r\n", baseFilename);
		DoExit(1);
	}

	baseFileSize=GetFileSize(baseFile);
	if(baseFileSize==BASE_BANK_COUNT * BANK_SIZE) {
		hasDriver=0;
	} else if(baseFileSize>=(BASE_BANK_COUNT+1) * BANK_SIZE) {
		hasDriver=1;
	} else {
		printf("*** The base file has not the expected length. Expected either %iK (for base file without driver) or >=%iK (for file with driver).\r\n",
			(BASE_BANK_COUNT * BANK_SIZE)/1024,
			((BASE_BANK_COUNT+1) * BANK_SIZE)/1024);
		DoExit(1);
	}


	//* Check for the presence or absence of driver file, this depends on the base file size

	if(driverFilename!=NULL && hasDriver) {
		printf("*** A driver file has been specified, but the base file appears to have a driver already.\r\n");
		DoExit(1);
	} else if(driverFilename==NULL && !hasDriver) {
		printf("*** No driver file has been specified, but the base file does not have driver.\r\n");
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
		if(GetFileSize(mapperFile)>MAPPER_CODE_SIZE) {
			printf("*** The mapper code file has not the expected size (%i bytes)\r\n", MAPPER_CODE_SIZE);
			DoExit(1);
		}
		readCount=fread(mapperCode, 1, MAPPER_CODE_SIZE, mapperFile);
		if(readCount==0) {
			printf("*** Can't read the mapper code file: %s\r\n", mapperFilename);
			DoExit(1);
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
			printf("*** The driver file is too small", driverFilename);
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


	//* Create the new ROM file, initially as a copy of the base file

	newFile=fopen(newFilename, "wb+");
	if(newFile==NULL) {
		printf("*** Can't create the ROM file: %s\r\n", newFilename);
		DoExit(1);
	}
	while(!feof(baseFile)) {
		readCount=fread(dataBuffer, 1, DATABUFFER_SIZE, baseFile);
		writeCount=fwrite(dataBuffer, 1, readCount, newFile);
		if(writeCount!=readCount) {
			printf("*** Can't write to the ROM file: %s\r\n", newFilename);
			DoExit(1);
		}
	}
	safeClose(baseFile);


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


	//* Set the mapper code on the intitialization block

	if(mapperFilename!=NULL) {
		fseek(newFile, 2, SEEK_SET);
		unsigned short position=0;
		fread(&position, 1, 2, newFile);
		position-=(0x4000-6);
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


	//* Done

	printf("ROM file %s created successfully.", newFilename);
	DoExit(0);
}


//* Auxiliary methods

void DisplayInfo()
{
	printf("MKDOSROM v1.0 - Make a MSX-DOS 2.50 kernel ROM\r\n"
		   "By Konamiman, 7/2009\r\n"
		   "\r\n"
		   "Usage:\r\n"
		   "mkdosrom <basefile> <newfile> [/d:<driverfile>] [/m:<mapperfile>] [/e:<extrafile>]\r\n"
		   );
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

int IsParam(char* arg, char paramLetter)
{
	return arg[0]=='/' && ((arg[1] | 0x20) == (paramLetter | 0x20)) && arg[2]==':';
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