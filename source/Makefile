# Top level makefile for Nextor.
# It builds the kernel base file, the standalone ROM drivers, NEXTOR.SYS,
# COMMAND3.COM and the command line tools. The generated files are copied
# under bin/: kernel-base (kernel base files), drivers (standalone ROMs
# and the example RAM driver) and tools (the command-line tools,
# NEXTOR.SYS and COMMAND3.COM). The makefiles in each subdirectory can be
# used independently as well.
#
# Set NO_UNDOC_CPU_INSTRUCTIONS=1 to build a Z180-compatible kernel
# variant that avoids all undocumented Z80 opcodes (those that operate
# on the high or low halves of IX/IY). See sdk/asm/macros/undoc.inc
# for details. Set INVERT_KANJI=1 to build a kernel variant with the "6" boot
# key inverted (the Kanji driver is installed at boot time unless the
# key is pressed); the generated files get a ".KANJI_INV" suffix. Both flags
# are propagated only to the kernel and drivers sub-builds; NEXTOR.SYS
# and the .COM tools contain no undoc opcodes and no boot key handling,
# so the flags have no effect on them.

# Absolute path so drivers/Makefile uses the kernel-built base verbatim
# (via its NEXTOR_BASE branch) instead of recursing back into ../kernel.
NEXTOR_BASE_ABS := $(abspath kernel/nextor_base.dat)

all:
	$(MAKE) -C kernel NO_UNDOC_CPU_INSTRUCTIONS=$(NO_UNDOC_CPU_INSTRUCTIONS) \
	    INVERT_KANJI=$(INVERT_KANJI)
	$(MAKE) -C drivers NEXTOR_BASE=$(NEXTOR_BASE_ABS) \
	    NO_UNDOC_CPU_INSTRUCTIONS=$(NO_UNDOC_CPU_INSTRUCTIONS) \
	    INVERT_KANJI=$(INVERT_KANJI)
	$(MAKE) -C nextor_sys
	$(MAKE) -C commandcom
	$(MAKE) -C tools
	$(MAKE) -C tools/C

# Build the full matrix in one go: all twelve kernel base variants and both
# standalone ROMs for each of them, plus NEXTOR.SYS, COMMAND3.COM and the
# assembler and C command-line tools. The drivers' own `everything` target
# builds every kernel base variant as it goes (it recurses into ../kernel per
# variant), so no separate kernel pass is needed here. NEXTOR.SYS, COMMAND3.COM
# and the .COM tools have no build variants, so they are built once.
everything:
	$(MAKE) -C drivers everything
	$(MAKE) -C nextor_sys
	$(MAKE) -C commandcom
	$(MAKE) -C tools
	$(MAKE) -C tools/C

# Build NEXTOR.SYS, COMMAND3.COM and all the command line tools, then pack
# them into the bin/tools/nextor.dsk disk image.
# The disk recipe itself lives in tools/Makefile (see its tools-disk target
# for the details, including the EXTRA_FILES variable);
# this wrapper just makes sure everything the disk needs is built first.
# WORKING_DIR tells it the directory make was invoked from (against which
# the file locations are resolved): $(PWD) is the invocation-time value
# here, since the environment variable doesn't survive the recipe's
# sub-shell.
tools-disk:
	$(MAKE) -C nextor_sys
	$(MAKE) -C commandcom
	$(MAKE) -C tools
	$(MAKE) -C tools/C
	$(MAKE) -C tools tools-disk WORKING_DIR="$(PWD)"

# Build all the command line tools, then pack them into the
# bin/tools/tools.zip archive (just the tools: no NEXTOR.SYS and no
# COMMAND3.COM there, so no NEXTOR.SYS build either - see the tools-zip
# target in tools/Makefile).
tools-zip:
	$(MAKE) -C tools
	$(MAKE) -C tools/C
	$(MAKE) -C tools tools-zip

# Everything tools-related in one go: NEXTOR.SYS, COMMAND3.COM and all the
# command line tools, the bin/tools/nextor.dsk disk image and the
# bin/tools/tools.zip archive. Accepts the same EXTRA_FILES variable as
# tools-disk.
tools-all:
	$(MAKE) -C nextor_sys
	$(MAKE) -C commandcom
	$(MAKE) -C tools
	$(MAKE) -C tools/C
	$(MAKE) -C tools tools-disk tools-zip WORKING_DIR="$(PWD)"

.PHONY: all everything tools-disk tools-zip tools-all clean distclean

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C drivers clean
	$(MAKE) -C nextor_sys clean
	$(MAKE) -C commandcom clean
	$(MAKE) -C tools clean
	$(MAKE) -C tools/C clean

# Clear everything: the source-tree intermediates (via `clean`) plus the
# generated outputs under <repo>/bin. The roms and ram-drivers directories
# are no longer generated (their content goes to drivers now), but they are
# still removed so that checkouts that built with the old layout get
# properly cleaned.
distclean: clean
	rm -rf ../bin/kernel-base ../bin/drivers ../bin/roms ../bin/ram-drivers ../bin/tools
