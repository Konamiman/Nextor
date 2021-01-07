# Makefile for the command line tools written in C.
# The list of files to process is in the COMS variable.

# The generated files are copied to the bin/tools directory.

define hex2bin
	objcopy -I ihex -O binary $(1) $(2)
endef

define copy_to_bin
	cp $(1) ../../../bin/tools/$(2)
endef

COMS := emufile.com fsize.com nexboot.com vsft.com

c-tools: $(COMS)

.phony: prerequisites

TOOLS := sdcc sdasz80 objcopy

prerequisites:
	@mkdir -p ../../../bin/tools
	$(foreach exec,$(TOOLS),\
		$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites

.SECONDEXPANSION:
$(COMS): \
	$$(patsubst %.com,%.c,$$@) \
	crt0_msxdos_advanced.rel

	sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196 --disable-warning 85 --no-std-crt0 crt0_msxdos_advanced.rel $(patsubst %.com,%.c,$@)
	$(call hex2bin,$(patsubst %.com,%.ihx,$@),$@)
	$(call copy_to_bin,$@)

crt0_msxdos_advanced.rel: \
	crt0_msxdos_advanced.s

	sdasz80 -o crt0_msxdos_advanced.rel crt0_msxdos_advanced.s

.PHONY: clean

clean:
	for ext in asm hex ihx lk lst map noi sym rel com; do find . -type f -name "*.$$ext" -delete; done
