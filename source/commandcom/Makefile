# Makefile for COMMAND2.COM
#
# The generated COMMAND2.COM file is copied to the bin/tools directory.
#
# Current state (2.41 porting phase): the modules hold the COMMAND2.COM 2.41
# code (mechanically split from the released 2.41 source) and are assembled
# as a single absolute program via the command2.mac master include file.
# The "verify" target checks that the binary is byte-identical to
# command241.com, the reference build of the unmodified 2.41 source; run it
# after every change while the code is being restyled.

ifeq ($(strip $(N80)),)
N80=N80
endif

export N80_ARGS=--no-string-escapes --no-show-banner --build-type abs --verbosity 0

define copy_to_bin
	cp $(1) ../../bin/tools/$(2)
endef

define assemble_as
	@printf "\n\033[0;36mAssembling %s as %s\033[0m\n\n" $(1) $(2)
	@$(N80) $(1) $(2) $(3)
endef

commandcom: COMMAND2.COM

.PHONY: commandcom prerequisites verify clean

prerequisites:
	@mkdir -p ../../bin/tools
	$(if $(shell which $(N80)),,$(error "ERROR: can't execute N80, is it installed/in PATH?"))

-include prerequisites

MODULES := command.inc start.mac cli.mac tables.mac intvar.mac cmd.mac \
	files.mac dirs.mac copy.mac io.mac batch.mac misc.mac editor.mac \
	messages.mac var.mac

COMMAND2.COM: command2.mac $(MODULES)
	$(call assemble_as,command2.mac,COMMAND2.COM)
	$(call copy_to_bin,COMMAND2.COM)

# Compare the built binary against the reference build of the original
# 2.41 source (useful while the code base is being restyled and must stay
# functionally identical to version 2.41)
verify: COMMAND2.COM
	cmp COMMAND2.COM command241.com
	@echo "OK: COMMAND2.COM is byte-identical to the 2.41 reference binary"

clean:
	for ext in BIN HEX REL rel SYM COM; do find . -type f -name "*.$$ext" -delete; done
