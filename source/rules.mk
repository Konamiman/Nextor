
SHELL := /bin/bash
.DELETE_ON_ERROR:
.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c

%.rel: %.mac version.inc condasm.inc
	@echo -e "\nAssembling \e[32m$<\e[0m"
	m80.sh "$<" "$@" 2>&1 | grep -v "Sorry, terminal not found, using cooked mode."

%.rel: %.s
	@sdasz80 -o $@ $<

%.com: %.hex
	@hex2bin -s 100 $<
	mv $(basename $<).bin $(basename $<).com

version.inc: $(SRC_ROOT_DIR)/kernel/condasm/version.inc
	@ln -sf "$(SRC_ROOT_DIR)/kernel/condasm/version.inc" ./version.inc

condasm.inc: $(SRC_ROOT_DIR)/kernel/condasm/$(BUILD_TYPE).inc
	@ln -sf "$(SRC_ROOT_DIR)/kernel/condasm/$(BUILD_TYPE).inc" ./condasm.inc
