#!/bin/sh

hex2bin() {
    objcopy -I ihex -O binary $1 $2
}

BuildTool() {
    echo "***"
    echo "*** $1"
    echo "***"
    echo
    sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196 --disable-warning 85 --no-std-crt0 crt0_msxdos_advanced.rel $1.c
    hex2bin $1.ihx $1.COM
    cp $1.COM ../../../bin/tools
}

set -e

mkdir -p ../../../bin/tools

if [ -f "crt0_msxdos_advanced.relx" ]; then
    cp -p crt0_msxdos_advanced.relx crt0_msxdos_advanced.rel
fi

if [ ! -f "crt0_msxdos_advanced.rel" ] || [ "crt0_msxdos_advanced.s" -nt "crt0_msxdos_advanced.rel" ]; then
    echo ----- Compiling crt0_msxdos_advanced ...
    sdasz80 -o crt0_msxdos_advanced.rel crt0_msxdos_advanced.s
    cp crt0_msxdos_advanced.rel crt0_msxdos_advanced.relx
fi

if [ -z "$1" ]; then
    for file in $(find *.c ! -name printf.c ! -name AsmCall.c ! -name strcmpi.c | sed 's/\.c//'); do
        BuildTool $file
    done
else
    BuildTool $1
fi

for ext in asm hex ihx lk lst map noi sym rel; do rm -f *.$ext; done

echo
echo Build completed succeesfully!
