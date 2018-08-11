@echo off
cls

copy ..\kernel\macros.inc
copy ..\kernel\const.inc
copy ..\kernel\codes.mac
copy ..\kernel\data.mac
for %%A in (CODES,DATA,SHARED) do cpm32 M80 =%%A
del codes.mac
del data.mac

vfd ulink 0
vfd link 0 x /L
vfd open 0: utils.dsk /w /720

if .%1==. (
set files=*.mac
) else (
set files=%1.mac
)

for /r %%f in (%files%) do (

if not %%~nf==SHARED (
echo .
echo ***
echo *** %%~nf
echo ***
echo .
cpm32 M80 =%%~nf
cpm32 l80 /P:100,CODES,DATA,%%~nf,SHARED,%%~nf/n/x/y/e
if exist %%~nf.hex (
hex2bin -s 100 %%~nf.hex
if exist %%~nf.COM del %%~nf.COM
ren %%~nf.bin %%~nf.COM
copy %%~nf.COM ..\..\bin
copy %%~nf.COM x:\
) else (echo *** ERROR: %%~nf not compiled)
)
)

vfd close 0
vfd ulink 0
del *.bin
del *.hex
del *.rel
del *.sym
:end
