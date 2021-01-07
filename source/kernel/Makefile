# Makefile for the Nextor kernel ROM files

# All the generated ROM files are copied to the bin/kernels directory.
# See the "all" main rule for some handy aliases to generate specific ROMs
# (e.g. you can run "make base" or "make ide").

VERSION := 2.1.1-alpha2

export X80_COMMAND_LINE=-t -nb
export M80_COMMAND_LINE=-8

define SymToEqus
    $(eval REGEX:=([0-9A-F]{4}) ($(3)))
    cat $(1) | grep -Eo "$(REGEX)" | sed -r "s/$(REGEX)/\2 equ \1h\n\tpublic \2\n/g" > $(2)
endef

define hex2bin_full
    objcopy -I ihex -O binary $(1) $(2)
endef

define hex2bin
    $(call hex2bin_full,$(1).HEX,$(1).BIN)
endef

define copy_to_bin
	cp $(1) ../../bin/kernels/$(2)
endef


###################
###  MAIN RULE  ###
###################

all: base ide ide-emu ascii8 ascii16 mfrsd flashjacks

TOOLS := M80 L80 LIB80 objcopy sdcc mknexrom dd

.phony: prerequisites

prerequisites:
	@mkdir -p ../../bin/kernels
	$(foreach exec,$(TOOLS),\
        $(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites

#################################
###  RULES FOR THE ROM FILES  ###
#################################

#TODO: The processing for ASCII8/16 and Sunrise IDE is basically the same, deduplicate a little bit


###  Kernel Base file

base: nextor_base.dat


### Sunrise IDE, normal ROM

ide: drivers/SunriseIDE/Nextor-$(VERSION).SunriseIDE.ROM

drivers/SunriseIDE/Nextor-$(VERSION).SunriseIDE.ROM: \
	nextor_base.dat \
	drivers/SunriseIDE/sunride.BIN \
	drivers/SunriseIDE/CHGBNK.BIN

	mknexrom nextor_base.dat $@ /d:drivers/SunriseIDE/sunride.BIN /m:drivers/SunriseIDE/CHGBNK.BIN
	$(call copy_to_bin,$@)

drivers/SunriseIDE/sunride.BIN: \
	drivers/SunriseIDE/sunride.asm

	sjasm -c drivers/SunriseIDE/sunride.asm drivers/SunriseIDE/sunride.BIN


### Sunrise IDE, ROM for emulators

ide-emu: drivers/SunriseIDE/Nextor-$(VERSION).SunriseIDE.emulators.ROM

drivers/SunriseIDE/Nextor-$(VERSION).SunriseIDE.emulators.ROM: \
	nextor_base.dat \
	drivers/SunriseIDE/DRIVER.BIN \
	drivers/SunriseIDE/CHGBNK.BIN \
	256.bytes

	cat 256.bytes drivers/SunriseIDE/DRIVER.BIN > drivers/SunriseIDE/_driver.BIN
	mknexrom nextor_base.dat $@ /d:drivers/SunriseIDE/_driver.BIN /m:drivers/SunriseIDE/CHGBNK.BIN
	$(call copy_to_bin,$@)

drivers/SunriseIDE/DRIVER.BIN: \
	drivers/SunriseIDE/driver.mac

	M80 -w drivers/SunriseIDE -p ../.. =DRIVER
	L80 -w drivers/SunriseIDE -p ../../ /P:4100,DRIVER,DRIVER/N/X/Y/E
	$(call hex2bin,$(patsubst %.BIN,%,$@))


### Sunrise IDE, common rules

drivers/SunriseIDE/CHGBNK.BIN: \
	drivers/SunriseIDE/chgbnk.mac

	M80 -w drivers/SunriseIDE -p ../.. =CHGBNK
	L80 -w drivers/SunriseIDE /P:7fd0,CHGBNK,CHGBNK/N/X/Y/E
	$(call hex2bin,$(patsubst %.BIN,%,$@))


### Standalone ASCII8

ascii8: drivers/StandaloneASCII8/Nextor-$(VERSION).StandaloneASCII8.ROM

drivers/StandaloneASCII8/Nextor-$(VERSION).StandaloneASCII8.ROM: \
	nextor_base.dat \
	drivers/StandaloneASCII8/DRIVER.BIN \
	drivers/StandaloneASCII8/CHGBNK.BIN \
	256.bytes

	cat 256.bytes drivers/StandaloneASCII8/DRIVER.BIN > drivers/StandaloneASCII8/_driver.BIN
	mknexrom nextor_base.dat $@ /d:drivers/StandaloneASCII8/_driver.BIN /m:drivers/StandaloneASCII8/CHGBNK.BIN
	$(call copy_to_bin,$@)

drivers/StandaloneASCII8/DRIVER.BIN: \
	drivers/StandaloneASCII8/driver.mac

	M80 -w drivers/StandaloneASCII8 -p ../.. =DRIVER
	L80 -w drivers/StandaloneASCII8 -p ../../ /P:4100,DRIVER,DRIVER/N/X/Y/E
	$(call hex2bin,$(patsubst %.BIN,%,$@))

drivers/StandaloneASCII8/CHGBNK.BIN: \
	drivers/StandaloneASCII8/chgbnk.mac

	M80 -w drivers/StandaloneASCII8 -p ../.. =CHGBNK
	L80 -w drivers/StandaloneASCII8 /P:7fd0,CHGBNK,CHGBNK/N/X/Y/E
	$(call hex2bin,$(patsubst %.BIN,%,$@))


### Standalone ASCII16

ascii16: drivers/StandaloneASCII16/Nextor-$(VERSION).StandaloneASCII16.ROM

drivers/StandaloneASCII16/Nextor-$(VERSION).StandaloneASCII16.ROM: \
	nextor_base.dat \
	drivers/StandaloneASCII16/DRIVER.BIN \
	drivers/StandaloneASCII16/CHGBNK.BIN \
	256.bytes

	cat 256.bytes drivers/StandaloneASCII16/DRIVER.BIN > drivers/StandaloneASCII16/_driver.BIN
	mknexrom nextor_base.dat $@ /d:drivers/StandaloneASCII16/_driver.BIN /m:drivers/StandaloneASCII16/CHGBNK.BIN
	$(call copy_to_bin,$@)

drivers/StandaloneASCII16/DRIVER.BIN: \
	drivers/StandaloneASCII16/driver.mac

	M80 -w drivers/StandaloneASCII16 -p ../.. =DRIVER
	L80 -w drivers/StandaloneASCII16 -p ../../ /P:4100,DRIVER,DRIVER/N/X/Y/E
	$(call hex2bin,$(patsubst %.BIN,%,$@))

drivers/StandaloneASCII16/CHGBNK.BIN: \
	drivers/StandaloneASCII16/chgbnk.mac

	M80 -w drivers/StandaloneASCII16 -p ../.. =CHGBNK
	L80 -w drivers/StandaloneASCII16 /P:7fd0,CHGBNK,CHGBNK/N/X/Y/E
	$(call hex2bin,$(patsubst %.BIN,%,$@))


### MegaFlashROM SCC+ SD

mfrsd: \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.ROM \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.Recovery.ROM \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.ROM \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.Recovery.ROM \

drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.ROM: \
	nextor_base.dat \
	drivers/StandaloneASCII8/CHGBNK.BIN

	mknexrom nextor_base.dat $@ /d:drivers/MegaFlashRomSD/driver-1slot.dat /m:drivers/StandaloneASCII8/CHGBNK.BIN
	$(call copy_to_bin,$@)

drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.Recovery.ROM: \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.ROM \
	drivers/MegaFlashRomSD/makerecoverykernel.asm

	cp drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.ROM drivers/MegaFlashRomSD/nextor2.rom
	sjasm drivers/MegaFlashRomSD/makerecoverykernel.asm $@
	$(call copy_to_bin,$@)
	
drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.ROM: \
	nextor_base.dat \
	drivers/StandaloneASCII8/CHGBNK.BIN

	mknexrom nextor_base.dat $@ /d:drivers/MegaFlashRomSD/driver-2slots.dat /m:drivers/StandaloneASCII8/CHGBNK.BIN
	$(call copy_to_bin,$@)

drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.Recovery.ROM: \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.ROM \
	drivers/MegaFlashRomSD/makerecoverykernel.asm

	cp drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.ROM drivers/MegaFlashRomSD/nextor2.rom
	sjasm drivers/MegaFlashRomSD/makerecoverykernel.asm $@
	$(call copy_to_bin,$@)


### Flashjacks

flashjacks: drivers/Flashjacks/Nextor-$(VERSION).Flashjacks.ROM

drivers/Flashjacks/Nextor-$(VERSION).Flashjacks.ROM: \
	nextor_base.dat \
	drivers/Flashjacks/flashjacks.asm

	sjasm drivers/Flashjacks/flashjacks.asm drivers/Flashjacks/driver.BIN
	mknexrom nextor_base.dat $@ /d:drivers/Flashjacks/driver.BIN /m:drivers/Flashjacks/chgbnk.dat
	$(call copy_to_bin,$@)


### Common rules

256.bytes:
	dd if=/dev/zero of=256.bytes bs=1 count=256


########################################
###  RULES FOR THE KERNEL BASE FILE  ###
########################################

nextor_base.dat: \
	255.bytes \
	bank0/B0.BIN \
	bank1/B1.BIN \
	bank2/B2.BIN \
	bank3/B3.BIN \
	bank4/B4.BIN \
	bank4/B4RD.BIN \
	bank5/B5.BIN \
	bank5/fdisk.dat \
	bank5/fdisk2.dat \
	bank6/B6.BIN

	cat bank0/B0.BIN 255.bytes bank1/B1.BIN 255.bytes bank2/B2.BIN bank3/B3.BIN 255.bytes bank4/B4.BIN 255.bytes bank5/B5.BIN 255.bytes bank6/B6.BIN > nextor_base.dat
	dd conv=notrunc if=nextor_base.dat of=doshead.BIN bs=1 count=255
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=16k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=32k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=64k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=80k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=96k
	dd conv=notrunc if=bank5/fdisk.dat of=nextor_base.dat bs=1 count=16000 seek=82176
	dd conv=notrunc if=bank5/fdisk2.dat of=nextor_base.dat bs=1 count=8000 seek=98560
	dd conv=notrunc if=bank4/B4RD.BIN of=nextor_base.dat bs=1 count=15 seek=65664

	$(call copy_to_bin,nextor_base.dat,Nextor-$(VERSION).base.dat)

255.bytes:
	dd if=/dev/zero of=255.bytes bs=1 count=255

COMRELS = CODES.REL KVAR.REL DATA.REL CHGBNK.REL
B0RELS = $(addprefix bank0/,DOSHEAD.REL 40FF.REL B0.REL INIT.REL ALLOC.REL DSKBASIC.REL DOSBOOT.REL BDOS.REL RAMDRV.REL)
B1RELS = $(addprefix bank1/,DOSINIT.REL MAPINIT.REL MSG.REL)
B2RELS = $(addprefix bank2/,B2.REL KINIT.REL CHAR.REL DEV.REL KBIOS.REL MISC.REL SEG.REL PATH.REL FIND.REL DIR.REL HANDLES.REL DEL.REL RW.REL FILES.REL BUF.REL FAT.REL VAL.REL ERR.REL)
B3RELS = $(addprefix bank3/,DOS1KER.REL)
B4RELS = $(addprefix bank4/,JUMP.REL ENV.REL CPM.REL PARTIT.REL RAMDRV.REL TIME.REL SEG.REL MISC.REL DSKAB.REL)
B5RELS = $(addprefix bank5/,B5.REL)
B6RELS = $(addprefix bank6/,B6.REL)

lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,\
	$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,\
	$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

.SECONDEXPANSION:
$(COMRELS) $(B0RELS) $(B1RELS) $(B2RELS) $(B3RELS) $(B4RELS) $(B5RELS) $(B6RELS) DRV.REL REL.REL: \
	macros.inc \
	const.inc \
	condasm.inc \
	$$(patsubst %.rel,%.mac,$$(call lc,$$@))

	M80 -w $(dir $@) -p ..,../bank0,../bank2 =$(notdir $(patsubst %.REL,%,$@))


###  Bank 0

bank0/B0.BIN: \
	bank0/B0.HEX

	$(call hex2bin,bank0/B0)

bank0/B0.HEX bank0/B0.SYM: \
	$(COMRELS) \
	$(B0RELS) \
	DRV.REL \
	REL.REL

	L80 -w bank0 -p .. /p:4000,CODES,KVAR,DATA,REL,DOSHEAD,40FF,B0,INIT,ALLOC,DSKBASIC,DOSBOOT,BDOS,RAMDRV,/p:7700,drv,/p:7fd0,chgbnk,b0/n/x/y/e
	$(call hex2bin,bank0/B0)

bank0/b0labels.inc: \
	bank0/B0.SYM

	$(call SymToEqus,bank0/B0.SYM,bank0/b0labels.inc,[?][^[:space:]]+|DOSV0|GETERR|BDOSE|KDERR|KABR|C4PBK)

bank0/b0lab_b3.inc: \
	bank0/B0.SYM

	$(call SymToEqus,bank0/B0.SYM,bank0/b0lab_b3.inc,INIT|TIMINT|MAPBIO|GWRK|R_[^[:space:]]+)


###  Bank 1

bank1/B1.REL: \
	bank1/b1.mac \
	bank0/b0labels.inc

	M80 -w bank1 -p ..,../bank0 M80 =B1

bank1/B1.BIN: \
	bank1/B1.HEX

	$(call hex2bin,bank1/B1)

bank1/B1.HEX: \
	$(COMRELS) \
	$(B1RELS) \
	bank1/B1.REL \
	bank0/ALLOC.REL \
	bank0/b0labels.inc

	L80 -w bank1 -p ..,../bank0 /P:40FF,CODES,KVAR,DATA,B1,DOSINIT,MAPINIT,ALLOC,MSG,/p:7fd0,chgbnk,B1/N/X/Y/E


###  Bank 2

bank2/B2.BIN: \
	bank2/B2.HEX

	$(call hex2bin,bank2/B2)

bank2/B2.HEX bank2/B2.SYM: \
	$(COMRELS) \
	$(B2RELS) \
	bank0/b0labels.inc

	LIB80 -w bank2 TEMP21=char.rel,dev.rel,kbios.rel,misc.rel,seg.rel/E
	LIB80 -w bank2 TEMP22=path.rel,find.rel,dir.rel,handles.rel,del.rel,rw.rel,files.rel/E
	LIB80 -w bank2 TEMP23=buf.rel,fat.rel,val.rel,err.rel/E
	L80 -w bank2 -p .. /P:40FF,CODES,KVAR,DATA,B2,KINIT,TEMP21,TEMP22,TEMP23,/p:7fd0,chgbnk,B2/N/X/Y/E

bank2/b2labels.inc: \
	bank2/B2.SYM

	$(call SymToEqus,bank2/B2.SYM,bank2/b2labels.inc,[?][^[:space:]]+)


###  Bank 3

bank3/B3.REL: \
	bank3/b3.mac \
	bank0/b0lab_b3.inc

	M80 -w bank3 -p ..,../bank0 M80 =B3

bank3/B3.BIN: \
	bank3/B3.HEX

	$(call hex2bin,bank3/B3)

bank3/B3.HEX: \
	$(COMRELS) \
	$(B3RELS) \
	bank3/B3.REL \
	bank0/b0lab_b3.inc \
	bank0/DOSHEAD.REL \
	bank0/40FF.REL

	L80 -w bank3 -p ..,../bank0 /p:4000,CODES,KVAR,DATA,DOSHEAD,40FF,B3,DOS1KER,/p:7700,drv,/p:7fd0,chgbnk,b3/N/X/Y/E


###  Bank 4

bank4/b4rdlabs.inc: \
	bank4/B4.SYM

	$(call SymToEqus,bank4/B4.SYM,bank4/b4rdlabs.inc,R4_[1-9])

bank4/B4.REL: \
	bank.inc \
	bank4/b4.mac \
	bank2/b2labels.inc

	M80 -w bank4 -p ..,../bank2 =B4

bank4/RAMDRVH.REL: \
	macros.inc \
	const.inc \
	bank4/b4rdlabs.inc

	M80 -w bank4 -p .. =RAMDRVH

bank4/B4.BIN: \
	bank4/B4.HEX

	$(call hex2bin,bank4/B4)

bank4/B4RD.BIN: \
	bank4/B4RD.HEX

	$(call hex2bin,bank4/B4RD)

bank4/B4RD.HEX: \
	bank4/RAMDRVH.REL \
	bank4/b4rdlabs.inc

	L80 -w bank4 -p .. /P:4080,RAMDRVH,B4RD/N/X/Y/E

bank4/B4.HEX bank4/B4.SYM: \
	$(COMRELS) \
	$(B4RELS) \
	bank4/B4.REL \
	bank0/b0labels.inc \
	bank2/b2labels.inc \

	L80 -w bank4 -p ..,../bank0,../bank2 /P:40FF,CODES,KVAR,DATA,B4,JUMP,ENV,CPM,PARTIT,RAMDRV,TIME,SEG,MISC,/p:7bc0,DSKAB,/p:7fd0,chgbnk,B4/N/X/Y/E


###  Banks 5 and 6 (FDISK)

bank5/B5.BIN: \
	bank5/B5.HEX

	$(call hex2bin,bank5/B5)

bank5/B5.HEX: \
	$(COMRELS) \
	$(B5RELS) \
	bank0/40FF.REL

	L80 -w bank5 -p .. /P:40FF,CODES,KVAR,DATA,B5,/p:7fd0,chgbnk,B5/N/X/Y/E

bank6/B6.BIN: \
	bank6/B6.HEX

	$(call hex2bin,bank6/B6)

bank6/B6.HEX: \
	$(COMRELS) \
	$(B6RELS) \
	bank0/40FF.REL

	L80 -w bank6 -p .. /P:40FF,CODES,KVAR,DATA,B6,/p:7fd0,chgbnk,B6/N/X/Y/E

bank5/fdisk.dat bank5/fdisk2.dat: \
	bank5/fdisk_crt0.rel \
	bank5/fdisk.h \
	$(patsubst %.dat,%.c,$@) \
	bank5/drivercall.h \
	bank5/drivercall.c \
	../tools/C/system.h \
	../tools/C/dos.h \
	../tools/C/types.h \
	../tools/C/asmcall.h \
	../tools/C/printf.c \
	../tools/C/asmcall.c \
	../tools/C/partit.h

	sdcc -o bank5/ --code-loc 0x4120 --data-loc 0x8020 -mz80 --disable-warning 196 --disable-warning 84 --disable-warning 85 \
	     --max-allocs-per-node 1000 --allow-unsafe-read --opt-code-size --no-std-crt0 bank5/fdisk_crt0.rel $(patsubst %.dat,%.c,$@)
	$(call hex2bin_full,$(patsubst %.dat,%.ihx,$@),$@)

bank5/fdisk_crt0.rel: \
	bank5/fdisk_crt0.s

	sdasz80 -o bank5/fdisk_crt0.rel bank5/fdisk_crt0.s


#################
###  CLEANUP  ###
#################

.PHONY: clean

clean:
	for ext in bin BIN HEX REL rel SYM sym bytes ihx lk lst map noi; do find . -type f -name "*.$$ext" -delete; done
	for ext in asm dat; do find ./bank5 -type f -name "*.$$ext" -delete; done
	for ext in lst rom ROM; do find ./drivers -type f -name "*.$$ext" -delete; done
	rm -f nextor_base.dat
	rm -f bank0/b0lab*.inc
	rm -f bank2/b2labels.inc
	rm -f bank4/b4rdlabs.inc
