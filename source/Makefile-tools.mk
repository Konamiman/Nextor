
.PHONY: all
all: drvinfo.com delall.com conclus.com drivers.com fastout.com lock.com mapdrv.com
	@

.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables

TOOL_DEPS := codes.rel data.rel shared.rel
define buildtool =
	l80.sh $@ /P:100,CODES,DATA,$(basename $<),SHARED,$(basename $<)/N/X/Y/E
endef

%.hex: %.rel $(TOOL_DEPS)
	$(buildtool)

include rules.mk
