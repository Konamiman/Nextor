# Makefile for a Nextor-aware .COM program, assembled with N80
# (Nestor80).
#
#   make            build the .COM
#   make clean      remove build artifacts
#
# Tools needed: just N80 on PATH (or override N80 below). The Nextor dev
# Docker image has it preinstalled and preconfigured - see docker/README.md
# in the Nextor repository.

# TODO: Name your tool; the output .COM is named after it.
NAME := mytool
COM  := $(NAME).COM

# Where to find the Nextor SDK. Inside the Nextor dev container this comes
# preset as an environment variable, so the default below doesn't apply
# there. Outside the container, the default works while this directory still
# sits at sdk/templates/tool/ in a Nextor checkout; if you copied the
# template elsewhere, TODO: point it at your SDK location (here or on the
# command line: `make NEXTOR_SDK=<path>`; sdk/README.md describes ways to
# bring the SDK into a project).
NEXTOR_SDK ?= ../..
N80        ?= N80
N80_FLAGS  := --no-string-escapes --no-show-banner --verbosity 0 --include-directory $(NEXTOR_SDK)

.PHONY: all clean
all: $(COM)

# org 0100h in tool.asm makes this an absolute binary, i.e. a ready-to-run .COM.
$(COM): tool.asm
	$(N80) tool.asm $(COM) $(N80_FLAGS)
	@echo "Built $(COM)"

clean:
	rm -f $(COM)
