# Top level makefile for Nextor.
# It will build the kernel ROM files, NEXTOR.SYS and the command line tools.
# The generated files will be copied to bin/kernels and bin/tools.
# The makefiles on each directory can be used independently as well.
#
# Set NO_UNDOC_CPU_INSTRUCTIONS=1 to build a Z180-compatible variant
# in which the kernel and tools avoid all undocumented Z80 opcodes
# (those that operate on the high or low halves of IX/IY). See
# sdk/asm/macros/undoc.inc for details.

all:
	$(MAKE) -C kernel NO_UNDOC_CPU_INSTRUCTIONS=$(NO_UNDOC_CPU_INSTRUCTIONS)
	$(MAKE) -C command/msxdos NO_UNDOC_CPU_INSTRUCTIONS=$(NO_UNDOC_CPU_INSTRUCTIONS)
	$(MAKE) -C tools NO_UNDOC_CPU_INSTRUCTIONS=$(NO_UNDOC_CPU_INSTRUCTIONS)
	$(MAKE) -C tools/C

.PHONY: clean

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C command/msxdos clean
	$(MAKE) -C tools clean
	$(MAKE) -C tools/C clean
