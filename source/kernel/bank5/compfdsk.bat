echo Compiling, please be patient...

sdasz80 -o fdisk_crt0.rel fdisk_crt0.s
if errorlevel 1 goto :error

sdcc --code-loc 0x4120 --data-loc 0x8020 -mz80 --disable-warning 196 --disable-warning 84 --disable-warning 85 --no-std-crt0 fdisk_crt0.rel msxchar.lib fdisk.c
if errorlevel 1 goto :error
hex2bin -e dat fdisk.ihx

sdcc --code-loc 0x4120 --data-loc 0xA000 -mz80 --disable-warning 196 --disable-warning 84 --disable-warning 85 --no-std-crt0 fdisk_crt0.rel msxchar.lib fdisk2.c
if errorlevel 1 goto :error
hex2bin -e dat fdisk2.ihx

goto :end

:error
exit /b 1

:end
