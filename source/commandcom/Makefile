# Makefile for COMMAND2.COM
#
# The generated COMMAND2.COM file is copied to the bin/tools directory.
#
# The modules are assembled as separate REL modules linked with LK80 (same
# scheme as NEXTOR.SYS). The code base is the COMMAND2.COM 2.41 port,
# restyled to the conventions of this repository and now diverging from
# 2.41 as the 2.44 features and the Nextor 3.0 changes are implemented.

ifeq ($(strip $(N80)),)
N80=N80
endif

export N80_ARGS=--no-string-escapes --no-show-banner --build-type rel --verbosity 0 --output-file-case lower

ifeq ($(strip $(LK80)),)
LK80=LK80
endif

export LK80_ARGS=--no-show-banner --verbosity 0 --output-file-case lower --output-format bin

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

commandcom: COMMAND2.COM

.PHONY: commandcom prerequisites clean

TOOLS := N80 LK80

prerequisites:
	@mkdir -p ../../bin/tools
	$(foreach exec,$(TOOLS),\
		$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites

# The link order must match the physical layout of the program
RELS := start.rel cli.rel tables.rel intvar.rel cmd.rel files.rel dirs.rel \
	copy.rel io.rel batch.rel misc.rel editor.rel messages.rel var.rel
SDK_RELS := dos_calls.rel dos_errors.rel

COMMAND2.COM: \
	$(SDK_RELS) \
	$(RELS)

	@$(LK80) --output-file COMMAND2.COM --code 0100h $(SDK_RELS) $(RELS)
	$(call copy_to_bin,COMMAND2.COM)

.SECONDEXPANSION:
$(RELS): \
	$$(patsubst %.rel,%.mac,$$@) \
	command.inc

	$(call assemble,$(patsubst %.rel,%.mac,$@))

dos_calls.rel: ../../sdk/asm/constants/dos_calls.inc
	$(call assemble_as,../../sdk/asm/constants/dos_calls.inc,dos_calls.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)

dos_errors.rel: ../../sdk/asm/constants/dos_errors.inc
	$(call assemble_as,../../sdk/asm/constants/dos_errors.inc,dos_errors.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)


clean:
	for ext in BIN HEX REL rel SYM COM; do find . -type f -name "*.$$ext" -delete; done
