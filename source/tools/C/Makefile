# Makefile for the command line tools written in C.
# The list of files to process is in the COMS variable: the generated .COM
# files get uppercase names (as they will appear in a real MSX filesystem),
# each built from the matching lowercase .c file.

# The generated files are copied to the bin/tools directory.

define hex2bin
	objcopy -I ihex -O binary $(1) $(2)
endef

define copy_to_bin
	cp $(1) ../../../bin/tools/$(2)
endef

lc = $(shell echo $(1) | tr A-Z a-z)

COMS := EMUFILE.COM FSIZE.COM NEXBOOT.COM VSFT.COM EPTCFT.COM DRVROP.COM DRVTEST.COM

c-tools: $(COMS)

.phony: prerequisites

TOOLS := sdcc sdasz80 objcopy

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
	for ext in asm hex ihx lk lst map noi sym rel com COM; do find . -type f -name "*.$$ext" -delete; done
