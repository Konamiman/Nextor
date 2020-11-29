
.PHONY: all
all: drvinfo.com delall.com
	@:

.ONESHELL:

drvinfo.hex: drvinfo.rel codes.rel data.rel shared.rel
	@l80.sh drvinfo.hex /P:100,CODES,DATA,DRVINFO,SHARED,DRVINFO/N/X/Y/E

delall.hex: delall.rel codes.rel data.rel shared.rel
	@l80.sh delall.hex /P:100,CODES,DATA,DELALL,SHARED,DELALL/N/X/Y/E

include rules.mk
