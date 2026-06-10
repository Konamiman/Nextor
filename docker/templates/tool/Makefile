# @@NAME@@ — a Nextor-aware MSX-DOS command (.COM).
#
# Scaffolded by `nextor-init tool`. Build inside the Nextor dev container,
# which supplies N80 and the SDK ($NEXTOR_SDK) as environment variables.
#
#   make            build @@NAME@@.COM
#   make clean      remove build artifacts

NAME := @@NAME@@
COM  := $(NAME).COM

NEXTOR_SDK ?= /opt/nextor/sdk
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
