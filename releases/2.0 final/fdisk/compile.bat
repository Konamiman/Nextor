@echo off
setlocal
set nextorfile="nextor.rom"
cls

if not exist %nextorfile% (
	echo %nextorfile% not found!
	goto :end
)

FOR /F "tokens=*" %%A IN ("%nextorfile%") DO set size=%%~zA
if %size% LSS 114688 (
	echo %nextorfile% is too small! At least 112K expected
	goto :end
)

sdasz80 -o fdisk_crt0.rel fdisk_crt0.s
if errorlevel 1 goto :end

sdcc --code-loc 0x4120 --data-loc 0x8020 -mz80 --disable-warning 196 --disable-warning 84 --disable-warning 85 --no-std-crt0 fdisk_crt0.rel msxchar.lib fdisk.c
if errorlevel 1 goto :end
hex2bin -e dat fdisk.ihx

sdcc --code-loc 0x4120 --data-loc 0xA000 -mz80 --disable-warning 196 --disable-warning 84 --disable-warning 85 --no-std-crt0 fdisk_crt0.rel msxchar.lib fdisk2.c
if errorlevel 1 goto :end
hex2bin -e dat fdisk2.ihx

dd if=fdisk.dat of=%nextorfile% bs=1 count=16000 seek=82176
dd if=fdisk2.dat of=%nextorfile% bs=1 count=8000 seek=98560

:end

del *.sym 2>nul
del *.ihx 2>nul
del *.lst 2>nul
del *.lk 2>nul
del *.noi 2>nul
del *.rel 2>nul
del *.asm 2>nul
del *.map 2>nul
