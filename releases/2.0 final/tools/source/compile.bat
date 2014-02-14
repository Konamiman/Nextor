@echo off
cls

if not exist bin md bin

if .%1==. (
    set files=*.mac
	del bin\*.COM 2>nul
) else if exist %1.mac (
    set files=%1.mac
	del bin\%1.COM 2>nul
) else (
    echo *** %1.MAC not found!
    goto :end
)

cpm32 M80 =SHARED.MAC
cpm32 M80 =CODES.MAC

for /r %%f in (%files%) do (
    if not %%~nf==SHARED (if not %%~nf==CODES (
    echo .
    echo ***
    echo *** %%~nf
    echo ***
    echo .
    cpm32 M80 =%%~nf
    cpm32 l80 /P:100,CODES,%%~nf,SHARED,%%~nf/n/x/y/e
    if exist %%~nf.hex (
        hex2bin -s 100 %%~nf.hex
        ren %%~nf.bin %%~nf.COM
        move %%~nf.COM bin\
    ) else (
        echo *** ERROR: %%~nf not compiled
		goto :end
    )
	))
)

:end
del *.bin 2>nul
del *.hex 2>nul
del *.rel 2>nul
del *.sym 2>nul
del *.rel 2>nul


