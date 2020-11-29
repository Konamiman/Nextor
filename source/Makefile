# ######################################
#
# Makefile system for building nextor units
# test on ubuntu only
#
# requires at least: bash 4.4, make 4.1
# other prerequisites can be installed using make install-prereq
#
# See README.md in ./linuxtools for more information
#
# #######################################

ifndef BUILD_TYPE
override export BUILD_TYPE = std
endif

export SRC_ROOT_DIR=$(PWD)
WRK_DIR=../bin/working
KERNEL_WRK_DIR=$(WRK_DIR)/kernel
COMMAND_WRK_DIR=$(WRK_DIR)/command
TOOLS_WRK_DIR=$(WRK_DIR)/tools
BANK0_WRK_DIR=$(KERNEL_WRK_DIR)/bank0
BANK1_WRK_DIR=$(KERNEL_WRK_DIR)/bank1
BANK2_WRK_DIR=$(KERNEL_WRK_DIR)/bank2
BANK3_WRK_DIR=$(KERNEL_WRK_DIR)/bank3
BANK4_WRK_DIR=$(KERNEL_WRK_DIR)/bank4
BANK5_WRK_DIR=$(KERNEL_WRK_DIR)/bank5
BANK6_WRK_DIR=$(KERNEL_WRK_DIR)/bank6
MSXDOS_WRK_DIR=$(COMMAND_WRK_DIR)/msxdos
CMD_WRK_DIR=$(COMMAND_WRK_DIR)/command
CHKDSK_WRK_DIR=$(COMMAND_WRK_DIR)/chkdsk
DRV_SUNRISE_WRK_DIR=$(KERNEL_WRK_DIR)/drivers/sunriseide

SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables

export VERSION=2.1.1-alpha2

export PATH := $(PWD)/../linuxtools/:$(PWD)/../linuxtools/prereq/sdcc-4.0.0/bin/:$(PWD)/../linuxtools/prereq/hex2bin/:$(PWD)/../linuxtools/prereq/cpm:$(PATH)

define prepdir
	mkdir -p $(1)
	linkfile.sh "kernel/M80.CPM" "$(PWD)/$(1)/M80.COM"
	linkfile.sh "kernel/L80.CPM" "$(PWD)/$(1)/L80.COM"
	linkfile.sh "kernel/LIB80.CPM" "$(PWD)/$(1)/LIB80.COM"
	linkfiles.sh "kernel/*.inc" "$(1)"
	rm -f $(1)/condasm.inc
	linkfile.sh "rules.mk" "$(PWD)/$(1)/"
endef

## Primary Targets

# Build all units excluding hdddsk
all: drivers tools msxdos command chkdsk
	@:

## Build the sunrise rom image into bin/drivers/sunriseide
sunrise: drivers/sunrise
	@:

## Build all drivers
drivers: drivers/sunrise
	@:

