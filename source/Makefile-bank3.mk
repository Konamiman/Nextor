
all: b3.bin
	@:

.ONESHELL:

b3.rel: b0labels.inc

b3.hex: codes.rel kbdos.rel kvar.rel data.rel doshead.rel 40ff.rel dos1ker.rel b3.rel chgbnk.rel drv.rel
	@l80.sh b3.hex /p:4000,CODES,KBDOS,KVAR,DATA,DOSHEAD,40FF,B3,DOS1KER,/p:7700,drv,/p:7fd0,chgbnk,b3/N/X/Y/E

b3.bin: b3.hex
	@rm -f b3.bin
	hex2bin -s 4000 b3.hex

include rules.mk

