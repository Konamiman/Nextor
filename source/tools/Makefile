# Makefile for the command line tools written in assembler.
# The list of files to process is in the COMS variable.

# The generated files are copied to the bin/tools directory.

ifeq ($(strip $(N80)),)
N80=N80
endif

export N80_ARGS=--no-string-escapes --no-show-banner --build-type rel --verbosity 0 --output-file-case upper --include-directory ../kernel

ifeq ($(strip $(LK80)),)
LK80=LK80
endif

export LK80_ARGS=--no-show-banner --verbosity 0 --output-file-case lower --output-format bin --code-before-data

define copy_to_bin
	cp $(1) ../../bin/tools/$(2)
endef

define assemble
	@printf "\n\033[0;36mAssembling %s\033[0m\n\n" $(1)
	@$(N80) $(1) $(2)
endef

define assemble_as
	@printf "\n\033[0;36mAssembling %s as %s\033[0m\n\n" $(1) $(2)
	@$(N80) $(1) $(2) $(3)
endef

uc = $(shell echo $(1) | tr a-z A-Z)

COMS := CONCLUS.COM DELALL.COM DEVINFO.COM DRIVERS.COM DRVINFO.COM FASTOUT.COM LOCK.COM MAPDRV.COM NSYSVER.COM RALLOC.COM
INCS := ../kernel/macros.inc ../kernel/const.inc
SDK_RELS := DOS_CALLS.REL DOS_ERRORS.REL DOS_FILE_HANDLES.REL
SHARED_NAMES := byte2asc extpar chk_nextor chkonoff chklet chklet_noterm prslot prpad ext8 extnum getslot
SHARED_RELS  := $(foreach n,$(SHARED_NAMES),$(call uc,$(n)).REL)
RELS := $(SDK_RELS) DATA.REL $(SHARED_RELS)

asm-tools: $(COMS)

.phony: prerequisites

TOOLS := N80 LK80

