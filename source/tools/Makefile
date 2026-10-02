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

BIN_TOOLS := ../../bin/tools

define assemble
	@printf "\n\033[0;36mAssembling %s\033[0m\n\n" $(1)
	@$(N80) $(1) $(2)
endef

define assemble_as
	@printf "\n\033[0;36mAssembling %s as %s\033[0m\n\n" $(1) $(2)
	@$(N80) $(1) $(2) $(3)
endef

uc = $(shell echo $(1) | tr a-z A-Z)

COMS := CHKDSK.COM CONCLUS.COM DELALL.COM DEVINFO.COM DRIVERS.COM DRVINFO.COM FASTOUT.COM FIXDISK.COM KMODE.COM LOCK.COM MAPDRV.COM NSYSVER.COM RALLOC.COM UNDEL.COM Z80MODE.COM

# Tools with their own link rule (see below), excluded from the generic
# one-MAC-per-COM rule.
SPECIAL_COMS := CHKDSK.COM UNDEL.COM
GENERIC_COMS := $(filter-out $(SPECIAL_COMS),$(COMS))
INCS := ../kernel/macros.inc ../kernel/const.inc
SDK_RELS := DOS_CALLS.REL DOS_ERRORS.REL DOS_FILE_HANDLES.REL
SHARED_NAMES := byte2asc extpar chk_nextor chkonoff chklet chklet_noterm prslot prpad ext8 extnum getslot
SHARED_RELS  := $(foreach n,$(SHARED_NAMES),$(call uc,$(n)).REL)
RELS := $(SDK_RELS) DATA.REL $(SHARED_RELS)

BIN_COMS := $(addprefix $(BIN_TOOLS)/,$(COMS))

asm-tools: $(BIN_COMS)

# The bin/tools copies are targets of their own (not a side effect of the
# link recipes), so they get re-created when bin/tools is missing or empty
# even if the .COM files in this directory are up to date. The directory
# itself is an order-only prerequisite: it gets created on demand, but its
# timestamp changes never trigger a re-copy.
$(BIN_COMS): $(BIN_TOOLS)/%.COM: %.COM | $(BIN_TOOLS)
	cp $< $@

$(BIN_TOOLS):
	@mkdir -p $@

.PHONY: asm-tools

TOOLS := N80 LK80

# Check that the required tools are available. This is done at parse time
# (not in a recipe) so that it runs before anything else regardless of the
# make version: the former "-include prerequisites" trick relied on make
# running a phony target's recipe at startup, which GNU make 4.4+ no longer
# does. Skipped when just cleaning.
ifneq ($(MAKECMDGOALS),clean)
$(foreach exec,$(TOOLS),\
	$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))
endif

.SECONDEXPANSION:
$(GENERIC_COMS): \
	$$(patsubst %.COM,%.MAC,$$@) \
	$(INCS) \
	$(SDK_RELS) \
	DATA.REL \
	$(SHARED_RELS)

	$(call assemble,$(patsubst %.COM,%.MAC,$@),--define-symbols GENERATE_PUBLIC_SYMBOLS)
	@$(LK80) --output-file $@ --code 0100h $(SDK_RELS) DATA.REL $(patsubst %.COM,%.REL,$@) $(SHARED_RELS)

# CHKDSK is self-contained (it carries its own Nextor version check instead
# of using CHK_NEXTOR, since it must run on Nextor 2.1) and its runtime data
# (cluster flag bitmaps, expanded FAT or FAT sector cache, file list) starts
# right at the end of the program, so it is linked without the SDK shared
# routine modules: they would otherwise sit as dead code between the program
# and its data area, wasting TPA space that big FAT16 volumes need.
CHKDSK.COM: CHKDSK.MAC $(SDK_RELS)
	$(call assemble,CHKDSK.MAC)
	@$(LK80) --output-file $@ --code 0100h $(SDK_RELS) CHKDSK.REL

# UNDEL is self-contained for the same reasons as CHKDSK (see above).
UNDEL.COM: UNDEL.MAC $(SDK_RELS)
	$(call assemble,UNDEL.MAC)
	@$(LK80) --output-file $@ --code 0100h $(SDK_RELS) UNDEL.REL

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
# Tools disk image and zip archive contents.
#
# The disk image and the zip archive (see below) have the same contents:
#
# - In the root directory: NEXTOR.SYS, NEXTOR.SYS.japanese (renamed to
#   NEXTORJ.SYS), COMMAND3.COM, an AUTOEXEC.BAT that points the HELP and
#   PATH environment items to the HELP and TOOLS directories, and a
#   README.TXT (sourced from tools-disk-readme.txt).
# - In a TOOLS directory: every other .COM file present in bin/tools.
# - In a HELP directory: the COMMAND3.COM help files from
#   ../commandcom/helpfiles.
#
# The text files are stored with CRLF line endings, whatever the line
# endings of their sources. The tree is assembled in a staging directory
# (one per target, so that they can run in parallel) that is deleted once
# the disk image or the zip archive has been created.
#
# The files are expected to exist in bin/tools already: if any is missing
# the build fails. Use the tools-disk and tools-zip targets of ../Makefile
# to build all of them and then create the disk or the archive in one go.

