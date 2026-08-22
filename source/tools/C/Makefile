# Makefile for the command line tools written in C.
# The list of files to process is in the COMS variable: the generated .COM
# files get uppercase names (as they will appear in a real MSX filesystem),
# each built from the matching lowercase .c file.

# The generated files are copied to the bin/tools directory.

# Nestor80 is used to assemble the message table files of the bilingual
# tools (*_msgs.mac -> SDCC relocatable format, linked with the C code).
ifeq ($(strip $(N80)),)
N80=N80
endif

define hex2bin
	objcopy -I ihex -O binary $(1) $(2)
endef

define copy_to_bin
	cp $(1) ../../../bin/tools/$(2)
endef

lc = $(shell echo $(1) | tr A-Z a-z)

COMS := EMUFILE.COM FSIZE.COM NEXBOOT.COM VSFT.COM EPTCFT.COM DRVROP.COM DRVTEST.COM

# Tools with their own build rule below (banner-bearing crt0, message
# tables assembled with Nestor80), excluded from the generic rule.
SPECIAL_COMS := DISKCOPY.COM XDIR.COM XCOPY.COM

c-tools: $(COMS) $(SPECIAL_COMS)

.phony: prerequisites

TOOLS := sdcc sdasz80 objcopy N80

prerequisites:
	@mkdir -p ../../../bin/tools
	$(foreach exec,$(TOOLS),\
		$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites

SDK_C_RELS := asmcall.rel printf.rel print_msxdos.rel strcmpi.rel

# The trailing rm removes the lowercase-named copy that builds prior to the
# uppercase renaming of the tools left in bin/tools, so that stale duplicates
# don't end up in the tools disk image.
.SECONDEXPANSION:
$(COMS): \
	$$(call lc,$$(patsubst %.COM,%.c,$$@)) \
	crt0_msxdos.rel \
	$(SDK_C_RELS) \
	../../../sdk/C/includes/asmcall.h

	sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196 --disable-warning 85 --no-std-crt0 -I../../../sdk/C/includes -I../../../sdk/C/code crt0_msxdos.rel $(SDK_C_RELS) $(call lc,$(patsubst %.COM,%.c,$@))
	$(call hex2bin,$(call lc,$(patsubst %.COM,%.ihx,$@)),$@)
	$(call copy_to_bin,$@)
	@rm -f ../../../bin/tools/$(call lc,$@)

crt0_msxdos.rel: \
	../../../sdk/C/code/crt0_msxdos.asm

	sdasz80 -o crt0_msxdos.rel ../../../sdk/C/code/crt0_msxdos.asm

# DISKCOPY: uses its own crt0 (with the TYPE-able banner) and message
# tables assembled from diskcopy_msgs.mac (Nestor80 converts the
# Japanese table to Shift-JIS at assembly time via .strenc).  The
# banner makes the header bigger, hence the higher --code-loc.
DISKCOPY.COM: \
	diskcopy.c \
	diskcopy_msgs.rel \
	crt0_diskcopy.rel \
	$(SDK_C_RELS) \
	../../../sdk/C/includes/asmcall.h

	sdcc --code-loc 0x480 --data-loc 0 -mz80 --disable-warning 196 --disable-warning 85 --no-std-crt0 -I../../../sdk/C/includes -I../../../sdk/C/code crt0_diskcopy.rel $(SDK_C_RELS) diskcopy_msgs.rel diskcopy.c
	$(call hex2bin,diskcopy.ihx,$@)
	$(call copy_to_bin,$@)
	@rm -f ../../../bin/tools/diskcopy.com

crt0_diskcopy.rel: crt0_diskcopy.mac
	sdasz80 -o crt0_diskcopy.rel crt0_diskcopy.mac

# XDIR: same scheme as DISKCOPY (own crt0 with the TYPE-able banner,
# message tables assembled with Nestor80).
XDIR.COM: \
	xdir.c \
	xdir_msgs.rel \
	crt0_xdir.rel \
	$(SDK_C_RELS) \
	../../../sdk/C/includes/asmcall.h

	sdcc --code-loc 0x400 --data-loc 0 -mz80 --disable-warning 196 --disable-warning 85 --no-std-crt0 -I../../../sdk/C/includes -I../../../sdk/C/code crt0_xdir.rel $(SDK_C_RELS) xdir_msgs.rel xdir.c
	$(call hex2bin,xdir.ihx,$@)
	$(call copy_to_bin,$@)
	@rm -f ../../../bin/tools/xdir.com

crt0_xdir.rel: crt0_xdir.mac
	sdasz80 -o crt0_xdir.rel crt0_xdir.mac

# XCOPY: same scheme as DISKCOPY (own crt0 with the TYPE-able banner,
# message tables assembled with Nestor80).
XCOPY.COM: \
	xcopy.c \
	xcopy_msgs.rel \
	crt0_xcopy.rel \
	$(SDK_C_RELS) \
	../../../sdk/C/includes/asmcall.h

	sdcc --code-loc 0x600 --data-loc 0 -mz80 --disable-warning 196 --disable-warning 85 --no-std-crt0 -I../../../sdk/C/includes -I../../../sdk/C/code crt0_xcopy.rel $(SDK_C_RELS) xcopy_msgs.rel xcopy.c
	$(call hex2bin,xcopy.ihx,$@)
	$(call copy_to_bin,$@)
	@rm -f ../../../bin/tools/xcopy.com

crt0_xcopy.rel: crt0_xcopy.mac
	sdasz80 -o crt0_xcopy.rel crt0_xcopy.mac

# Message table files: assembled to SDCC relocatable format, so the C
# code sees the tables as extern arrays.
%_msgs.rel: %_msgs.mac
	$(N80) $< $@ --build-type sdcc --no-show-banner --verbosity 0

# SDK C library .rel files (compiled once, linked into every tool).
asmcall.rel: ../../../sdk/C/code/asmcall.c ../../../sdk/C/includes/asmcall.h
	sdcc -c -mz80 --disable-warning 196 --disable-warning 85 -I../../../sdk/C/includes ../../../sdk/C/code/asmcall.c

printf.rel: ../../../sdk/C/code/printf.c ../../../sdk/C/includes/msx_bios.h
	sdcc -c -mz80 --disable-warning 196 --disable-warning 85 --max-allocs-per-node 100000 --allow-unsafe-read --opt-code-size -DCOM_FILE -DSUPPORT_LONG -I../../../sdk/C/includes ../../../sdk/C/code/printf.c

print_msxdos.rel: ../../../sdk/C/code/print_msxdos.c
	sdcc -c -mz80 --disable-warning 196 --disable-warning 85 -I../../../sdk/C/includes ../../../sdk/C/code/print_msxdos.c

strcmpi.rel: ../../../sdk/C/code/strcmpi.c ../../../sdk/C/includes/strcmpi.h
	sdcc -c -mz80 --disable-warning 196 --disable-warning 85 -I../../../sdk/C/includes ../../../sdk/C/code/strcmpi.c

.PHONY: clean

clean:
	for ext in asm hex ihx lk lst map noi sym rel com COM; do find . -type f -name "*.$$ext" ! -name "crt0_*.asm" -delete; done
