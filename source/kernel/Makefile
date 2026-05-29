# Makefile for the Nextor kernel base file.
#
# This Makefile only builds the kernel base file (nextor_base.dat),
# which is the input to MKNEXROM for producing complete kernel ROMs.
# The standalone ROM drivers (Standalone ASCII8/16) and the example RAM
# driver live in source/drivers; third-party drivers (Sunrise IDE,
# MegaFlash ROM SCC+ SD...) have their own dedicated repositories.
#
# The build is controlled by three external variables (set via env or on
# the make command line). Each variable, when non-empty, both defines a
# symbol that gets passed to N80 via --define-symbols, and adds a
# fragment to the output-filename suffix:
#
#   NO_UNDOC_CPU_INSTRUCTIONS  -> defines NO_UNDOC_CPU_INSTRUCTIONS,
#                                 suffix fragment ".NO_UNDOC". Replaces
#                                 undocumented Z80 opcodes (ixh/ixl/iyh/
#                                 iyl) with documented equivalents, for
#                                 compatibility with Z180-based MSX
#                                 machines.
#   INVERT_SHIFT               -> defines INVERT_SHIFT, suffix fragment
#                                 ".SHIFT_INV". Inverts the meaning of
#                                 the SHIFT boot key.
#   INVERT_CTRL                -> defines INVERT_CTRL, suffix fragment
#                                 ".CTRL_INV". Inverts the meaning of
#                                 the CTRL boot key.
#
# Example: `NO_UNDOC_CPU_INSTRUCTIONS=1 INVERT_SHIFT=1 make`
# produces Nextor-$(VERSION).base.NO_UNDOC.SHIFT_INV.dat.
#
# A single variant-specific stamp file (its filename encodes the active
# variant signature) is listed as a prereq of every assembled .rel, so
# switching variants forces a full reassembly and previously built .rels
# from a different variant are never silently reused.
#
# `make everything` invokes make six times to build the kernel base file
# for all six variant combinations (default, INVERT_SHIFT, INVERT_CTRL,
# NO_UNDOC, NO_UNDOC+INVERT_SHIFT, NO_UNDOC+INVERT_CTRL).
#
# The generated base files are copied to ../../bin/kernel-base.
#
# The N80 and LK80 binaries to use can be overriden with environment
# variables, e.g.:
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

_VARIANT_DEFINES :=
_VARIANT_SUFFIX :=

ifneq ($(strip $(NO_UNDOC_CPU_INSTRUCTIONS)),)
_VARIANT_DEFINES += NO_UNDOC_CPU_INSTRUCTIONS
_VARIANT_SUFFIX := $(_VARIANT_SUFFIX).NO_UNDOC
endif

ifneq ($(strip $(INVERT_SHIFT)),)
_VARIANT_DEFINES += INVERT_SHIFT
_VARIANT_SUFFIX := $(_VARIANT_SUFFIX).SHIFT_INV
endif

ifneq ($(strip $(INVERT_CTRL)),)
_VARIANT_DEFINES += INVERT_CTRL
_VARIANT_SUFFIX := $(_VARIANT_SUFFIX).CTRL_INV
endif

UNDOC_MODE_STAMP := .build-variant$(if $(_VARIANT_SUFFIX),$(_VARIANT_SUFFIX),.default)

$(UNDOC_MODE_STAMP):
	@rm -f .build-variant*
	@touch $@

define SymToEqus
    $(eval REGEX:=([0-9A-F]{4}) ($(3)))
    cat $(1) | grep -Eo "$(REGEX)" | sed -r "s/$(REGEX)/\2 equ \1h\n\tpublic \2\n/g" > $(2)
endef

define hex2bin
    objcopy -I ihex -O binary $(1) $(2)
endef

define copy_to_bin
	cp $(1) ../../bin/kernel-base/$(2)
endef

define assemble
	@printf "\n\033[0;36mAssembling %s\033[0m\n\n" $(1)
	@$(N80) $(1) $$ $(2) $(foreach def,$(_VARIANT_DEFINES),--define-symbols $(def))
