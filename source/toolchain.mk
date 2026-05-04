# Toolchain selection shared by the Linux makefiles.
#
# By default, keep using the historical uppercase Nestor80 binary names.
# Use NEXT80=1 for the native C next80 tools, which install lowercase
# n80/lk80/lb80 binaries.

NEXT80_ENABLED := $(strip $(NEXT80))

ifeq ($(NEXT80_ENABLED),1)
N80 ?= n80
LK80 ?= lk80
LB80 ?= lb80
else
N80 ?= N80
LK80 ?= LK80
LB80 ?= LB80
endif

MKNEXROM ?= mknexrom

NEXT80_N80_HELP := $(shell $(N80) --help 2>/dev/null | sed -n '1p')
NEXT80_TOOLCHAIN := unknown
ifneq ($(findstring Nestor80 compatible,$(NEXT80_N80_HELP)),)
NEXT80_TOOLCHAIN := next80
else ifneq ($(strip $(NEXT80_N80_HELP)),)
NEXT80_TOOLCHAIN := nestor80
endif
export NEXT80_TOOLCHAIN
