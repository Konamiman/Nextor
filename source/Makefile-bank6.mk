
all: b6.bin
	@:

.ONESHELL:

b6.hex: codes.rel kvar.rel data.rel b6.rel chgbnk.rel
	@l80.sh b6.hex /P:40FF,CODES,KVAR,DATA,B6,/p:7fd0,chgbnk,B6/N/X/Y/E

b6.bin: b6.hex
	@rm -f b6.bin
	hex2bin -s 4000 b6.hex

include rules.mk

