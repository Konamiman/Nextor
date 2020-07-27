@echo off
cls

copy ..\..\kernel\*.inc
copy ..\..\kernel\codes.mac
copy ..\..\kernel\data.mac
for %%A in (CODES,DATA,KMSG,MESSAGES,REAL,REF,RELOC,VER,END) do cpm32 M80 =%%A
copy NOKMSG.MAC USEKMSG.MAC
cpm32 M80 =SYS
cpm32 l80 /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,END,NEXTOR/n/x/y/e
copy YESKMSG.MAC USEKMSG.MAC
cpm32 M80 =SYS
cpm32 l80 /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,KMSG,END,NEXTORK/n/x/y/e
del USEKMSG.MAC
hex2bin -s 100 nextor.hex
hex2bin -s 100 nextork.hex
del nextor.sys
del nextork.sys
ren nextor.bin NEXTOR.SYS
ren nextork.bin NEXTORK.SYS
del *.bin
del *.hex
del *.rel
del *.sym
if not exist ..\..\..\bin\tools md ..\..\..\bin\tools
copy NEXTOR.SYS ..\..\..\bin\tools
copy NEXTORK.SYS ..\..\..\bin\tools
