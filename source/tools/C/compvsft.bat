@echo off
cls
sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196 --no-std-crt0 crt0msx_msxdos_advanced.rel msxchar.lib asm.lib vsft.c
if errorlevel 1 goto :end
hex2bin -e com vsft.ihx
copy vsft.com ..\..\..\bin\tools\
:end
