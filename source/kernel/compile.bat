@echo off
cls

if .%1==.drivers goto :drivers

echo .
echo ****************
echo ***  COMMON  ***
echo ****************
echo .

if not exist ..\..\bin\kernels md ..\..\bin\kernels

copy *.inc bank0\
copy *.inc bank1\
copy *.inc bank2\
copy *.inc bank3\
copy *.inc bank4\
copy *.inc bank5\
copy *.inc bank6\
copy *.inc drivers\

for %%A in (CODES,KVAR,DATA,REL,CHGBNK,DRV) do cpm32 M80 =%%A

echo .
echo ****************
echo ***  BANK 0  ***
echo ****************
echo .

cd bank0
copy ..\codes.rel
copy ..\kvar.rel
copy ..\data.rel
copy ..\rel.rel
copy ..\chgbnk.rel
copy ..\drv.rel
for %%A in (DOSHEAD,40FF,B0,INIT,ALLOC,DSKBASIC,DOSBOOT,BDOS,RAMDRV) do cpm32 M80 =%%A
cpm32 l80 /p:4000,CODES,KVAR,DATA,REL,DOSHEAD,40FF,B0,INIT,ALLOC,DSKBASIC,DOSBOOT,BDOS,RAMDRV,/p:781F,drv,/p:7fd0,chgbnk,b0/n/x/y/e
hex2bin b0.hex
..\SymToEqus b0.sym b0labels.inc "\?[^ \t]+|DOSV0|GETERR|BDOSE"
..\SymToEqus b0.sym b0lab_b3.inc "INIT|TIMINT|MAPBIO|GWRK|R_[^ \t]+"

echo .
echo ****************
echo ***  BANK 1  ***
echo ****************
echo .

cd ..\bank1
copy ..\codes.rel
copy ..\kvar.rel
copy ..\data.rel
copy ..\chgbnk.rel
copy ..\bank0\alloc.rel
copy ..\bank0\b0labels.inc
copy ..\calbnk.rel
for %%A in (B1,DOSINIT,MAPINIT,MSG) do cpm32 M80 =%%A
cpm32 L80 /P:40FF,CODES,KVAR,DATA,B1,DOSINIT,MAPINIT,ALLOC,MSG,/p:7fd0,chgbnk,B1/N/X/Y/E
hex2bin -s 4000 b1.hex

echo .
echo ****************
echo ***  BANK 2  ***
echo ****************
echo .

cd ..\bank2
copy ..\codes.rel
copy ..\kvar.rel
copy ..\data.rel
copy ..\chgbnk.rel
copy ..\bank0\b0labels.inc
for %%A in (KINIT,CHAR,DEV,KBIOS,MISC,SEG,PATH,FIND,DIR,HANDLES,DEL,RW,FILES,BUF,FAT,VAL,ERR,B2) do cpm32 M80 =%%A
cpm32 LIB80 TEMP21=char.rel,dev.rel,kbios.rel,misc.rel,seg.rel/E
cpm32 LIB80 TEMP22=path.rel,find.rel,dir.rel,handles.rel,del.rel,rw.rel,files.rel/E
cpm32 LIB80 TEMP23=buf.rel,fat.rel,val.rel,err.rel/E
cpm32 L80 /P:40FF,CODES,KVAR,DATA,B2,KINIT,TEMP21,TEMP22,TEMP23,/p:7fd0,chgbnk,B2/N/X/Y/E
del TEMP21.REL
del TEMP22.REL
del TEMP23.REL
hex2bin -s 4000 b2.hex
..\SymToEqus b2.sym b2labels.inc "\?[^ \t]+"

echo .
echo ****************
echo ***  BANK 3  ***
echo ****************
echo .

cd ..\bank3
copy ..\drv.rel
copy ..\codes.rel
copy ..\kvar.rel
copy ..\data.rel
copy ..\chgbnk.rel
copy ..\bank0\doshead.rel
copy ..\bank0\40FF.rel
copy ..\bank0\b0lab_b3.inc b0labels.inc
for %%A in (DOS1KER,B3) do cpm32 M80 =%%A
cpm32 l80 /p:4000,CODES,KVAR,DATA,DOSHEAD,40FF,B3,DOS1KER,/p:781F,drv,/p:7fd0,chgbnk,b3/N/X/Y/E
rem cpm32 l80 /p:4000,CODES,KVAR,DATA,DOSHEAD,40FF,B3,DOS1KER,drv,/p:7fd0,chgbnk,b3/N/X/Y/E
hex2bin -s 4000 b3.hex

echo .
echo ****************
echo ***  BANK 4  ***
echo ****************
echo .

cd ..\bank4
copy ..\codes.rel
copy ..\kvar.rel
copy ..\data.rel
copy ..\chgbnk.rel
copy ..\bank2\b2labels.inc
copy ..\bank0\b0labels.inc
for %%A in (B4,JUMP,ENV,CPM,BKALLOC,PARTIT,RAMDRV,TIME,SEG,MISC) do cpm32 M80 =%%A
cpm32 L80 /P:40FF,CODES,KVAR,DATA,B4,JUMP,ENV,CPM,BKALLOC,PARTIT,RAMDRV,TIME,SEG,MISC,/p:7fd0,chgbnk,B4/N/X/Y/E
hex2bin -s 4000 b4.hex
..\SymToEqus b4.sym b4rdlabs.inc "R4_[1-9]"
for %%A in (RAMDRVH) do cpm32 M80 =%%A
cpm32 L80 /P:4080,RAMDRVH,B4RD/N/X/Y/E
hex2bin -s 4080 b4rd.hex