endef

define assemble_as
	@printf "\n\033[0;36mAssembling %s as %s\033[0m\n\n" $(1) $(2)
	@$(N80) $(1) $(2) $(3) $(foreach def,$(_VARIANT_DEFINES),--define-symbols $(def))
endef

define print_linking
	@printf "\n\033[0;36mLinking %s\033[0m\n\n" $(1)
endef


###################
###  MAIN RULE  ###
###################

all: base

# `everything` builds the kernel base file across all six variant
# combinations (default, INVERT_SHIFT, INVERT_CTRL, NO_UNDOC,
# NO_UNDOC+INVERT_SHIFT, NO_UNDOC+INVERT_CTRL). Each variant runs as a
# recursive make invocation with the corresponding env vars set, so the
# variant stamp inside the sub-make is correctly tracked.
#
# Sub-makes within a single variant are incremental, but switching
# variants between sub-makes forces a full reassembly (intrinsic to the
# single-stamp mechanism). The default variant is built last so the
# on-disk intermediate file (nextor_base.dat) is left as the
# non-inverted, undoc kernel that downstream consumers (source/drivers)
# pick up by default.
#
# Each sub-make is prefixed with `-` and passes --keep-going so a
# failure in one variant does not stop the other five from running.
.PHONY: everything
everything:
	-@$(MAKE) --no-print-directory --keep-going all INVERT_SHIFT=1
	-@$(MAKE) --no-print-directory --keep-going all INVERT_CTRL=1
	-@$(MAKE) --no-print-directory --keep-going all NO_UNDOC_CPU_INSTRUCTIONS=1
	-@$(MAKE) --no-print-directory --keep-going all NO_UNDOC_CPU_INSTRUCTIONS=1 INVERT_SHIFT=1
	-@$(MAKE) --no-print-directory --keep-going all NO_UNDOC_CPU_INSTRUCTIONS=1 INVERT_CTRL=1
	-@$(MAKE) --no-print-directory --keep-going all

TOOLS := $(N80) $(LK80) $(LB80) objcopy sdcc dd

.phony: prerequisites