.PHONY: hdddsk
## Build a FAT12 hard disk image containing nextor.sys, command2.com and all other tools
hdddsk: msxdos command chkdsk tools ## sdfsdf
	@sudo umount -df /media/hdddsk > /dev/null 2>&1 || true
	rm -f ../bin/hdd.dsk
	mkfs.vfat -C "../bin/hdd.dsk" 10000
	sudo mkdir -p /media/hdddsk
	sudo mount -t vfat ../bin/hdd.dsk /media/hdddsk
	sudo cp ../bin/cli/*.com /media/hdddsk
	sudo cp ../bin/cli/nextor.sys /media/hdddsk
	sudo umount -df /media/hdddsk


fdddsk: ../bin/fdd.dsk
## Build a FAT12 hard disk image containing nextor.sys, command2.com and all other tools
../bin/fdd.dsk: ../bin/cli/command2.com ../bin/cli/nextor.sys ../bin/cli/drvinfo.com
	@sudo umount -df /media/fdddsk > /dev/null 2>&1 || true
	rm -f ../bin/fdd.dsk
	dd if=/dev/zero of=../bin/fdd.dsk bs=64k count=1
	mkfs.vfat -F 12 -f 1 ../bin/fdd.dsk
	sudo mkdir -p /media/fdddsk
	sudo mount -t vfat ../bin/fdd.dsk /media/fdddsk
	sudo cp ../bin/cli/command2.com /media/fdddsk
	sudo cp ../bin/cli/nextor.sys /media/fdddsk
	sudo cp ../bin/cli/drvinfo.com /media/fdddsk
	sudo umount -df /media/fdddsk

../bin/cli/nextor.sys: msxdos
	@:
../bin/cli/command2.com: command
	@:
../bin/cli/drvinfo.com: tools
	@:

.PHONY: msxdos
## Build the nextor.sys and nextork.sys files (bin/cli/)
msxdos: kernel
	@$(call prepdir,$(MSXDOS_WRK_DIR))
	linkfiles.sh "command/msxdos/*.mac" "$(MSXDOS_WRK_DIR)"
	rm -f $(MSXDOS_WRK_DIR)/usekmsg.mac
	linkfile.sh "Makefile-msxdos.mk" "$(MSXDOS_WRK_DIR)/Makefile"
	linkfiles.sh "$(KERNEL_WRK_DIR)/*.rel" "$(MSXDOS_WRK_DIR)"
	$(MAKE) -C $(MSXDOS_WRK_DIR) --no-print-directory
	mkdir -p ../bin/cli
	cp -up $(MSXDOS_WRK_DIR)/*.sys ../bin/cli

.PHONY: command
## Build the bin/cli/command2.com unit
command: kernel
	@$(call prepdir,$(CMD_WRK_DIR))
	linkfiles.sh "command/command/*.mac" "$(CMD_WRK_DIR)"
	linkfiles.sh "command/command/*.MAC" "$(CMD_WRK_DIR)"
	linkfiles.sh "command/command/*.inc" "$(CMD_WRK_DIR)"
	linkfile.sh "Makefile-command.mk" "$(CMD_WRK_DIR)/Makefile"
	linkfiles.sh "$(KERNEL_WRK_DIR)/*.rel" "$(CMD_WRK_DIR)"
	$(MAKE) -C $(CMD_WRK_DIR) --no-print-directory
	mkdir -p ../bin/cli
	cp -up $(CMD_WRK_DIR)/*.com ../bin/cli

.PHONY: chkdsk
## Build the bin/cli/chkdsk.com unit
chkdsk: command kernel
	@$(call prepdir,$(CHKDSK_WRK_DIR))
	linkfiles.sh "command/chkdsk/*.mac" "$(CHKDSK_WRK_DIR)"
	linkfiles.sh "command/chkdsk/*.MAC" "$(CHKDSK_WRK_DIR)"
	linkfiles.sh "command/chkdsk/*.inc" "$(CHKDSK_WRK_DIR)"
	linkfile.sh "$(CMD_WRK_DIR)/ver.rel" "$(CHKDSK_WRK_DIR)"
	linkfile.sh "Makefile-chkdsk.mk" "$(CHKDSK_WRK_DIR)/Makefile"
	linkfiles.sh "$(KERNEL_WRK_DIR)/*.rel" "$(CHKDSK_WRK_DIR)"
	$(MAKE) -C $(CHKDSK_WRK_DIR) --no-print-directory
	mkdir -p ../bin/cli
	cp -up $(CHKDSK_WRK_DIR)/*.com ../bin/cli


../linuxtools/mknexrom: ../wintools/mknexrom.c
	@gcc ../wintools/mknexrom.c -o ../linuxtools/mknexrom

.PHONY: kernel
kernel:
	@$(call prepdir,$(KERNEL_WRK_DIR))
	linkfiles.sh "kernel/*.mac" "$(KERNEL_WRK_DIR)"
	linkfile.sh "Makefile-kernel.mk" "$(KERNEL_WRK_DIR)/Makefile"
	$(MAKE) -C $(KERNEL_WRK_DIR) --no-print-directory

bank0: kernel
	@$(call prepdir,$(BANK0_WRK_DIR))
	linkfiles.sh "kernel/bank0/*.mac" "$(BANK0_WRK_DIR)"
	linkfile.sh "Makefile-bank0.mk" "$(BANK0_WRK_DIR)/Makefile"
	linkfiles.sh "$(KERNEL_WRK_DIR)/*.rel" "$(BANK0_WRK_DIR)"
	$(MAKE) -C $(BANK0_WRK_DIR) --no-print-directory

bank1: bank0 kernel
	@$(call prepdir,$(BANK1_WRK_DIR))
	linkfiles.sh "kernel/bank1/*.mac" "$(BANK1_WRK_DIR)"
	linkfile.sh "Makefile-bank1.mk" "$(BANK1_WRK_DIR)/Makefile"
	linkfiles.sh "$(KERNEL_WRK_DIR)/*.rel" "$(BANK1_WRK_DIR)"
	linkfiles.sh "$(BANK0_WRK_DIR)/*.rel" "$(BANK1_WRK_DIR)"
	linkfiles.sh "$(BANK0_WRK_DIR)/b0*.inc" "$(BANK1_WRK_DIR)"
	$(MAKE) -C $(BANK1_WRK_DIR) --no-print-directory

bank2: bank0 kernel
	@$(call prepdir,$(BANK2_WRK_DIR))
	linkfiles.sh "kernel/bank2/*.mac" "$(BANK2_WRK_DIR)"
	linkfile.sh "Makefile-bank2.mk" "$(BANK2_WRK_DIR)/Makefile"
	linkfiles.sh "$(KERNEL_WRK_DIR)/*.rel" "$(BANK2_WRK_DIR)"
	linkfiles.sh "$(BANK0_WRK_DIR)/b0*.inc" "$(BANK2_WRK_DIR)"
	$(MAKE) -C $(BANK2_WRK_DIR) --no-print-directory

bank3: bank0 kernel
	@$(call prepdir,$(BANK3_WRK_DIR))
	linkfiles.sh "kernel/bank3/*.mac" "$(BANK3_WRK_DIR)"
	linkfile.sh "Makefile-bank3.mk" "$(BANK3_WRK_DIR)/Makefile"
	linkfiles.sh "$(KERNEL_WRK_DIR)/*.rel" "$(BANK3_WRK_DIR)"
	linkfile.sh "$(BANK0_WRK_DIR)/doshead.rel" "$(BANK3_WRK_DIR)/"
	linkfile.sh "$(BANK0_WRK_DIR)/40ff.rel" "$(BANK3_WRK_DIR)/"
	linkfile.sh "$(BANK0_WRK_DIR)/b0lab_b3.inc" "$(BANK3_WRK_DIR)/b0labels.inc"
	$(MAKE) -C $(BANK3_WRK_DIR) --no-print-directory

bank4: bank0 bank2 kernel
	@$(call prepdir,$(BANK4_WRK_DIR))
	linkfiles.sh "kernel/bank4/*.mac" "$(BANK4_WRK_DIR)"
	linkfile.sh "Makefile-bank4.mk" "$(BANK4_WRK_DIR)/Makefile"
	linkfiles.sh "$(KERNEL_WRK_DIR)/*.rel" "$(BANK4_WRK_DIR)"
	linkfiles.sh "$(BANK0_WRK_DIR)/b0*.inc" "$(BANK4_WRK_DIR)"
	linkfiles.sh "$(BANK2_WRK_DIR)/b2*.inc" "$(BANK4_WRK_DIR)"
	$(MAKE) -C $(BANK4_WRK_DIR) --no-print-directory

bank5: kernel
	@$(call prepdir,$(BANK5_WRK_DIR))
	linkfiles.sh "kernel/bank5/*.mac" "$(BANK5_WRK_DIR)"
	linkfiles.sh "kernel/bank5/*.s" "$(BANK5_WRK_DIR)"
	linkfiles.sh "kernel/bank5/*.c" "$(BANK5_WRK_DIR)"
	linkfiles.sh "kernel/bank5/*.h" "$(BANK5_WRK_DIR)"
	linkfile.sh "kernel/bank5/drivercall.h" "$(BANK5_WRK_DIR)/drivercall.h"
	linkfile.sh "Makefile-bank5.mk" "$(BANK5_WRK_DIR)/Makefile"
	linkfiles.sh "$(KERNEL_WRK_DIR)/*.rel" "$(BANK5_WRK_DIR)"
	$(MAKE) -C $(BANK5_WRK_DIR) --no-print-directory

bank6: kernel
	@$(call prepdir,$(BANK6_WRK_DIR))
	linkfiles.sh "kernel/bank6/*.mac" "$(BANK6_WRK_DIR)"
	linkfile.sh "Makefile-bank6.mk" "$(BANK6_WRK_DIR)/Makefile"
	linkfiles.sh "$(KERNEL_WRK_DIR)/*.rel" "$(BANK6_WRK_DIR)"
	$(MAKE) -C $(BANK6_WRK_DIR) --no-print-directory

base: kernel bank0 bank1 bank2 bank3 bank4 bank5 bank6
	@$(MAKE) -C $(KERNEL_WRK_DIR) dos250ba.dat -s

.PHONY: ../bin/nextor-$(VERSION).sunriseide.rom
../bin/nextor-$(VERSION).sunriseide.rom: base ../linuxtools/mknexrom
	@$(call prepdir,$(DRV_SUNRISE_WRK_DIR))
	linkfiles.sh "kernel/drivers/SunriseIDE/*.mac" "$(DRV_SUNRISE_WRK_DIR)"
	linkfile.sh "Makefile-driver-sunrise-ide.mk" "$(DRV_SUNRISE_WRK_DIR)/Makefile"
	linkfile.sh "kernel/bank6/b6.mac" "$(DRV_SUNRISE_WRK_DIR)"
	linkfiles.sh "$(BANK0_WRK_DIR)/b0*.inc" "$(DRV_SUNRISE_WRK_DIR)"
	$(MAKE) -C $(DRV_SUNRISE_WRK_DIR) --no-print-directory
	cp -up $(DRV_SUNRISE_WRK_DIR)/*.rom ../bin/

.PHONY: drivers/sunrise
drivers/sunrise: ../bin/nextor-$(VERSION).sunriseide.rom

deps:
	findmacincls.sh > ./Makefile.deps.mk


.PHONY: tools
## Build all the tools binaries into bin/cli
tools: kernel
	@$(call prepdir,$(TOOLS_WRK_DIR))
	linkfiles.sh "tools/*.mac" "$(TOOLS_WRK_DIR)"
	linkfiles.sh "tools/*.MAC" "$(TOOLS_WRK_DIR)"
	linkfile.sh "Makefile-tools.mk" "$(TOOLS_WRK_DIR)/Makefile"
	linkfiles.sh "$(KERNEL_WRK_DIR)/*.rel" "$(TOOLS_WRK_DIR)"
	$(MAKE) -C $(TOOLS_WRK_DIR) --no-print-directory
	mkdir -p ../bin/cli
	cp -up $(TOOLS_WRK_DIR)/*.com ../bin/cli


## Other targets

## Remove the bin directory
clean:
	rm -rf ../bin

../linuxtools/prereq/sdcc-4.0.0/bin/sz80:
	@mkdir -p ../linuxtools/prereq/
	cd ../linuxtools/prereq
	wget https://sourceforge.net/projects/sdcc/files/sdcc-linux-amd64/4.0.0/sdcc-4.0.0-amd64-unknown-linux2.5.tar.bz2/download -O "sdcc-4.0.0-amd64-unknown-linux2.5.tar.bz2"
	tar -xjf  sdcc-4.0.0-amd64-unknown-linux2.5.tar.bz2
	rm sdcc-4.0.0-amd64-unknown-linux2.5.tar.bz2

../linuxtools/prereq/hex2bin/hex2bin:
	@mkdir -p ../linuxtools/prereq/
	cd ../linuxtools/prereq
	git clone --depth 1 git@github.com:E3V3A/hex2bin.git
	cd hex2bin
	make

../linuxtools/prereq/cpm/cpm:
	@mkdir -p ../linuxtools/prereq/
	cd ../linuxtools/prereq
	git clone --depth 1 git@github.com:jhallen/cpm.git
	cd cpm
	OS=linux make -B --trace

## Install required tooling (sdcc, hex2bin, cpm) into (linuxtools/prereq)
install-prereq: ../linuxtools/prereq/sdcc-4.0.0/bin/sz80 ../linuxtools/prereq/hex2bin/hex2bin ../linuxtools/prereq/cpm/cpm
	@:

.PHONY: help
## Display this help message
help:
	@printf "Usage\n";
	awk '{ \
			if ($$0 ~ /^.PHONY: [a-zA-Z\-\_0-9]+$$/) { \
				helpCommand = substr($$0, index($$0, ":") + 2); \
				if (helpMessage) { \
					printf "\033[36m%-20s\033[0m %s\n", \
						helpCommand, helpMessage; \
					helpMessage = ""; \
				} \
			} else if ($$0 ~ /^[a-zA-Z\-\_0-9.]+:/) { \
				helpCommand = substr($$0, 0, index($$0, ":")); \
				if (helpMessage) { \
					printf "\033[36m%-20s\033[0m %s\n", \
						helpCommand, helpMessage; \
					helpMessage = ""; \
				} \
			} else if ($$0 ~ /^##/) { \
				if (helpMessage) { \
					helpMessage = helpMessage"\n                     "substr($$0, 3); \
				} else { \
					helpMessage = substr($$0, 3); \
				} \
			} else { \
				if (helpMessage) { \
					print "\n                     "helpMessage"\n" \
				} \
				helpMessage = ""; \
			} \
		}' \
		$(MAKEFILE_LIST)
