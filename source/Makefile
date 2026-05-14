# Top level makefile for Nextor.
# It will build the kernel ROM files, NEXTOR.SYS and the command line tools.
# The generated files will be copied to bin/kernels and bin/tools.
# The makefiles on each directory can be used independently as well.
#
# Set NO_UNDOC_CPU_INSTRUCTIONS=1 to build a Z180-compatible kernel
# variant that avoids all undocumented Z80 opcodes (those that operate
# on the high or low halves of IX/IY). See sdk/asm/macros/undoc.inc
# for details. The flag is propagated only to the kernel sub-build;
# NEXTOR.SYS and the .COM tools contain no undoc opcodes so the flag
# has no effect on them.

all:
	$(MAKE) -C kernel NO_UNDOC_CPU_INSTRUCTIONS=$(NO_UNDOC_CPU_INSTRUCTIONS)
	$(MAKE) -C nextor_sys
	$(MAKE) -C tools
	$(MAKE) -C tools/C

.PHONY: clean

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C nextor_sys clean
	$(MAKE) -C tools clean
	$(MAKE) -C tools/C clean
