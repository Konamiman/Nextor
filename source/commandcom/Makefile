# Makefile for COMMAND2.COM

# The generated COMMAND2.COM file is copied to the bin/tools directory.

ifeq ($(strip $(N80)),)
N80=N80
endif

export N80_ARGS=--no-string-escapes --no-show-banner --build-type rel --verbosity 0 --output-file-case lower --include-directory ../kernel

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

.PHONY: commandcom prerequisites verify clean

TOOLS := N80 LK80

prerequisites:
	@mkdir -p ../../bin/tools
	$(foreach exec,$(TOOLS),\
		$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites

RELS := ver.rel start.rel cli.rel cmd.rel files.rel dirs.rel copy.rel io.rel misc.rel messages.rel jtext.rel var.rel
# jtext.mac contains Japanese (Shift-JIS) text, so it needs to be assembled
# with a byte-preserving single-byte encoding; codepage 437 maps all the
# bytes in the 80h-FFh range to printable characters and back unchanged
# (ISO 8859-1 wouldn't work: it maps bytes 80h-9Fh, which appear in
# Shift-JIS text, to control characters that don't survive the round trip)
MAC_RELS := $(filter-out jtext.rel,$(RELS))
SDK_RELS := dos_calls.rel dos_errors.rel dos_file_handles.rel

COMMAND2.COM: \
	$(SDK_RELS) \
	data.rel \
	$(RELS)

	@$(LK80) --output-file COMMAND2.COM --code 0100h $(SDK_RELS) data.rel $(RELS)
	$(call copy_to_bin,COMMAND2.COM)

.SECONDEXPANSION:
$(MAC_RELS): \
	$$(patsubst %.rel,%.mac,$$@) \
	command.inc

	$(call assemble,$(patsubst %.rel,%.mac,$@))

jtext.rel: jtext.mac
	$(call assemble,jtext.mac,--input-encoding 437 --string-encoding 437)

dos_calls.rel: ../../sdk/asm/constants/dos_calls.inc
	$(call assemble_as,../../sdk/asm/constants/dos_calls.inc,dos_calls.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)

dos_errors.rel: ../../sdk/asm/constants/dos_errors.inc
	$(call assemble_as,../../sdk/asm/constants/dos_errors.inc,dos_errors.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)

dos_file_handles.rel: ../../sdk/asm/constants/dos_file_handles.inc
	$(call assemble_as,../../sdk/asm/constants/dos_file_handles.inc,dos_file_handles.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)

data.rel: ../kernel/data.mac
	$(call assemble,../kernel/data.mac,--define-symbols GENERATE_PUBLIC_SYMBOLS)

# Compare the built binary against the original prebuilt 2.31 binary
# (useful while the code base is still a faithful port of version 2.31)
verify: COMMAND2.COM
	cmp COMMAND2.COM ../command/command/command2.com
	@echo "OK: COMMAND2.COM is byte-identical to the 2.31 reference binary"

clean:
	for ext in BIN HEX REL rel SYM COM; do find . -type f -name "*.$$ext" -delete; done
