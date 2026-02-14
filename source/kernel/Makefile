# Makefile for the Nextor kernel ROM files

# All the generated ROM files are copied to the bin/kernels directory.
# See the "all" main rule for some handy aliases to generate specific ROMs
# (e.g. you can run "make base" or "make ide").

# The N80 and LK80 binaries to use can be overriden with environment variables, e.g.:
# N80=/path/to/N80 make


VERSION := 3.0.0

ifeq ($(strip $(N80)),)
N80=N80
endif

export N80_ARGS=--no-string-escapes --no-show-banner --build-type rel --verbosity 0 --output-file-case lower

ifeq ($(strip $(LK80)),)
LK80=LK80
endif

export LK80_ARGS=--no-show-banner --verbosity 0 --output-file-case lower --output-format bin

ifeq ($(strip $(LB80)),)
LB80=LB80
endif

export LB80_ARGS=--no-show-banner --verbosity 0

ifeq ($(strip $(MKNEXROM)),)
MKNEXROM=mknexrom
endif

define SymToEqus
    $(eval REGEX:=([0-9A-F]{4}) ($(3)))
    cat $(1) | grep -Eo "$(REGEX)" | sed -r "s/$(REGEX)/\2 equ \1h\n\tpublic \2\n/g" > $(2)
endef

define hex2bin
    objcopy -I ihex -O binary $(1) $(2)
endef

define copy_to_bin
	cp $(1) ../../bin/kernels/$(2)
endef

define assemble
	@printf "\n\033[0;36mAssembling %s\033[0m\n\n" $(1)
	@$(N80) $(1) $$ $(2)
endef

define assemble_as
	@printf "\n\033[0;36mAssembling %s as %s\033[0m\n\n" $(1) $(2)
	@$(N80) $(1) $(2) $(3)
endef

define print_linking
	@printf "\n\033[0;36mLinking %s\033[0m\n\n" $(1)
endef


###################
###  MAIN RULE  ###
###################

all: base ide ide-masteronly ide-blue ide-masteronly-emu ascii8 ascii16 mfrsd flashjacks ocm dsk-tr

TOOLS := $(N80) $(LK80) $(LB80) objcopy sdcc $(MKNEXROM) dd

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
	drivers/SunriseIDE/sunride.bin \
	drivers/SunriseIDE/chgbnk.bin

	$(MKNEXROM) nextor_base.dat $@ /d:drivers/SunriseIDE/sunride.bin /m:drivers/SunriseIDE/chgbnk.bin
	$(call copy_to_bin,$@)

drivers/SunriseIDE/sunride.bin: \
	drivers/SunriseIDE/sunride.asm

	$(call assemble,drivers/SunriseIDE/sunride.asm,--build-type abs --output-file-extension bin --define-symbols BAD_POPS)


### Sunrise IDE, normal ROM, master device only

ide-masteronly: drivers/SunriseIDE/Nextor-$(VERSION).SunriseIDE.MasterOnly.ROM

drivers/SunriseIDE/Nextor-$(VERSION).SunriseIDE.MasterOnly.ROM: \
	nextor_base.dat \
	drivers/SunriseIDE/sunride.masteronly.bin \
	drivers/SunriseIDE/chgbnk.bin

	$(MKNEXROM) nextor_base.dat $@ /d:drivers/SunriseIDE/sunride.masteronly.bin /m:drivers/SunriseIDE/chgbnk.bin
	$(call copy_to_bin,$@)

drivers/SunriseIDE/sunride.masteronly.bin: \
	drivers/SunriseIDE/sunride.asm

	$(call assemble_as,drivers/SunriseIDE/sunride.asm,$$/sunride.masteronly.bin,--build-type abs --output-file-extension bin --define-symbols MASTER_ONLY,BAD_POPS)


### Sunrise IDE, ROM for blueMSX

ide-blue: drivers/SunriseIDE/Nextor-$(VERSION).SunriseIDE.blueMSX.ROM

drivers/SunriseIDE/Nextor-$(VERSION).SunriseIDE.blueMSX.ROM: \
	nextor_base.dat \
	drivers/SunriseIDE/driver.bin \
	drivers/SunriseIDE/chgbnk.bin \
	256.bytes

	cat 256.bytes drivers/SunriseIDE/driver.bin > drivers/SunriseIDE/_driver.BIN
	$(MKNEXROM) nextor_base.dat $@ /d:drivers/SunriseIDE/_driver.BIN /m:drivers/SunriseIDE/chgbnk.bin
	$(call copy_to_bin,$@)

