#!/bin/sh

VERSION="2.1.1-alpha2"

hex2bin_full() {
    objcopy -I ihex -O binary $1 $2
}

hex2bin() {
    hex2bin_full $1.hex $1.bin
}

SymToEqus() {
    REGEX="([0-9A-F]{4}) ($3)"
    cat $1 | grep -Eo "$REGEX" | sed -r "s/$REGEX/\2 equ \1h\n\tpublic \2\n/g" > $2
}

set -e

export X80_COMMAND_LINE="-t -nb"
export M80_COMMAND_LINE="-8"

if [ "$1" != "drivers" ]; then

echo
echo "****************"
echo "***  COMMON  ***"
echo "****************"
echo

mkdir -p ../../bin/kernels

#cp *.inc bank0/
#cp *.inc bank1/
#cp *.inc bank2/
#cp *.inc bank3/
#cp *.inc bank4/
#cp *.inc bank5/
#cp *.inc bank6/
#cp *.inc drivers/

for file in CODES KVAR DATA REL CHGBNK DRV; do M80 =$file; done

echo
echo "****************"
echo "***  BANK 0  ***"
echo "****************"
echo

cd bank0
#cp ../codes.rel .
#cp ../kvar.rel .
#cp ../data.rel .
#cp ../rel.rel .
#cp ../chgbnk.rel .
#cp ../drv.rel .
for file in DOSHEAD 40FF B0 INIT ALLOC DSKBASIC DOSBOOT BDOS RAMDRV; do M80 -p .. =$file; done
L80 -p .. /p:4000,CODES,KVAR,DATA,REL,DOSHEAD,40FF,B0,INIT,ALLOC,DSKBASIC,DOSBOOT,BDOS,RAMDRV,/p:7700,drv,/p:7fd0,chgbnk,b0/n/x/y/e
hex2bin b0
SymToEqus b0.sym b0labels.inc "[?][^[:space:]]+|DOSV0|GETERR|BDOSE|KDERR|KABR|C4PBK"
SymToEqus b0.sym b0lab_b3.inc "INIT|TIMINT|MAPBIO|GWRK|R_[^[:space:]]+"

echo
echo "****************"
echo "***  BANK 1  ***"
echo "****************"
echo

cd ../bank1
#cp ../codes.rel .
#cp ../kvar.rel .
#cp ../data.rel .
#cp ../chgbnk.rel .
#cp ../bank0/alloc.rel .
#cp ../bank0/b0labels.inc .
for file in B1 DOSINIT MAPINIT MSG; do M80 -p ..,../bank0 =$file; done
L80 -p ..,../bank0 /P:40FF,CODES,KVAR,DATA,B1,DOSINIT,MAPINIT,ALLOC,MSG,/p:7fd0,chgbnk,B1/N/X/Y/E
hex2bin b1

echo 
echo "****************"
echo "***  BANK 2  ***"
echo "****************"
echo 

cd ../bank2
#cp ../codes.rel .
#cp ../kvar.rel .
#cp ../data.rel .
#cp ../chgbnk.rel .
#cp ../bank0/b0labels.inc .
for file in KINIT CHAR DEV KBIOS MISC SEG PATH FIND DIR HANDLES DEL RW FILES BUF FAT VAL ERR B2; do M80 -p ..,../bank0 =$file; done
LIB80 TEMP21=char.rel,dev.rel,kbios.rel,misc.rel,seg.rel/E
LIB80 TEMP22=path.rel,find.rel,dir.rel,handles.rel,del.rel,rw.rel,files.rel/E
LIB80 TEMP23=buf.rel,fat.rel,val.rel,err.rel/E
L80 -p .. /P:40FF,CODES,KVAR,DATA,B2,KINIT,TEMP21,TEMP22,TEMP23,/p:7fd0,chgbnk,B2/N/X/Y/E
rm TEMP21.REL
rm TEMP22.REL
rm TEMP23.REL
hex2bin b2
SymToEqus b2.sym b2labels.inc "[?][^[:space:]]+"

echo
echo "****************"
echo "***  BANK 3  ***"
echo "****************"
echo

