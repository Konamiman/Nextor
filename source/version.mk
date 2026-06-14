# Generate a NEXTOR_KERNEL_VERSION variable from the Nextor KERNEL version
# by parsing the version.inc file.
# 
# Include this file in any script or makefile that requires
# the Nextor kernel version in human-readable form.

_VER_INC := $(dir $(lastword $(MAKEFILE_LIST)))kernel/version.inc

# $(call _ver,NAME) -> value of a `NAME equ <n>` line in version.inc
_ver = $(shell sed -nE 's/^[[:space:]]*$(1)[[:space:]]+equ[[:space:]]+([0-9]+).*/\1/p' $(_VER_INC))

_VER_MAIN := $(call _ver,MAIN_NEXTOR_VERSION)
_VER_HIGH := $(call _ver,SEC_NEXTOR_VERSION_HIGH)
_VER_LOW  := $(call _ver,SEC_NEXTOR_VERSION_LOW)

# Fail loudly if a numeric constant was renamed/removed, rather than silently
# building a "Nextor-..base.dat" with an empty version.
ifeq ($(_VER_MAIN),)
$(error version.mk: could not read MAIN_NEXTOR_VERSION from $(_VER_INC))
endif
ifeq ($(_VER_HIGH),)
$(error version.mk: could not read SEC_NEXTOR_VERSION_HIGH from $(_VER_INC))
endif
ifeq ($(_VER_LOW),)
$(error version.mk: could not read SEC_NEXTOR_VERSION_LOW from $(_VER_INC))
endif

NEXTOR_KERNEL_VERSION := $(_VER_MAIN).$(_VER_HIGH).$(_VER_LOW)

# Append the pre-release marker, mirroring betainfo.mac. A level of 0 (or an
# absent line) means "none"; normally at most one level is non-zero.
_VER_ALPHA := $(call _ver,ALPHA)
_VER_BETA  := $(call _ver,BETA)
_VER_RC    := $(call _ver,RC)
_VER_RR    := $(call _ver,RR)

ifneq ($(filter-out 0,$(_VER_ALPHA)),)
NEXTOR_KERNEL_VERSION := $(NEXTOR_KERNEL_VERSION)-alpha$(_VER_ALPHA)
else ifneq ($(filter-out 0,$(_VER_BETA)),)
NEXTOR_KERNEL_VERSION := $(NEXTOR_KERNEL_VERSION)-beta$(_VER_BETA)
else ifneq ($(filter-out 0,$(_VER_RC)),)
NEXTOR_KERNEL_VERSION := $(NEXTOR_KERNEL_VERSION)-rc$(_VER_RC)
else ifneq ($(filter-out 0,$(_VER_RR)),)
NEXTOR_KERNEL_VERSION := $(NEXTOR_KERNEL_VERSION)-r$(_VER_RR)
endif
