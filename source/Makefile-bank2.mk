
all: b2.bin b2labels.inc
	@:

.ONESHELL:

temp21.rel: char.rel dev.rel kbios.rel misc.rel seg.rel
	@lib80.sh temp21.rel TEMP21=char.rel,dev.rel,kbios.rel,misc.rel,seg.rel/E

temp22.rel: path.rel find.rel dir.rel handles.rel del.rel rw.rel files.rel
	@lib80.sh TEMP22.rel TEMP22=path.rel,find.rel,dir.rel,handles.rel,del.rel,rw.rel,files.rel/E

temp23.rel: buf.rel fat.rel val.rel err.rel
	@lib80.sh temp23.rel TEMP23=buf.rel,fat.rel,val.rel,err.rel/E

b2.rel: b0labels.inc

b2.hex: temp21.rel temp22.rel temp23.rel codes.rel kvar.rel data.rel b2.rel kinit.rel chgbnk.rel
	@l80.sh b2.hex /P:40FF,CODES,KVAR,DATA,B2,KINIT,TEMP21,TEMP22,TEMP23,/p:7fd0,chgbnk,B2/N/X/Y/E

b2.bin: b2.hex
	@rm -f b2.bin
	hex2bin -s 4000 b2.hex

b2labels.inc: b2.hex
	@symtoequs.sh b2.sym b2labels.inc "\?\S*"

include rules.mk

