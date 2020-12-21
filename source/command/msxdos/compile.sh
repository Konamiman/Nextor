#!/bin/sh

hex2bin() {
    objcopy -I ihex -O binary $1 $2
}

set -e

export X80_COMMAND_LINE="-t -nb"
export M80_COMMAND_LINE="-8"

cp ../../kernel/*.inc .
cp ../../kernel/codes.mac .
cp ../../kernel/data.mac .

for file in CODES DATA KMSG MESSAGES REAL REF RELOC VER END; do M80 =$file; done
cp NOKMSG.MAC USEKMSG.MAC

M80 =SYS
L80 /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,END,NEXTOR/n/x/y/e
cp YESKMSG.MAC USEKMSG.MAC

M80 =SYS
L80 /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,KMSG,END,NEXTORK/n/x/y/e
rm USEKMSG.MAC

hex2bin nextor.hex NEXTOR.SYS
hex2bin nextork.hex NEXTORK.SYS

mkdir -p ../../../bin/tools
cp NEXTOR.SYS ../../../bin/tools
cp NEXTORK.SYS ../../../bin/tools/NEXTOR.SYS.japanese

rm -f *.bin
rm -f *.hex
rm -f *.rel
rm -f *.sym

echo
echo Build successful!
echo
