
all: b4.bin b4rd.bin
	@:

.ONESHELL:

b4.rel: b0labels.inc

b4.hex: codes.rel kvar.rel data.rel b4.rel jump.rel env.rel cpm.rel partit.rel ramdrv.rel time.rel seg.rel misc.rel dskab.rel chgbnk.rel
	@l80.sh b4.hex /P:40FF,CODES,KVAR,DATA,B4,JUMP,ENV,CPM,PARTIT,RAMDRV,TIME,SEG,MISC,/p:7bc0,DSKAB,/p:7fd0,chgbnk,B4/N/X/Y/E

b4.bin: b4.hex
	@rm -f b4.bin
	hex2bin -s 4000 b4.hex

b4rdlabs.inc: b4.hex
	@symtoequs.sh b4.sym b4rdlabs.inc "R4_[1-9]"

ramdrvh.rel: b4rdlabs.inc

b4rd.hex: ramdrvh.rel
	@l80.sh b4rd.hex /P:4080,RAMDRVH,B4RD/N/X/Y/E

b4rd.bin: b4rd.hex
	@hex2bin -s 4080 b4rd.hex

include rules.mk