drivers/SunriseIDE/driver.bin: \
	drivers/SunriseIDE/driver.mac

	$(call assemble,drivers/SunriseIDE/driver.mac,--build-type abs --output-file-extension bin)


### Sunrise IDE, ROM for blueMSX, master device only

ide-masteronly-emu: drivers/SunriseIDE/Nextor-$(VERSION).SunriseIDE.MasterOnly.blueMSX.ROM

drivers/SunriseIDE/Nextor-$(VERSION).SunriseIDE.MasterOnly.blueMSX.ROM: \
	nextor_base.dat \
	drivers/SunriseIDE/drvmonly.bin \
	drivers/SunriseIDE/chgbnk.bin \
	256.bytes

	cat 256.bytes drivers/SunriseIDE/drvmonly.bin > drivers/SunriseIDE/_driver.BIN
	$(MKNEXROM) nextor_base.dat $@ /d:drivers/SunriseIDE/_driver.BIN /m:drivers/SunriseIDE/chgbnk.bin
	$(call copy_to_bin,$@)

drivers/SunriseIDE/drvmonly.bin: \
	drivers/SunriseIDE/driver.mac

	$(call assemble_as,drivers/SunriseIDE/driver.mac,$$/drvmonly.bin,--build-type abs  --output-file-extension bin --define-symbols MASTER_ONLY)


### Sunrise IDE, common rules

drivers/SunriseIDE/chgbnk.bin: \
	drivers/SunriseIDE/chgbnk.mac

	$(call assemble,drivers/SunriseIDE/chgbnk.mac,--build-type abs --output-file-extension bin)


### Standalone ASCII8

ascii8: drivers/StandaloneASCII8/Nextor-$(VERSION).StandaloneASCII8.ROM

drivers/StandaloneASCII8/Nextor-$(VERSION).StandaloneASCII8.ROM: \
	nextor_base.dat \
	drivers/StandaloneASCII8/driver.bin \
	drivers/StandaloneASCII8/chgbnk.bin \
	256.bytes

	cat 256.bytes drivers/StandaloneASCII8/driver.bin > drivers/StandaloneASCII8/_driver.bin
	$(MKNEXROM) nextor_base.dat $@ /d:drivers/StandaloneASCII8/_driver.bin /m:drivers/StandaloneASCII8/chgbnk.bin
	$(call copy_to_bin,$@)

drivers/StandaloneASCII8/driver.bin: \
	drivers/StandaloneASCII8/driver.mac

	$(call assemble,drivers/StandaloneASCII8/driver.mac,--build-type abs --output-file-extension bin)

drivers/StandaloneASCII8/chgbnk.bin: \
	drivers/StandaloneASCII8/chgbnk.mac

	$(call assemble,drivers/StandaloneASCII8/chgbnk.mac,--build-type abs --output-file-extension bin)


### Standalone ASCII16

ascii16: drivers/StandaloneASCII16/Nextor-$(VERSION).StandaloneASCII16.ROM

drivers/StandaloneASCII16/Nextor-$(VERSION).StandaloneASCII16.ROM: \
	nextor_base.dat \
	drivers/StandaloneASCII8/driver.bin \
	drivers/StandaloneASCII16/chgbnk.bin \
	256.bytes

	cat 256.bytes drivers/StandaloneASCII8/driver.bin > drivers/StandaloneASCII16/_driver.bin
	$(MKNEXROM) nextor_base.dat $@ /d:drivers/StandaloneASCII16/_driver.bin /m:drivers/StandaloneASCII16/chgbnk.bin
	$(call copy_to_bin,$@)

drivers/StandaloneASCII16/chgbnk.bin: \
	drivers/StandaloneASCII16/chgbnk.mac

	$(call assemble,drivers/StandaloneASCII16/chgbnk.mac,--build-type abs --output-file-extension bin)


### MegaFlashROM SCC+ SD

mfrsd: \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.ROM \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.Recovery.ROM \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.ROM \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.Recovery.ROM \

drivers/MegaFlashRomSD/mfrsd-1slot.bin: \
	drivers/MegaFlashRomSD/mfrsd.asm \
	drivers/MegaFlashRomSD/romdisk.asm

	$(call assemble_as,drivers/MegaFlashRomSD/mfrsd.asm,$$/mfrsd-1slot.bin,--build-type abs --output-file-extension bin --define-symbols NUM_SLOTS=1)

drivers/MegaFlashRomSD/mfrsd-2slots.bin: \
	drivers/MegaFlashRomSD/mfrsd.asm \
	drivers/MegaFlashRomSD/romdisk.asm

	$(call assemble_as,drivers/MegaFlashRomSD/mfrsd.asm,$$/mfrsd-2slots.bin,--build-type abs --output-file-extension bin --define-symbols NUM_SLOTS=2)

