# @@NAME@@ — Nextor disk driver, built into a bootable kernel ROM.
#
# Scaffolded by `nextor-init driver`. Build inside the Nextor dev container,
# which supplies N80, mknexrom, the SDK ($NEXTOR_SDK) and a kernel base file
# ($NEXTOR_BASE) as environment variables.
#
#   make            build @@NAME@@.ROM
#   make clean      remove build artifacts
#
# To target a different kernel variant, point NEXTOR_BASE at another base file.
# When you pick a .NO_UNDOC. base, also pass NO_UNDOC=1 so the driver is
# assembled without undocumented Z80 instructions (Z180-safe):
#
#   make NEXTOR_BASE=/opt/nextor/kernel_base/kernel_base.NO_UNDOC.dat NO_UNDOC=1

NAME := @@NAME@@
ROM  := $(NAME).ROM

# Supplied by the dev image as environment variables; override if needed.
NEXTOR_BASE ?= /opt/nextor/kernel_base/kernel_base.dat
NEXTOR_SDK  ?= /opt/nextor/sdk
N80         ?= N80
MKNEXROM    ?= mknexrom

N80_FLAGS := --no-string-escapes --no-show-banner --verbosity 0 --include-directory $(NEXTOR_SDK)

# NO_UNDOC=1 -> assemble undoc-free (the undoc.inc macros expand to documented
# instruction sequences). Match this to a .NO_UNDOC. NEXTOR_BASE.
ifeq ($(NO_UNDOC),1)
N80_FLAGS += --define-symbols NO_UNDOC_CPU_INSTRUCTIONS
endif

.PHONY: all clean
all: $(ROM)

# Combine the driver and bank-switching code with the kernel base into a ROM.
$(ROM): driver.bin chgbnk.bin
	$(MKNEXROM) $(NEXTOR_BASE) $@ /d:driver.bin /m:chgbnk.bin
	@echo "Built $@"

driver.bin: driver.asm
	$(N80) driver.asm driver.bin $(N80_FLAGS)

chgbnk.bin: chgbnk.asm
	$(N80) chgbnk.asm chgbnk.bin $(N80_FLAGS)

clean:
	rm -f driver.bin chgbnk.bin $(ROM)
