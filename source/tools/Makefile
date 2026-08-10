# Makefile for the command line tools written in assembler.
# The list of files to process is in the COMS variable.

# The generated files are copied to the bin/tools directory.

ifeq ($(strip $(N80)),)
N80=N80
endif

export N80_ARGS=--no-string-escapes --no-show-banner --build-type rel --verbosity 0 --output-file-case upper --include-directory ../kernel

ifeq ($(strip $(LK80)),)
LK80=LK80
endif

export LK80_ARGS=--no-show-banner --verbosity 0 --output-file-case lower --output-format bin --code-before-data

define copy_to_bin
	cp $(1) ../../bin/tools/$(2)
endef

define assemble
	@printf "\n\033[0;36mAssembling %s\033[0m\n\n" $(1)
	@$(N80) $(1) $(2)
endef

define assemble_as
	@printf "\n\033[0;36mAssembling %s as %s\033[0m\n\n" $(1) $(2)
	@$(N80) $(1) $(2) $(3)
endef

uc = $(shell echo $(1) | tr a-z A-Z)

COMS := CONCLUS.COM DELALL.COM DEVINFO.COM DRIVERS.COM DRVINFO.COM FASTOUT.COM LOCK.COM MAPDRV.COM NSYSVER.COM RALLOC.COM
INCS := ../kernel/macros.inc ../kernel/const.inc
SDK_RELS := DOS_CALLS.REL DOS_ERRORS.REL DOS_FILE_HANDLES.REL
SHARED_NAMES := byte2asc extpar chk_nextor chkonoff chklet chklet_noterm prslot prpad ext8 extnum getslot
SHARED_RELS  := $(foreach n,$(SHARED_NAMES),$(call uc,$(n)).REL)
RELS := $(SDK_RELS) DATA.REL $(SHARED_RELS)

asm-tools: $(COMS)

.phony: prerequisites

TOOLS := N80 LK80

prerequisites:
	@mkdir -p ../../bin/tools
	$(foreach exec,$(TOOLS),\
		$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites

.SECONDEXPANSION:
$(COMS): \
	$$(patsubst %.COM,%.MAC,$$@) \
	$(INCS) \
	$(SDK_RELS) \
	DATA.REL \
	$(SHARED_RELS)

	$(call assemble,$(patsubst %.COM,%.MAC,$@),--define-symbols GENERATE_PUBLIC_SYMBOLS)
	@$(LK80) --output-file $@ --code 0100h $(SDK_RELS) DATA.REL $(patsubst %.COM,%.REL,$@) $(SHARED_RELS)
	$(call copy_to_bin,$@)

# Build rules for each individual SDK shared routine .asm file (one per public routine).
define shared_rel_rule
$(2).REL: ../../sdk/asm/code/$(1).asm
	$$(call assemble_as,../../sdk/asm/code/$(1).asm,$(2).REL,--define-symbols GENERATE_PUBLIC_SYMBOLS --define-symbols COM_FILE)
endef

$(foreach n,$(SHARED_NAMES),$(eval $(call shared_rel_rule,$(n),$(call uc,$(n)))))

DOS_CALLS.REL: ../../sdk/asm/constants/dos_calls.inc ../../sdk/asm/macros/const.inc
	$(call assemble_as,../../sdk/asm/constants/dos_calls.inc,DOS_CALLS.REL,--define-symbols GENERATE_PUBLIC_SYMBOLS)

DOS_ERRORS.REL: ../../sdk/asm/constants/dos_errors.inc ../../sdk/asm/macros/const.inc
	$(call assemble_as,../../sdk/asm/constants/dos_errors.inc,DOS_ERRORS.REL,--define-symbols GENERATE_PUBLIC_SYMBOLS)

DOS_FILE_HANDLES.REL: ../../sdk/asm/constants/dos_file_handles.inc ../../sdk/asm/macros/const.inc
	$(call assemble_as,../../sdk/asm/constants/dos_file_handles.inc,DOS_FILE_HANDLES.REL,--define-symbols GENERATE_PUBLIC_SYMBOLS)

DATA.REL: \
	../kernel/data.mac \
	$(INCS) \
	../../sdk/asm/macros/const.inc \
	../../sdk/asm/constants/driver_routines.inc \
	../../sdk/asm/constants/driver_result_codes.inc \
	../../sdk/asm/constants/driver_driver_queries.inc \
	../../sdk/asm/constants/driver_device_queries.inc

	$(call assemble,../kernel/data.mac,--define-symbols GENERATE_PUBLIC_SYMBOLS)

.PHONY: clean

clean:
	for ext in REL rel SYM sym HEX COM; do find . -type f -name "*.$$ext" -delete; done
