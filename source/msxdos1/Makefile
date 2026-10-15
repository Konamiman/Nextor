# Makefile for MSXDOS.SYS 1.03 and COMMAND.COM 1.11, the MSX-DOS 1 system
# files.
#
# The files are built in the build directory and copied to the bin/tools
# directory.
#
# How the relocation tables are generated: each program has a "relocatable
# part" that gets copied to high memory and adjusted there for its new
# address, using a table of the bytes that hold the high byte of an address
# (see msxdos/msxdos.mac). This part is assembled alone twice, at two
# addresses one page apart, and the two results are compared with "cmp -l":
# the differing bytes are the ones that need relocating, and tools/reltab.awk
# turns their offsets into reltab.inc, which is then included when
# assembling the complete program. The build fails if cmp fails, if the two
# assemblies have different sizes, or if no bytes differ (see reltab.awk).

ifeq ($(strip $(N80)),)
N80=N80
endif

BUILD := build
BIN_TOOLS := ../../bin/tools

N80_ARGS := --no-show-banner --verbosity 0 --build-type abs \
	--no-string-escapes --include-directory ../../sdk

INCS := include/dos1.inc

# Two arbitrary page aligned addresses, one page apart, for the two
# assemblies of the relocatable part.
RELBASE1 := 1000h
RELBASE2 := 1100h

define assemble
	@printf "\n\033[0;36mAssembling %s\033[0m\n\n" $(1)
	@$(N80) $(1) $(2) $(N80_ARGS) $(3)
endef

msxdos1: $(BIN_TOOLS)/MSXDOS.SYS $(BIN_TOOLS)/COMMAND.COM

# The bin/tools copies are targets of their own (not a side effect of the
# assembly recipes), so they get re-created when bin/tools is missing or
# empty even if the files in the build directory are up to date. The
# directory itself is an order-only prerequisite: it gets created on demand,
# but its timestamp changes never trigger a re-copy.
$(BIN_TOOLS)/%: $(BUILD)/% | $(BIN_TOOLS)
	cp $< $@

$(BIN_TOOLS):
	@mkdir -p $@

.PHONY: msxdos1 clean

TOOLS := N80

# Check that the required tools are available. This is done at parse time
# (not in a recipe) so that it runs before anything else regardless of the
# make version. Skipped when just cleaning.
ifneq ($(MAKECMDGOALS),clean)
$(foreach exec,$(TOOLS),\
	$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))
endif

# $(call program,<output file>,<source directory>,<master file>)
define program
$(BUILD)/$(2)/reltab.inc: $(wildcard $(2)/*.mac) $(INCS) tools/reltab.awk | $(BUILD)/$(2)
	$$(call assemble,$(2)/$(3),$(BUILD)/$(2)/rel1.bin,--define-symbols RELBASE=$(RELBASE1))
	$$(call assemble,$(2)/$(3),$(BUILD)/$(2)/rel2.bin,--define-symbols RELBASE=$(RELBASE2))
	@cmp -l $(BUILD)/$(2)/rel1.bin $(BUILD)/$(2)/rel2.bin > $$@.cmp; [ $$$$? -le 1 ]
	@[ $$$$(wc -c < $(BUILD)/$(2)/rel1.bin) -eq $$$$(wc -c < $(BUILD)/$(2)/rel2.bin) ] || \
		{ echo "ERROR: the two assemblies of $(2)/$(3) have different sizes" >&2; exit 1; }
	@awk -f tools/reltab.awk $$@.cmp > $$@.tmp
	@rm $$@.cmp
	@mv $$@.tmp $$@

$(BUILD)/$(1): $(wildcard $(2)/*.mac) $(INCS) $(BUILD)/$(2)/reltab.inc
	$$(call assemble,$(2)/$(3),$$@,--include-directory $(BUILD)/$(2))

$(BUILD)/$(2):
	@mkdir -p $$@
endef

$(eval $(call program,MSXDOS.SYS,msxdos,msxdos.mac))
$(eval $(call program,COMMAND.COM,command,command.mac))

clean:
	rm -rf $(BUILD)
