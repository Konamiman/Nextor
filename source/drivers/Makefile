# Makefile for the Nextor standalone ROM drivers and example RAM driver.
#
# Default target builds:
#   - The kernel base file (Nextor-$(NEXTOR_KERNEL_VERSION).base$(_VARIANT_SUFFIX).dat).
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
# All generated files (standalone ROMs, RAM drivers and the assembly
# intermediates) go to <repo>/bin/drivers; nothing is left in this
# source directory. The kernel base file is published to
# <repo>/bin/kernel-base, the same location the kernel makefile
# copies it to.
#
# The N80 and MKNEXROM binaries to use can be overriden with environment
# variables, e.g.:
#     N80=/path/to/N80 make
#
# To use a pre-built kernel base file (e.g. one produced by a separate
# `make` invocation in ../kernel), set NEXTOR_BASE to its path. When
# NEXTOR_BASE is not set, the default target recurses into ../kernel to
# build it.

include ../version.mk

ifeq ($(strip $(N80)),)
N80=N80
endif

ifeq ($(strip $(MKNEXROM)),)
MKNEXROM=mknexrom
endif

export N80_ARGS=--no-show-banner --verbosity 0

BIN_DRIVERS := ../../bin/drivers
BIN_KERNEL_BASE := ../../bin/kernel-base

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

UNDOC_MODE_STAMP := $(BIN_DRIVERS)/.build-variant$(if $(_VARIANT_SUFFIX),$(_VARIANT_SUFFIX),.default)

$(UNDOC_MODE_STAMP):
	@rm -f $(BIN_DRIVERS)/.build-variant*
	@touch $@

define assemble
	@printf "\n\033[0;36mAssembling %s\033[0m\n\n" $(1)
	@$(N80) $(1) $(BIN_DRIVERS) $(2) $(foreach def,$(_VARIANT_DEFINES),--define-symbols $(def))
endef

# $(1)=output ROM, $(2)=driver bin, $(3)=mapper (chgbnk) bin
define mknexrom_rom
	@printf "\n\033[0;36mBuilding ROM %s\033[0m\n\n" $(1)
	@$(MKNEXROM) $(NEXTOR_BASE) $(1) /d:$(2) /m:$(3)
endef


###################
###  MAIN RULE  ###
###################

.PHONY: all base ascii8 ascii16 ram-example clean FORCE

# The rules defined earlier in the file have targets under $(BIN_DRIVERS);
# their paths contain slashes, so make won't skip them when picking the
# default goal (it only skips dot-prefixed targets without slashes).
.DEFAULT_GOAL := all

all: base ascii8 ascii16

