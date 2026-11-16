# Makefile for the Nextor standalone ROM drivers and example RAM driver.
#
# Default target builds:
#   - The kernel base file (Nextor-$(NEXTOR_KERNEL_VERSION).base.dat).
#   - The Standalone ASCII8 ROM.
#   - The Standalone ASCII16 ROM.
#
# Both standalone ROMs are built from a single driver source
# (standalone-rom-driver.asm); only the bank switching routine differs.
#
# The example RAM driver (ram-driver-example.asm) is opt-in via the
# `ram-example` target.
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

define assemble
	@printf "\n\033[0;36mAssembling %s\033[0m\n\n" $(1)
	@$(N80) $(1) $(BIN_DRIVERS) $(2)
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

###################
###  TOOLCHECK  ###
###################

TOOLS := $(N80) $(MKNEXROM) dd cat

# Check that the required tools are available. This is done at parse time
# (not in a recipe) so that it runs before anything else regardless of the
# make version: the former "-include prerequisites" trick relied on make
# running a phony target's recipe at startup, which GNU make 4.4+ no longer
# does. Skipped when just cleaning.
ifneq ($(MAKECMDGOALS),clean)
$(foreach exec,$(TOOLS),\
        $(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))
endif

# The output directory is created on demand: every rule that writes into
# it lists it as an order-only prerequisite, so that it exists before the
# recipe runs but its timestamp changes never trigger rebuilds.
$(BIN_DRIVERS):
	@mkdir -p $@


########################
###  KERNEL BASE     ###
########################

# Path to the kernel base file used as input by MKNEXROM. If NEXTOR_BASE
# is set, use it as-is. Otherwise build it via the kernel Makefile, whose
# `base` target publishes the versioned copy to bin/kernel-base.

ifeq ($(strip $(NEXTOR_BASE)),)

LOCAL_BASE := $(BIN_KERNEL_BASE)/Nextor-$(NEXTOR_KERNEL_VERSION).base.dat

base: $(LOCAL_BASE)

# Always re-invoke the kernel sub-make so source/kernel changes are
# picked up. The sub-make is incremental and only touches $(LOCAL_BASE)
# if something there actually changed; that file's mtime drives whether
# the ROMs that depend on it need to rebuild.
$(LOCAL_BASE): FORCE
	@$(MAKE) --no-print-directory -C ../kernel base

NEXTOR_BASE := $(LOCAL_BASE)

else

base:
	@printf "Using pre-built kernel base: $(NEXTOR_BASE)\n"

endif


########################
###  STANDALONE ROMS ###
########################

ROM_ASCII8  := $(BIN_DRIVERS)/Nextor-$(NEXTOR_KERNEL_VERSION).StandaloneASCII8.ROM
ROM_ASCII16 := $(BIN_DRIVERS)/Nextor-$(NEXTOR_KERNEL_VERSION).StandaloneASCII16.ROM

ascii8:  $(ROM_ASCII8)
ascii16: $(ROM_ASCII16)

$(BIN_DRIVERS)/standalone-rom-driver.bin: \
	standalone-rom-driver.asm \
	| $(BIN_DRIVERS)

	$(call assemble,standalone-rom-driver.asm,--build-type abs --output-file-extension bin)

$(BIN_DRIVERS)/chgbnk-ascii8.bin: \
	chgbnk-ascii8.asm \
	| $(BIN_DRIVERS)

	$(call assemble,chgbnk-ascii8.asm,--build-type abs --output-file-extension bin)

$(BIN_DRIVERS)/chgbnk-ascii16.bin: \
	chgbnk-ascii16.asm \
	| $(BIN_DRIVERS)

	$(call assemble,chgbnk-ascii16.asm,--build-type abs --output-file-extension bin)

# The standalone driver is built at 4100h; MKNEXROM expects it to start
# at 4000h (the first 256 bytes are reserved for the ROM header), so
# we prepend 256 zero bytes before passing it to MKNEXROM.

$(BIN_DRIVERS)/_standalone-rom-driver.bin: \
	$(BIN_DRIVERS)/standalone-rom-driver.bin \
	$(BIN_DRIVERS)/256.bytes \
	| $(BIN_DRIVERS)

	cat $(BIN_DRIVERS)/256.bytes $(BIN_DRIVERS)/standalone-rom-driver.bin > $@

$(ROM_ASCII8): \
	$(NEXTOR_BASE) \
	$(BIN_DRIVERS)/_standalone-rom-driver.bin \
	$(BIN_DRIVERS)/chgbnk-ascii8.bin \
	| $(BIN_DRIVERS)

	$(call mknexrom_rom,$@,$(BIN_DRIVERS)/_standalone-rom-driver.bin,$(BIN_DRIVERS)/chgbnk-ascii8.bin)

$(ROM_ASCII16): \
	$(NEXTOR_BASE) \
	$(BIN_DRIVERS)/_standalone-rom-driver.bin \
	$(BIN_DRIVERS)/chgbnk-ascii16.bin \
	| $(BIN_DRIVERS)

	$(call mknexrom_rom,$@,$(BIN_DRIVERS)/_standalone-rom-driver.bin,$(BIN_DRIVERS)/chgbnk-ascii16.bin)


########################
###  RAM DRIVER      ###
########################

# Example RAM driver (loadable .drv). Opt-in: not built by `make all`.

ram-example: $(BIN_DRIVERS)/ram-driver-example.drv

$(BIN_DRIVERS)/ram-driver-example.drv: \
	ram-driver-example.asm \
	| $(BIN_DRIVERS)

	$(call assemble,ram-driver-example.asm,--build-type abs --output-file-extension drv)


########################
###  COMMON          ###
########################

$(BIN_DRIVERS)/256.bytes: | $(BIN_DRIVERS)
	dd if=/dev/zero of=$@ bs=1 count=256

clean:
	rm -f $(BIN_DRIVERS)/*.bin $(BIN_DRIVERS)/*.drv $(BIN_DRIVERS)/*.ROM \
	    $(BIN_DRIVERS)/256.bytes
