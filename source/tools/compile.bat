@echo off
cls

copy ..\msxdos25\macros.inc
copy ..\msxdos25\const.inc
copy ..\msxdos25\codes.mac
copy ..\msxdos25\data.mac
for %%A in (CODES,DATA,SHARED) do cpm32 M80 =%%A
del codes.mac
del data.mac

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
copy %%~nf.COM ..\..\bin\tools
) else (echo *** ERROR: %%~nf not compiled)
)
)

del *.bin
del *.hex
del *.rel
rem del *.sym
:end
