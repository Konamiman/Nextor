;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.6.0 #9615 (MINGW64)
;--------------------------------------------------------
	.module emufile
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl __itoa
	.globl __uitoa
	.globl _main
	.globl _toupper
	.globl _strlen
	.globl _atoi
	.globl _strCRLF
	.globl _strInvParam
	.globl _strHelp
	.globl _strUsage
	.globl _strTitle
	.globl _resetComputer
	.globl _setupPartitionLunIndex
	.globl _setupPartitionDeviceIndex
	.globl _fileHandle
	.globl _fileNamesAppendAddress
	.globl _fileNamesBase
	.globl _fileContentsBase
	.globl _driveParameters
	.globl _driveInfo
	.globl _totalFilesProcessed
	.globl _fib
	.globl _workAreaAddress
	.globl _printFilenames
	.globl _bootFileIndex
	.globl _outputFileName
	.globl _mallocPointer
	.globl _OUT_FLAGS
	.globl _ASMRUT
	.globl _regs
	.globl _CheckPreconditions
	.globl _CheckPrimaryControllerIsNextor
	.globl _Initialize
	.globl _ProcessArguments
	.globl _ProcessCreateFileArguments
	.globl _ProcessSetupFileArguments
	.globl _ProcessOption
	.globl _AddFileExtension
	.globl _ProcessBootIndexOption
	.globl _ProcessWorkAreaAddressOption
	.globl _ProcessPrintFilenamesOption
	.globl _ProcessFilename
	.globl _TooManyFiles
	.globl _StartSearchingFiles
	.globl _ProcessFileFound
	.globl _GetDriveInfoForFileInFib
	.globl _CheckControllerForFileInFib
	.globl _GetFirstFileSectorForFileInFib
	.globl _AddFileInFibToFilesTable
	.globl _AddFileInFibToFilenamesInfo
	.globl _GenerateFile
	.globl _SetupFile
	.globl _VerifyDataFileSignature
	.globl _DeviceSectorRW
	.globl _DriverCall
	.globl _ResetComputer
	.globl _Terminate
	.globl _TerminateWithDosError
	.globl _print
	.globl _CheckDosVersion
	.globl _malloc
	.globl _ParseHex
	.globl _DoDosCall
	.globl _printf
	.globl _sprintf
	.globl _DosCall
	.globl _AsmCallAlt
	.globl _strcmpi
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_regs::
	.ds 12
_ASMRUT::
	.ds 4
_OUT_FLAGS::
	.ds 1
_mallocPointer::
	.ds 2
_outputFileName::
	.ds 2
_bootFileIndex::
	.ds 2
_printFilenames::
	.ds 1
_workAreaAddress::
	.ds 2
_fib::
	.ds 2
_totalFilesProcessed::
	.ds 2
_driveInfo::
	.ds 2
_driveParameters::
	.ds 2
_fileContentsBase::
	.ds 2
_fileNamesBase::
	.ds 2
_fileNamesAppendAddress::
	.ds 2
_fileHandle::
	.ds 1
_setupPartitionDeviceIndex::
	.ds 2
_setupPartitionLunIndex::
	.ds 2
_resetComputer::
	.ds 1
_format_string_buffer_1_213:
	.ds 16
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
_strTitle::
	.ds 2
_strUsage::
	.ds 2
_strHelp::
	.ds 2
_strInvParam::
	.ds 2
_strCRLF::
	.ds 2
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;emufile.c:184: int main(char** argv, int argc)
;	---------------------------------
; Function main
; ---------------------------------
_main::
	push	ix
	ld	ix,#0
	add	ix,sp
;emufile.c:188: ASMRUT[0] = 0xC3;
	ld	hl,#_ASMRUT
	ld	(hl),#0xc3
;emufile.c:189: print(strTitle);
	ld	hl,(_strTitle)
	push	hl
	call	_print
	pop	af
;emufile.c:191: CheckPreconditions();
	call	_CheckPreconditions
;emufile.c:192: Initialize();
	call	_Initialize
;emufile.c:193: isSetupFile = ProcessArguments(argv, argc);
	ld	l,6 (ix)
	ld	h,7 (ix)
	push	hl
	ld	l,4 (ix)
	ld	h,5 (ix)
	push	hl
	call	_ProcessArguments
	pop	af
	pop	af