echo .
echo ****************
echo ***  BANK 5  ***
echo ****************
echo .

cd ..\bank5

if not exist fdisk.dat (
echo !!! FDISK is not compiled!
call compfdsk.bat
echo !!! FDISK compiled. Resuming kernel compilation now...
)

copy ..\codes.rel
copy ..\kvar.rel
copy ..\data.rel
copy ..\chgbnk.rel
for %%A in (B5) do cpm32 M80 =%%A
cpm32 L80 /P:40FF,CODES,KVAR,DATA,B5,/p:7fd0,chgbnk,B5/N/X/Y/E
hex2bin -s 4000 b5.hex

echo .
echo ****************
echo ***  BANK 6  ***
echo ****************
echo .

cd ..\bank6
copy ..\codes.rel
copy ..\kvar.rel
copy ..\data.rel
copy ..\chgbnk.rel
for %%A in (B6) do cpm32 M80 =%%A
cpm32 L80 /P:40FF,CODES,KVAR,DATA,B6,/p:7fd0,chgbnk,B6/N/X/Y/E
hex2bin -s 4000 b6.hex

echo .
echo *******************
echo ***  BASE FILE  ***
echo *******************
echo .

cd ..
copy /b bank0\b0.bin+bank1\b1.bin+bank2\b2.bin+bank3\b3.bin+bank4\b4.bin+bank5\b5.bin+bank6\b6.bin dos250ba.dat
dd if=dos250ba.dat of=doshead.bin bs=1 count=255
dd if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=16k
dd if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=32k
rem dd if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=48k
dd if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=64k
dd if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=80k
dd if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=96k
dd if=bank5\fdisk.dat of=dos250ba.dat bs=1 count=16000 seek=82176
dd if=bank5\fdisk2.dat of=dos250ba.dat bs=1 count=8000 seek=98560
copy bank4\b4rd.bin
dd if=b4rd.bin of=dos250ba.dat bs=1 count=15 seek=65664
copy dos250ba.dat Nextor-2.1.0-beta1.base.dat
copy dos250ba.dat ..\..\bin\kernels\Nextor-2.1.0-beta1.base.dat

:drivers

echo .
echo *****************
echo ***  DRIVERS  ***
echo *****************
echo .

cd drivers

for /R %%c in (.) do (

if exist %%~nc\driver.mac (

echo .
echo ***
echo ***  Driver: %%~nc
echo ***
echo .

cd %%~nc
copy ..\..\*.inc
copy ..\..\codes.rel
copy ..\..\kvar.rel
copy ..\..\data.rel
copy ..\..\chgbnk.rel
copy ..\..\bank6\b6.mac
copy ..\..\bank0\b0labels.inc
for %%A in (B6,DRIVER,CHGBNK) do cpm32 M80 =%%A
cpm32 L80 /P:4100,DRIVER,DRIVER/N/X/Y/E
cpm32 L80 /P:7fd0,CHGBNK,CHGBNK/N/X/Y/E
hex2bin -s 4000 driver.hex
hex2bin -s 7FD0 CHGBNK.hex

..\..\..\..\wintools\mknexrom  ..\..\Nextor-2.1.0-beta1.base.dat Nextor-2.1.0-beta1.%%~nc.ROM /d:driver.bin /m:chgbnk.bin
copy Nextor-2.1.0-beta1.%%~nc.ROM ..\..\..\..\bin\kernels
del b6.mac
del *.rom
cd ..
)
)

echo .
echo ***
echo ***  Driver: MegaFlashROM SD SCC+
echo ***
echo .

cd MegaFlashRomSD
..\..\..\..\wintools\mknexrom ..\..\Nextor-2.1.0-beta1.base.dat nextor2.rom /d:driver.bin /m:Mapper.ASCII8.bin
copy nextor2.rom ..\..\..\..\bin\kernels\Nextor-2.1.0-beta1.MegaFlashSDSCC.ROM
sjasm makerecoverykernel.asm kernel.dat
copy kernel.dat ..\..\..\..\bin\kernels\Nextor-2.1.0-beta1.MegaFlashSDSCC.Recovery.ROM
cd ..

echo .
echo ***
echo ***  Driver: Sunrise IDE
echo ***
echo .

cd SunriseIDE
sjasm -c sunride.asm driver.bin
..\..\..\..\wintools\mknexrom ..\..\Nextor-2.1.0-beta1.base.dat nextor2.rom /d:driver.bin /m:chgbnk.bin
copy nextor2.rom ..\..\..\..\bin\kernels\Nextor-2.1.0-beta1.SunriseIDE.ROM
cd ..

echo .
echo ***************
echo ***  FINAL  ***
echo ***************
echo .

cd ..

rem del *.bin /s > nul
rem del *.hex /s >nul
rem del *.sym /s > nul
rem for %%A in (bank0, bank1, bank2, bank3, bank4, bank5, drivers) do del %%A\*.inc /s > nul
rem for %%A in (., bank0, bank1, bank2, bank4, drivers) do del %%A\*.rel /s > nul
rem del bank3\chgbnk.rel
rem del bank3\drv.rel

:end
