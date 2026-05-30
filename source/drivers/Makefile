# Makefile for the Nextor standalone ROM drivers and example RAM driver.
#
# Default target builds:
#   - The kernel base file (Nextor-$(VERSION).base$(_VARIANT_SUFFIX).dat).
#   - The Standalone ASCII8 ROM.
#   - The Standalone ASCII16 ROM.
#
# Both standalone ROMs are built from a single driver source
# (standalone-rom-driver.asm); only the bank switching routine differs.
#
# The example RAM driver (ram-driver-example.asm) is opt-in via the
# `ram-example` target.
#
# Build variants: same three knobs as the kernel Makefile. Each, when
# non-empty, both defines a symbol passed to N80 via --define-symbols
# and adds a fragment to the output-filename suffix.
#
#   NO_UNDOC_CPU_INSTRUCTIONS  -> defines NO_UNDOC_CPU_INSTRUCTIONS,
#                                 suffix fragment ".NO_UNDOC"
#   INVERT_SHIFT               -> defines INVERT_SHIFT, suffix fragment
#                                 ".SHIFT_INV"
#   INVERT_CTRL                -> defines INVERT_CTRL, suffix fragment
#                                 ".CTRL_INV"
#
# Outputs are copied to <repo>/bin/kernel-base (kernel base files),
# <repo>/bin/roms (standalone ROMs), and <repo>/bin/ram-drivers (RAM
# drivers).
#
# The N80 and MKNEXROM binaries to use can be overriden with environment
# variables, e.g.:
#     N80=/path/to/N80 make
#
# To use a pre-built kernel base file (e.g. one produced by a separate
# `make` invocation in ../kernel), set NEXTOR_BASE to its path. When
# NEXTOR_BASE is not set, the default target recurses into ../kernel to
# build it.

VERSION := 3.0.0

ifeq ($(strip $(N80)),)
N80=N80
endif

ifeq ($(strip $(MKNEXROM)),)
MKNEXROM=mknexrom
endif

_VARIANT_DEFINES :=
_VARIANT_SUFFIX :=

ifneq ($(strip $(NO_UNDOC_CPU_INSTRUCTIONS)),)
_VARIANT_DEFINES += NO_UNDOC_CPU_INSTRUCTIONS
_VARIANT_SUFFIX := $(_VARIANT_SUFFIX).NO_UNDOC
endif

ifneq ($(strip $(INVERT_SHIFT)),)
_VARIANT_DEFINES += INVERT_SHIFT
_VARIANT_SUFFIX := $(_VARIANT_SUFFIX).SHIFT_INV
endif

ifneq ($(strip $(INVERT_CTRL)),)
_VARIANT_DEFINES += INVERT_CTRL
_VARIANT_SUFFIX := $(_VARIANT_SUFFIX).CTRL_INV
endif

UNDOC_MODE_STAMP := .build-variant$(if $(_VARIANT_SUFFIX),$(_VARIANT_SUFFIX),.default)

$(UNDOC_MODE_STAMP):
	@rm -f .build-variant*
	@touch $@

define assemble
	@printf "\n\033[0;36mAssembling %s\033[0m\n\n" $(1)
	@$(N80) $(1) $$ $(2) $(foreach def,$(_VARIANT_DEFINES),--define-symbols $(def))
endef


###################
###  MAIN RULE  ###
###################

.PHONY: all base ascii8 ascii16 ram-example clean

all: base ascii8 ascii16

# `everything` builds the kernel base file and both standalone ROMs
# across all six variant combinations (default, INVERT_SHIFT, INVERT_CTRL,
# NO_UNDOC, NO_UNDOC+INVERT_SHIFT, NO_UNDOC+INVERT_CTRL). Each variant
# runs as a recursive make invocation with the corresponding env vars
# set. The default variant is built last so the on-disk intermediate
# files are left in the non-inverted, undoc state.
#
# Sub-make failures don't abort the matrix — the loop collects the worst
# exit code and exits with it at the end, so CI / release scripts see a
# failure if any variant broke while still attempting all six.
.PHONY: everything
everything:
	@status=0; \
	for args in \
	    "INVERT_SHIFT=1" \
	    "INVERT_CTRL=1" \
	    "NO_UNDOC_CPU_INSTRUCTIONS=1" \
	    "NO_UNDOC_CPU_INSTRUCTIONS=1 INVERT_SHIFT=1" \
	    "NO_UNDOC_CPU_INSTRUCTIONS=1 INVERT_CTRL=1" \
	    ""; do \
	    $(MAKE) --no-print-directory --keep-going all $$args || status=$$?; \
	done; \
	exit $$status