prerequisites:
	@mkdir -p ../../bin/tools
	$(foreach exec,$(TOOLS),\
		$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites

.SECONDEXPANSION:
$(COMS): \
	$$(patsubst %.COM,%.MAC,$$@) \
	$(INCS) \
	$(SDK_RELS) \
	DATA.REL \
	$(SHARED_RELS)

	$(call assemble,$(patsubst %.COM,%.MAC,$@),--define-symbols GENERATE_PUBLIC_SYMBOLS)
	@$(LK80) --output-file $@ --code 0100h $(SDK_RELS) DATA.REL $(patsubst %.COM,%.REL,$@) $(SHARED_RELS)
	$(call copy_to_bin,$@)

# Build rules for each individual SDK shared routine .asm file (one per public routine).
define shared_rel_rule
$(2).REL: ../../sdk/asm/code/$(1).asm
	$$(call assemble_as,../../sdk/asm/code/$(1).asm,$(2).REL,--define-symbols GENERATE_PUBLIC_SYMBOLS --define-symbols COM_FILE)
endef

$(foreach n,$(SHARED_NAMES),$(eval $(call shared_rel_rule,$(n),$(call uc,$(n)))))

DOS_CALLS.REL: ../../sdk/asm/constants/dos_calls.inc ../../sdk/asm/macros/const.inc
	$(call assemble_as,../../sdk/asm/constants/dos_calls.inc,DOS_CALLS.REL,--define-symbols GENERATE_PUBLIC_SYMBOLS)

DOS_ERRORS.REL: ../../sdk/asm/constants/dos_errors.inc ../../sdk/asm/macros/const.inc
	$(call assemble_as,../../sdk/asm/constants/dos_errors.inc,DOS_ERRORS.REL,--define-symbols GENERATE_PUBLIC_SYMBOLS)

DOS_FILE_HANDLES.REL: ../../sdk/asm/constants/dos_file_handles.inc ../../sdk/asm/macros/const.inc
	$(call assemble_as,../../sdk/asm/constants/dos_file_handles.inc,DOS_FILE_HANDLES.REL,--define-symbols GENERATE_PUBLIC_SYMBOLS)

DATA.REL: \
	../kernel/data.mac \
	$(INCS)

	$(call assemble,../kernel/data.mac,--define-symbols GENERATE_PUBLIC_SYMBOLS)

# ---------------------------------------------------------------------------
# Tools disk image.
#
# `make tools-disk` creates bin/tools/nextor.dsk: a 720K double sided 3.5"
# FAT12 disk image containing NEXTOR.SYS, NEXTOR.SYS.japanese (renamed to
# NEXTORJ.SYS, since its real name doesn't fit the 8.3 convention), every
# .COM file present in bin/tools (which includes COMMAND3.COM), a README.TXT
# (sourced from tools-disk-readme.txt), the COMMAND3.COM help files from
# ../commandcom/helpfiles in a HELP directory, and any files supplied via
# the EXTRA_FILES variable. The image is recreated from scratch on every
# run using mformat, mmd and mcopy from the mtools package, plus dd to
# install the boot sector code.
# (The image was a 360K single sided one until the help files came in:
# they don't fit in the space a 360K disk leaves free.)
#
# The image gets the same MSX-DOS 2.20 style boot sector that the kernel's
# built-in FORMAT command creates (assembled standalone via
# tools-disk-bootsect.mac), so it boots both in Nextor/MSX-DOS 2 mode
# (straight to the DOS prompt) and in MSX-DOS 1 mode (loading MSXDOS.SYS
# if present, or falling back to Disk BASIC).
#
# The files to put in the disk are expected to exist in bin/tools already:
# if any is missing the build fails. Use the tools-disk target of ../Makefile
# to build all of them and then create the disk in one go.
#
# Files that are not built by this repository can be added to the disk with
# the EXTRA_FILES variable: a comma (or space) separated list of files, with
# relative paths resolved against the current directory (the one make was
# invoked from), stored in the disk with their names uppercased. Listed
# files that don't exist make the build fail. Typical usage is
# EXTRA_FILES=MSXDOS.SYS,COMMAND.COM for a disk that also boots to the DOS
# prompt in MSX-DOS 1 mode (in Nextor/MSX-DOS 2 mode the disk is already
# bootable thanks to the COMMAND3.COM built by this repository).
#
# The current (invocation) directory is taken from the PWD environment
# variable, which (unlike CURDIR) survives make -C. It does NOT survive the
# sub-shell that runs the recipes of ../Makefile, so its tools-disk target
# passes WORKING_DIR explicitly instead.

BIN_TOOLS := ../../bin/tools
TOOLS_DSK := $(BIN_TOOLS)/nextor.dsk

WORKING_DIR ?= $(or $(PWD),$(CURDIR))

comma := ,
EXTRA_FILES ?=
EXTRA_FILES_ABS := $(foreach f,$(subst $(comma), ,$(EXTRA_FILES)),$(if $(filter /%,$(f)),$(f),$(WORKING_DIR)/$(f)))
EXTRA_MISSING = $(filter-out $(wildcard $(EXTRA_FILES_ABS)),$(EXTRA_FILES_ABS))

# The C tools list, derived from the .c files (it matches the COMS variable
# in C/Makefile).
C_COMS := $(foreach n,$(notdir $(wildcard C/*.c)),$(call uc,$(patsubst %.c,%.com,$(n))))

DISK_REQUIRED := NEXTOR.SYS NEXTOR.SYS.japanese COMMAND3.COM $(COMS) $(C_COMS)
DISK_MISSING = $(filter-out $(notdir $(wildcard $(addprefix $(BIN_TOOLS)/,$(DISK_REQUIRED)))),$(DISK_REQUIRED))

# Everything with a .COM extension goes in the disk. $(sort ...) also removes
# duplicates, in case the filesystem is case-insensitive.
DISK_COMS = $(sort $(wildcard $(BIN_TOOLS)/*.COM) $(wildcard $(BIN_TOOLS)/*.com))

MTOOLS := mformat mmd mcopy

# The COMMAND3.COM help files, stored in the HELP directory of the disk.
HELPFILES_DIR := ../commandcom/helpfiles
HELPFILES = $(wildcard $(HELPFILES_DIR)/*.HLP)

# The boot sector code, assembled standalone from the kernel source (see
# tools-disk-bootsect.mac) and linked at 0C01Eh, where boot sector code runs.
# DOS_CALLS.REL and DATA.REL provide the kernel symbols it references.
BOOTSECT_BIN := tools-disk-bootsect.bin

$(BOOTSECT_BIN): \
	tools-disk-bootsect.mac \
	../kernel/bank2/bootsect.mac \
	$(INCS) \
	DOS_CALLS.REL \
	DATA.REL

	$(call assemble,tools-disk-bootsect.mac)
	@$(LK80) --output-file $@ --code 0C01Eh DOS_CALLS.REL DATA.REL TOOLS-DISK-BOOTSECT.REL

.PHONY: tools-disk

# The dd invocations patch the boot sector created by mformat: they set the
# OEM name (8 bytes at offset 3, "NEXTOR3" padded with a space), zero the
# sector from offset 30 (1Eh, right after the disk parameter block) to the
# end (30+482=512), and install the boot sector code at that offset.
tools-disk: $(BOOTSECT_BIN)
	$(foreach exec,$(MTOOLS),\
		$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), from the mtools package, is it installed/in PATH?")))
	$(if $(DISK_MISSING),$(error "ERROR: missing in $(BIN_TOOLS): $(DISK_MISSING) - build the tools first, or use the tools-disk target of ../Makefile"))
	$(if $(EXTRA_MISSING),$(error "ERROR: EXTRA_FILES not found: $(EXTRA_MISSING)"))
	$(if $(HELPFILES),,$(error "ERROR: no help files found in $(HELPFILES_DIR)"))
	@rm -f $(TOOLS_DSK)
	@mformat -C -i $(TOOLS_DSK) -t 80 -h 2 -n 9 -v NEXTOR ::
	@printf 'NEXTOR3 ' | dd of=$(TOOLS_DSK) bs=1 seek=3 conv=notrunc status=none
	@dd if=/dev/zero of=$(TOOLS_DSK) bs=1 seek=30 count=482 conv=notrunc status=none
	@dd if=$(BOOTSECT_BIN) of=$(TOOLS_DSK) bs=1 seek=30 conv=notrunc status=none
	@mcopy -i $(TOOLS_DSK) $(BIN_TOOLS)/NEXTOR.SYS ::NEXTOR.SYS
	@mcopy -i $(TOOLS_DSK) $(BIN_TOOLS)/NEXTOR.SYS.japanese ::NEXTORJ.SYS
	@mcopy -i $(TOOLS_DSK) $(DISK_COMS) ::
	@mcopy -t -i $(TOOLS_DSK) tools-disk-readme.txt ::README.TXT
	@mmd -i $(TOOLS_DSK) ::HELP
	@mcopy -i $(TOOLS_DSK) $(HELPFILES) ::HELP/
	$(if $(EXTRA_FILES_ABS),@for f in $(EXTRA_FILES_ABS); do \
		mcopy -i $(TOOLS_DSK) "$$f" "::$$(basename $$f | tr a-z A-Z)" || exit 1; \
	done)
	@printf "\n\033[0;36mCreated %s\033[0m\n\n" $(TOOLS_DSK)

# ---------------------------------------------------------------------------
# Tools zip archive.
#
# `make tools-zip` creates bin/tools/tools.zip with every .COM file present
# in bin/tools (the same set of command line tools that goes in the disk
# image), without NEXTOR.SYS and without COMMAND3.COM. As with tools-disk,
# the tools are expected to have been built already: if any is missing the
# build fails. Use the tools-zip target of ../Makefile to build the tools
# and create the archive in one go. The archive is recreated from scratch
# on every run.

TOOLS_ZIP := $(BIN_TOOLS)/tools.zip

ZIP_REQUIRED := $(COMS) $(C_COMS)
ZIP_MISSING = $(filter-out $(notdir $(wildcard $(addprefix $(BIN_TOOLS)/,$(ZIP_REQUIRED)))),$(ZIP_REQUIRED))

# The .COM files to pack: everything that goes in the disk image except
# COMMAND3.COM (in either letter case, in case the filesystem preserved a
# lowercase copy).
ZIP_COMS = $(filter-out $(BIN_TOOLS)/COMMAND3.COM $(BIN_TOOLS)/command3.com,$(DISK_COMS))

.PHONY: tools-zip

tools-zip:
	$(if $(shell which zip),,$(error "ERROR: can't execute zip, is it installed/in PATH?"))
	$(if $(ZIP_MISSING),$(error "ERROR: missing in $(BIN_TOOLS): $(ZIP_MISSING) - build the tools first, or use the tools-zip target of ../Makefile"))
	@rm -f $(TOOLS_ZIP)
	@cd $(BIN_TOOLS) && zip -q tools.zip $(notdir $(ZIP_COMS))
	@printf "\n\033[0;36mCreated %s\033[0m\n\n" $(TOOLS_ZIP)

.PHONY: clean

clean:
	for ext in REL rel SYM sym HEX COM bin; do find . -type f -name "*.$$ext" -delete; done
