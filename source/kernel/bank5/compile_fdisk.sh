#!/bin/sh

hex2bin() {
    objcopy -I ihex -O binary $1 $2
}

CompileFdisk() {
    echo ----- Compiling FDISK$1...
    sdcc --code-loc 0x4120 --data-loc 0x8020 -mz80 --disable-warning 196 --disable-warning 84 --disable-warning 85 --max-allocs-per-node 1000 --allow-unsafe-read --opt-code-size --no-std-crt0 fdisk_crt0.rel fdisk$1.c
    hex2bin fdisk$1.ihx fdisk$1.dat
}

set -e

if [ -f "fdisk_crt0.relx" ]; then
    cp -p fdisk_crt0.relx fdisk_crt0.rel
fi

if [ ! -f "fdisk_crt0.rel" ] || [ "fdisk_crt0.s" -nt "fdisk_crt0.rel" ]; then
    echo ----- Compiling FDISK_CRT0 ...
    sdasz80 -o fdisk_crt0.rel fdisk_crt0.s
    CompileFdisk
    CompileFdisk 2
    cp -p fdisk_crt0.rel fdisk_crt0.relx
fi

if [ ! -f "fdisk.dat" ] || [ "fdisk.c" -nt "fdisk.dat" ]; then
    CompileFdisk
fi

if [ ! -f "fdisk2.dat" ] || [ "fdisk2.c" -nt "fdisk2.dat" ]; then
    CompileFdisk 2
fi

for ext in ihx lk lst map noi asm sym; do rm -f *.$ext; done

