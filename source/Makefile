# Top level makefile for Nextor.
# It builds the kernel base file, the standalone ROM drivers, NEXTOR.SYS
# and the command line tools. The generated files are copied to
# bin/kernel-base (kernel base files), bin/roms (standalone ROMs),
# bin/nextor_sys and bin/tools. The makefiles in each subdirectory can
# be used independently as well.
#
# Set NO_UNDOC_CPU_INSTRUCTIONS=1 to build a Z180-compatible kernel
# variant that avoids all undocumented Z80 opcodes (those that operate
# on the high or low halves of IX/IY). See sdk/asm/macros/undoc.inc
# for details. The flag is propagated only to the kernel and drivers
# sub-builds; NEXTOR.SYS and the .COM tools contain no undoc opcodes
# so the flag has no effect on them.

# Absolute path so drivers/Makefile uses the kernel-built base verbatim
# (via its NEXTOR_BASE branch) instead of recursing back into ../kernel.
NEXTOR_BASE_ABS := $(abspath kernel/nextor_base.dat)

all:
	$(MAKE) -C kernel NO_UNDOC_CPU_INSTRUCTIONS=$(NO_UNDOC_CPU_INSTRUCTIONS)
	$(MAKE) -C drivers NEXTOR_BASE=$(NEXTOR_BASE_ABS) \
	    NO_UNDOC_CPU_INSTRUCTIONS=$(NO_UNDOC_CPU_INSTRUCTIONS)
	$(MAKE) -C nextor_sys
	$(MAKE) -C tools
	$(MAKE) -C tools/C

.PHONY: all clean

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C drivers clean
	$(MAKE) -C nextor_sys clean
	$(MAKE) -C tools clean
	$(MAKE) -C tools/C clean