;emufile.c:195: if(isSetupFile) {
	ld	a,l
	or	a, a
	jr	Z,00105$
;emufile.c:196: SetupFile();
	call	_SetupFile
;emufile.c:197: if(resetComputer) {
	ld	a,(#_resetComputer + 0)
	or	a, a
	jr	Z,00102$
;emufile.c:198: printf("Done. Resetting computer...");
	ld	hl,#___str_0
	push	hl
	call	_printf
	pop	af
;emufile.c:199: ResetComputer();
	call	_ResetComputer
	jr	00105$
00102$:
;emufile.c:204: "Remember: press 0 while booting to disable disk emulation mode.\r\n");
	ld	hl,#___str_1
	push	hl
	call	_printf
;emufile.c:205: Terminate(null);
	ld	hl, #0x0000
	ex	(sp),hl
	call	_Terminate
	pop	af
00105$:
;emufile.c:208: if(totalFilesProcessed > 0) {
	xor	a, a
	ld	iy,#_totalFilesProcessed
	cp	a, 0 (iy)
	sbc	a, 1 (iy)
	jp	PO, 00129$
	xor	a, #0x80
00129$:
	jp	P,00107$
;emufile.c:209: GenerateFile();
	call	_GenerateFile
;emufile.c:212: printFilenames ? "\r\n" : "", outputFileName, totalFilesProcessed);
	ld	a,(#_printFilenames + 0)
	or	a, a
	jr	Z,00111$
	ld	bc,#___str_3+0
	jr	00112$
00111$:
	ld	bc,#___str_4+0
00112$:
;emufile.c:211: "%s%s successfully generated!\r\n%i disk image file(s) registered\r\n",
	ld	hl,(_totalFilesProcessed)
	push	hl
	ld	hl,(_outputFileName)
	push	hl
	push	bc
	ld	hl,#___str_2
	push	hl
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
	jr	00108$
00107$:
;emufile.c:214: print(strUsage);
	ld	hl,(_strUsage)
	push	hl
	call	_print
	pop	af
00108$:
;emufile.c:217: Terminate(null);
	ld	hl,#0x0000
	push	hl
	call	_Terminate
	pop	af
;emufile.c:218: return 0;
	ld	hl,#0x0000
	pop	ix
	ret
___str_0:
	.ascii "Done. Resetting computer..."
	.db 0x00
___str_1:
	.ascii "Done. Reset the computer to start in disk emulation mode."
	.db 0x0d
	.db 0x0a
	.ascii "R"
	.ascii "emember: press 0 while booting to disable disk emulation mod"
	.ascii "e."
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_2:
	.ascii "%s%s successfully generated!"
	.db 0x0d
	.db 0x0a
	.ascii "%i disk image file(s) register"
	.ascii "ed"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_3:
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_4:
	.db 0x00
;emufile.c:223: void CheckPreconditions()
;	---------------------------------
; Function CheckPreconditions
; ---------------------------------
_CheckPreconditions::
;emufile.c:225: CheckDosVersion();
	call	_CheckDosVersion
;emufile.c:226: CheckPrimaryControllerIsNextor();
	jp  _CheckPrimaryControllerIsNextor
;emufile.c:229: void CheckPrimaryControllerIsNextor()
;	---------------------------------
; Function CheckPrimaryControllerIsNextor
; ---------------------------------
_CheckPrimaryControllerIsNextor::
;emufile.c:233: regs.Bytes.A = 0;
	ld	hl,#(_regs + 0x0001)
	ld	(hl),#0x00
;emufile.c:234: regs.Bytes.D = PrimaryControllerSlot();
	ld	bc,#_regs + 5
	ld	a,(#0xf348)
	ld	(bc),a
;emufile.c:235: regs.Bytes.E = 0xFF;
	ld	hl,#(_regs + 0x0004)
	ld	(hl),#0xff
;emufile.c:236: regs.Words.HL = (int)MallocBase;
	ld	hl,#0x8000
	ld	((_regs + 0x0006)), hl
;emufile.c:238: DoDosCall(_GDRVR);
	ld	a,#0x78
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;emufile.c:240: flags = ((byte*)MallocBase)[4];
	ld	a,(#0x8004)
;emufile.c:241: if((flags & (IS_NEXTOR | IS_DEVICE_BASED)) != (IS_NEXTOR | IS_DEVICE_BASED)) {
	and	a, #0x81
	sub	a, #0x81
	ret	Z
;emufile.c:242: Terminate("The primary controller is not a Nextor kernel with a device-based driver.");
	ld	hl,#___str_5
	push	hl
	call	_Terminate
	pop	af
	ret
___str_5:
	.ascii "The primary controller is not a Nextor kernel with a device-"
	.ascii "based driver."
	.db 0x00
;emufile.c:246: void Initialize()
;	---------------------------------
; Function Initialize
; ---------------------------------
_Initialize::
;emufile.c:248: mallocPointer = (void*)MallocBase;
	ld	hl,#0x8000
	ld	(_mallocPointer),hl
;emufile.c:250: outputFileName = malloc(128);
	ld	hl,#0x0080
	push	hl
	call	_malloc
	pop	af
	ld	(_outputFileName),hl
;emufile.c:251: *outputFileName = (char)0;
	ld	hl,(_outputFileName)
	ld	(hl),#0x00
;emufile.c:253: fib = malloc(sizeof(fileInfoBlock));
	ld	hl,#0x0040
	push	hl
	call	_malloc
	pop	af
	ld	(_fib),hl
;emufile.c:254: driveInfo = malloc(sizeof(driveLetterInfo));
	ld	hl,#0x0040
	push	hl
	call	_malloc
	pop	af
	ld	(_driveInfo),hl
;emufile.c:255: driveParameters = malloc(32);
	ld	hl,#0x0020
	push	hl
	call	_malloc
	pop	af
	ld	(_driveParameters),hl
;emufile.c:258: (sizeof(GeneratedFileTableEntry) * MaxFilesToProcess));
	ld	hl,#0x0118
	push	hl
	call	_malloc
	pop	af
	ld	(_fileContentsBase),hl
;emufile.c:259: fileNamesBase = malloc(19 * MaxFilesToProcess);
	ld	hl,#0x0260
	push	hl
	call	_malloc
	pop	af
	ld	(_fileNamesBase),hl
;emufile.c:260: fileNamesAppendAddress = fileNamesBase;
	ld	hl,(_fileNamesBase)
	ld	(_fileNamesAppendAddress),hl
;emufile.c:262: fileHandle = 0;
	ld	hl,#_fileHandle + 0
	ld	(hl), #0x00
;emufile.c:263: bootFileIndex = 1;
	ld	hl,#0x0001
	ld	(_bootFileIndex),hl
;emufile.c:264: printFilenames = false;
	ld	iy,#_printFilenames
	ld	0 (iy),#0x00
;emufile.c:265: workAreaAddress = 0;
;emufile.c:266: totalFilesProcessed = 0;
	ld	l,#0x00
	ld	(_workAreaAddress),hl
	ld	(_totalFilesProcessed),hl
	ret
;emufile.c:269: bool ProcessArguments(char** argv, int argc) 
;	---------------------------------
; Function ProcessArguments
; ---------------------------------
_ProcessArguments::
	push	ix
	ld	ix,#0
	add	ix,sp
;emufile.c:271: if(argc > 0 && argv[0][0] == '?') {
	ld	c,4 (ix)
	ld	b,5 (ix)
	xor	a, a
	cp	a, 6 (ix)
	sbc	a, 7 (ix)
	jp	PO, 00126$
	xor	a, #0x80
00126$:
	jp	P,00102$
	ld	l, c
	ld	h, b
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	a,(de)
	sub	a, #0x3f
	jr	NZ,00102$
;emufile.c:272: print(strHelp);
	push	bc
	ld	hl,(_strHelp)
	push	hl
	call	_print
	ld	hl, #0x0000
	ex	(sp),hl
	call	_Terminate
	pop	af
	pop	bc
00102$:
;emufile.c:276: if(argc < 2) {
	ld	a,6 (ix)
	sub	a, #0x02
	ld	a,7 (ix)
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	NC,00105$
;emufile.c:277: print(strUsage);
	push	bc
	ld	hl,(_strUsage)
	push	hl
	call	_print
	ld	hl, #0x0000
	ex	(sp),hl
	call	_Terminate
	pop	af
	pop	bc
00105$:
;emufile.c:281: if(strcmpi(argv[0], "set") == 0) {
	ld	l, c
	ld	h, b
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	push	bc
	ld	hl,#___str_6
	push	hl
	push	de
	call	_strcmpi
	pop	af
	pop	af
	pop	bc
	ld	a,h
	or	a,l
	jr	NZ,00107$
;emufile.c:282: ProcessSetupFileArguments(argv, argc);
	ld	l,6 (ix)
	ld	h,7 (ix)
	push	hl
	push	bc
	call	_ProcessSetupFileArguments
	pop	af
	pop	af
;emufile.c:283: return true;
	ld	l,#0x01
	jr	00108$
00107$:
;emufile.c:286: ProcessCreateFileArguments(argv, argc);
	ld	l,6 (ix)
	ld	h,7 (ix)
	push	hl
	push	bc
	call	_ProcessCreateFileArguments
	pop	af
	pop	af
;emufile.c:287: return false;
	ld	l,#0x00
00108$:
	pop	ix
	ret
___str_6:
	.ascii "set"
	.db 0x00
;emufile.c:290: void ProcessCreateFileArguments(char** argv, int argc)
;	---------------------------------
; Function ProcessCreateFileArguments
; ---------------------------------
_ProcessCreateFileArguments::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-6
	add	hl,sp
	ld	sp,hl
;emufile.c:296: processingOptions = true;
	ld	c,#0x01
;emufile.c:298: for(i=0; i<argc; i++) {
	ld	hl,#0x0000
	ex	(sp), hl
00119$:
;emufile.c:306: } else if(*outputFileName == null) {
	ld	de,(_outputFileName)
;emufile.c:298: for(i=0; i<argc; i++) {
	ld	a,-6 (ix)
	sub	a, 6 (ix)
	ld	a,-5 (ix)
	sbc	a, 7 (ix)
	jp	PO, 00155$
	xor	a, #0x80
00155$:
	jp	P,00113$
;emufile.c:299: currentArg = argv[i];
	ld	a,-6 (ix)
	ld	-2 (ix),a
	ld	a,-5 (ix)
	ld	-1 (ix),a
	sla	-2 (ix)
	rl	-1 (ix)
	ld	a,-2 (ix)
	add	a, 4 (ix)
	ld	l,a
	ld	a,-1 (ix)
	adc	a, 5 (ix)
	ld	h,a
	ld	a,(hl)
	ld	-4 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-3 (ix),a
;emufile.c:300: if(currentArg[0] == '-') {
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	ld	a,(hl)
;emufile.c:301: if(processingOptions) {
	sub	a,#0x2d
	jr	NZ,00111$
	or	a,c
	jr	Z,00102$
;emufile.c:302: i += ProcessOption(currentArg[1], argv[i+1]);
	pop	hl
	push	hl
	inc	hl
	add	hl, hl
	ex	de,hl
	ld	l,4 (ix)
	ld	h,5 (ix)
	add	hl,de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	inc	hl
	ld	b,(hl)
	push	bc
	push	de
	push	bc
	inc	sp
	call	_ProcessOption
	pop	af
	inc	sp
	pop	bc
	ld	a,-6 (ix)
	add	a, l
	ld	-6 (ix),a
	ld	a,-5 (ix)
	adc	a, h
	ld	-5 (ix),a
	jr	00120$
00102$:
;emufile.c:304: Terminate("Can't process more options after filenames");
	push	bc
	ld	hl,#___str_7
	push	hl
	call	_Terminate
	pop	af
	pop	bc
	jr	00120$
00111$:
;emufile.c:306: } else if(*outputFileName == null) {
	ld	a,(de)
;emufile.c:307: processingOptions = false;
	or	a,a
	jr	NZ,00108$
	ld	c,a
;emufile.c:308: AddFileExtension(currentArg);
	push	bc
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	call	_AddFileExtension
	pop	af
	pop	bc
	jr	00120$
00108$:
;emufile.c:309: } else if(totalFilesProcessed < MaxFilesToProcess) {
	ld	iy,#_totalFilesProcessed
	ld	a,0 (iy)
	sub	a, #0x20
	ld	a,1 (iy)
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	NC,00105$
;emufile.c:310: ProcessFilename(currentArg);
	push	bc
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	call	_ProcessFilename
	pop	af
	pop	bc
	jr	00120$
00105$:
;emufile.c:312: TooManyFiles();
	push	bc
	call	_TooManyFiles
	pop	bc
00120$:
;emufile.c:298: for(i=0; i<argc; i++) {
	inc	-6 (ix)
	jp	NZ,00119$
	inc	-5 (ix)
	jp	00119$
00113$:
;emufile.c:316: if(*outputFileName == null) {
	ld	a,(de)
	or	a, a
	jr	NZ,00115$
;emufile.c:317: Terminate("No output file name specified");
	ld	hl,#___str_8
	push	hl
	call	_Terminate
	pop	af
00115$:
;emufile.c:320: if(totalFilesProcessed == 0) {
	ld	iy,#_totalFilesProcessed
	ld	a,1 (iy)
	or	a,0 (iy)
	jr	NZ,00121$
;emufile.c:321: Terminate("No disk image files to emulate specified");
	ld	hl,#___str_9
	push	hl
	call	_Terminate
	pop	af
00121$:
	ld	sp, ix
	pop	ix
	ret
___str_7:
	.ascii "Can't process more options after filenames"
	.db 0x00
___str_8:
	.ascii "No output file name specified"
	.db 0x00
___str_9:
	.ascii "No disk image files to emulate specified"
	.db 0x00
;emufile.c:325: void ProcessSetupFileArguments(char** argv, int argc)
;	---------------------------------
; Function ProcessSetupFileArguments
; ---------------------------------
_ProcessSetupFileArguments::
	push	ix
	ld	ix,#0
	add	ix,sp
;emufile.c:329: resetComputer = false;
	ld	iy,#_resetComputer
	ld	0 (iy),#0x00
;emufile.c:330: setupPartitionDeviceIndex = 0;
	ld	hl,#0x0000
	ld	(_setupPartitionDeviceIndex),hl
;emufile.c:331: setupPartitionLunIndex = 0;
	ld	l, #0x00
	ld	(_setupPartitionLunIndex),hl
;emufile.c:333: strcpy(outputFileName, argv[1]);
	ld	l,4 (ix)
	ld	h,5 (ix)
	inc	hl
	inc	hl
	ld	c,(hl)
	inc	hl
	ld	h,(hl)
	ld	de,(_outputFileName)
	ld	l, c
	xor	a, a
00140$:
	cp	a, (hl)
	ldi
	jr	NZ, 00140$
;emufile.c:335: if(argc == 2) {
	ld	a,6 (ix)
	sub	a, #0x02
	jr	NZ,00102$
	ld	a,7 (ix)
	or	a, a
	jp	Z,00114$
	jr	00102$
;emufile.c:336: return;
	jp	00114$
00102$:
;emufile.c:339: if((argv[2][0] | 32) == 'r') {
	ld	l,4 (ix)
	ld	h,5 (ix)
	ld	de, #0x0004
	add	hl, de
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	ld	a,(bc)
	set	5, a
	sub	a, #0x72
	jr	NZ,00104$
;emufile.c:340: resetComputer = true;
	ld	hl,#_resetComputer + 0
	ld	(hl), #0x01
;emufile.c:341: deviceArgIndex = 3;
	ld	bc,#0x0003
	jr	00105$
00104$:
;emufile.c:344: deviceArgIndex = 2;
	ld	bc,#0x0002
00105$:
;emufile.c:347: if(argc <= deviceArgIndex) {
	ld	a,c
	sub	a, 6 (ix)
	ld	a,b
	sbc	a, 7 (ix)
	jp	PO, 00145$
	xor	a, #0x80
00145$:
	jp	M,00107$
;emufile.c:348: return;
	jp	00114$
00107$:
;emufile.c:351: setupPartitionDeviceIndex = atoi(argv[deviceArgIndex]);
	ld	e, c
	ld	d, b
	sla	e
	rl	d
	ld	l,4 (ix)
	ld	h,5 (ix)
	add	hl,de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	push	bc
	push	de
	call	_atoi
	pop	af
	pop	bc
	ld	(_setupPartitionDeviceIndex),hl
;emufile.c:352: if(setupPartitionDeviceIndex < 1) {
	ld	iy,#_setupPartitionDeviceIndex
	ld	a,0 (iy)
	sub	a, #0x01
	ld	a,1 (iy)
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	NC,00109$
;emufile.c:353: Terminate("Invalid device index");
	push	bc
	ld	hl,#___str_10
	push	hl
	call	_Terminate
	pop	af
	pop	bc
00109$:
;emufile.c:356: if(argc <= deviceArgIndex + 1) {
	inc	bc
	ld	a,c
	sub	a, 6 (ix)
	ld	a,b
	sbc	a, 7 (ix)
	jp	PO, 00148$
	xor	a, #0x80
00148$:
	jp	M,00111$
;emufile.c:357: setupPartitionLunIndex = 1;
	ld	hl,#0x0001
	ld	(_setupPartitionLunIndex),hl
;emufile.c:358: return;
	jr	00114$
00111$:
;emufile.c:361: setupPartitionLunIndex = atoi(argv[deviceArgIndex + 1]);
	sla	c
	rl	b
	ld	l,4 (ix)
	ld	h,5 (ix)
	add	hl,bc
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	push	bc
	call	_atoi
	pop	af
	ld	(_setupPartitionLunIndex),hl
;emufile.c:362: if(setupPartitionLunIndex < 1) {
	ld	iy,#_setupPartitionLunIndex
	ld	a,0 (iy)
	sub	a, #0x01
	ld	a,1 (iy)
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	NC,00114$
;emufile.c:363: Terminate("Invalid LUN index");
	ld	hl,#___str_11
	push	hl
	call	_Terminate
	pop	af
00114$:
	pop	ix
	ret
___str_10:
	.ascii "Invalid device index"
	.db 0x00
___str_11:
	.ascii "Invalid LUN index"
	.db 0x00
;emufile.c:367: int ProcessOption(char optionLetter, char* optionValue)
;	---------------------------------
; Function ProcessOption
; ---------------------------------
_ProcessOption::
	push	ix
	ld	ix,#0
	add	ix,sp
;emufile.c:369: optionLetter |= 32;
;emufile.c:371: if(optionLetter == 'b') {
	set	5, 4 (ix)
	ld	a, 4 (ix)
	sub	a, #0x62
	jr	NZ,00102$
;emufile.c:372: ProcessBootIndexOption(optionValue);
	ld	l,5 (ix)
	ld	h,6 (ix)
	push	hl
	call	_ProcessBootIndexOption
	pop	af
;emufile.c:373: return 1;
	ld	hl,#0x0001
	jr	00107$
00102$:
;emufile.c:376: if(optionLetter == 'a') {
	ld	a,4 (ix)
	sub	a, #0x61
	jr	NZ,00104$
;emufile.c:377: ProcessWorkAreaAddressOption(optionValue);
	ld	l,5 (ix)
	ld	h,6 (ix)
	push	hl
	call	_ProcessWorkAreaAddressOption
	pop	af
;emufile.c:378: return 1;
	ld	hl,#0x0001
	jr	00107$
00104$:
;emufile.c:381: if(optionLetter == 'p') {
	ld	a,4 (ix)
	sub	a, #0x70
	jr	NZ,00106$
;emufile.c:382: ProcessPrintFilenamesOption();
	call	_ProcessPrintFilenamesOption
;emufile.c:383: return 0;
	ld	hl,#0x0000
	jr	00107$
00106$:
;emufile.c:386: InvalidParameter();
	ld	hl,(_strInvParam)
	push	hl
	call	_Terminate
	pop	af
;emufile.c:387: return 0;
	ld	hl,#0x0000
00107$:
	pop	ix
	ret
;emufile.c:390: void AddFileExtension(char* fileName)
;	---------------------------------
; Function AddFileExtension
; ---------------------------------
_AddFileExtension::
	push	ix
	ld	ix,#0
	add	ix,sp
;emufile.c:392: strcpy(outputFileName, fileName);
	ld	de,(_outputFileName)
	ld	l,4 (ix)
	ld	h,5 (ix)
	xor	a, a
00109$:
	cp	a, (hl)
	ldi
	jr	NZ, 00109$
;emufile.c:394: regs.Bytes.B = 0;
	ld	hl,#(_regs + 0x0003)
	ld	(hl),#0x00
;emufile.c:395: regs.Words.DE = (int)outputFileName;
	ld	bc,(_outputFileName)
	ld	((_regs + 0x0004)), bc
;emufile.c:396: DoDosCall(_PARSE);
	ld	a,#0x5b
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;emufile.c:398: if(!(regs.Bytes.B & PARSE_FLAG_HAS_EXTENSION)) {
	ld	a, (#(_regs + 0x0003) + 0)
	bit	4, a
	jr	NZ,00103$
;emufile.c:399: strcpy((char*)regs.Words.DE, ".EMU");
	ld	de, (#(_regs + 0x0004) + 0)
	ld	hl,#___str_12
	xor	a, a
00112$:
	cp	a, (hl)
	ldi
	jr	NZ, 00112$
00103$:
	pop	ix
	ret
___str_12:
	.ascii ".EMU"
	.db 0x00
;emufile.c:403: void ProcessBootIndexOption(char* optionValue)
;	---------------------------------
; Function ProcessBootIndexOption
; ---------------------------------
_ProcessBootIndexOption::
;emufile.c:407: if(optionValue[1] != 0) {
	pop	bc
	pop	hl
	push	hl
	push	bc
	inc	hl
	ld	a,(hl)
	or	a, a
	jr	Z,00102$
;emufile.c:408: InvalidParameter();
	ld	hl,(_strInvParam)
	push	hl
	call	_Terminate
	pop	af
00102$:
;emufile.c:411: index = *optionValue | 32;
	pop	de
	pop	bc
	push	bc
	push	de
	ld	a,(bc)
	set	5, a
	ld	c,a
;emufile.c:414: bootFileIndex = index - '0';
	ld	e,c
	ld	d,#0x00
;emufile.c:413: if(index >= '1' && index <= '9') {
	ld	a,c
	sub	a, #0x31
	jr	C,00108$
	ld	a,#0x39
	sub	a, c
	jr	C,00108$
;emufile.c:414: bootFileIndex = index - '0';
	ld	hl,#_bootFileIndex
	ld	a,e
	add	a,#0xd0
	ld	(hl),a
	ld	a,d
	adc	a,#0xff
	inc	hl
	ld	(hl),a
	ret
00108$:
;emufile.c:415: } else if(index >= 'a' && index <= 'w') {
	ld	a,c
	sub	a, #0x61
	jr	C,00104$
	ld	a,#0x77
	sub	a, c
	jr	C,00104$
;emufile.c:416: bootFileIndex = index - 'a' + 10;
	ld	hl,#0xffa9
	add	hl,de
	ld	(_bootFileIndex),hl
	ret
00104$:
;emufile.c:418: InvalidParameter();
	ld	hl,(_strInvParam)
	push	hl
	call	_Terminate
	pop	af
	ret
;emufile.c:422: void ProcessWorkAreaAddressOption(char* optionValue)
;	---------------------------------
; Function ProcessWorkAreaAddressOption
; ---------------------------------
_ProcessWorkAreaAddressOption::
;emufile.c:424: workAreaAddress = ParseHex(optionValue);
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	call	_ParseHex
	pop	af
	ld	(_workAreaAddress),hl
;emufile.c:426: if(workAreaAddress != 0 && (workAreaAddress < 0xC000 || workAreaAddress > 0xFFEF)) {
	ld	iy,#_workAreaAddress
	ld	a,1 (iy)
	or	a,0 (iy)
	ret	Z
	ld	a,1 (iy)
	sub	a, #0xc0
	jr	C,00101$
	ld	a,#0xef
	cp	a, 0 (iy)
	ld	a,#0xff
	sbc	a, 1 (iy)
	ret	NC
00101$:
;emufile.c:427: InvalidParameter();
	ld	hl,(_strInvParam)
	push	hl
	call	_Terminate
	pop	af
	ret
;emufile.c:431: void ProcessPrintFilenamesOption()
;	---------------------------------
; Function ProcessPrintFilenamesOption
; ---------------------------------
_ProcessPrintFilenamesOption::
;emufile.c:433: printFilenames = true;
	ld	hl,#_printFilenames + 0
	ld	(hl), #0x01
	ret
;emufile.c:436: void ProcessFilename(char* fileName) 
;	---------------------------------
; Function ProcessFilename
; ---------------------------------
_ProcessFilename::
;emufile.c:438: StartSearchingFiles(fileName);
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	call	_StartSearchingFiles
	pop	af
;emufile.c:440: while(regs.Bytes.A == 0 && totalFilesProcessed < MaxFilesToProcess) {
00102$:
	ld	a, (#(_regs + 0x0001) + 0)
	or	a, a
	jr	NZ,00104$
	ld	iy,#_totalFilesProcessed
	ld	a,0 (iy)
	sub	a, #0x20
	ld	a,1 (iy)
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	NC,00104$
;emufile.c:441: ProcessFileFound();
	call	_ProcessFileFound
;emufile.c:442: DoDosCall(_FNEXT);
	ld	a,#0x41
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
	jr	00102$
00104$:
;emufile.c:445: if(regs.Bytes.A == 0 && totalFilesProcessed == MaxFilesToProcess) {
	ld	a, (#(_regs + 0x0001) + 0)
	or	a, a
	ret	NZ
	ld	iy,#_totalFilesProcessed
	ld	a,0 (iy)
	sub	a, #0x20
	ret	NZ
	ld	a,1 (iy)
	or	a, a
	ret	NZ
;emufile.c:446: TooManyFiles();
	jp  _TooManyFiles
;emufile.c:450: void TooManyFiles()
;	---------------------------------
; Function TooManyFiles
; ---------------------------------
_TooManyFiles::
;emufile.c:452: printf("*** Too many files specified, maximum is %i\r\n", MaxFilesToProcess);
	ld	hl,#0x0020
	push	hl
	ld	hl,#___str_13
	push	hl
	call	_printf
	pop	af
;emufile.c:453: Terminate(null);
	ld	hl, #0x0000
	ex	(sp),hl
	call	_Terminate
	pop	af
	ret
___str_13:
	.ascii "*** Too many files specified, maximum is %i"
	.db 0x0d
	.db 0x0a
	.db 0x00
;emufile.c:456: void StartSearchingFiles(char* fileName)
;	---------------------------------
; Function StartSearchingFiles
; ---------------------------------
_StartSearchingFiles::
;emufile.c:458: regs.Words.DE = (int)fileName;
	pop	de
	pop	bc
	push	bc
	push	de
	ld	((_regs + 0x0004)), bc
;emufile.c:459: regs.Bytes.B = 0;
	ld	hl,#(_regs + 0x0003)
	ld	(hl),#0x00
;emufile.c:460: regs.Words.IX = (int)fib;
	ld	bc,(_fib)
	ld	((_regs + 0x0008)), bc
;emufile.c:462: DoDosCall(_FFIRST);
	ld	a,#0x40
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
	ret
;emufile.c:465: void ProcessFileFound()
;	---------------------------------
; Function ProcessFileFound
; ---------------------------------
_ProcessFileFound::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-7
	add	hl,sp
	ld	sp,hl
;emufile.c:470: GetDriveInfoForFileInFib();
	call	_GetDriveInfoForFileInFib
;emufile.c:471: CheckControllerForFileInFib();
	call	_CheckControllerForFileInFib
;emufile.c:473: if(fib->fileSize < 512) {
	ld	bc,(_fib)
	push	bc
	pop	iy
	ld	a,21 (iy)
	ld	-4 (ix),a
	ld	a,22 (iy)
	ld	-3 (ix),a
	ld	a,23 (iy)
	ld	-2 (ix),a
	ld	a,24 (iy)
	ld	-1 (ix),a
;emufile.c:474: printf("*** %s is too small (< 512 bytes) or empty - skipped\r\n", fib->filename);
	inc	bc
	ld	-6 (ix),c
	ld	-5 (ix),b
;emufile.c:473: if(fib->fileSize < 512) {
	ld	a,-3 (ix)
	sub	a, #0x02
	ld	a,-2 (ix)
	sbc	a, #0x00
	ld	a,-1 (ix)
	sbc	a, #0x00
	jr	NC,00102$
;emufile.c:474: printf("*** %s is too small (< 512 bytes) or empty - skipped\r\n", fib->filename);
	ld	l,-6 (ix)
	ld	h,-5 (ix)
	push	hl
	ld	hl,#___str_14
	push	hl
	call	_printf
	pop	af
	pop	af
;emufile.c:475: return;
	jp	00107$
00102$:
;emufile.c:478: if(fib->fileSize >= (32768 * 1024)) {
	ld	a,-1 (ix)
	sub	a, #0x02
	jr	C,00104$
;emufile.c:479: printf("*** %s is too big (> 32 MBytes) - skipped\r\n", fib->filename);
	ld	l,-6 (ix)
	ld	h,-5 (ix)
	push	hl
	ld	hl,#___str_15
	push	hl
	call	_printf
	pop	af
	pop	af
;emufile.c:480: return;
	jp	00107$
00104$:
;emufile.c:483: sector = driveInfo->firstSectorNumber + GetFirstFileSectorForFileInFib();
	ld	iy,(_driveInfo)
	ld	a,6 (iy)
	ld	-4 (ix),a
	ld	a,7 (iy)
	ld	-3 (ix),a
	ld	a,8 (iy)
	ld	-2 (ix),a
	ld	a,9 (iy)
	ld	-1 (ix),a
	call	_GetFirstFileSectorForFileInFib
	ld	a,-4 (ix)
	add	a, l
	ld	c,a
	ld	a,-3 (ix)
	adc	a, h
	ld	b,a
	ld	a,-2 (ix)
	adc	a, e
	ld	e,a
	ld	a,-1 (ix)
	adc	a, d
	ld	d,a
;emufile.c:484: AddFileInFibToFilesTable(sector);
	push	de
	push	bc
	call	_AddFileInFibToFilesTable
	pop	af
	pop	af
;emufile.c:485: AddFileInFibToFilenamesInfo();
	call	_AddFileInFibToFilenamesInfo
;emufile.c:487: totalFilesProcessed++;
	ld	iy,#_totalFilesProcessed
	inc	0 (iy)
	jr	NZ,00127$
	inc	1 (iy)
00127$:
;emufile.c:489: if(printFilenames) {
	ld	a,(#_printFilenames + 0)
	or	a, a
	jr	Z,00107$
;emufile.c:490: key = totalFilesProcessed < 10 ? totalFilesProcessed + '0' : totalFilesProcessed - 10 + 'A';
	ld	iy,#_totalFilesProcessed
	ld	a,0 (iy)
	ld	-6 (ix),a
	ld	a,0 (iy)
	sub	a, #0x0a
	ld	a,1 (iy)
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	NC,00109$
	ld	a,-6 (ix)
	add	a, #0x30
	ld	-7 (ix),a
	jr	00110$
00109$:
	ld	a,-6 (ix)
	add	a, #0x37
	ld	-7 (ix),a
00110$:
	ld	e,-7 (ix)
;emufile.c:491: printf("%c -> %s\r\n", key, fib->filename);
	ld	hl,(_fib)
	inc	hl
	ld	d,#0x00
	ld	bc,#___str_16+0
	push	hl
	push	de
	push	bc
	call	_printf
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
00107$:
	ld	sp, ix
	pop	ix
	ret
___str_14:
	.ascii "*** %s is too small (< 512 bytes) or empty - skipped"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_15:
	.ascii "*** %s is too big (> 32 MBytes) - skipped"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_16:
	.ascii "%c -> %s"
	.db 0x0d
	.db 0x0a
	.db 0x00
;emufile.c:495: void GetDriveInfoForFileInFib()
;	---------------------------------
; Function GetDriveInfoForFileInFib
; ---------------------------------
_GetDriveInfoForFileInFib::
;emufile.c:497: regs.Bytes.A = fib->logicalDrive - 1;
	ld	iy,(_fib)
	ld	c,25 (iy)
	dec	c
	ld	hl,#(_regs + 0x0001)
	ld	(hl),c
;emufile.c:498: regs.Words.HL = (int)driveInfo;
	ld	bc,(_driveInfo)
	ld	((_regs + 0x0006)), bc
;emufile.c:499: DoDosCall(_GDLI);
	ld	a,#0x79
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
	ret
;emufile.c:502: void CheckControllerForFileInFib()
;	---------------------------------
; Function CheckControllerForFileInFib
; ---------------------------------
_CheckControllerForFileInFib::
;emufile.c:504: if(driveInfo->driveStatus != DRIVE_STATUS_ASSIGNED_TO_DEVICE || driveInfo->driverSlotNumber != PrimaryControllerSlot()) {
	ld	hl,(_driveInfo)
	ld	c,(hl)
	dec	c
	jr	NZ,00101$
	inc	hl
	ld	c,(hl)
	ld	hl,#0xf348
	ld	b,(hl)
	ld	a,c
	sub	a, b
	ret	Z
00101$:
;emufile.c:505: printf("*** Drive %c: is not controlled by the primary Nextor kernel\r\n", fib->logicalDrive - 1 + 'A');
	ld	hl,(_fib)
	ld	de, #0x0019
	add	hl, de
	ld	c,(hl)
	ld	b,#0x00
	ld	hl,#0x0040
	add	hl,bc
	ld	bc,#___str_17+0
	push	hl
	push	bc
	call	_printf
	pop	af
;emufile.c:506: Terminate(null);
	ld	hl, #0x0000
	ex	(sp),hl
	call	_Terminate
	pop	af
	ret
___str_17:
	.ascii "*** Drive %c: is not controlled by the primary Nextor kernel"
	.db 0x0d
	.db 0x0a
	.db 0x00
;emufile.c:510: ulong GetFirstFileSectorForFileInFib()
;	---------------------------------
; Function GetFirstFileSectorForFileInFib
; ---------------------------------
_GetFirstFileSectorForFileInFib::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-8
	add	hl,sp
	ld	sp,hl
;emufile.c:515: regs.Words.DE = (int)driveParameters;
	ld	bc,(_driveParameters)
	ld	((_regs + 0x0004)), bc
;emufile.c:516: regs.Bytes.L = fib->logicalDrive;
	ld	bc,#_regs+6
	ld	iy,(_fib)
	ld	a,25 (iy)
	ld	(bc),a
;emufile.c:517: DoDosCall(_DPARM);
	ld	a,#0x31
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;emufile.c:518: firstDataSector = *(uint*)(driveParameters+15);
	ld	iy,#0x000f
	ld	de,(_driveParameters)
	add	iy, de
	push	iy
	pop	hl
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	ld	-8 (ix),c
	ld	-7 (ix),b
	ld	-6 (ix),#0x00
	ld	-5 (ix),#0x00
;emufile.c:519: sectorsPerCluster = *(byte*)(driveParameters+3);
	ld	hl,(_driveParameters)
	inc	hl
	inc	hl
	inc	hl
	ld	c,(hl)
;emufile.c:523: ((ulong)(fib->startCluster - 2) * (ulong)sectorsPerCluster);
	ld	hl,(_fib)
	ld	de, #0x0013
	add	hl, de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	dec	de
	dec	de
	ld	-4 (ix),e
	ld	-3 (ix),d
	ld	-2 (ix),#0x00
	ld	-1 (ix),#0x00
	ld	b,#0x00
	ld	de,#0x0000
	push	de
	push	bc
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	push	hl
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	call	__mullong
	pop	af
	pop	af
	pop	af
	pop	af
	ld	a,-8 (ix)
	add	a, l
	ld	l,a
	ld	a,-7 (ix)
	adc	a, h
	ld	h,a
	ld	a,-6 (ix)
	adc	a, e
	ld	e,a
	ld	a,-5 (ix)
	adc	a, d
	ld	d,a
	ld	sp, ix
	pop	ix
	ret
;emufile.c:526: void AddFileInFibToFilesTable(ulong sector)
;	---------------------------------
; Function AddFileInFibToFilesTable
; ---------------------------------
_AddFileInFibToFilesTable::
	push	ix
;emufile.c:533: sizeof(GeneratedFileHeader) + 
	ld	iy,#_fileContentsBase
	ld	a,0 (iy)
	add	a, #0x18
	ld	c,a
	ld	a,1 (iy)
	adc	a, #0x00
	ld	b,a
;emufile.c:534: (sizeof(GeneratedFileTableEntry) * totalFilesProcessed));
	ld	hl,(_totalFilesProcessed)
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl,bc
	ld	c,l
	ld	b,h
;emufile.c:536: tableEntry->deviceIndex = driveInfo->deviceIndex;
	ld	iy,(_driveInfo)
	ld	a,4 (iy)
	ld	(bc),a
;emufile.c:537: tableEntry->logicalUnitNumber = driveInfo->logicalUnitNumber;
	ld	e, c
	ld	d, b
	inc	de
	ld	iy,(_driveInfo)
	ld	a,5 (iy)
	ld	(de),a
;emufile.c:538: tableEntry->firstFileSector = sector;
	ld	e, c
	ld	d, b
	inc	de
	inc	de
	push	bc
	ld	hl, #0x0006
	add	hl, sp
	ld	bc, #0x0004
	ldir
	pop	bc
;emufile.c:539: tableEntry->fileSizeInSector = (uint)(fib->fileSize >> 9);
	ld	hl,#0x0006
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	hl,(_fib)
	ld	de, #0x0015
	add	hl, de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	inc	hl
	ld	a,(hl)
	dec	hl
	ld	l,(hl)
	ld	h,a
	ld	a,#0x09
00103$:
	srl	h
	rr	l
	rr	d
	rr	e
	dec	a
	jr	NZ,00103$
	ld	a,e
	ld	(bc),a
	inc	bc
	ld	a,d
	ld	(bc),a
	pop	ix
	ret
;emufile.c:542: void AddFileInFibToFilenamesInfo()
;	---------------------------------
; Function AddFileInFibToFilenamesInfo
; ---------------------------------
_AddFileInFibToFilenamesInfo::
	push	ix
	ld	ix,#0
	add	ix,sp
	dec	sp
;emufile.c:544: int fileIndex = totalFilesProcessed + 1;
	ld	bc,(_totalFilesProcessed)
	inc	bc
;emufile.c:546: sprintf(fileNamesAppendAddress, "%c -> ", fileIndex <= 9 ? fileIndex + '0' : fileIndex - 10 + 'A');
	ld	a,#0x09
	cp	a, c
	ld	a,#0x00
	sbc	a, b
	jp	PO, 00109$
	xor	a, #0x80
00109$:
	jp	M,00103$
	ld	hl,#0x0030
	add	hl,bc
	ld	c,l
	jr	00104$
00103$:
	ld	hl,#0x0037
	add	hl,bc
	ld	c,l
00104$:
	ld	de,#___str_18+0
	ld	l, c
	push	hl
	push	de
	ld	hl,(_fileNamesAppendAddress)
	push	hl
	call	_sprintf
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
;emufile.c:547: fileNamesAppendAddress += 5;
	ld	hl,#_fileNamesAppendAddress
	ld	a,(hl)
	add	a, #0x05
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	adc	a, #0x00
	ld	(hl),a
;emufile.c:548: strcpy(fileNamesAppendAddress, fib->filename);
	ld	hl,(_fib)
	inc	hl
	ld	de,(_fileNamesAppendAddress)
	xor	a, a
00110$:
	cp	a, (hl)
	ldi
	jr	NZ, 00110$
;emufile.c:549: fileNamesAppendAddress += strlen(fib->filename);
	ld	hl,(_fib)
	inc	hl
	push	hl
	call	_strlen
	pop	af
	ld	c,l
	ld	b,h
	ld	hl,#_fileNamesAppendAddress
	ld	a,(hl)
	add	a, c
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	adc	a, b
	ld	(hl),a
;emufile.c:550: *fileNamesAppendAddress++ = '\r';
	ld	hl,(_fileNamesAppendAddress)
	ld	(hl),#0x0d
	ld	iy,#_fileNamesAppendAddress
	inc	0 (iy)
	jr	NZ,00111$
	inc	1 (iy)
00111$:
;emufile.c:551: *fileNamesAppendAddress++ = '\n';
	ld	hl,(_fileNamesAppendAddress)
	ld	(hl),#0x0a
	inc	0 (iy)
	jr	NZ,00112$
	inc	1 (iy)
00112$:
	inc	sp
	pop	ix
	ret
___str_18:
	.ascii "%c -> "
	.db 0x00
;emufile.c:554: void GenerateFile() 
;	---------------------------------
; Function GenerateFile
; ---------------------------------
_GenerateFile::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
;emufile.c:561: if(bootFileIndex > totalFilesProcessed) {
	ld	hl,#_totalFilesProcessed
	ld	a,(hl)
	ld	iy,#_bootFileIndex
	sub	a, 0 (iy)
	inc	hl
	ld	a,(hl)
	sbc	a, 1 (iy)
	jp	PO, 00115$
	xor	a, #0x80
00115$:
	jp	P,00102$
;emufile.c:562: bootFileIndex = totalFilesProcessed;
	ld	hl,(_totalFilesProcessed)
	ld	(_bootFileIndex),hl
;emufile.c:564: bootFileIndex <= 9 ? bootFileIndex + '0' : bootFileIndex - 10 + 'A');
	ld	a,#0x09
	ld	iy,#_bootFileIndex
	cp	a, 0 (iy)
	ld	a,#0x00
	sbc	a, 1 (iy)
	jp	PO, 00116$
	xor	a, #0x80
00116$:
	jp	M,00105$
	ld	iy,#_bootFileIndex
	ld	a,0 (iy)
	add	a, #0x30
	ld	c,a
	ld	a,1 (iy)
	adc	a, #0x00
	ld	b,a
	jr	00106$
00105$:
	ld	iy,#_bootFileIndex
	ld	a,0 (iy)
	add	a, #0x37
	ld	c,a
	ld	a,1 (iy)
	adc	a, #0x00
	ld	b,a
00106$:
;emufile.c:563: printf("\r\n*** Warning: boot file index is greater than number of files processed.\r\n    Set to %c instead in the generated file.\r\n",
	push	bc
	ld	hl,#___str_19
	push	hl
	call	_printf
	pop	af
	pop	af
00102$:
;emufile.c:567: header = (GeneratedFileHeader*)fileContentsBase;
	ld	bc,(_fileContentsBase)
;emufile.c:568: strcpy(header->signature, "Nextor DSK file");
	ld	e, c
	ld	d, b
	push	bc
	ld	hl,#___str_20
	xor	a, a
00117$:
	cp	a, (hl)
	ldi
	jr	NZ, 00117$
	pop	bc
;emufile.c:569: header->numberOfEntriesInImagesTable = totalFilesProcessed;
	ld	hl,#0x0010
	add	hl,bc
	ex	de,hl
	ld	a,(#_totalFilesProcessed + 0)
	ld	(de),a
;emufile.c:570: header->indexOfImageToMountAtBoot = bootFileIndex;
	ld	hl,#0x0011
	add	hl,bc
	ex	de,hl
	ld	a,(#_bootFileIndex + 0)
	ld	(de),a
;emufile.c:571: header->workAreaAddress = workAreaAddress;
	ld	hl,#0x0012
	add	hl,bc
	ld	iy,#_workAreaAddress
	ld	a,0 (iy)
	ld	(hl),a
	inc	hl
	ld	a,1 (iy)
	ld	(hl),a
;emufile.c:572: memset(header->reserved, 0, 4);
	ld	hl,#0x0014
	add	hl,bc
	ld	b, #0x04
00118$:
	ld	(hl), #0x00
	inc	hl
	djnz	00118$
;emufile.c:574: regs.Words.DE = (int)outputFileName;
	ld	bc,(_outputFileName)
	ld	((_regs + 0x0004)), bc
;emufile.c:575: regs.Bytes.A = 0;
	ld	hl,#(_regs + 0x0001)
	ld	(hl),#0x00
;emufile.c:576: regs.Bytes.B = 0;
	ld	hl,#_regs + 3
	ld	(hl),#0x00
;emufile.c:577: DoDosCall(_CREATE);
	push	hl
	ld	a,#0x44
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
	pop	hl
;emufile.c:578: fileHandle = regs.Bytes.B;
	ld	a,(hl)
	ld	-2 (ix),a
;emufile.c:580: regs.Words.DE = (int)fileContentsBase;
	ld	bc,(_fileContentsBase)
	ld	((_regs + 0x0004)), bc
;emufile.c:581: regs.Words.HL = 
;emufile.c:584: (sizeof(GeneratedFileTableEntry) * totalFilesProcessed));
	ld	hl,(_totalFilesProcessed)
	add	hl, hl
	add	hl, hl
	add	hl, hl
	ld	bc,#0x0018
	add	hl,bc
	ld	c, l
	ld	b, h
	ld	((_regs + 0x0006)), bc
;emufile.c:585: DoDosCall(_WRITE);
	ld	a,#0x49
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;emufile.c:587: fileNamesHeader = "\fDisk image files registered:\r\n\r\n";
	ld	bc,#___str_21+0
;emufile.c:588: regs.Bytes.B = fileHandle;
	ld	hl,#(_regs + 0x0003)
	ld	a,-2 (ix)
	ld	(hl),a
;emufile.c:589: regs.Words.DE = (int)fileNamesHeader;
	ld	e, c
	ld	d, b
	ld	((_regs + 0x0004)), de
;emufile.c:590: regs.Words.HL = strlen(fileNamesHeader);
	push	bc
	call	_strlen
	pop	af
	ld	c,l
	ld	b,h
	ld	((_regs + 0x0006)), bc
;emufile.c:591: DoDosCall(_WRITE);
	ld	a,#0x49
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;emufile.c:593: regs.Bytes.B = fileHandle;
	ld	hl,#(_regs + 0x0003)
	ld	a,-2 (ix)
	ld	(hl),a
;emufile.c:594: regs.Words.DE = (int)fileNamesBase;
	ld	bc,(_fileNamesBase)
	ld	((_regs + 0x0004)), bc
;emufile.c:595: regs.Words.HL = (int)(fileNamesAppendAddress - fileNamesBase);
	ld	hl,#_fileNamesBase
	ld	iy,#_fileNamesAppendAddress
	ld	a,0 (iy)
	sub	a, (hl)
	ld	c,a
	ld	a,1 (iy)
	inc	hl
	sbc	a, (hl)
	ld	b,a
	ld	((_regs + 0x0006)), bc
;emufile.c:596: DoDosCall(_WRITE);
	ld	a,#0x49
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;emufile.c:598: bootFileIndexString = malloc(32);
	ld	hl,#0x0020
	push	hl
	call	_malloc
	pop	af
	ld	c,l
	ld	b,h
;emufile.c:599: sprintf(bootFileIndexString, "\r\nBoot file index: %i\r\n", bootFileIndex);
	push	bc
	ld	hl,(_bootFileIndex)
	push	hl
	ld	hl,#___str_22
	push	hl
	push	bc
	call	_sprintf
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
	pop	bc
;emufile.c:600: regs.Bytes.B = fileHandle;
	ld	hl,#(_regs + 0x0003)
	ld	a,-2 (ix)
	ld	(hl),a
;emufile.c:601: regs.Words.DE = (int)bootFileIndexString;
	ld	e, c
	ld	d, b
	ld	((_regs + 0x0004)), de
;emufile.c:602: regs.Words.HL = strlen(bootFileIndexString);
	push	bc
	call	_strlen
	pop	af
	ld	c,l
	ld	b,h
	ld	((_regs + 0x0006)), bc
;emufile.c:603: DoDosCall(_WRITE);
	ld	a,#0x49
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;emufile.c:605: regs.Bytes.B = fileHandle;
	ld	hl,#(_regs + 0x0003)
	ld	a,-2 (ix)
	ld	(hl),a
;emufile.c:606: DoDosCall(_CLOSE);
	ld	a,#0x45
	push	af
	inc	sp
	call	_DoDosCall
	ld	sp,ix
	pop	ix
	ret
___str_19:
	.db 0x0d
	.db 0x0a
	.ascii "*** Warning: boot file index is greater than number of fil"
	.ascii "es processed."
	.db 0x0d
	.db 0x0a
	.ascii "    Set to %c instead in the generated file."
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_20:
	.ascii "Nextor DSK file"
	.db 0x00
___str_21:
	.db 0x0c
	.ascii "Disk image files registered:"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_22:
	.db 0x0d
	.db 0x0a
	.ascii "Boot file index: %i"
	.db 0x0d
	.db 0x0a
	.db 0x00
;emufile.c:609: void SetupFile()
;	---------------------------------
; Function SetupFile
; ---------------------------------
_SetupFile::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-11
	add	hl,sp
	ld	sp,hl
;emufile.c:616: AddFileExtension(outputFileName);
	ld	hl,(_outputFileName)
	push	hl
	call	_AddFileExtension
	pop	af
;emufile.c:617: StartSearchingFiles(outputFileName);
	ld	hl,(_outputFileName)
	push	hl
	call	_StartSearchingFiles
	pop	af
;emufile.c:618: GetDriveInfoForFileInFib();
	call	_GetDriveInfoForFileInFib
;emufile.c:619: CheckControllerForFileInFib();
	call	_CheckControllerForFileInFib
;emufile.c:621: if(fib->fileSize == 0) {
	ld	hl,(_fib)
	ld	de, #0x0015
	add	hl, de
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	inc	hl
	ld	e,(hl)
	inc	hl
	ld	a, (hl)
	or	a, e
	or	a, b
	or	a,c
	jr	NZ,00102$
;emufile.c:622: Terminate("*** The emulation data file is empty");
	ld	hl,#___str_23
	push	hl
	call	_Terminate
	pop	af
00102$:
;emufile.c:625: sectorBuffer = malloc(sizeof(masterBootRecord));
	ld	hl,#0x0200
	push	hl
	call	_malloc
;emufile.c:627: VerifyDataFileSignature((byte*)sectorBuffer);
	ex	(sp),hl
	call	_VerifyDataFileSignature
	pop	af
;emufile.c:629: sector = driveInfo->firstSectorNumber + GetFirstFileSectorForFileInFib();
	ld	hl,(_driveInfo)
	ld	de, #0x0006
	add	hl, de
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	inc	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	push	bc
	push	de
	call	_GetFirstFileSectorForFileInFib
	ld	-4 (ix),d
	ld	-5 (ix),e
	ld	-6 (ix),h
	ld	-7 (ix),l
	pop	de
	pop	bc
	ld	a,c
	add	a, -7 (ix)
	ld	c,a
	ld	a,b
	adc	a, -6 (ix)
	ld	b,a
	ld	a,e
	adc	a, -5 (ix)
	ld	e,a
	ld	a,d
	adc	a, -4 (ix)
	ld	d,a
	ld	-11 (ix),c
	ld	-10 (ix),b
	ld	-9 (ix),e
	ld	-8 (ix),d
;emufile.c:631: if(setupPartitionDeviceIndex == 0) {
	ld	iy,#_setupPartitionDeviceIndex
	ld	a,1 (iy)
	or	a,0 (iy)
	jr	NZ,00104$
;emufile.c:629: sector = driveInfo->firstSectorNumber + GetFirstFileSectorForFileInFib();
	ld	bc,(_driveInfo)
;emufile.c:632: setupPartitionDeviceIndex = driveInfo->deviceIndex;
	push	bc
	pop	iy
	ld	e,4 (iy)
	ld	iy,#_setupPartitionDeviceIndex
	ld	0 (iy),e
	ld	1 (iy),#0x00
;emufile.c:633: setupPartitionLunIndex = driveInfo->logicalUnitNumber;
	push	bc
	pop	iy
	ld	c,5 (iy)
	ld	iy,#_setupPartitionLunIndex
	ld	0 (iy),c
	ld	1 (iy),#0x00
00104$:
;emufile.c:636: sectorBuffer = malloc(sizeof(masterBootRecord));
	ld	hl,#0x0200
	push	hl
	call	_malloc
	pop	af
	ld	c,l
	ld	b,h
;emufile.c:627: VerifyDataFileSignature((byte*)sectorBuffer);
	ld	-7 (ix),c
	ld	-6 (ix),b
;emufile.c:637: error = ReadDeviceSector(driveInfo->driverSlotNumber, setupPartitionDeviceIndex, setupPartitionLunIndex, 0, (byte*)sectorBuffer);
	ld	a,(#_setupPartitionLunIndex + 0)
	ld	-1 (ix),a
	ld	hl,#_setupPartitionDeviceIndex + 0
	ld	e, (hl)
	ld	hl,(_driveInfo)
	inc	hl
	ld	d,(hl)
	push	bc
	xor	a, a
	push	af
	inc	sp
	ld	l,-7 (ix)
	ld	h,-6 (ix)
	push	hl
	ld	hl,#0x0000
	push	hl
	ld	hl,#0x0000
	push	hl
	ld	a,-1 (ix)
	push	af
	inc	sp
	ld	a,e
	push	af
	inc	sp
	push	de
	inc	sp
	call	_DeviceSectorRW
	ld	iy,#10
	add	iy,sp
	ld	sp,iy
	ld	d,l
	pop	bc
;emufile.c:638: if(error != 0) {
	ld	a,d
	or	a, a
	jr	Z,00106$
;emufile.c:639: print("*** Error when reading MBR of device:");
	push	bc
	push	de
	ld	hl,#___str_24
	push	hl
	call	_print
	pop	af
	inc	sp
	call	_TerminateWithDosError
	inc	sp
	pop	bc
00106$:
;emufile.c:643: partition = &(sectorBuffer->primaryPartitions[0]);
	ld	hl,#0x01be
	add	hl,bc
	ld	c,l
	ld	b,h
;emufile.c:644: partition->status |= 1;
	ld	a,(bc)
	set	0, a
	ld	(bc),a
;emufile.c:645: partition->chsOfFirstSector[0] = driveInfo->deviceIndex;
	ld	e, c
	ld	d, b
	inc	de
	ld	iy,(_driveInfo)
	ld	a,4 (iy)
	ld	(de),a
;emufile.c:646: partition->chsOfFirstSector[1] = driveInfo->logicalUnitNumber;
	ld	e, c
	ld	d, b
	inc	de
	inc	de
	ld	iy,(_driveInfo)
	ld	a,5 (iy)
	ld	(de),a
;emufile.c:647: partition->chsOfFirstSector[2] = ((byte*)&sector)[3];   //MSB
	ld	hl,#0x0003
	add	hl,bc
	ld	-3 (ix),l
	ld	-2 (ix),h
	ld	hl,#0x0000
	add	hl,sp
	ld	e,l
	ld	d,h
	inc	hl
	inc	hl
	inc	hl
	ld	a,(hl)
	ld	l,-3 (ix)
	ld	h,-2 (ix)
	ld	(hl),a
;emufile.c:648: partition->chsOfLastSector[0] = ((byte*)&sector)[2];
	ld	iy,#0x0005
	add	iy, bc
	ld	l, e
	ld	h, d
	inc	hl
	inc	hl
	ld	l,(hl)
	ld	0 (iy), l
;emufile.c:649: partition->chsOfLastSector[1] = ((byte*)&sector)[1];
	ld	iy,#0x0006
	add	iy, bc
	ld	l, e
	ld	h, d
	inc	hl
	ld	l,(hl)
	ld	0 (iy), l
;emufile.c:650: partition->chsOfLastSector[2] = ((byte*)&sector)[0];    //LSB
	ld	hl,#0x0007
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	a,(de)
	ld	(bc),a
;emufile.c:652: error = WriteDeviceSector(driveInfo->driverSlotNumber, setupPartitionDeviceIndex, setupPartitionLunIndex, 0, (byte*)sectorBuffer);
	ld	hl,#_setupPartitionLunIndex + 0
	ld	c, (hl)
	ld	hl,#_setupPartitionDeviceIndex + 0
	ld	b, (hl)
	ld	hl,(_driveInfo)
	inc	hl
	ld	d,(hl)
	ld	a,#0x01
	push	af
	inc	sp
	ld	l,-7 (ix)
	ld	h,-6 (ix)
	push	hl
	ld	hl,#0x0000
	push	hl
	ld	hl,#0x0000
	push	hl
	ld	a,c
	push	af
	inc	sp
	ld	c, d
	push	bc
	call	_DeviceSectorRW
	ld	iy,#10
	add	iy,sp
	ld	sp,iy
	ld	b,l
;emufile.c:653: if(error != 0) {
	ld	a,b
	or	a, a
	jr	Z,00109$
;emufile.c:654: print("*** Error when writing MBR of device:");
	push	bc
	ld	hl,#___str_25
	push	hl
	call	_print
	pop	af
	inc	sp
	call	_TerminateWithDosError
	inc	sp
00109$:
	ld	sp, ix
	pop	ix
	ret
___str_23:
	.ascii "*** The emulation data file is empty"
	.db 0x00
___str_24:
	.ascii "*** Error when reading MBR of device:"
	.db 0x00
___str_25:
	.ascii "*** Error when writing MBR of device:"
	.db 0x00
;emufile.c:659: void VerifyDataFileSignature(byte* sectorBuffer)
;	---------------------------------
; Function VerifyDataFileSignature
; ---------------------------------
_VerifyDataFileSignature::
	push	ix
	ld	ix,#0
	add	ix,sp
;emufile.c:661: regs.Words.DE = (int)outputFileName;
	ld	bc,(_outputFileName)
	ld	((_regs + 0x0004)), bc
;emufile.c:662: regs.Bytes.A = 1;    //read-only
	ld	hl,#(_regs + 0x0001)
	ld	(hl),#0x01
;emufile.c:663: DoDosCall(_OPEN);
	ld	a,#0x43
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;emufile.c:664: fileHandle = regs.Bytes.B;
	ld	a,(#_regs + 3)
	ld	(#_fileHandle + 0),a
;emufile.c:666: regs.Words.DE = (int)sectorBuffer;
	ld	c,4 (ix)
	ld	b,5 (ix)
	ld	((_regs + 0x0004)), bc
;emufile.c:667: regs.Words.HL = 16;
	ld	hl,#0x0010
	ld	((_regs + 0x0006)), hl
;emufile.c:668: DoDosCall(_READ);
	ld	a,#0x48
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;emufile.c:670: if(regs.Words.HL < 16 || strcmpi((char*)sectorBuffer, "Nextor DSK file") != 0) {
	ld	hl, (#(_regs + 0x0006) + 0)
	ld	de, #0x8010
	add	hl, hl
	ccf
	rr	h
	rr	l
	sbc	hl, de
	jr	C,00101$
	ld	hl,#___str_26
	push	hl
	ld	l,4 (ix)
	ld	h,5 (ix)
	push	hl
	call	_strcmpi
	pop	af
	pop	af
	ld	a,h
	or	a,l
	jr	Z,00104$
00101$:
;emufile.c:671: Terminate("Invalid emulation data file");
	ld	hl,#___str_27
	push	hl
	call	_Terminate
	pop	af
00104$:
	pop	ix
	ret
___str_26:
	.ascii "Nextor DSK file"
	.db 0x00
___str_27:
	.ascii "Invalid emulation data file"
	.db 0x00
;emufile.c:675: byte DeviceSectorRW(byte driverSlot, byte deviceIndex, byte lunIndex, ulong sectorNumber, byte* buffer, bool write)
;	---------------------------------
; Function DeviceSectorRW
; ---------------------------------
_DeviceSectorRW::
	push	ix
	ld	ix,#0
	add	ix,sp
;emufile.c:677: regs.Flags.C = write;
	ld	hl,#_regs
	ld	a,13 (ix)
	and	a,#0x01
	ld	c,a
	ld	a,(hl)
	and	a,#0xfe
	or	a,c
	ld	(hl),a
;emufile.c:678: regs.Bytes.A = deviceIndex;
	ld	hl,#(_regs + 0x0001)
	ld	a,5 (ix)
	ld	(hl),a
;emufile.c:679: regs.Bytes.B = 1;
	ld	hl,#(_regs + 0x0003)
	ld	(hl),#0x01
;emufile.c:680: regs.Bytes.C = lunIndex;
	ld	hl,#(_regs + 0x0002)
	ld	a,6 (ix)
	ld	(hl),a
;emufile.c:681: regs.Words.HL = (int)buffer;
	ld	c,11 (ix)
	ld	b,12 (ix)
	ld	((_regs + 0x0006)), bc
;emufile.c:682: regs.Words.DE = (int)&sectorNumber;
	ld	hl,#0x0007
	add	hl,sp
	ld	c,l
	ld	b,h
	ld	((_regs + 0x0004)), bc
;emufile.c:684: DriverCall(driverSlot, DEV_RW);
	ld	hl,#0x4160
	push	hl
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_DriverCall
	pop	af
	inc	sp
;emufile.c:685: return regs.Bytes.A;
	ld	hl, #(_regs + 0x0001) + 0
	ld	l,(hl)
	pop	ix
	ret
;emufile.c:688: void DriverCall(byte slot, uint routineAddress)
;	---------------------------------
; Function DriverCall
; ---------------------------------
_DriverCall::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-8
	add	hl,sp
	ld	sp,hl
;emufile.c:693: memcpy(registerData, &regs, 8);
	ld	hl,#0x0000
	add	hl,sp
	ld	c,l
	ld	b,h
	ld	e, c
	ld	d, b
	push	bc
	ld	hl,#_regs
	ld	bc,#0x0008
	ldir
	pop	bc
;emufile.c:695: regs.Bytes.A = slot;
	ld	hl,#(_regs + 0x0001)
	ld	a,4 (ix)
	ld	(hl),a
;emufile.c:696: regs.Bytes.B = 0xFF;
	ld	hl,#(_regs + 0x0003)
	ld	(hl),#0xff
;emufile.c:697: regs.UWords.DE = routineAddress;
	ld	hl,#(_regs + 0x0004)
	ld	a,5 (ix)
	ld	(hl),a
	inc	hl
	ld	a,6 (ix)
	ld	(hl),a
;emufile.c:698: regs.Words.HL = (int)registerData;
	ld	((_regs + 0x0006)), bc
;emufile.c:700: DoDosCall(_CDRVR);
	ld	a,#0x7b
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;emufile.c:702: regs.Words.AF = regs.Words.IX;
	ld	bc, (#_regs + 8)
	ld	(_regs), bc
	ld	sp, ix
	pop	ix
	ret
;emufile.c:705: void ResetComputer()
;	---------------------------------
; Function ResetComputer
; ---------------------------------
_ResetComputer::
;emufile.c:707: regs.Bytes.IYh = *(byte*)EXPTBL;
	ld	bc,#_regs+11
	ld	a,(#0xfcc1)
	ld	(bc),a
;emufile.c:708: regs.Words.IX = 0;
	ld	hl,#0x0000
	ld	((_regs + 0x0008)), hl
;emufile.c:709: AsmCall(CALSLT, &regs, REGS_ALL, REGS_NONE);
	ld	l, #0x00
	push	hl
	ld	l, #0x03
	push	hl
	ld	hl,#_regs
	push	hl
	ld	hl,#0x001c
	push	hl
	call	_AsmCallAlt
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
	ret
;emufile.c:712: void Terminate(const char* errorMessage)
;	---------------------------------
; Function Terminate
; ---------------------------------
_Terminate::
;emufile.c:714: if(fileHandle != 0) {
	ld	iy,#_fileHandle
	ld	a,0 (iy)
	or	a, a
	jr	Z,00102$
;emufile.c:715: regs.Bytes.B = fileHandle;
	ld	hl,#(_regs + 0x0003)
	ld	a,0 (iy)
	ld	(hl),a
;emufile.c:716: DoDosCall(_CLOSE);
	ld	a,#0x45
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
00102$:
;emufile.c:719: if(errorMessage != NULL) {
	ld	hl, #2+1
	add	hl, sp
	ld	a, (hl)
	dec	hl
	or	a,(hl)
	jr	Z,00104$
;emufile.c:720: printf("\r\x1BK*** %s\r\n", errorMessage);
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	ld	hl,#___str_28
	push	hl
	call	_printf
	pop	af
	pop	af
00104$:
;emufile.c:723: regs.Bytes.B = (errorMessage == NULL ? 0 : 1);
	ld	bc,#_regs+3
	ld	hl, #2+1
	add	hl, sp
	ld	a, (hl)
	dec	hl
	or	a,(hl)
	jr	Z,00108$
	ld	a,#0x01
00108$:
	ld	(bc),a
;emufile.c:724: DoDosCall(_TERM);
	ld	a,#0x62
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;emufile.c:725: DoDosCall(_TERM0);
	xor	a, a
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
	ret
___str_28:
	.db 0x0d
	.db 0x1b
	.ascii "K*** %s"
	.db 0x0d
	.db 0x0a
	.db 0x00
;emufile.c:729: void TerminateWithDosError(byte errorCode)
;	---------------------------------
; Function TerminateWithDosError
; ---------------------------------
_TerminateWithDosError::
;emufile.c:731: regs.Bytes.B = errorCode;
	ld	hl,#(_regs + 0x0003)
	ld	iy,#2
	add	iy,sp
	ld	a,0 (iy)
	ld	(hl),a
;emufile.c:732: DoDosCall(_TERM);
	ld	a,#0x62
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
	ret
;emufile.c:736: void print(char* s) __naked
;	---------------------------------
; Function print
; ---------------------------------
_print::
;emufile.c:758: __endasm;    
	push	ix
	ld	ix,#4
	add	ix,sp
	ld	l,(ix)
	ld	h,1(ix)
	loop:
	ld	a,(hl)
	or	a
	jr	z,end
	ld	e,a
	ld	c,#2
	push	hl
	call	#5
	pop	hl
	inc	hl
	jr	loop
	end:
	pop	ix
	ret
;emufile.c:762: void CheckDosVersion()
;	---------------------------------
; Function CheckDosVersion
; ---------------------------------
_CheckDosVersion::
;emufile.c:764: regs.Bytes.B = 0x5A;
	ld	hl,#(_regs + 0x0003)
	ld	(hl),#0x5a
;emufile.c:765: regs.Words.HL = 0x1234;
	ld	hl,#0x1234
	ld	((_regs + 0x0006)), hl
;emufile.c:766: regs.Words.DE = (int)0xABCD;
	ld	hl,#0xabcd
	ld	((_regs + 0x0004)), hl
;emufile.c:767: regs.Words.IX = 0;
	ld	hl,#0x0000
	ld	((_regs + 0x0008)), hl
;emufile.c:768: DosCall(_DOSVER, &regs, REGS_ALL, REGS_ALL);
	ld	hl,#0x0303
	push	hl
	ld	hl,#_regs
	push	hl
	ld	a,#0x6f
	push	af
	inc	sp
	call	_DosCall
	pop	af
	pop	af
	inc	sp
;emufile.c:770: if(regs.Bytes.B < 2 || regs.Bytes.IXh != 1) {
	ld	a,(#_regs + 3)
	sub	a, #0x02
	jr	C,00101$
	ld	a, (#_regs + 9)
	dec	a
	ret	Z
00101$:
;emufile.c:771: Terminate("This program is for Nextor only.");
	ld	hl,#___str_29
	push	hl
	call	_Terminate
	pop	af
	ret
___str_29:
	.ascii "This program is for Nextor only."
	.db 0x00
;emufile.c:775: void* malloc(int size)
;	---------------------------------
; Function malloc
; ---------------------------------
_malloc::
	push	ix
	ld	ix,#0
	add	ix,sp
;emufile.c:777: void* value = mallocPointer;
	ld	bc,(_mallocPointer)
;emufile.c:778: mallocPointer = (void*)(((int)mallocPointer) + size);
	ld	de,(_mallocPointer)
	ld	l,4 (ix)
	ld	h,5 (ix)
	add	hl,de
	ld	(_mallocPointer),hl
;emufile.c:779: return value;
	ld	l, c
	ld	h, b
	pop	ix
	ret
;emufile.c:782: uint ParseHex(char* hexString)
;	---------------------------------
; Function ParseHex
; ---------------------------------
_ParseHex::
	push	ix
	ld	ix,#0
	add	ix,sp
	dec	sp
;emufile.c:787: result = 0;
	ld	de,#0x0000
;emufile.c:788: while((digit = *hexString) != 0) {
	ld	c,4 (ix)
	ld	b,5 (ix)
00109$:
	ld	a,(bc)
;emufile.c:789: digit |= 32;
	or	a,a
	jr	Z,00111$
	set	5, a
	ld	-1 (ix),a
;emufile.c:790: result *= 16;
	sla	e
	rl	d
	sla	e
	rl	d
	sla	e
	rl	d
	sla	e
	rl	d
;emufile.c:792: result += digit - '0';
	ld	l,-1 (ix)
	ld	h,#0x00
;emufile.c:791: if(digit >= '0' && digit <= '9') {
	ld	a,-1 (ix)
	sub	a, #0x30
	jr	C,00106$
	ld	a,#0x39
	sub	a, -1 (ix)
	jr	C,00106$
;emufile.c:792: result += digit - '0';
	ld	a,l
	add	a,#0xd0
	ld	l,a
	ld	a,h
	adc	a,#0xff
	ld	h,a
	add	hl,de
	ex	de,hl
	jr	00107$
00106$:
;emufile.c:794: else if(digit >= 'a' && digit <='f') {
	ld	a,-1 (ix)
	sub	a, #0x61
	jr	C,00102$
	ld	a,#0x66
	sub	a, -1 (ix)
	jr	C,00102$
;emufile.c:795: result += digit - 'a' + 10;
	push	de
	ld	de,#0xffa9
	add	hl, de
	pop	de
	add	hl,de
	ex	de,hl
	jr	00107$
00102$:
;emufile.c:798: InvalidParameter();
	push	bc
	push	de
	ld	hl,(_strInvParam)
	push	hl
	call	_Terminate
	pop	af
	pop	de
	pop	bc
00107$:
;emufile.c:800: hexString++;
	inc	bc
	jr	00109$
00111$:
;emufile.c:803: return result;
	ex	de,hl
	inc	sp
	pop	ix
	ret
;emufile.c:806: void DoDosCall(byte functionCode)
;	---------------------------------
; Function DoDosCall
; ---------------------------------
_DoDosCall::
;emufile.c:808: DosCall(functionCode, &regs, REGS_ALL, REGS_ALL);
	ld	hl,#0x0303
	push	hl
	ld	hl,#_regs
	push	hl
	ld	hl, #6+0
	add	hl, sp
	ld	a, (hl)
	push	af
	inc	sp
	call	_DosCall
	pop	af
	pop	af
	inc	sp
;emufile.c:809: if(regs.Bytes.A != 0 && !(functionCode == _FNEXT && regs.Bytes.A == _NOFIL)) {
	ld	hl,#_regs + 1
	ld	a,(hl)
	or	a, a
	ret	Z
	ld	iy,#2
	add	iy,sp
	ld	a,0 (iy)
	sub	a, #0x41
	jr	NZ,00101$
	ld	a,(hl)
	sub	a, #0xd7
	ret	Z
00101$:
;emufile.c:810: TerminateWithDosError(regs.Bytes.A);
	ld	b,(hl)
	push	bc
	inc	sp
	call	_TerminateWithDosError
	inc	sp
	ret
;printf.c:43: int printf(const char *fmt, ...)
;	---------------------------------
; Function printf
; ---------------------------------
_printf::
;printf.c:46: va_start(arg, fmt);
	ld	hl,#0x0002+1+1
	add	hl,sp
;printf.c:47: return format_string(0, fmt, arg);
	push	hl
	ld	hl, #4
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	push	bc
	ld	hl,#0x0000
	push	hl
	call	_format_string
	pop	af
	pop	af
	pop	af
	ret
;printf.c:50: int sprintf(const char* buf, const char* fmt, ...)
;	---------------------------------
; Function sprintf
; ---------------------------------
_sprintf::
;printf.c:53: va_start(arg, fmt);
	ld	hl,#0x0004+1+1
	add	hl,sp
;printf.c:54: return format_string(buf, fmt, arg);
	push	hl
	ld	hl, #6
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	push	bc
	ld	hl, #6
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	push	bc
	call	_format_string
	pop	af
	pop	af
	pop	af
	ret
;printf.c:57: static void do_char(const char* buf, char c) __naked
;	---------------------------------
; Function do_char
; ---------------------------------
_do_char:
;printf.c:83: __endasm;
	ld	hl,#4
	add	hl,sp
	ld	e,(hl)
	dec	hl
	ld	a,(hl)
	dec	hl
	ld	l,(hl)
	ld	h,a
	or	l
	ld	c,#2
	jp	z,5
	ld	(hl),e
	ret
;printf.c:88: static int format_string(const char* buf, const char *fmt, va_list ap)
;	---------------------------------
; Function format_string
; ---------------------------------
_format_string:
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-28
	add	hl,sp
	ld	sp,hl
;printf.c:101: int count=0;
	ld	hl,#0x0000
	ex	(sp), hl
;printf.c:103: fmtPnt = fmt;
	ld	a,6 (ix)
	ld	-22 (ix),a
	ld	a,7 (ix)
	ld	-21 (ix),a
;printf.c:104: bufPnt = buf;
	ld	a,4 (ix)
	ld	-25 (ix),a
	ld	a,5 (ix)
	ld	-24 (ix),a
;printf.c:106: while((theChar = *fmtPnt)!=0)
00135$:
	ld	l,-22 (ix)
	ld	h,-21 (ix)
	ld	a,(hl)
	ld	-18 (ix), a
	ld	-17 (ix),a
	ld	a,-18 (ix)
	or	a, a
	jp	Z,00137$
;printf.c:111: isUnsigned = 0;
	ld	-23 (ix),#0x00
;printf.c:112: base = 10;
	ld	-26 (ix),#0x0a
;printf.c:114: fmtPnt++;
	inc	-22 (ix)
	jr	NZ,00222$
	inc	-21 (ix)
00222$:
;printf.c:117: do_char_inc(theChar);
	ld	a,-25 (ix)
	add	a, #0x01
	ld	-6 (ix),a
	ld	a,-24 (ix)
	adc	a, #0x00
	ld	-5 (ix),a
	ld	a,-28 (ix)
	add	a, #0x01
	ld	-2 (ix),a
	ld	a,-27 (ix)
	adc	a, #0x00
	ld	-1 (ix),a
;printf.c:116: if(theChar != '%') {
	ld	a,-17 (ix)
	sub	a, #0x25
	jr	Z,00104$
;printf.c:117: do_char_inc(theChar);
	ld	a,-17 (ix)
	push	af
	inc	sp
	ld	l,-25 (ix)
	ld	h,-24 (ix)
	push	hl
	call	_do_char
	pop	af
	inc	sp
	ld	a,-24 (ix)
	or	a,-25 (ix)
	jr	Z,00102$
	ld	a,-6 (ix)
	ld	-25 (ix),a
	ld	a,-5 (ix)
	ld	-24 (ix),a
00102$:
	ld	a,-2 (ix)
	ld	-28 (ix),a
	ld	a,-1 (ix)
	ld	-27 (ix),a
;printf.c:118: continue;
	jp	00135$
00104$:
;printf.c:121: theChar = *fmtPnt;
	ld	l,-22 (ix)
	ld	h,-21 (ix)
	ld	a,(hl)
	ld	-17 (ix),a
;printf.c:122: fmtPnt++;
	inc	-22 (ix)
	jr	NZ,00224$
	inc	-21 (ix)
00224$:
;printf.c:126: strPnt = va_arg(ap, char *);
	ld	a,8 (ix)
	add	a, #0x02
	ld	-8 (ix),a
	ld	a,9 (ix)
	adc	a, #0x00
	ld	-7 (ix),a
	ld	a,-8 (ix)
	add	a,#0xfe
	ld	-12 (ix),a
	ld	a,-7 (ix)
	adc	a,#0xff
	ld	-11 (ix),a
;printf.c:124: if(theChar == 's')
	ld	a,-17 (ix)
	sub	a, #0x73
	jp	NZ,00111$
;printf.c:126: strPnt = va_arg(ap, char *);
	ld	a,-8 (ix)
	ld	8 (ix),a
	ld	a,-7 (ix)
	ld	9 (ix),a
	ld	a,-12 (ix)
	ld	-10 (ix),a
	ld	a,-11 (ix)
	ld	-9 (ix),a
	ld	l,-10 (ix)
	ld	h,-9 (ix)
	ld	a,(hl)
	ld	-10 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-9 (ix),a
;printf.c:127: while((theChar = *strPnt++) != 0) 
	ld	a,-25 (ix)
	ld	-20 (ix),a
	ld	a,-24 (ix)
	ld	-19 (ix),a
	ld	a,-28 (ix)
	ld	-4 (ix),a
	ld	a,-27 (ix)
	ld	-3 (ix),a
00107$:
	ld	l,-10 (ix)
	ld	h,-9 (ix)
	ld	a,(hl)
	inc	-10 (ix)
	jr	NZ,00227$
	inc	-9 (ix)
00227$:
	ld	b,a
	or	a, a
	jp	Z,00135$
;printf.c:128: do_char_inc(theChar);
	push	bc
	inc	sp
	ld	l,-20 (ix)
	ld	h,-19 (ix)
	push	hl
	call	_do_char
	pop	af
	inc	sp
	ld	a,-19 (ix)
	or	a,-20 (ix)
	jr	Z,00106$
	inc	-20 (ix)
	jr	NZ,00228$
	inc	-19 (ix)
00228$:
	ld	a,-20 (ix)
	ld	-25 (ix),a
	ld	a,-19 (ix)
	ld	-24 (ix),a
00106$:
	inc	-4 (ix)
	jr	NZ,00229$
	inc	-3 (ix)
00229$:
	ld	a,-4 (ix)
	ld	-28 (ix),a
	ld	a,-3 (ix)
	ld	-27 (ix),a
	jr	00107$
;printf.c:130: continue;
00111$:
;printf.c:135: val = va_arg(ap, int);
	ld	a,-12 (ix)
	ld	-4 (ix),a
	ld	a,-11 (ix)
	ld	-3 (ix),a
;printf.c:133: if(theChar == 'c')
	ld	a,-17 (ix)
	sub	a, #0x63
	jr	NZ,00115$
;printf.c:135: val = va_arg(ap, int);
	ld	a,-8 (ix)
	ld	8 (ix),a
	ld	a,-7 (ix)
	ld	9 (ix),a
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	ld	a,(hl)
	ld	-20 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-19 (ix),a
	ld	a,-20 (ix)
	ld	-16 (ix),a
	ld	a,-19 (ix)
	ld	-15 (ix),a
	ld	a,-19 (ix)
	rla
	sbc	a, a
	ld	-14 (ix),a
	ld	-13 (ix),a
;printf.c:136: do_char_inc((char) val);
	ld	b,-16 (ix)
	push	bc
	inc	sp
	ld	l,-25 (ix)
	ld	h,-24 (ix)
	push	hl
	call	_do_char
	pop	af
	inc	sp
	ld	a,-24 (ix)
	or	a,-25 (ix)
	jr	Z,00113$
	ld	a,-6 (ix)
	ld	-25 (ix),a
	ld	a,-5 (ix)
	ld	-24 (ix),a
00113$:
	ld	a,-2 (ix)
	ld	-28 (ix),a
	ld	a,-1 (ix)
	ld	-27 (ix),a
;printf.c:138: continue;
	jp	00135$
00115$:
;printf.c:150: if(theChar == 'u') {
	ld	a,-17 (ix)
	sub	a, #0x75
	jr	NZ,00125$
;printf.c:151: isUnsigned = 1;
	ld	-23 (ix),#0x01
	jr	00126$
00125$:
;printf.c:153: else if(theChar == 'x') {
	ld	a,-17 (ix)
	sub	a, #0x78
	jr	NZ,00122$
;printf.c:154: base = 16;
	ld	-26 (ix),#0x10
	jr	00126$
00122$:
;printf.c:156: else if(theChar != 'd' && theChar != 'i') {
	ld	a,-17 (ix)
	sub	a, #0x64
	jr	Z,00126$
	ld	a,-17 (ix)
	sub	a, #0x69
	jr	Z,00126$
;printf.c:157: do_char_inc(theChar);
	ld	a,-17 (ix)
	push	af
	inc	sp
	ld	l,-25 (ix)
	ld	h,-24 (ix)
	push	hl
	call	_do_char
	pop	af
	inc	sp
	ld	a,-24 (ix)
	or	a,-25 (ix)
	jr	Z,00117$
	ld	a,-6 (ix)
	ld	-25 (ix),a
	ld	a,-5 (ix)
	ld	-24 (ix),a
00117$:
	ld	a,-2 (ix)
	ld	-28 (ix),a
	ld	a,-1 (ix)
	ld	-27 (ix),a
;printf.c:158: continue;
	jp	00135$
00126$:
;printf.c:176: val = va_arg(ap, int);
	ld	a,-8 (ix)
	ld	8 (ix),a
	ld	a,-7 (ix)
	ld	9 (ix),a
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	ld	a,b
	rla
	sbc	a, a
;printf.c:179: _uitoa(val, buffer, base);
	ld	-16 (ix),c
	ld	-15 (ix),b
;printf.c:178: if(isUnsigned)
	ld	a,-23 (ix)
	or	a, a
	jr	Z,00128$
;printf.c:179: _uitoa(val, buffer, base);
	ld	a,-26 (ix)
	push	af
	inc	sp
	ld	hl,#_format_string_buffer_1_213
	push	hl
	ld	l,-16 (ix)
	ld	h,-15 (ix)
	push	hl
	call	__uitoa
	pop	af
	pop	af
	inc	sp
	jr	00129$
00128$:
;printf.c:181: _itoa(val, buffer, base);
	ld	a,-26 (ix)
	push	af
	inc	sp
	ld	hl,#_format_string_buffer_1_213
	push	hl
	ld	l,-16 (ix)
	ld	h,-15 (ix)
	push	hl
	call	__itoa
	pop	af
	pop	af
	inc	sp
00129$:
;printf.c:184: strPnt = buffer;
	ld	bc,#_format_string_buffer_1_213
;printf.c:185: while((theChar = *strPnt++) != 0) 
	ld	e,-25 (ix)
	ld	d,-24 (ix)
	ld	a,-28 (ix)
	ld	-16 (ix),a
	ld	a,-27 (ix)
	ld	-15 (ix),a
00132$:
	ld	a,(bc)
	inc	bc
	ld	h,a
	or	a, a
	jp	Z,00135$
;printf.c:186: do_char_inc(theChar);
	push	bc
	push	de
	push	hl
	inc	sp
	push	de
	call	_do_char
	pop	af
	inc	sp
	pop	de
	pop	bc
	ld	a,d
	or	a,e
	jr	Z,00131$
	inc	de
	ld	-25 (ix),e
	ld	-24 (ix),d
00131$:
	inc	-16 (ix)
	jr	NZ,00238$
	inc	-15 (ix)
00238$:
	ld	a,-16 (ix)
	ld	-28 (ix),a
	ld	a,-15 (ix)
	ld	-27 (ix),a
	jr	00132$
00137$:
;printf.c:189: if(bufPnt) *bufPnt = '\0';
	ld	a,-24 (ix)
	or	a,-25 (ix)
	jr	Z,00139$
	ld	l,-25 (ix)
	ld	h,-24 (ix)
	ld	(hl),#0x00
00139$:
;printf.c:191: return count;
	pop	hl
	push	hl
	ld	sp, ix
	pop	ix
	ret
;asmcall.c:12: void DosCall(byte function, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail)
;	---------------------------------
; Function DosCall
; ---------------------------------
_DosCall::
;asmcall.c:14: regs->Bytes.C = function;
	ld	hl, #3
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	ld	e, c
	ld	d, b
	inc	de
	inc	de
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	ld	(de),a
;asmcall.c:15: AsmCall(0x0005,regs,inRegistersDetail < REGS_MAIN ? REGS_MAIN : inRegistersDetail, outRegistersDetail);
	ld	hl, #5+0
	add	hl, sp
	ld	a, (hl)
	sub	a, #0x02
	jr	NC,00103$
	ld	d,#0x02
	jr	00104$
00103$:
	ld	hl, #5+0
	add	hl, sp
	ld	d, (hl)
00104$:
	ld	hl,#0x0000
	push	hl
	ld	iy,#8
	add	iy,sp
	ld	a,0 (iy)
	push	af
	inc	sp
	push	de
	inc	sp
	push	bc
	ld	l, #0x05
	push	hl
	call	_AsmCallAlt
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
	ret
;asmcall.c:19: void AsmCallAlt(uint address, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail, int alternateAf) __naked
;	---------------------------------
; Function AsmCallAlt
; ---------------------------------
_AsmCallAlt::
;asmcall.c:132: __endasm;
	push	ix
	ld	ix,#4
	add	ix,sp
	ld	e,6(ix) ;Alternate AF
	ld	d,7(ix)
	ex	af,af
	push	de
	pop	af
	ex	af,af
	ld	l,(ix) ;HL=Routine address
	ld	h,1(ix)
	ld	e,2(ix) ;DE=regs address
	ld	d,3(ix)
	ld	a,5(ix)
	ld	(_OUT_FLAGS),a
	ld	a,4(ix) ;A=in registers detail
	ld	(_ASMRUT+1),hl
	push	de
	or	a
	jr	z,ASMRUT_DO
	push	de
	pop	ix ;IX=&Z80regs
	exx
	ld	l,(ix)
	ld	h,1(ix) ;AF
	dec	a
	jr	z,ASMRUT_DOAF
	exx
	ld	c,2(ix) ;BC, DE, HL
	ld	b,3(ix)
	ld	e,4(ix)
	ld	d,5(ix)
	ld	l,6(ix)
	ld	h,7(ix)
	dec	a
	exx
	jr	z,ASMRUT_DOAF
	ld	c,8(ix) ;IX
	ld	b,9(ix)
	ld	e,10(ix) ;IY
	ld	d,11(ix)
	push	de
	push	bc
	pop	ix
	pop	iy
	ASMRUT_DOAF:
	push	hl
	pop	af
	exx
	ASMRUT_DO:
	call	_ASMRUT
;ASMRUT:	call 0
	ex	(sp),ix ;IX to stack, now IX=&Z80regs
	ex	af,af ;Alternate AF
	ld	a,(_OUT_FLAGS)
	or	a
	jr	z,CALL_END
	exx	;Alternate HLDEBC
	ex	af,af ;Main AF
	push	af
	pop	hl
	ld	(ix),l
	ld	1(ix),h
	exx	;Main HLDEBC
	ex	af,af ;Alternate AF
	dec	a
	jr	z,CALL_END
	ld	2(ix),c ;BC, DE, HL
	ld	3(ix),b
	ld	4(ix),e
	ld	5(ix),d
	ld	6(ix),l
	ld	7(ix),h
	dec	a
	jr	z,CALL_END
	exx	;Alternate HLDEBC
	pop	hl
	ld	8(ix),l ;IX
	ld	9(ix),h
	push	iy
	pop	hl
	ld	10(ix),l ;IY
	ld	11(ix),h
	exx	;Main HLDEBC
	ex	af,af
	pop	ix
	ret
	CALL_END:
	ex	af,af
	pop	hl
	pop	ix
	ret
;OUT_FLAGS:	.db #0
;strcmpi.c:1: int strcmpi(const char *a1, const char *a2) {
;	---------------------------------
; Function strcmpi
; ---------------------------------
_strcmpi::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-11
	add	hl,sp
	ld	sp,hl
;strcmpi.c:4: while((c1=*a1) | (c2=*a2)) {
	ld	a,4 (ix)
	ld	-2 (ix),a
	ld	a,5 (ix)
	ld	-1 (ix),a
	ld	a,6 (ix)
	ld	-6 (ix),a
	ld	a,7 (ix)
	ld	-5 (ix),a
00105$:
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	ld	c,(hl)
	ld	-10 (ix),c
	ld	l,-6 (ix)
	ld	h,-5 (ix)
	ld	a,(hl)
	ld	-11 (ix),a
	or	a,c
	jp	Z,00107$
;strcmpi.c:6: (islower(c1) ? toupper(c1) : c1) != (islower(c2) ? toupper(c2) : c2))
	ld	e,-10 (ix)
	ld	d,#0x00
	ld	a,-11 (ix)
	ld	-4 (ix),a
	ld	-3 (ix),#0x00
;strcmpi.c:5: if (!c1 || !c2 || /* Unneccesary? */
	ld	a,-10 (ix)
	or	a, a
	jr	Z,00101$
	ld	a,-11 (ix)
	or	a, a
	jr	Z,00101$
;strcmpi.c:6: (islower(c1) ? toupper(c1) : c1) != (islower(c2) ? toupper(c2) : c2))
	ld	c, e
;C:/Program Files/SDCC/bin/../include/ctype.h:71: return ((unsigned char)c >= 'a' && (unsigned char)c <= 'z');
	ld	a,c
	sub	a, #0x61
	jr	C,00112$
	ld	a,#0x7a
	sub	a, c
	jr	C,00112$
;strcmpi.c:6: (islower(c1) ? toupper(c1) : c1) != (islower(c2) ? toupper(c2) : c2))
	push	de
	push	de
	call	_toupper
	pop	af
	ld	c,l
	ld	b,h
	pop	de
	jr	00113$
00112$:
	ld	c, e
	ld	b, d
00113$:
	ld	l,-4 (ix)
;C:/Program Files/SDCC/bin/../include/ctype.h:71: return ((unsigned char)c >= 'a' && (unsigned char)c <= 'z');
	ld	a,l
	sub	a, #0x61
	jr	C,00117$
	ld	a,#0x7a
	sub	a, l
	jr	C,00117$
;strcmpi.c:6: (islower(c1) ? toupper(c1) : c1) != (islower(c2) ? toupper(c2) : c2))
	push	bc
	push	de
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	call	_toupper
	pop	af
	pop	de
	pop	bc
	ld	-9 (ix),l
	ld	-8 (ix),h
	jr	00118$
00117$:
	ld	a,-4 (ix)
	ld	-9 (ix),a
	ld	a,-3 (ix)
	ld	-8 (ix),a
00118$:
	ld	a,c
	sub	a, -9 (ix)
	jr	NZ,00150$
	ld	a,b
	sub	a, -8 (ix)
	jr	Z,00102$
00150$:
00101$:
;strcmpi.c:7: return (c1 - c2);
	ld	a,e
	sub	a, -4 (ix)
	ld	l,a
	ld	a,d
	sbc	a, -3 (ix)
	ld	h,a
	jr	00110$
00102$:
;strcmpi.c:8: a1++;
	inc	-2 (ix)
	jr	NZ,00151$
	inc	-1 (ix)
00151$:
;strcmpi.c:9: a2++;
	inc	-6 (ix)
	jp	NZ,00105$
	inc	-5 (ix)
	jp	00105$
00107$:
;strcmpi.c:11: return 0;
	ld	hl,#0x0000
00110$:
	ld	sp, ix
	pop	ix
	ret
	.area _CODE
___str_30:
	.ascii "Disk image emulation data file creation tool for Nextor v1.1"
	.db 0x0d
	.db 0x0a
	.ascii "By Konamiman, 3/2019"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_31:
	.ascii "Usage: emufile [<options>] <output file> <files> [<files> .."
	.ascii ".]"
	.db 0x0d
	.db 0x0a
	.ascii "       emufile set <data file> [<device index> [<LUN ind"
	.ascii "ex>]]"
	.db 0x0d
	.db 0x0a
	.ascii "       emufile ?"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_32:
	.ascii "* To create an emulation data file:"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.ascii "emufile [<options>] <"
	.ascii "output file> <files> [<files> ...]"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.ascii "<output file>: Path an"
	.ascii "d name of emulation data file to create."
	.db 0x0d
	.db 0x0a
	.ascii "               Def"
	.ascii "ault extension is .EMU"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.ascii "<options>:"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.ascii "-b <number>: The ind"
	.ascii "ex of the image file to mount at boot time."
	.db 0x0d
	.db 0x0a
	.ascii "             Mu"
	.ascii "st be 1-9 or A-W. Default is 1."
	.db 0x0d
	.db 0x0a
	.ascii "-a <address>: Page 3 addres"
	.ascii "s for the 16 byte work area."
	.db 0x0d
	.db 0x0a
	.ascii "              Must be a hexade"
	.ascii "cimal number between C000 and FFEF."
	.db 0x0d
	.db 0x0a
	.ascii "              If missin"
	.ascii "g or 0, the work area is allocated at boot time."
	.db 0x0d
	.db 0x0a
	.ascii "-p : Print"
	.ascii " filenames and associated keys."
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.ascii "TYPE /B the generated fil"
	.ascii "e to see the names of the registered files."
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.ascii "* To setup "
	.ascii "an existing emulation data file for booting:"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.ascii "emufile set "
	.ascii "<data file> [r] [<device index> [<LUN index>]]"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.ascii "Use <devic"
	.ascii "e index> and <LUN index> to specify the device whose"
	.db 0x0d
	.db 0x0a
	.ascii "partit"
	.ascii "ion table will be written. Default is the device where the"
	.db 0x0d
	.db 0x0a
	.ascii "emulation data file is located."
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.ascii "r: reset the computer aft"
	.ascii "er successfully finishing the setup."
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_33:
	.ascii "Invalid parameter"
	.db 0x00
___str_34:
	.db 0x0d
	.db 0x0a
	.db 0x00
	.area _INITIALIZER
__xinit__strTitle:
	.dw ___str_30
__xinit__strUsage:
	.dw ___str_31
__xinit__strHelp:
	.dw ___str_32
__xinit__strInvParam:
	.dw ___str_33
__xinit__strCRLF:
	.dw ___str_34
	.area _CABS (ABS)
