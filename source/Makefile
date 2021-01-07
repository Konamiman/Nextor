# Top level makefile for Nextor.
# It will build the kernel ROM files, NEXTOR.SYS and the command line tools.
# The generated files will be copied to bin/kernels and bin/tools.
# The makefiles on each directory can be used independently as well.

all:
	$(MAKE) -C kernel
	$(MAKE) -C command/msxdos
	$(MAKE) -C tools
	$(MAKE) -C tools/C

.PHONY: clean

clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C command/msxdos clean
	$(MAKE) -C tools clean
	$(MAKE) -C tools/C clean
