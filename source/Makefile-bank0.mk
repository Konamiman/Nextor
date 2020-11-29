
all: b0.bin b0labels.inc b0lab_b3.inc
	@:

.ONESHELL:

b0.hex: codes.rel kvar.rel data.rel rel.rel doshead.rel 40ff.rel b0.rel init.rel alloc.rel dskbasic.rel dosboot.rel bdos.rel ramdrv.rel chgbnk.rel
	@l80.sh b0.hex /p:4000,CODES,KVAR,DATA,REL,DOSHEAD,40FF,B0,INIT,ALLOC,DSKBASIC,DOSBOOT,BDOS,RAMDRV,/p:7700,drv,/p:7fd0,chgbnk,b0/n/x/y/e
	cleancpmfile.sh b0.sym

b0.bin: b0.hex
	@rm -f b0.bin
	hex2bin -s 4000 b0.hex

codes.rel:
kvar.rel: macros.inc const.inc
rel.rel:
doshead.rel: macros.inc const.inc
40ff:
b0.rel: bank.inc
init.rel: const.inc bank.inc
alloc.rel:
bdos.rel: macros.inc const.inc
ramdrv.rel: macros.inc const.inc
chgbnk.rel:

b0labels.inc: b0.hex
	@symtoequs.sh b0.sym b0labels.inc "\?\S*" DOSV0 GETERR BDOSE KDERR KABR

b0lab_b3.inc: b0.hex
	@symtoequs.sh b0.sym b0lab_b3.inc INIT TIMINT MAPBIO GWRK "R_\S*"

include rules.mk
	