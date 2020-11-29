
all: b1.bin
	@:

.ONESHELL:

b1.rel: b0labels.inc

b1.hex: codes.rel kvar.rel data.rel b1.rel dosinit.rel mapinit.rel alloc.rel msg.rel chgbnk.rel
	@l80.sh b1.hex /P:40FF,CODES,KVAR,DATA,B1,DOSINIT,MAPINIT,ALLOC,MSG,/p:7fd0,chgbnk,B1/N/X/Y/E

b1.bin: b1.hex
	@rm -f b1.bin
	hex2bin -s 4000 b1.hex

include rules.mk

