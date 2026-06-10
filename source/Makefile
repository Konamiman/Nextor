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

# Build the full matrix in one go: all six kernel base variants and both
# standalone ROMs for each of them, plus NEXTOR.SYS and the assembler and C
# command-line tools. The drivers' own `everything` target builds every kernel
# base variant as it goes (it recurses into ../kernel per variant), so no
# separate kernel pass is needed here. NEXTOR.SYS and the .COM tools have no
# build variants, so they are built once.
everything:
	$(MAKE) -C drivers everything
	$(MAKE) -C nextor_sys
	$(MAKE) -C tools
	$(MAKE) -C tools/C

.PHONY: all everything clean distclean

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C drivers clean
	$(MAKE) -C nextor_sys clean
	$(MAKE) -C tools clean
	$(MAKE) -C tools/C clean

# Clear everything: the source-tree intermediates (via `clean`) plus the
# generated outputs under <repo>/bin.
distclean: clean
	rm -rf ../bin/kernel-base ../bin/roms ../bin/ram-drivers ../bin/tools