drivers/MegaFlashRomSD/recovery_header.bin: \
	drivers/MegaFlashRomSD/recovery_header.asm

	$(call assemble,drivers/MegaFlashRomSD/recovery_header.asm,--build-type abs --output-file-extension bin)

drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.ROM: \
	nextor_base.dat \
	drivers/StandaloneASCII8/chgbnk.bin \
	drivers/MegaFlashRomSD/mfrsd-1slot.bin

	$(MKNEXROM) nextor_base.dat $@ /d:drivers/MegaFlashRomSD/mfrsd-1slot.bin /m:drivers/StandaloneASCII8/chgbnk.bin
	$(call copy_to_bin,$@)

drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.Recovery.ROM: \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.ROM \
	drivers/MegaFlashRomSD/recovery_header.bin

	cat drivers/MegaFlashRomSD/recovery_header.bin \
		drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.ROM \
		> drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.1-slot.Recovery.ROM

	$(call copy_to_bin,$@)
	
drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.ROM: \
	nextor_base.dat \
	drivers/StandaloneASCII8/chgbnk.bin \
	drivers/MegaFlashRomSD/mfrsd-2slots.bin

	$(MKNEXROM) nextor_base.dat $@ /d:drivers/MegaFlashRomSD/mfrsd-2slots.bin /m:drivers/StandaloneASCII8/chgbnk.bin
	$(call copy_to_bin,$@)

drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.Recovery.ROM: \
	drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.ROM \
	drivers/MegaFlashRomSD/recovery_header.bin

	cat drivers/MegaFlashRomSD/recovery_header.bin \
		drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.ROM \
		> drivers/MegaFlashRomSD/Nextor-$(VERSION).MegaFlashSDSCC.2-slots.Recovery.ROM

	$(call copy_to_bin,$@)


### Flashjacks

flashjacks: drivers/Flashjacks/Nextor-$(VERSION).Flashjacks.ROM

drivers/Flashjacks/flashjacks.bin: \
	drivers/Flashjacks/flashjacks.asm

	$(call assemble,drivers/Flashjacks/flashjacks.asm,--build-type abs --output-file-extension bin)

drivers/Flashjacks/Nextor-$(VERSION).Flashjacks.ROM: \
	nextor_base.dat \
	drivers/Flashjacks/flashjacks.bin \
	drivers/SunriseIDE/chgbnk.bin

	$(MKNEXROM) nextor_base.dat $@ /d:drivers/Flashjacks/flashjacks.bin /m:drivers/SunriseIDE/chgbnk.bin
	$(call copy_to_bin,$@)


### OCM (1-chip MSX, SX|2, and compatibles)

ocm: drivers/OCM/Nextor-$(VERSION).OCM.ROM

drivers/OCM/Nextor-$(VERSION).OCM.ROM: \
	nextor_base.dat \
	drivers/StandaloneASCII8/chgbnk.bin

	$(MKNEXROM) nextor_base.dat $@ /d:drivers/OCM/driver.dat /m:drivers/StandaloneASCII8/chgbnk.bin
	$(call copy_to_bin,$@)


### Turbo-R FDD

dsk-tr: drivers/TurboRFDD/Nextor-$(VERSION).TurboRFDD.ROM

drivers/TurboRFDD/Nextor-$(VERSION).TurboRFDD.ROM: \
	nextor_base.dat \
	drivers/TurboRFDD/driver.bin \
	drivers/TurboRFDD/chgbnk.bin \
	256.bytes

	cat 256.bytes drivers/TurboRFDD/driver.bin > drivers/TurboRFDD/_driver.bin
	$(MKNEXROM) nextor_base.dat $@ /d:drivers/TurboRFDD/_driver.bin /m:drivers/TurboRFDD/chgbnk.bin
	$(call copy_to_bin,$@)

drivers/TurboRFDD/driver.bin: \
	drivers/TurboRFDD/driver.mac

	$(call assemble,drivers/TurboRFDD/driver.mac,--build-type abs --output-file-extension bin)

