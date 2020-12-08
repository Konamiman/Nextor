
all: tools chkdsk.com command2.com nextor.sys nextork.sys nextor-$(VERSION).sunriseide.rom
	@:

.ONESHELL:

MAKEFLAGS += --no-builtin-rules

############################################################
# NEXTOR.SYS and NEXTORK.SYS

nextor.hex: nokmsg.mac codes.rel data.rel reloc.rel ver.rel ref.rel sys.mac real.rel messages.rel end.rel
	@ln -sf nokmsg.mac usekmsg.mac
	m80.sh sys.mac SYS
	l80.sh nextor.hex /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,END,NEXTOR/n/x/y/e

nextor.sys: nextor.hex
	@hex2bin -s 100 nextor.hex
	mv nextor.bin nextor.sys

nextork.hex: yeskmsg.mac codes.rel data.rel reloc.rel ver.rel ref.rel sys.mac real.rel messages.rel kmsg.rel end.rel
	@ln -sf yeskmsg.mac usekmsg.mac
	m80.sh sys.mac SYS
	l80.sh nextork.hex /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,KMSG,END,NEXTORK/n/x/y/e

nextork.sys: nextork.hex
	@hex2bin -s 100 nextork.hex
	mv nextork.bin nextork.sys

############################################################
# command2.com

command2.hex: codes.rel data.rel start.rel cli.rel cmd.rel copy.rel dirs.rel cmdfiles.rel io.rel jtext.rel cmdmsgs.rel cmdmisc.rel var.rel ver.rel
	@l80.sh command2.hex /P:100,CODES,DATA,START,CLI,CMD,COPY,DIRS,CMDFILES,IO,JTEXT,CMDMSGS,CMDMISC,VAR,VER,COMMAND2/n/x/y/e

############################################################
# chkdsk.com

chkdsk.hex: main.rel codes.rel text.rel chjtext.rel chvar.rel chend.rel chmisc.rel nodebug.rel ver.rel chdir.rel
	@l80.sh chkdsk.hex /P:100,CODES,TEXT,CHJTEXT,CHDIR,CHVAR,CHEND,MAIN,CHMISC,NODEBUG,VER,CHKDSK/N/X/Y/E


############################################################
# TOOLS

TOOLS_LIST := drvinfo.com delall.com conclus.com drivers.com fastout.com lock.com mapdrv.com
.PHONY: tools
## Build all the tools binaries into bin/cli
tools: $(TOOLS_LIST)

TOOL_DEPS := codes.rel data.rel shared.rel
define buildtool =
	ls data.rel -l
	l80.sh $@ /P:100,CODES,DATA,$(basename $<),SHARED,$(basename $<)/N/X/Y/E
endef

drvinfo.hex: $(TOOL_DEPS)
delall.hex: $(TOOL_DEPS)
conclus.hex: $(TOOL_DEPS)
drivers.hex: $(TOOL_DEPS)
fastout.hex: $(TOOL_DEPS)
lock.hex: $(TOOL_DEPS)
mapdrv.hex: $(TOOL_DEPS)

%.hex: %.rel
	$(buildtool)

############################################################
# HARD DISK IMAGE

hdd.dsk: nextork.sys command2.com chkdsk.com $(TOOLS_LIST)
	sudo umount -df /media/hdddsk > /dev/null 2>&1 || true
	rm -f hdd.dsk
	mkfs.vfat -C "hdd.dsk" 10000
	sudo mkdir -p /media/hdddsk
	sudo mount -t vfat hdd.dsk /media/hdddsk
	sudo cp *.com /media/hdddsk
	sudo cp nextor.sys /media/hdddsk
	sudo umount -df /media/hdddsk
	cp -u hdd.dsk ../

############################################################
# FLOPPY DISK IMAGE

## Build a FAT12 hard disk image containing nextor.sys, command2.com and all other tools
fdd.dsk: command2.com nextor.sys drvinfo.com
	sudo umount -df /media/fdddsk > /dev/null 2>&1 || true
	rm -f fdd.dsk
	dd if=/dev/zero of=fdd.dsk bs=64k count=1
	mkfs.vfat -F 12 -f 1 fdd.dsk
	sudo mkdir -p /media/fdddsk
	sudo mount -t vfat fdd.dsk /media/fdddsk
	sudo cp command2.com /media/fdddsk
	sudo cp nextor.sys /media/fdddsk
	sudo cp drvinfo.com /media/fdddsk
	sudo umount -df /media/fdddsk
	cp -u fdd.dsk ../

############################################################
# BANK 0

b0.hex: codes.rel kvar.rel data.rel rel.rel doshead.rel 40ff.rel b0.rel init.rel alloc.rel dskbasic.rel dosboot.rel bdos.rel ramdrv.rel chgbnk.rel drv.rel
	@l80.sh b0.hex /p:4000,CODES,KVAR,DATA,REL,DOSHEAD,40FF,B0,INIT,ALLOC,DSKBASIC,DOSBOOT,BDOS,RAMDRV,/p:7700,drv,/p:7fd0,chgbnk,b0/n/x/y/e
	cleancpmfile.sh b0.sym

