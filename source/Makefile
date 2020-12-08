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
export WRK_DIR=../bin/working
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

SUBMAKE := $(MAKE) -C $(WRK_DIR) --no-print-directory
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

export VERSION=2.1.1-alpha2

export PATH := $(PWD)/../linuxtools/:$(PWD)/../linuxtools/prereq/sdcc-4.0.0/bin/:$(PWD)/../linuxtools/prereq/hex2bin/:$(PWD)/../linuxtools/prereq/cpm:$(PATH)

.PHONY: all
all: prep
	@$(SUBMAKE) -O -j

.PHONY: prep
prep: $(SRC_FILES)
	@mkdir -p $(WRK_DIR)
	rm -f $(WRK_DIR)/*.mac
	rm -f $(WRK_DIR)/*.h
	sf "kernel/M80.CPM" "M80.COM"
	sf "kernel/L80.CPM" "L80.COM"
	sf "kernel/LIB80.CPM" "LIB80.COM"
	sf "Makefile-main.mk" "Makefile"
	sf "rules.mk"
	sf -s kernel bank.inc const.inc macros.inc
	sf "kernel/*.mac"
	sf "kernel/bank0/*.mac"
	sf "kernel/bank1/*.mac"
	sf "kernel/bank2/*.mac"
	sf "kernel/bank3/*.mac"
	sf kernel/bank4/dskab.mac dskab4.mac
	sf kernel/bank4/dskab.mac dskab4.mac
	sf kernel/bank4/misc.mac misc4.mac
	sf kernel/bank4/ramdrv.mac ramdrv4.mac
	sf kernel/bank4/seg.mac seg4.mac
	sf -s kernel/bank4 ramdrvh.mac env.mac b4.mac time.mac cpm.mac jump.mac partit.mac
		# 40ff.mac and doshead.mac bkalloc.mac in bank4 are not used???
	sf "kernel/bank5/*.mac"
	sf "kernel/bank5/*.s"
	sf "kernel/bank5/*.c"
	sf "kernel/bank5/*.h"
	sf "kernel/bank6/*.mac"
	sf kernel/drivers/SunriseIDE/chgbnk.mac srchgbnk.mac
	sf kernel/drivers/SunriseIDE/driver.mac sunrise.mac
	sf "command/msxdos/*.mac"
	sf command/command/files.mac cmdfiles.mac
	sf command/command/messages.mac cmdmsgs.mac
	sf command/command/misc.mac cmdmisc.mac
	sf command/command/ver.mac cmdver.mac
	sf -s command/command start.mac cli.mac cmd.mac copy.mac dirs.mac io.mac jtext.mac var.mac command.inc
	sf "command/chkdsk/main.mac"  main.mac
	sf "command/chkdsk/text.mac"  text.mac
	sf "command/chkdsk/jtext.mac" chjtext.mac
	sf "command/chkdsk/var.mac" chvar.mac
	sf "command/chkdsk/end.mac" chend.mac
	sf "command/chkdsk/misc.mac" chmisc.mac
	sf "command/chkdsk/nodebug.mac" nodebug.mac
	sf "command/chkdsk/dir.mac" chdir.mac
	sf "command/chkdsk/*.inc"
	sf "tools/*.MAC"

## Build the common tools
tools: prep
	@$(SUBMAKE) tools -j -O

## Build the sunrise kernel rom
sunrise: prep
	@$(SUBMAKE) nextor-$(VERSION).sunriseide.rom

## Build the chkdsk.com
chkdsk: prep
	@$(SUBMAKE) chkdsk

## Build the command2.com
command2: prep
	@$(SUBMAKE) command2.com

## Build the nextor.sys
nextor: prep
	@$(SUBMAKE) nextor.sys

## Build the nextork.sys
nextork: prep
	@$(SUBMAKE) nextork.sys

## Build a FAT12 hard disk image containing nextor.sys, command2.com and all other tools
hdddsk: prep
	@$(SUBMAKE) hdd.dsk

## Build a FAT12 hard disk image containing nextor.sys, command2.com and all other tools
fdddsk: prep
	@$(SUBMAKE) fdd.dsk

## build the mknexrom utility
mknexrom: prep
	@$(SUBMAKE) ../../linuxtools/mknexrom


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
