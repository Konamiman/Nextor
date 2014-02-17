@echo off
cls

copy ..\..\msxdos25\*.inc
copy ..\..\msxdos25\codes.mac
copy ..\..\msxdos25\data.mac
for %%A in (CODES,DATA,KMSG,MESSAGES,REAL,REF,RELOC,SYS,VER,END) do cpm32 M80 =%%A
cpm32 l80 /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,END,NEXTOR/n/x/y/e
hex2bin -s 100 nextor.hex
del nextor.sys
ren nextor.bin NEXTOR.SYS
del *.bin
del *.hex
del *.rel
del *.sym
if not exist ..\..\..\bin\tools md ..\..\..\bin\tools
copy NEXTOR.SYS ..\..\..\bin\tools

