#!/bin/sh

hex2bin() {
    objcopy -I ihex -O binary $1 $2
}

set -e

export X80_COMMAND_LINE="-t -nb"
export M80_COMMAND_LINE="-8"

#cp ../../kernel/*.inc .
#cp ../../kernel/codes.mac .
#cp ../../kernel/data.mac .

for file in CODES DATA KMSG MESSAGES REAL REF RELOC VER END; do M80 -p ../../kernel =$file; done
cp nokmsg.mac USEKMSG.MAC

M80 -p ../../kernel =SYS
L80 /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,END,NEXTOR/n/x/y/e
cp yeskmsg.mac USEKMSG.MAC

M80 -p ../../kernel =SYS
L80 /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,KMSG,END,NEXTORK/n/x/y/e
rm -f USEKMSG.MAC

hex2bin NEXTOR.HEX NEXTOR.SYS
hex2bin NEXTORK.HEX NEXTORK.SYS

mkdir -p ../../../bin/tools
cp NEXTOR.SYS ../../../bin/tools
cp NEXTORK.SYS ../../../bin/tools/NEXTOR.SYS.japanese

rm -f *.BIN
rm -f *.HEX
rm -f *.REL
rm -f *.SYM

echo
echo Build successful!
echo