# The C tools list, derived from the .c files (it matches the COMS variable
# in C/Makefile).
C_COMS := $(foreach n,$(notdir $(wildcard C/*.c)),$(call uc,$(patsubst %.c,%.com,$(n))))

PACKAGE_REQUIRED := NEXTOR.SYS NEXTOR.SYS.japanese COMMAND3.COM $(COMS) $(C_COMS)
PACKAGE_MISSING = $(filter-out $(notdir $(wildcard $(addprefix $(BIN_TOOLS)/,$(PACKAGE_REQUIRED)))),$(PACKAGE_REQUIRED))

# The .COM files for the TOOLS directory: every .COM file in bin/tools
# except COMMAND3.COM (in either letter case, in case the filesystem
# preserved a lowercase copy). $(sort ...) also removes duplicates, in case
# the filesystem is case-insensitive.
TOOL_COMS = $(filter-out $(BIN_TOOLS)/COMMAND3.COM $(BIN_TOOLS)/command3.com,\
	$(sort $(wildcard $(BIN_TOOLS)/*.COM) $(wildcard $(BIN_TOOLS)/*.com)))

# The COMMAND3.COM help files, stored in the HELP directory.
# JINDEX.HLP (the Japanese help index, shown by HELP with no arguments
# when the Kanji mode is active) is generated from its UTF-8 source,
# since the machine needs it in Shift-JIS with CRLF line endings and a
# trailing CTRL-Z, like the other help files; these are ASCII and stored
# as they are. The Makefile is a prerequisite so that a change in the
# conversion recipe regenerates the file.
HELPFILES_DIR := ../commandcom/helpfiles
HELPFILES = $(wildcard $(HELPFILES_DIR)/*.HLP)

$(HELPFILES_DIR)/JINDEX.HLP: $(HELPFILES_DIR)/JINDEX.HLP.utf8 Makefile
	$(if $(shell which iconv),,$(error "ERROR: can't execute iconv, is it installed/in PATH?"))
	@sed 's/\r*$$/\r/' $< | iconv -f UTF-8 -t SHIFT_JIS > $@
	@printf '\032' >> $@

# Assemble the disk/archive tree in the staging directory $(1).
define stage_package
	$(if $(PACKAGE_MISSING),$(error "ERROR: missing in $(BIN_TOOLS): $(PACKAGE_MISSING) - build the tools first, or use the tools-disk/tools-zip targets of ../Makefile"))
	$(if $(HELPFILES),,$(error "ERROR: no help files found in $(HELPFILES_DIR)"))
	@rm -rf $(1)
	@mkdir -p $(1)/TOOLS $(1)/HELP
	@cp $(BIN_TOOLS)/NEXTOR.SYS $(1)/NEXTOR.SYS
	@cp $(BIN_TOOLS)/NEXTOR.SYS.japanese $(1)/NEXTORJ.SYS
	@cp $(BIN_TOOLS)/COMMAND3.COM $(1)/COMMAND3.COM
	@sed 's/\r*$$/\r/' tools-disk-autoexec.bat > $(1)/AUTOEXEC.BAT
	@sed 's/\r*$$/\r/' tools-disk-readme.txt > $(1)/README.TXT
	@for f in $(TOOL_COMS); do \
		cp "$$f" "$(1)/TOOLS/$$(basename $$f | tr a-z A-Z)" || exit 1; \
	done
	@cp $(filter-out %JINDEX.HLP,$(HELPFILES)) $(HELPFILES_DIR)/JINDEX.HLP $(1)/HELP/
endef

# The root directory files, in the order they are added to the disk and
# the archive.
PACKAGE_ROOT_FILES := NEXTOR.SYS NEXTORJ.SYS COMMAND3.COM AUTOEXEC.BAT README.TXT

# ---------------------------------------------------------------------------
# Tools disk image.
#
# `make tools-disk` creates bin/tools/nextor.dsk: a 720K double sided 3.5"
# FAT12 disk image with the contents described above, plus any files
# supplied via the EXTRA_FILES variable (in the root directory). The image
# is recreated from scratch on every run using mformat, mmd and mcopy from
# the mtools package, plus dd to install the boot sector code.
# (The image was a 360K single sided one until the help files came in:
# they don't fit in the space a 360K disk leaves free.)
#
# The image gets the same MSX-DOS 2.20 style boot sector that the kernel's
# built-in FORMAT command creates (assembled standalone via
# tools-disk-bootsect.mac), so it boots both in Nextor/MSX-DOS 2 mode
# (straight to the DOS prompt) and in MSX-DOS 1 mode (loading MSXDOS.SYS
# if present, or falling back to Disk BASIC).
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

TOOLS_DSK := $(BIN_TOOLS)/nextor.dsk
DISK_STAGING := tools-disk-staging

WORKING_DIR ?= $(or $(PWD),$(CURDIR))

comma := ,
EXTRA_FILES ?=
EXTRA_FILES_ABS := $(foreach f,$(subst $(comma), ,$(EXTRA_FILES)),$(if $(filter /%,$(f)),$(f),$(WORKING_DIR)/$(f)))
EXTRA_MISSING = $(filter-out $(wildcard $(EXTRA_FILES_ABS)),$(EXTRA_FILES_ABS))

MTOOLS := mformat mmd mcopy

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
# end (30+482=512), install the boot sector code at that offset, and put a
# random volume id (4 bytes in the 00h-7Fh range, like the kernel's FORMAT
# generates) into the boot code's VOL_ID block at offset 39 (27h) - the
# assembled code carries an FFh placeholder there, which the kernel reads
# as "no volume id", disabling undeletion support for the disk.
tools-disk: $(BOOTSECT_BIN) $(HELPFILES_DIR)/JINDEX.HLP
	$(foreach exec,$(MTOOLS),\
		$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), from the mtools package, is it installed/in PATH?")))
	$(if $(EXTRA_MISSING),$(error "ERROR: EXTRA_FILES not found: $(EXTRA_MISSING)"))
	$(call stage_package,$(DISK_STAGING))
	@rm -f $(TOOLS_DSK)
	@mformat -C -i $(TOOLS_DSK) -t 80 -h 2 -n 9 -v NEXTOR ::
	@printf 'NEXTOR3 ' | dd of=$(TOOLS_DSK) bs=1 seek=3 conv=notrunc status=none
	@dd if=/dev/zero of=$(TOOLS_DSK) bs=1 seek=30 count=482 conv=notrunc status=none
	@dd if=$(BOOTSECT_BIN) of=$(TOOLS_DSK) bs=1 seek=30 conv=notrunc status=none

# Add a VOL_ID in the boot sector code, so that undeletion works on the disk.
# The kernel's FORMAT command generates a random 4-byte id in the 00h-7Fh range, so we do the same here.
	@dd if=/dev/urandom bs=1 count=4 status=none | tr '\200-\377' '\000-\177' | dd of=$(TOOLS_DSK) bs=1 seek=39 conv=notrunc status=none

	@cd $(DISK_STAGING) && mcopy -i $(abspath $(TOOLS_DSK)) $(PACKAGE_ROOT_FILES) ::
	@mmd -i $(TOOLS_DSK) ::TOOLS ::HELP
	@cd $(DISK_STAGING) && mcopy -i $(abspath $(TOOLS_DSK)) TOOLS/* ::TOOLS/
	@cd $(DISK_STAGING) && mcopy -i $(abspath $(TOOLS_DSK)) HELP/* ::HELP/
	$(if $(EXTRA_FILES_ABS),@for f in $(EXTRA_FILES_ABS); do \
		mcopy -i $(TOOLS_DSK) "$$f" "::$$(basename $$f | tr a-z A-Z)" || exit 1; \
	done)
	@rm -rf $(DISK_STAGING)
	@printf "\n\033[0;36mCreated %s\033[0m\n\n" $(TOOLS_DSK)

# ---------------------------------------------------------------------------
# Tools zip archive.
#
# `make tools-zip` creates bin/tools/tools.zip with the same contents as the
# disk image (see above), minus the EXTRA_FILES. The archive is recreated
# from scratch on every run.

TOOLS_ZIP := $(BIN_TOOLS)/tools.zip
ZIP_STAGING := tools-zip-staging

.PHONY: tools-zip

tools-zip: $(HELPFILES_DIR)/JINDEX.HLP
	$(if $(shell which zip),,$(error "ERROR: can't execute zip, is it installed/in PATH?"))
	$(call stage_package,$(ZIP_STAGING))
	@rm -f $(TOOLS_ZIP)
	@cd $(ZIP_STAGING) && zip -q -X $(abspath $(TOOLS_ZIP)) $(PACKAGE_ROOT_FILES) TOOLS/ TOOLS/* HELP/ HELP/*
	@rm -rf $(ZIP_STAGING)
	@printf "\n\033[0;36mCreated %s\033[0m\n\n" $(TOOLS_ZIP)

.PHONY: clean

clean:
	for ext in REL rel SYM sym HEX COM bin; do find . -type f -name "*.$$ext" -delete; done
	rm -f $(HELPFILES_DIR)/JINDEX.HLP
	rm -rf $(DISK_STAGING) $(ZIP_STAGING)
