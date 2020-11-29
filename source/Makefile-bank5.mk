
all: b5.bin fdisk_crt0.rel fdisk.dat fdisk2.dat
	@:

TOOLS_SRC := ../../../../source/tools/C/

.ONESHELL:

b5.hex: codes.rel kvar.rel data.rel b5.rel chgbnk.rel
	@l80.sh b5.hex /P:40FF,CODES,KVAR,DATA,B5,/p:7fd0,chgbnk,B5/N/X/Y/E

b5.bin: b5.hex
	@rm -f b5.bin
	hex2bin -s 4000 b5.hex

fdisk.ihx: fdisk.c fdisk_crt0.rel fdisk.c $(TOOLS_SRC)AsmCall.h fdisk.h $(TOOLS_SRC)asm.h $(TOOLS_SRC)system.h $(TOOLS_SRC)dos.h $(TOOLS_SRC)types.h $(TOOLS_SRC)partit.h drivercall.h
	sdcc -DMAKEBUILD -I$(TOOLS_SRC) --code-loc 0x4120 --data-loc 0x8020 -mz80 --disable-warning 196 --disable-warning 84 --disable-warning 85 --max-allocs-per-node 10000 --allow-unsafe-read --opt-code-size --no-std-crt0 fdisk_crt0.rel fdisk.c

fdisk.dat: fdisk.ihx
	hex2bin -e dat fdisk.ihx

fdisk2.ihx: fdisk2.c fdisk_crt0.rel fdisk.c $(TOOLS_SRC)AsmCall.h fdisk.h $(TOOLS_SRC)asm.h $(TOOLS_SRC)system.h $(TOOLS_SRC)dos.h $(TOOLS_SRC)types.h $(TOOLS_SRC)partit.h drivercall.h
	sdcc -DMAKEBUILD -I$(TOOLS_SRC) --code-loc 0x4120 --data-loc 0xA000 -mz80 --disable-warning 196 --disable-warning 84 --disable-warning 85 --max-allocs-per-node 10000 --allow-unsafe-read --opt-code-size --no-std-crt0 fdisk_crt0.rel fdisk2.c

fdisk2.dat: fdisk2.ihx
	hex2bin -e dat fdisk2.ihx

include rules.mk