cd ../bank3
#cp ../drv.rel .
#cp ../codes.rel .
#cp ../kvar.rel .
#cp ../data.rel .
#cp ../chgbnk.rel .
#cp ../bank0/doshead.rel .
#cp ../bank0/40FF.rel .
cp ../bank0/b0lab_b3.inc b0labels.inc
for file in DOS1KER B3; do M80 -p ..,../bank0 =$file; done
L80 -p ..,../bank0 /p:4000,CODES,KVAR,DATA,DOSHEAD,40FF,B3,DOS1KER,/p:7700,drv,/p:7fd0,chgbnk,b3/N/X/Y/E
hex2bin b3

echo
echo "****************"
echo "***  BANK 4  ***"
echo "****************"
echo

cd ../bank4
#cp ../codes.rel .
#cp ../kvar.rel .
#cp ../data.rel .
#cp ../chgbnk.rel .
#cp ../bank2/b2labels.inc .
#cp ../bank0/b0labels.inc .
for file in B4 JUMP ENV CPM PARTIT RAMDRV TIME SEG MISC DSKAB; do M80 -p ..,../bank0,../bank2 =$file; done
L80 -p .. /P:40FF,CODES,KVAR,DATA,B4,JUMP,ENV,CPM,PARTIT,RAMDRV,TIME,SEG,MISC,/p:7bc0,DSKAB,/p:7fd0,chgbnk,B4/N/X/Y/E
hex2bin b4
SymToEqus b4.sym b4rdlabs.inc "R4_[1-9]"
M80 -p .. =RAMDRVH
L80 -p .. /P:4080,RAMDRVH,B4RD/N/X/Y/E
hex2bin b4rd

echo
echo "****************"
echo "***  BANK 5  ***"
echo "****************"
echo

cd ../bank5
sh ./compile_fdisk.sh

#cp ../codes.rel .
#cp ../kvar.rel .
#cp ../data.rel .
#cp ../chgbnk.rel .
M80 -p .. =B5
L80 -p .. /P:40FF,CODES,KVAR,DATA,B5,/p:7fd0,chgbnk,B5/N/X/Y/E
hex2bin b5

echo
echo "****************"
echo "***  BANK 6  ***"
echo "****************"
echo

cd ../bank6
#cp ../codes.rel .
#cp ../kvar.rel .
#cp ../data.rel .
#cp ../chgbnk.rel .
M80 -p .. =B6
L80 -p .. /P:40FF,CODES,KVAR,DATA,B6,/p:7fd0,chgbnk,B6/N/X/Y/E
hex2bin b6

echo
echo "*******************"
echo "***  BASE FILE  ***"
echo "*******************"
echo

cd ..
dd if=/dev/zero of=255.bytes bs=1 count=255
cat bank0/b0.bin 255.bytes bank1/b1.bin 255.bytes bank2/b2.bin bank3/b3.bin 255.bytes bank4/b4.bin 255.bytes bank5/b5.bin 255.bytes bank6/b6.bin > nextor_base.dat
rm 255.bytes
dd conv=notrunc if=nextor_base.dat of=doshead.bin bs=1 count=255
dd conv=notrunc if=doshead.bin of=nextor_base.dat bs=1 count=255 seek=16k
dd conv=notrunc if=doshead.bin of=nextor_base.dat bs=1 count=255 seek=32k
dd conv=notrunc if=doshead.bin of=nextor_base.dat bs=1 count=255 seek=64k
dd conv=notrunc if=doshead.bin of=nextor_base.dat bs=1 count=255 seek=80k
dd conv=notrunc if=doshead.bin of=nextor_base.dat bs=1 count=255 seek=96k
dd conv=notrunc if=bank5/fdisk.dat of=nextor_base.dat bs=1 count=16000 seek=82176
dd conv=notrunc if=bank5/fdisk2.dat of=nextor_base.dat bs=1 count=8000 seek=98560
dd conv=notrunc if=bank4/b4rd.bin of=nextor_base.dat bs=1 count=15 seek=65664
cp nextor_base.dat ../../bin/kernels/Nextor-$VERSION.base.dat

fi #if [ $1 != "drivers" ];