prerequisites:
	@mkdir -p ../../bin/kernel-base
	$(foreach exec,$(TOOLS),\
        $(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites


base: nextor_base.dat


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
	bank5/B5.BIN \
	bank5/fdisk.dat \
	bank6/B6.BIN

	@for f in bank0/B0.BIN bank3/B3.BIN; do \
		sz=$$(stat -c '%s' $$f); \
		if [ $$sz -gt 16384 ]; then \
			echo "ERROR: $$f is $$sz bytes; max is 16384 (16 KB) per bank"; exit 1; \
		fi; \
	done
	@for f in bank1/B1.BIN bank2/B2.BIN bank4/B4.BIN bank5/B5.BIN bank6/B6.BIN; do \
		sz=$$(stat -c '%s' $$f); \
		if [ $$sz -gt 16129 ]; then \
			echo "ERROR: $$f is $$sz bytes; max is 16129 (16 KB minus the 4000h-40FEh doshead gap)"; exit 1; \
		fi; \
	done

	cat bank0/B0.BIN 255.bytes bank1/B1.BIN 255.bytes bank2/B2.BIN bank3/B3.BIN 255.bytes bank4/B4.BIN 255.bytes bank5/B5.BIN 255.bytes bank6/B6.BIN > nextor_base.dat
	dd conv=notrunc if=nextor_base.dat of=doshead.BIN bs=1 count=255
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=16k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=32k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=64k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=80k
	dd conv=notrunc if=doshead.BIN of=nextor_base.dat bs=1 count=255 seek=96k
	dd conv=notrunc if=bank5/fdisk.dat of=nextor_base.dat bs=1 count=16048 seek=82176

	$(call copy_to_bin,nextor_base.dat,Nextor-$(VERSION).base$(_VARIANT_SUFFIX).dat)

255.bytes:
	dd if=/dev/zero of=255.bytes bs=1 count=255

SDK_COMRELS = dos_calls.rel dos_errors.rel dos_file_handles.rel
COMRELS = $(SDK_COMRELS) kvar.rel data.rel chgbnk.rel
B0RELS = $(addprefix bank0/,doshead.rel 40ff.rel b0.rel init.rel alloc.rel dskbasic.rel dosboot.rel bdos.rel ramdrv.rel)
B1RELS = $(addprefix bank1/,dosinit.rel mapinit.rel msg.rel)
B2RELS = $(addprefix bank2/,kinit.rel char.rel dev.rel kbios.rel misc.rel seg.rel path.rel find.rel dir.rel handles.rel del.rel rw.rel files.rel buf.rel fat.rel val.rel err.rel)
B3RELS = $(addprefix bank3/,dos1ker.rel)
B4RELS = $(addprefix bank4/,jump.rel env.rel cpm.rel time.rel seg.rel misc.rel bootmenu.rel)
B5RELS = $(addprefix bank5/,b5.rel)
B6RELS = $(addprefix bank6/,b6.rel jump.rel ghost.rel format.rel bootmsg.rel idrvauto.rel)

lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,\
	$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,\
	$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

.SECONDEXPANSION:
$(filter-out $(SDK_COMRELS),$(COMRELS)) $(B0RELS) $(B1RELS) $(B2RELS) $(B3RELS) $(B4RELS) partit.rel $(B5RELS) $(B6RELS) print_call_help.rel drv.rel rel.rel: \
	$(UNDOC_MODE_STAMP) \
	macros.inc \
	const.inc \
	condasm.inc \
	$$(patsubst %.rel,%.mac,$$@)

	$(call assemble,$(patsubst %.rel,%.mac,$@),--define-symbols GENERATE_PUBLIC_SYMBOLS)


dos_calls.rel: $(UNDOC_MODE_STAMP) ../../sdk/asm/constants/dos_calls.inc
	$(call assemble_as,../../sdk/asm/constants/dos_calls.inc,dos_calls.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)

dos_errors.rel: $(UNDOC_MODE_STAMP) ../../sdk/asm/constants/dos_errors.inc
	$(call assemble_as,../../sdk/asm/constants/dos_errors.inc,dos_errors.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)

dos_file_handles.rel: $(UNDOC_MODE_STAMP) ../../sdk/asm/constants/dos_file_handles.inc
	$(call assemble_as,../../sdk/asm/constants/dos_file_handles.inc,dos_file_handles.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)


data.rel: bank4/jtable.inc bank6/jtable.inc
bank4/jump.rel: bank4/jtable.inc
bank6/jump.rel: bank6/jtable.inc

###  Bank 0

bank6/b6.rel: bank2/b2labels.inc bank0/b0lab_b6.inc

bank0/B0.BIN bank0/B0.SYM: \
	$(COMRELS) \
	$(B0RELS) \
	drv.rel \
	rel.rel

	$(call print_linking,"bank 0")
	@$(LK80) --working-dir bank0 --symbols-file B0.SYM --output-file B0.BIN \
			--code 4000h ../dos_calls.rel ../dos_errors.rel ../dos_file_handles.rel ../kvar.rel ../data.rel ../rel.rel doshead.rel 40ff.rel b0.rel init.rel alloc.rel dskbasic.rel dosboot.rel bdos.rel ramdrv.rel \
			--code 7700h ../drv.rel \
			--code 7fd0h ../chgbnk.rel

bank0/b0labels.inc: \
	bank0/B0.SYM

	$(call SymToEqus,bank0/B0.SYM,bank0/b0labels.inc,[?][^[:space:]]+|DOSV0|GETERR|BDOSE|KDERR|KABR|C4PBK|PROMPT|R_DSK[^[:space:]]+|R_GETDPB|R_CHOICE)

bank0/b0lab_b3.inc: \
	bank0/B0.SYM

	$(call SymToEqus,bank0/B0.SYM,bank0/b0lab_b3.inc,INIT|TIMINT|MAPBIO|GWRK|FDISK|PROMPT|R_[^[:space:]]+)

bank0/b0lab_b6.inc: \
	bank0/B0.SYM

	$(call SymToEqus,bank0/B0.SYM,bank0/b0lab_b6.inc,ALLOC|BDOSE|GETERR|C4PBK)


###  Bank 1

bank1/b1.rel: \
	$(UNDOC_MODE_STAMP) \
	bank1/b1.mac \
	bank0/b0labels.inc

	$(call assemble,bank1/b1.mac,--define-symbols GENERATE_PUBLIC_SYMBOLS)

bank1/B1.BIN: \
	$(COMRELS) \
	$(B1RELS) \
	bank1/msg.rel \
	bank1/b1.rel \
	bank0/alloc.rel \
	bank4/bootmenu.rel \
	bank0/b0labels.inc

	$(call print_linking,"bank 1")
	@$(LK80) --working-dir bank1 --symbols-file B1.SYM --output-file B1.BIN \
			--code 40FFh ../dos_calls.rel ../dos_errors.rel ../dos_file_handles.rel ../kvar.rel ../data.rel b1.rel dosinit.rel mapinit.rel ../bank0/alloc.rel ../bank4/bootmenu.rel msg.rel \
			--code 7fd0h ../chgbnk.rel


###  Bank 2

bank2/b2.rel: \
	$(UNDOC_MODE_STAMP) \
	bank2/b2.mac \
	bank0/b0labels.inc

	$(call assemble,bank2/b2.mac,--define-symbols GENERATE_PUBLIC_SYMBOLS)

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
			--code 40FFh ../dos_calls.rel ../dos_errors.rel ../dos_file_handles.rel ../kvar.rel ../data.rel b2.rel kinit.rel TEMP21.REL TEMP22.REL TEMP23.REL \
			--code 7fd0h ../chgbnk.rel

bank2/b2labels.inc: \
	bank2/B2.SYM

	$(call SymToEqus,bank2/B2.SYM,bank2/b2labels.inc,[?][^[:space:]]+)


###  Bank 3

bank3/b3.rel: \
	$(UNDOC_MODE_STAMP) \
	bank3/b3.mac \
	bank0/b0lab_b3.inc

	$(call assemble,bank3/b3.mac,--define-symbols GENERATE_PUBLIC_SYMBOLS)

bank3/B3.BIN: \
	$(COMRELS) \
	$(B3RELS) \
	bank3/b3.rel \
	bank0/b0lab_b3.inc \
	bank0/doshead.rel \
	bank0/40ff.rel

	$(call print_linking,"bank 3")
	@$(LK80) --working-dir bank3 --symbols-file B3.SYM --output-file B3.BIN \
			--code 4000h ../dos_calls.rel ../dos_errors.rel ../dos_file_handles.rel ../kvar.rel ../data.rel ../bank0/doshead.rel ../bank0/40ff.rel b3.rel dos1ker.rel \
			--code 7700h ../drv.rel \
			--code 7fd0h ../chgbnk.rel


###  Bank 4

bank4/b4.rel: \
	$(UNDOC_MODE_STAMP) \
	bank.inc \
	bank4/b4.mac \
	bank2/b2labels.inc

	$(call assemble,bank4/b4.mac,--define-symbols GENERATE_PUBLIC_SYMBOLS)

bank4/partit.rel: \
	$(UNDOC_MODE_STAMP) \
	bank4/partit.mac \
	bank0/b0labels.inc

	$(call assemble,bank4/partit.mac,--define-symbols GENERATE_PUBLIC_SYMBOLS)

bank4/B4.BIN bank4/B4.SYM: \
	$(COMRELS) \
	$(B4RELS) \
	bank4/partit.rel \
	print_call_help.rel \
	bank4/b4.rel \
	bank0/b0labels.inc \
	bank2/b2labels.inc \

	$(call print_linking,"bank 4")
	@$(LK80) --working-dir bank4 --symbols-file B4.SYM --output-file B4.BIN \
			--code 40FFh ../dos_calls.rel ../dos_errors.rel ../dos_file_handles.rel ../kvar.rel ../data.rel b4.rel jump.rel env.rel cpm.rel partit.rel time.rel seg.rel misc.rel ../print_call_help.rel \
			--code 7fd0h ../chgbnk.rel


###  Banks 5 and 6 (FDISK)

bank5/B5.BIN: \
	$(COMRELS) \
	$(B5RELS) \
	bank0/40ff.rel

	$(call print_linking,"bank 5")
	@$(LK80) --working-dir bank5 --symbols-file B5.SYM --output-file B5.BIN \
			--code 40FFh ../dos_calls.rel ../dos_errors.rel ../dos_file_handles.rel ../kvar.rel ../data.rel b5.rel \
			--code 7fd0h ../chgbnk.rel

bank6/B6.BIN bank6/B6.SYM: \
	$(COMRELS) \
	$(B6RELS) \
	print_call_help.rel \
	chgbnk.rel \
	bank0/b0lab_b6.inc

	$(call print_linking,"bank 6")
	@$(LK80) --working-dir bank6 --symbols-file B6.SYM --output-file B6.BIN \
			--code 40FFh ../dos_calls.rel ../dos_errors.rel ../dos_file_handles.rel ../kvar.rel ../data.rel b6.rel jump.rel ghost.rel format.rel bootmsg.rel idrvauto.rel ../print_call_help.rel \
			--code 7fd0h ../chgbnk.rel

bank5/fdisk.dat: \
	bank5/fdisk_crt0.rel \
	bank5/asmcall.rel \
	bank5/printf.rel \
	bank5/fdisk.h \
	bank5/fdisk.c \
	$(patsubst %.dat,%.c,$@) \
	bank5/drivercall.h \
	bank5/drivercall.c \
	../../sdk/C/includes/msx_bios.h \
	../../sdk/C/includes/msx_workarea.h \
	../../sdk/C/includes/dos_functions.h \
	../../sdk/C/includes/dos_errors.h \
	../../sdk/C/includes/drivers.h \
	../../sdk/C/includes/types.h \
	../../sdk/C/includes/asmcall.h \
	../../sdk/C/includes/partit.h

	sdcc -o bank5/ --code-loc 0x4120 --data-loc 0x8020 -mz80 --disable-warning 196 --disable-warning 84 --disable-warning 85 \
	     --max-allocs-per-node 1000 --allow-unsafe-read --opt-code-size --no-std-crt0 -I../../sdk/C/includes -I../../sdk/C/code bank5/fdisk_crt0.rel bank5/asmcall.rel bank5/printf.rel $(patsubst %.dat,%.c,$@)
	$(call hex2bin,$(patsubst %.dat,%.ihx,$@),$@)

bank5/fdisk_crt0.rel: \
	bank5/fdisk_crt0.s

	sdasz80 -o bank5/fdisk_crt0.rel bank5/fdisk_crt0.s

# SDK C library .rel files compiled with fdisk's flags (so they share code/data sections cleanly).
bank5/%.rel: ../../sdk/C/code/%.c
	sdcc -c -o bank5/ -mz80 --disable-warning 196 --disable-warning 84 --disable-warning 85 \
	     --max-allocs-per-node 1000 --allow-unsafe-read --opt-code-size -I../../sdk/C/includes $<

bank5/asmcall.rel: ../../sdk/C/includes/asmcall.h
bank5/printf.rel:  ../../sdk/C/includes/msx_bios.h


#################
###  CLEANUP  ###
#################

.PHONY: clean

clean:
	for ext in bin BIN HEX REL rel SYM sym bytes ihx lk lst map noi; do find . -type f -name "*.$$ext" -delete; done
	for ext in asm dat; do find ./bank5 -type f -name "*.$$ext" -delete; done
	rm -f nextor_base.dat
	rm -f bank0/b0lab*.inc
	rm -f bank2/b2labels.inc
	rm -f .build-variant*