FORCE:

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
	@mkdir -p $(BIN_KERNEL_BASE) $(BIN_DRIVERS)
	$(foreach exec,$(TOOLS),\
        $(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites


########################
###  KERNEL BASE     ###
########################

# Path to the kernel base file used as input by MKNEXROM. If NEXTOR_BASE
# is set, use it as-is. Otherwise build it via the kernel Makefile and
# publish a variant-named copy to bin/kernel-base (the same file the
# kernel makefile itself publishes; recreating it here keeps the build
# working after bin/ has been wiped while ../kernel/nextor_base.dat is
# still up to date).

ifeq ($(strip $(NEXTOR_BASE)),)

LOCAL_BASE := $(BIN_KERNEL_BASE)/Nextor-$(NEXTOR_KERNEL_VERSION).base$(_VARIANT_SUFFIX).dat

base: $(LOCAL_BASE)

# Always re-invoke the kernel sub-make so source/kernel changes are
# picked up. The sub-make is incremental and only touches
# ../kernel/nextor_base.dat if something there actually changed; that
# file's mtime drives whether $(LOCAL_BASE) (and the ROMs that depend
# on it) need to rebuild.
../kernel/nextor_base.dat: FORCE
	@$(MAKE) --no-print-directory -C ../kernel base \
	    NO_UNDOC_CPU_INSTRUCTIONS=$(NO_UNDOC_CPU_INSTRUCTIONS) \
	    INVERT_SHIFT=$(INVERT_SHIFT) \
	    INVERT_CTRL=$(INVERT_CTRL)

$(LOCAL_BASE): ../kernel/nextor_base.dat
	cp ../kernel/nextor_base.dat $@

NEXTOR_BASE := $(LOCAL_BASE)

else

base:
	@printf "Using pre-built kernel base: $(NEXTOR_BASE)\n"

endif


########################
###  STANDALONE ROMS ###
########################

ROM_ASCII8  := $(BIN_DRIVERS)/Nextor-$(NEXTOR_KERNEL_VERSION).StandaloneASCII8$(_VARIANT_SUFFIX).ROM
ROM_ASCII16 := $(BIN_DRIVERS)/Nextor-$(NEXTOR_KERNEL_VERSION).StandaloneASCII16$(_VARIANT_SUFFIX).ROM

ascii8:  $(ROM_ASCII8)
ascii16: $(ROM_ASCII16)

$(BIN_DRIVERS)/standalone-rom-driver.bin: \
	$(UNDOC_MODE_STAMP) \
	standalone-rom-driver.asm

	$(call assemble,standalone-rom-driver.asm,--build-type abs --output-file-extension bin)

$(BIN_DRIVERS)/chgbnk-ascii8.bin: \
	$(UNDOC_MODE_STAMP) \
	chgbnk-ascii8.asm

	$(call assemble,chgbnk-ascii8.asm,--build-type abs --output-file-extension bin)

$(BIN_DRIVERS)/chgbnk-ascii16.bin: \
	$(UNDOC_MODE_STAMP) \
	chgbnk-ascii16.asm

	$(call assemble,chgbnk-ascii16.asm,--build-type abs --output-file-extension bin)

# The standalone driver is built at 4100h; MKNEXROM expects it to start
# at 4000h (the first 256 bytes are reserved for the ROM header), so
# we prepend 256 zero bytes before passing it to MKNEXROM.

$(BIN_DRIVERS)/_standalone-rom-driver.bin: \
	$(BIN_DRIVERS)/standalone-rom-driver.bin \
	$(BIN_DRIVERS)/256.bytes

	cat $(BIN_DRIVERS)/256.bytes $(BIN_DRIVERS)/standalone-rom-driver.bin > $@

$(ROM_ASCII8): \
	$(NEXTOR_BASE) \
	$(BIN_DRIVERS)/_standalone-rom-driver.bin \
	$(BIN_DRIVERS)/chgbnk-ascii8.bin

	$(call mknexrom_rom,$@,$(BIN_DRIVERS)/_standalone-rom-driver.bin,$(BIN_DRIVERS)/chgbnk-ascii8.bin)

$(ROM_ASCII16): \
	$(NEXTOR_BASE) \
	$(BIN_DRIVERS)/_standalone-rom-driver.bin \
	$(BIN_DRIVERS)/chgbnk-ascii16.bin

	$(call mknexrom_rom,$@,$(BIN_DRIVERS)/_standalone-rom-driver.bin,$(BIN_DRIVERS)/chgbnk-ascii16.bin)


########################
###  RAM DRIVER      ###
########################

# Example RAM driver (loadable .drv). Opt-in: not built by `make all`.

ram-example: $(BIN_DRIVERS)/ram-driver-example.drv

$(BIN_DRIVERS)/ram-driver-example.drv: \
	$(UNDOC_MODE_STAMP) \
	ram-driver-example.asm

	$(call assemble,ram-driver-example.asm,--build-type abs --output-file-extension drv)


########################
###  COMMON          ###
########################

$(BIN_DRIVERS)/256.bytes:
	dd if=/dev/zero of=$@ bs=1 count=256

clean:
	rm -f $(BIN_DRIVERS)/*.bin $(BIN_DRIVERS)/*.drv $(BIN_DRIVERS)/*.ROM \
	    $(BIN_DRIVERS)/256.bytes $(BIN_DRIVERS)/.build-variant*