drivers/TurboRFDD/chgbnk.bin: \
	drivers/TurboRFDD/chgbnk.mac

	$(call assemble,drivers/TurboRFDD/chgbnk.mac,--build-type abs --output-file-extension bin)


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
	bank6/B6.BIN

	cat bank0/B0.BIN 255.bytes bank1/B1.BIN 255.bytes bank2/B2.BIN bank3/B3.BIN 255.bytes bank4/B4.BIN 255.bytes bank5/B5.BIN 255.bytes bank6/B6.BIN > nextor_base.dat
	dd conv=notrunc if=nextor_base.dat of=doshead.BIN bs=1 count=255
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=16k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=32k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=64k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=80k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=96k
	dd conv=notrunc if=bank5/fdisk.dat of=nextor_base.dat bs=1 count=16048 seek=82176
	dd conv=notrunc if=bank4/B4RD.BIN of=nextor_base.dat bs=1 count=15 seek=65664

	$(call copy_to_bin,nextor_base.dat,Nextor-$(VERSION).base.dat)

255.bytes:
	dd if=/dev/zero of=255.bytes bs=1 count=255

COMRELS = codes.rel kvar.rel data.rel chgbnk.rel
B0RELS = $(addprefix bank0/,doshead.rel 40ff.rel b0.rel init.rel alloc.rel dskbasic.rel dosboot.rel bdos.rel ramdrv.rel)
B1RELS = $(addprefix bank1/,dosinit.rel mapinit.rel msg.rel)
B2RELS = $(addprefix bank2/,kinit.rel char.rel dev.rel kbios.rel misc.rel seg.rel path.rel find.rel dir.rel handles.rel del.rel rw.rel files.rel buf.rel fat.rel val.rel err.rel)
B3RELS = $(addprefix bank3/,dos1ker.rel)
B4RELS = $(addprefix bank4/,jump.rel env.rel cpm.rel ramdrv.rel time.rel seg.rel misc.rel bootmenu.rel)
B5RELS = $(addprefix bank5/,b5.rel)
B6RELS = $(addprefix bank6/,b6.rel)

lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,\
	$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,\
	$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

.SECONDEXPANSION:
$(COMRELS) $(B0RELS) $(B1RELS) $(B2RELS) $(B3RELS) $(B4RELS) partit.rel $(B5RELS) $(B6RELS) drv.rel rel.rel: \
	macros.inc \
	const.inc \
	condasm.inc \
	$$(patsubst %.rel,%.mac,$$@)

	$(call assemble,$(patsubst %.rel,%.mac,$@))


###  Bank 0

bank0/B0.BIN bank0/B0.SYM: \
	$(COMRELS) \
	$(B0RELS) \
	drv.rel \
	rel.rel

	$(call print_linking,"bank 0")
	@$(LK80) --working-dir bank0 --symbols-file B0.SYM --output-file B0.BIN \
			--code 4000h ../codes.rel ../kvar.rel ../data.rel ../rel.rel doshead.rel 40ff.rel b0.rel init.rel alloc.rel dskbasic.rel dosboot.rel bdos.rel ramdrv.rel \
			--code 7700h ../drv.rel \
			--code 7fd0h ../chgbnk.rel

bank0/b0labels.inc: \
	bank0/B0.SYM

	$(call SymToEqus,bank0/B0.SYM,bank0/b0labels.inc,[?][^[:space:]]+|DOSV0|GETERR|BDOSE|KDERR|KABR|C4PBK)

bank0/b0lab_b3.inc: \
	bank0/B0.SYM

	$(call SymToEqus,bank0/B0.SYM,bank0/b0lab_b3.inc,INIT|TIMINT|MAPBIO|GWRK|FDISK|R_[^[:space:]]+)


###  Bank 1

bank1/b1.rel: \
	bank1/b1.mac \
	bank0/b0labels.inc

	$(call assemble,bank1/b1.mac)

bank1/B1.BIN: \
	$(COMRELS) \
	$(B1RELS) \
	bank1/msg.rel \
	bank1/b1.rel \
	bank0/alloc.rel \
	bank0/b0labels.inc

	$(call print_linking,"bank 1")
	@$(LK80) --working-dir bank1 --symbols-file B1.SYM --output-file B1.BIN \
			--code 40FFh ../codes.rel ../kvar.rel ../data.rel b1.rel dosinit.rel mapinit.rel ../bank0/alloc.rel msg.rel \
			--code 7fd0h ../chgbnk.rel


###  Bank 2

bank2/b2.rel: \
	bank2/b2.mac \
	bank0/b0labels.inc

	$(call assemble,bank2/b2.mac)

