# Makefile for COMMAND3.COM
#
# The generated COMMAND3.COM file is copied to the bin/tools directory.
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

BIN_TOOLS := ../../bin/tools

define assemble
	@printf "\n\033[0;36mAssembling %s\033[0m\n\n" $(1)
	@$(N80) $(1) $(2)
endef

define assemble_as
	@printf "\n\033[0;36mAssembling %s as %s\033[0m\n\n" $(1) $(2)
	@$(N80) $(1) $(2) $(3)
endef

commandcom: $(BIN_TOOLS)/COMMAND3.COM

# The bin/tools copy is a target of its own (not a side effect of the link
# recipe), so it gets re-created when bin/tools is missing or empty even if
# the COMMAND3.COM in this directory is up to date. The directory itself is
# an order-only prerequisite: it gets created on demand, but its timestamp
# changes never trigger a re-copy.
$(BIN_TOOLS)/COMMAND3.COM: COMMAND3.COM | $(BIN_TOOLS)
	cp $< $@

$(BIN_TOOLS):
	@mkdir -p $@

.PHONY: commandcom clean

TOOLS := N80 LK80

# Check that the required tools are available. This is done at parse time
# (not in a recipe) so that it runs before anything else regardless of the
# make version: the former "-include prerequisites" trick relied on make
# running a phony target's recipe at startup, which GNU make 4.4+ no longer
# does. Skipped when just cleaning.
ifneq ($(MAKECMDGOALS),clean)
$(foreach exec,$(TOOLS),\
	$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))
endif

# The link order must match the physical layout of the program.
# COMMAND3.COM is a dual-language binary: it carries both the English
# (messages.mac) and the Japanese (jmessages.mac) message tables and picks
# one at run time from the Kanji mode and the ERRLANG environment item
# (see GETMSG and SETLANG in io.mac).
RELS := start.rel cli.rel tables.rel intvar.rel cmd.rel files.rel dirs.rel \
	copy.rel io.rel batch.rel misc.rel editor.rel messages.rel \
	jmessages.rel var.rel
SDK_RELS := dos_calls.rel dos_errors.rel driver_const.rel

# The transient image plus the runtime variables must end below address
# #8000: COMMAND3 switches its private mapper segment into page 2
# (#8000-#BFFF) while it accesses the alias list, the command history and
# the variable swap area (SETPG2 in io.mac), so no part of the program -
# and the variables area includes the stack - may live there. That limits
# the .COM file to #8000 - #100 (the load address) - 2884 (the variables,
# see the total in var.mac) = 29628 bytes. Carrying both message tables
# only fits because the ~4K HELP index text lives in the INDEX.HLP and
# JINDEX.HLP files (see @HELP in copy.mac) instead of in the tables.
MAX_COM_SIZE := 29628

define check_size
	@size=$$(wc -c < $(1)); \
	if [ $$size -gt $(MAX_COM_SIZE) ]; then \
		echo "ERROR: $(1) is $$size bytes, the maximum is $(MAX_COM_SIZE): together with the runtime variables it would reach address 8000h, where the program must not live (see the MAX_COM_SIZE note in this Makefile)"; \
		rm -f $(1); \
		exit 1; \
	fi
endef

COMMAND3.COM: \
	$(SDK_RELS) \
	$(RELS)

	@$(LK80) --output-file COMMAND3.COM --code 0100h $(SDK_RELS) $(RELS)
	$(call check_size,COMMAND3.COM)

.SECONDEXPANSION:
$(RELS): \
	$$(patsubst %.rel,%.mac,$$@) \
	command.inc

	$(call assemble,$(patsubst %.rel,%.mac,$@))

dos_calls.rel: ../../sdk/asm/constants/dos_calls.inc
	$(call assemble_as,../../sdk/asm/constants/dos_calls.inc,dos_calls.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)

dos_errors.rel: ../../sdk/asm/constants/dos_errors.inc
	$(call assemble_as,../../sdk/asm/constants/dos_errors.inc,dos_errors.rel,--define-symbols GENERATE_PUBLIC_SYMBOLS)

driver_const.rel: \
	driver_const.mac \
	../../sdk/asm/constants/driver_routines.inc \
	../../sdk/asm/constants/driver_result_codes.inc \
	../../sdk/asm/constants/driver_device_queries.inc

	$(call assemble,driver_const.mac,--define-symbols GENERATE_PUBLIC_SYMBOLS)


clean:
	for ext in BIN HEX REL rel SYM COM; do find . -type f -name "*.$$ext" -delete; done