echo
echo "*****************"
echo "***  DRIVERS  ***"
echo "*****************"
echo

cd drivers

for d in $(ls -d */ | sed 's#/##'); do
    if [ ! -f "$d/driver.mac" ]; then continue; fi

    echo 
    echo "***"
    echo "***  Driver: $d"
    echo "***"
    echo 

    cd $d
    #cp ../../*.inc .
    #cp ../../codes.rel .
    #cp ../../kvar.rel .
    #cp ../../data.rel .
    #cp ../../chgbnk.rel .
    #cp ../../bank6/b6.mac .
    #cp ../../bank0/b0labels.inc .
    for file in B6 DRIVER CHGBNK; do M80 -p ../..,../../bank0,../../bank6 =$file; done
    L80 -p ../../ /P:4100,DRIVER,DRIVER/N/X/Y/E
    L80 /P:7fd0,CHGBNK,CHGBNK/N/X/Y/E
    hex2bin driver
    hex2bin CHGBNK

    dd if=/dev/zero of=256.bytes bs=1 count=256
    cat 256.bytes driver.bin > _driver.bin
    mknexrom  ../../nextor_base.dat Nextor-$VERSION.$d.ROM /d:_driver.bin /m:chgbnk.bin
    cp Nextor-$VERSION.$d.ROM ../../../../bin/kernels
    rm -f *.rom
    cd ..
done

echo
echo "***"
echo "***  Driver: MegaFlashROM SD SCC+"
echo "***"
echo

cd MegaFlashRomSD

M80 -p ../StandaloneASCII8 =CHGBNK
L80 /P:7fd0,CHGBNK,CHGBNK/N/X/Y/E
hex2bin CHGBNK
mknexrom ../../nextor_base.dat nextor2.rom /d:driver-1slot.dat /m:CHGBNK.bin
cp nextor2.rom ../../../../bin/kernels/Nextor-$VERSION.MegaFlashSDSCC.1-slot.ROM
sjasm makerecoverykernel.asm kernel.rom
cp kernel.rom ../../../../bin/kernels/Nextor-$VERSION.MegaFlashSDSCC.1-slot.Recovery.ROM
rm nextor2.rom
mknexrom ../../nextor_base.dat nextor2.rom /d:driver-2slots.dat /m:CHGBNK.bin
cp nextor2.rom ../../../../bin/kernels/Nextor-$VERSION.MegaFlashSDSCC.2-slots.ROM
sjasm makerecoverykernel.asm kernel.rom
cp kernel.rom ../../../../bin/kernels/Nextor-$VERSION.MegaFlashSDSCC.2-slots.Recovery.ROM
cd ..

echo
echo "***"
echo "***  Driver: Sunrise IDE (for emulators)"
echo "***"
echo

cd SunriseIDE

rm -f ../../../../bin/kernels/Nextor-$VERSION.SunriseIDE.emulators.ROM
mv ../../../../bin/kernels/Nextor-$VERSION.SunriseIDE.ROM ../../../../bin/kernels/Nextor-$VERSION.SunriseIDE.emulators.ROM 
sjasm -c sunride.asm driver.bin
mknexrom ../../nextor_base.dat nextor2.rom /d:driver.bin /m:chgbnk.bin
cp nextor2.rom ../../../../bin/kernels/Nextor-$VERSION.SunriseIDE.ROM
cd ..

echo
echo "***"
echo "***  Driver: Flashjacks"
echo "***"
echo

cd Flashjacks
sjasm flashjacks.asm driver.bin
mknexrom ../../nextor_base.dat nextor2.rom /d:driver.bin /m:chgbnk.dat
cp nextor2.rom ../../../../bin/kernels/Nextor-$VERSION.Flashjacks.ROM
cd ..

echo
echo "*****************"
echo "***  CLEANUP  ***"
echo "*****************"
echo

cd ..
for ext in bin HEX REL rel SYM sym bytes; do find . -type f -name "*.$ext" -delete; done
for n in 0 1 2 3 4 5 6; do rm -f bank$n/*.inc; done
for ext in lst rom ROM; do find ./drivers -type f -name "*.$ext" -delete; done

echo Build completed successfully!
echo
