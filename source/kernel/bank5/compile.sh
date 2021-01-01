#!/bin/sh

set -e

./compile_fdisk.sh

for file in $(find ../../../bin/kernels -name '*.ROM'); do
    echo
    echo Patching $(basename $file) ...
    dd conv=notrunc if=fdisk.dat of=$file bs=1 count=16000 seek=82176
    dd conv=notrunc if=fdisk2.dat of=$file bs=1 count=8000 seek=98560    
done
