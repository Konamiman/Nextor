# Makefile for the command line tools written in assembler.
# The list of files to process is in the COMS variable.

# The generated files are copied to the bin/tools directory.

export X80_COMMAND_LINE=-t -nb
export M80_COMMAND_LINE=-8

define hex2bin
	objcopy -I ihex -O binary $(1) $(2)
endef

define copy_to_bin
	cp $(1) ../../bin/tools/$(2)
endef

COMS := CONCLUS.COM DELALL.COM DEVINFO.COM DRIVERS.COM DRVINFO.COM FASTOUT.COM LOCK.COM MAPDRV.COM NSYSVER.COM RALLOC.COM
INCS := ../kernel/macros.inc ../kernel/const.inc
RELS := CODES.REL DATA.REL SHARED.REL

asm-tools: $(COMS)

.phony: prerequisites

TOOLS := M80 L80 objcopy

prerequisites:
	@mkdir -p ../../bin/tools
	$(foreach exec,$(TOOLS),\
		$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites

.SECONDEXPANSION:
$(COMS): \
	$$(patsubst %.COM,%.MAC,$$@) \
	$(INCS) \
	CODES.REL \
	DATA.REL \
	SHARED.REL

	echo $(patsubst %.COM,%.MAC,$@)
	M80 =$(patsubst %.COM,%,$@)
	L80 /P:100,CODES,DATA,$(patsubst %.COM,%,$@),SHARED,$(patsubst %.COM,%,$@)/n/x/y/e
	$(call hex2bin,$(patsubst %.COM,%.HEX,$@),$@)
	$(call copy_to_bin,$@)

SHARED.REL: \
	SHARED.MAC \
	$(INCS)

	M80 -p ../kernel =SHARED

CODES.REL: \
	../kernel/codes.mac \
	$(INCS)

	M80 -p ../kernel =CODES

DATA.REL: \
	../kernel/data.mac \
	$(INCS)

	M80 -p ../kernel =DATA

.PHONY: clean

clean:
	for ext in REL SYM HEX COM; do find . -type f -name "*.$$ext" -delete; done
