hex2bin() {
    objcopy -I ihex -O binary $1 $2
}

BuildTool() {
    echo "***"
    echo "*** $1"
    echo "***"
    M80 =$1
    L80 /P:100,CODES,DATA,$1,SHARED,$1/n/x/y/e
    hex2bin $1.hex $1.COM
    cp $1.COM ../../bin/tools
}

set -e

export X80_COMMAND_LINE="-t -nb"
export M80_COMMAND_LINE="-8"

mkdir -p ../../bin/tools

#copy ..\kernel\macros.inc
#copy ..\kernel\const.inc
#copy ..\kernel\codes.mac
#copy ..\kernel\data.mac
for file in CODES DATA SHARED; do M80 -p ../kernel =$file; done

if [ -z "$1" ]; then
    for file in $(find *.MAC ! -name SHARED.MAC | sed 's/\.MAC//'); do
        BuildTool $file
    done
else
    BuildTool $1
fi

for ext in REL HEX SYM; do rm -f *.$ext; done

echo
echo Build completed succseefully!
