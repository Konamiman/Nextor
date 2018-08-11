@echo off
cls
copy ..\..\kernel\*.inc
copy ..\..\kernel\codes.mac
copy ..\..\kernel\data.mac
for %%A in (CODES,DATA,START,CLI,CMD,COPY,DIRS,FILES,IO,JTEXT,MESSAGES,MISC,VAR,VER) do cpm32 M80 =%%A
cpm32 l80 /P:100,CODES,DATA,START,CLI,CMD,COPY,DIRS,FILES,IO,JTEXT,MESSAGES,MISC,VAR,VER,COMMAND2/n/x/y/e
hex2bin -s 100 command2.hex
del command2.com
ren command2.bin COMMAND2.COM
del *.bin
del *.hex
del *.rel
del *.sym
if not exist ..\..\..\bin\tools md ..\..\..\bin\tools
copy COMMAND2.COM ..\..\..\bin\tools
