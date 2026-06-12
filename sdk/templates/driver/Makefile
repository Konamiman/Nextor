# Makefile for a Nextor ROM driver: assembles driver.asm and chgbnk.asm with
# N80 (Nestor80) and combines them with a kernel base file into a bootable ROM.
#
#   make            build the ROM
#   make clean      remove build artifacts
#
# Tools needed: N80 and mknexrom, both on PATH (or override N80/MKNEXROM
# below). The Nextor dev Docker image has everything preinstalled and
# preconfigured - see docker/README.md in the Nextor repository.

# TODO: Name your driver; the output ROM is named after it.
NAME := mydriver
ROM  := $(NAME).ROM

# Where to find the Nextor SDK and a kernel base file. Inside the Nextor dev
# container both come preset as environment variables, so the defaults below
# don't apply there. Outside the container:
#
# - NEXTOR_SDK is the sdk/ directory of a Nextor checkout. The default works
#   while this directory still sits at sdk/templates/driver/ in the Nextor
#   repository; if you copied the template elsewhere, TODO: point it at your
#   SDK location (sdk/README.md describes ways to bring the SDK into a
#   project: git submodule, symlink, or plain copy).
#
# - NEXTOR_BASE is a Nextor kernel base file. There is no canonical location
#   outside the container, so TODO: either build one (`make` in source/kernel
#   of a Nextor checkout leaves them in bin/kernel-base/) or take the
#   Nextor-X.Y.Z.base.dat file from a Nextor release, then point NEXTOR_BASE
#   at it (here or on the command line: `make NEXTOR_BASE=<path>`).
NEXTOR_SDK  ?= ../..
NEXTOR_BASE ?= /opt/nextor/kernel_base/kernel_base.dat
N80         ?= N80
MKNEXROM    ?= mknexrom

N80_FLAGS := --no-string-escapes --no-show-banner --verbosity 0 --include-directory $(NEXTOR_SDK)

# NO_UNDOC=1 -> assemble undoc-free (the undoc.inc macros expand to documented
# instruction sequences, safe on Z180-based machines). Match this to a
# .NO_UNDOC. kernel base file:
#
#   make NO_UNDOC=1 NEXTOR_BASE=<path to a .NO_UNDOC. base file>
ifeq ($(NO_UNDOC),1)
N80_FLAGS += --define-symbols NO_UNDOC_CPU_INSTRUCTIONS
endif

.PHONY: all clean
all: $(ROM)

# Combine the driver and bank-switching code with the kernel base into a ROM.
$(ROM): driver.bin chgbnk.bin
	@test -f "$(NEXTOR_BASE)" || { \
	    echo "*** Kernel base file not found: $(NEXTOR_BASE)"; \
	    echo "*** Set NEXTOR_BASE to a kernel base file (see the comments at the top of this Makefile)."; \
	    exit 1; }
	$(MKNEXROM) $(NEXTOR_BASE) $@ /d:driver.bin /m:chgbnk.bin
	@echo "Built $@"

driver.bin: driver.asm
	$(N80) driver.asm driver.bin $(N80_FLAGS)

chgbnk.bin: chgbnk.asm
	$(N80) chgbnk.asm chgbnk.bin $(N80_FLAGS)

clean:
	rm -f driver.bin chgbnk.bin $(ROM)
