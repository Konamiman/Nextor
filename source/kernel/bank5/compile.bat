@echo off
cls

call compfdsk.bat
if errorlevel 1 goto :end

for /R %%I in (..\..\..\bin\kernels\*.*) do (
dd if=fdisk.dat of=%%I bs=1 count=16000 seek=82176
dd if=fdisk2.dat of=%%I bs=1 count=8000 seek=98560
)

:end