###################
###  TOOLCHECK  ###
###################

TOOLS := $(N80) $(MKNEXROM) dd cat

.PHONY: prerequisites

prerequisites:
	@mkdir -p ../../bin/kernel-base ../../bin/roms ../../bin/ram-drivers
	$(foreach exec,$(TOOLS),\
        $(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites


########################
###  KERNEL BASE     ###
########################

# Path to the kernel base file used as input by MKNEXROM. If NEXTOR_BASE
# is set, use it as-is. Otherwise build it via the kernel Makefile and
# stash a local copy named after the active variant.

ifeq ($(strip $(NEXTOR_BASE)),)

LOCAL_BASE := Nextor-$(VERSION).base$(_VARIANT_SUFFIX).dat

base: $(LOCAL_BASE)

$(LOCAL_BASE):
	@printf "\n\033[0;36mBuilding kernel base via ../kernel\033[0m\n\n"
	@$(MAKE) -C ../kernel base \
	    NO_UNDOC_CPU_INSTRUCTIONS=$(NO_UNDOC_CPU_INSTRUCTIONS) \
	    INVERT_SHIFT=$(INVERT_SHIFT) \
	    INVERT_CTRL=$(INVERT_CTRL)
	cp ../kernel/nextor_base.dat $@
	@mkdir -p ../../bin/kernel-base
	cp $@ ../../bin/kernel-base/$@

NEXTOR_BASE := $(LOCAL_BASE)

else

base:
	@printf "Using pre-built kernel base: $(NEXTOR_BASE)\n"

endif


########################
###  STANDALONE ROMS ###
########################

ROM_ASCII8  := Nextor-$(VERSION).StandaloneASCII8$(_VARIANT_SUFFIX).ROM
ROM_ASCII16 := Nextor-$(VERSION).StandaloneASCII16$(_VARIANT_SUFFIX).ROM

ascii8:  $(ROM_ASCII8)
ascii16: $(ROM_ASCII16)

standalone-rom-driver.bin: \
	$(UNDOC_MODE_STAMP) \
	standalone-rom-driver.asm

	$(call assemble,standalone-rom-driver.asm,--build-type abs --output-file-extension bin)

chgbnk-ascii8.bin: \
	$(UNDOC_MODE_STAMP) \
	chgbnk-ascii8.asm

	$(call assemble,chgbnk-ascii8.asm,--build-type abs --output-file-extension bin)

chgbnk-ascii16.bin: \
	$(UNDOC_MODE_STAMP) \
	chgbnk-ascii16.asm

	$(call assemble,chgbnk-ascii16.asm,--build-type abs --output-file-extension bin)

# The standalone driver is built at 4100h; MKNEXROM expects it to start
# at 4000h (the first 256 bytes are reserved for the ROM header), so
# we prepend 256 zero bytes before passing it to MKNEXROM.

_standalone-rom-driver.bin: standalone-rom-driver.bin 256.bytes
	cat 256.bytes standalone-rom-driver.bin > $@

$(ROM_ASCII8): \
	$(NEXTOR_BASE) \
	_standalone-rom-driver.bin \
	chgbnk-ascii8.bin

	$(MKNEXROM) $(NEXTOR_BASE) $@ /d:_standalone-rom-driver.bin /m:chgbnk-ascii8.bin
	@mkdir -p ../../bin/roms
	cp $@ ../../bin/roms/$@

$(ROM_ASCII16): \
	$(NEXTOR_BASE) \
	_standalone-rom-driver.bin \
	chgbnk-ascii16.bin

	$(MKNEXROM) $(NEXTOR_BASE) $@ /d:_standalone-rom-driver.bin /m:chgbnk-ascii16.bin
	@mkdir -p ../../bin/roms
	cp $@ ../../bin/roms/$@


########################
###  RAM DRIVER      ###
########################

# Example RAM driver (loadable .drv). Opt-in: not built by `make all`.

ram-example: ram-driver-example.drv

ram-driver-example.drv: \
	$(UNDOC_MODE_STAMP) \
	ram-driver-example.asm

	$(call assemble,ram-driver-example.asm,--build-type abs --output-file-extension drv)
	@mkdir -p ../../bin/ram-drivers
	cp $@ ../../bin/ram-drivers/$@


########################
###  COMMON          ###
########################

256.bytes:
	dd if=/dev/zero of=256.bytes bs=1 count=256

clean:
	rm -f *.bin *.drv *.ROM *.dat 256.bytes _standalone-rom-driver.bin .build-variant*