bank2/B2.BIN bank2/B2.SYM: \
	$(COMRELS) \
	$(B2RELS) \
	bank2/b2.rel \
	bank0/b0labels.inc

	@$(LB80) --working-dir bank2 create TEMP21.REL char.rel dev.rel kbios.rel misc.rel seg.rel
	@$(LB80) --working-dir bank2 create TEMP22.REL path.rel find.rel dir.rel handles.rel del.rel rw.rel files.rel
	@$(LB80) --working-dir bank2 create TEMP23.REL buf.rel fat.rel val.rel err.rel

	$(call print_linking,"bank 2")
	@$(LK80) --working-dir bank2 --symbols-file B2.SYM --output-file B2.BIN \
			--code 40FFh ../codes.rel ../kvar.rel ../data.rel b2.rel kinit.rel TEMP21.REL TEMP22.REL TEMP23.REL \
			--code 7fd0h ../chgbnk.rel

bank2/b2labels.inc: \
	bank2/B2.SYM

	$(call SymToEqus,bank2/B2.SYM,bank2/b2labels.inc,[?][^[:space:]]+)


###  Bank 3

bank3/b3.rel: \
	bank3/b3.mac \
	bank0/b0lab_b3.inc

	$(call assemble,bank3/b3.mac)

bank3/B3.BIN: \
	$(COMRELS) \
	$(B3RELS) \
	bank3/b3.rel \
	bank0/b0lab_b3.inc \
	bank0/doshead.rel \
	bank0/40ff.rel

	$(call print_linking,"bank 3")
	@$(LK80) --working-dir bank3 --symbols-file B3.SYM --output-file B3.BIN \
			--code 4000h ../codes.rel ../kvar.rel ../data.rel ../bank0/doshead.rel ../bank0/40ff.rel b3.rel dos1ker.rel \
			--code 7700h ../drv.rel \
			--code 7fd0h ../chgbnk.rel


###  Bank 4

bank4/b4rdlabs.inc: \
	bank4/B4.SYM

	$(call SymToEqus,bank4/B4.SYM,bank4/b4rdlabs.inc,R4_[1-9])

bank4/b4.rel: \
	bank.inc \
	bank4/b4.mac \
	bank2/b2labels.inc

	$(call assemble,bank4/b4.mac)

bank4/partit.rel: \
	bank4/partit.mac \
	bank0/b0labels.inc

	$(call assemble,bank4/partit.mac)

bank4/bootmsg.rel: \
	macros.inc \
	const.inc \
	condasm.inc \
	bank4/bootmsg.mac \
	bank0/betainfo.mac

	$(call assemble,bank4/bootmsg.mac)

bank4/ramdrvh.rel: \
	macros.inc \
	const.inc \
	bank4/ramdrvh.mac \
	bank4/b4rdlabs.inc

	$(call assemble,bank4/ramdrvh.mac)

bank4/B4RD.BIN: \
	bank4/ramdrvh.rel \
	bank4/b4rdlabs.inc

	$(call print_linking,"bank 4 B4RD")
	@$(LK80) --working-dir bank4 --symbols-file B4RD.SYM --output-file B4RD.BIN \
			--code 4080h ramdrvh.rel

bank4/B4.BIN bank4/B4.SYM: \
	$(COMRELS) \
	$(B4RELS) \
	bank4/partit.rel \
	bank4/bootmsg.rel \
	bank4/b4.rel \
	bank4/ramdrv.rel \
	bank0/b0labels.inc \
	bank2/b2labels.inc \

	$(call print_linking,"bank 4")
	@$(LK80) --working-dir bank4 --symbols-file B4.SYM --output-file B4.BIN \
			--code 40FFh ../codes.rel ../kvar.rel ../data.rel b4.rel jump.rel env.rel cpm.rel partit.rel ramdrv.rel time.rel seg.rel misc.rel bootmenu.rel bootmsg.rel \
			--code 7fd0h ../chgbnk.rel


###  Banks 5 and 6 (FDISK)

bank5/B5.BIN: \
	$(COMRELS) \
	$(B5RELS) \
	bank0/40ff.rel

	$(call print_linking,"bank 5")
	@$(LK80) --working-dir bank5 --symbols-file B5.SYM --output-file B5.BIN \
			--code 40FFh ../codes.rel ../kvar.rel ../data.rel b5.rel \
			--code 7fd0h ../chgbnk.rel

bank6/B6.BIN: \
	$(COMRELS) \
	$(B6RELS) \
	bank0/40ff.rel

	$(call print_linking,"bank 6")
	@$(LK80) --working-dir bank6 --symbols-file B6.SYM --output-file B6.BIN \
			--code 40FFh ../codes.rel ../kvar.rel ../data.rel b6.rel \
			--code 7fd0h ../chgbnk.rel

bank5/fdisk.dat: \
	bank5/fdisk_crt0.rel \
	bank5/fdisk.h \
	bank5/fdisk.c \
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
	$(call hex2bin,$(patsubst %.dat,%.ihx,$@),$@)

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
