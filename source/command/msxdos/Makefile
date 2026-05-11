# Makefile for NEXTOR.SYS

# The generated NEXTOR.SYS and NEXTOR.SYS.japanese files are copied to the bin/tools directory.

ifeq ($(strip $(N80)),)
N80=N80
endif

export N80_ARGS=--no-string-escapes --no-show-banner --build-type rel --verbosity 0 --output-file-case lower --include-directory ../../kernel

ifeq ($(strip $(LK80)),)
LK80=LK80
endif

export LK80_ARGS=--no-show-banner --verbosity 0 --output-file-case lower --output-format bin

# Optional: replace undocumented Z80 opcodes (the ones operating on
# ixh/ixl/iyh/iyl) with documented equivalents, for compatibility with
# Z180-based MSX machines. Set NO_UNDOC_CPU_INSTRUCTIONS=1 on the make
# command line. See sdk/asm/macros/undoc.inc for details.
ifeq ($(strip $(NO_UNDOC_CPU_INSTRUCTIONS)),)
UNDOC_DEFINE=
else
UNDOC_DEFINE=NO_UNDOC_CPU_INSTRUCTIONS
endif

define copy_to_bin
	cp $(1) ../../../bin/tools/$(2)
endef

define assemble
	@printf "\n\033[0;36mAssembling %s\033[0m\n\n" $(1)
	@$(N80) $(1) $(2) $(if $(UNDOC_DEFINE),--define-symbols $(UNDOC_DEFINE))
endef

define assemble_as
	@printf "\n\033[0;36mAssembling %s as %s\033[0m\n\n" $(1) $(2)
	@$(N80) $(1) $(2) $(3) $(if $(UNDOC_DEFINE),--define-symbols $(UNDOC_DEFINE))
endef

nextor-sys: NEXTOR.SYS NEXTORK.SYS

.phony: prerequisites

TOOLS := N80 LK80

prerequisites:
	@mkdir -p ../../../bin/tools
	$(foreach exec,$(TOOLS),\
		$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites

INCS := ../../kernel/macros.inc ../../kernel/const.inc ../../kernel/condasm.inc
RELS := messages.rel real.rel ref.rel reloc.rel ver.rel end.rel
JRELS := $(RELS) kmsg.rel
SDK_RELS := dos_calls.rel dos_errors.rel dos_file_handles.rel

NEXTOR.SYS: \
	$(RELS) \
	$(SDK_RELS) \
	data.rel \
	sys.mac

	$(call assemble,sys.mac,--define-symbols NO_J_MSG --define-symbols GENERATE_PUBLIC_SYMBOLS)
	@$(LK80) --output-file NEXTOR.SYS --code 0100h $(SDK_RELS) data.rel reloc.rel ver.rel ref.rel sys.rel real.rel sys.rel messages.rel end.rel
	$(call copy_to_bin,NEXTOR.SYS)

NEXTORK.SYS: \
	$(JRELS) \
	$(SDK_RELS) \
	data.rel \
	sys.mac

	$(call assemble,sys.mac,--define-symbols GENERATE_PUBLIC_SYMBOLS)
	@$(LK80) --output-file NEXTOR.SYS.japanese --code 0100h $(SDK_RELS) data.rel reloc.rel ver.rel ref.rel sys.rel real.rel sys.rel messages.rel kmsg.rel end.rel
	$(call copy_to_bin,NEXTOR.SYS.japanese)

.SECONDEXPANSION:
$(JRELS): \
	$$(patsubst %.rel,%.mac,$$@) \
	$(INCS) \

	$(call assemble,$(patsubst %.rel,%.mac,$@),--define-symbols GENERATE_PUBLIC_SYMBOLS)

dos_calls.rel: ../../../sdk/asm/constants/dos_calls.inc
	$(call assemble_as,../../../sdk/asm/constants/dos_calls.inc,dos_calls.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)

dos_errors.rel: ../../../sdk/asm/constants/dos_errors.inc
	$(call assemble_as,../../../sdk/asm/constants/dos_errors.inc,dos_errors.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)

dos_file_handles.rel: ../../../sdk/asm/constants/dos_file_handles.inc
	$(call assemble_as,../../../sdk/asm/constants/dos_file_handles.inc,dos_file_handles.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)

data.rel: \
	../../kernel/data.mac \
	$(INCS)

	$(call assemble,../../kernel/data.mac,--define-symbols GENERATE_PUBLIC_SYMBOLS)

.PHONY: clean

clean:
	for ext in BIN HEX REL rel SYM; do find . -type f -name "*.$$ext" -delete; done