b0.bin: b0.hex
	hex2bin -s 4000 b0.hex

codes.rel:
kvar.rel: macros.inc const.inc
rel.rel:
doshead.rel: macros.inc const.inc
40ff:
b0.rel: bank.inc
init.rel: const.inc bank.inc
alloc.rel:
bdos.rel: macros.inc const.inc
ramdrv.rel: macros.inc const.inc
chgbnk.rel:

b0labels.inc: b0.hex
	@symtoequs.sh b0.sym b0labels.inc "\?\S*" DOSV0 GETERR BDOSE KDERR KABR

b0lab_b3.inc: b0.hex
	@symtoequs.sh b0.sym b0lab_b3.inc INIT TIMINT MAPBIO GWRK "R_\S*"

############################################################
# BANK 1
b1.rel: b0labels.inc

b1.hex: codes.rel kvar.rel data.rel b1.rel dosinit.rel mapinit.rel alloc.rel msg.rel chgbnk.rel
	@l80.sh b1.hex /P:40FF,CODES,KVAR,DATA,B1,DOSINIT,MAPINIT,ALLOC,MSG,/p:7fd0,chgbnk,B1/N/X/Y/E

b1.bin: b1.hex
	hex2bin -s 4000 b1.hex

############################################################
# BANK 2

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

############################################################
# BANK 3

b3.rel: b0lab_b3.inc

b3.hex: codes.rel kbdos.rel kvar.rel data.rel doshead.rel 40ff.rel dos1ker.rel b3.rel chgbnk.rel drv.rel
	@l80.sh b3.hex /p:4000,CODES,KBDOS,KVAR,DATA,DOSHEAD,40FF,B3,DOS1KER,/p:7700,drv,/p:7fd0,chgbnk,b3/N/X/Y/E

b3.bin: b3.hex
	@rm -f b3.bin
	hex2bin -s 4000 b3.hex

############################################################
# BANK 4

partit.rel: b0labels.inc

b4.rel: b0labels.inc b2labels.inc

b4.hex: codes.rel kvar.rel data.rel b4.rel jump.rel env.rel cpm.rel partit.rel ramdrv4.rel time.rel seg4.rel misc4.rel dskab4.rel chgbnk.rel
	@l80.sh b4.hex /P:40FF,CODES,KVAR,DATA,B4,JUMP,ENV,CPM,PARTIT,RAMDRV4,TIME,SEG4,MISC4,/p:7bc0,DSKAB4,/p:7fd0,chgbnk,B4/N/X/Y/E

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

############################################################
# BANK 5

TOOLS_SRC := ../../source/tools/C/

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

############################################################
# BANK 6

b6.hex: codes.rel kvar.rel data.rel b6.rel chgbnk.rel
	@l80.sh b6.hex /P:40FF,CODES,KVAR,DATA,B6,/p:7fd0,chgbnk,B6/N/X/Y/E

b6.bin: b6.hex
	@rm -f b6.bin
	hex2bin -s 4000 b6.hex

############################################################
# BASE IMAGE

dos250ba.dat: b0.bin b1.bin b2.bin b3.bin b4.bin b5.bin b6.bin fdisk.dat fdisk2.dat b4rd.bin
	@cat b0.bin b1.bin b2.bin b3.bin b4.bin b5.bin b6.bin > dos250ba.dat
	dd conv=notrunc if=dos250ba.dat of=doshead.bin bs=1 count=255
	dd conv=notrunc if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=16k
	dd conv=notrunc if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=32k
	dd conv=notrunc if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=64k
	dd conv=notrunc if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=96k
	dd conv=notrunc if=fdisk.dat of=dos250ba.dat bs=1 count=16000 seek=82176
	dd conv=notrunc if=fdisk2.dat of=dos250ba.dat bs=1 count=8000 seek=98560
	dd conv=notrunc if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=80k
	dd conv=notrunc if=b4rd.bin of=dos250ba.dat bs=1 count=15 seek=65664

############################################################
# DRIVER: sunrise

sunrise.hex: sunrise.rel
	@l80.sh sunrise.hex /P:4100,SUNRISE,SUNRISE/N/X/Y/E

sunrise.bin: sunrise.hex
	@rm -f sunrise.bin
	hex2bin -s 4000 sunrise.hex

srchgbnk.hex: srchgbnk.rel
	@l80.sh srchgbnk.hex /P:7fd0,SRCHGBNK,SRCHGBNK/N/X/Y/E

srchgbnk.bin: srchgbnk.hex
	@rm -f srchgbnk.bin
	hex2bin -s 7FD0 srchgbnk.hex

nextor-$(VERSION).sunriseide.rom:  dos250ba.dat sunrise.bin srchgbnk.bin ../../linuxtools/mknexrom
	@mknexrom  dos250ba.dat nextor-$(VERSION).sunriseide.rom -d:sunrise.bin -m:srchgbnk.bin
	cp -u nextor-$(VERSION).sunriseide.rom ../

include rules.mk

../../linuxtools/mknexrom: ../../wintools/mknexrom.c
	@gcc ../../wintools/mknexrom.c -o ../../linuxtools/mknexrom
