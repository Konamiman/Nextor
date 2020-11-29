

all:  nextor-$(VERSION).sunriseide.rom
	@:

.ONESHELL:

driver.hex: driver.rel
	@l80.sh driver.hex /P:4100,DRIVER,DRIVER/N/X/Y/E

driver.bin: driver.hex
	@rm -f driver.bin
	hex2bin -s 4000 driver.hex

chgbnk.hex: chgbnk.rel
	@l80.sh chgbnk.hex /P:7fd0,CHGBNK,CHGBNK/N/X/Y/E

chgbnk.bin: chgbnk.hex
	@rm -f chgbnk.bin
	hex2bin -s 7FD0 chgbnk.hex

nextor-$(VERSION).sunriseide.rom:  ../../dos250ba.dat driver.bin chgbnk.bin ../../../../../linuxtools/mknexrom
	@mknexrom  ../../dos250ba.dat nextor-$(VERSION).sunriseide.rom -d:driver.bin -m:chgbnk.bin

nextor-$(VERSION).sunriseide.emulators.rom:

# del ..\..\..\..\bin\kernels\Nextor-2.1.1-alpha2.SunriseIDE.emulators.ROM
# ren ..\..\..\..\bin\kernels\Nextor-2.1.1-alpha2.SunriseIDE.ROM Nextor-2.1.1-alpha2.SunriseIDE.emulators.ROM 
# sjasm -c sunride.asm driver.bin
# ..\..\..\..\wintools\mknexrom ..\..\Nextor-2.1.1-alpha2.base.dat nextor2.rom /d:driver.bin /m:chgbnk.bin
# copy nextor2.rom ..\..\..\..\bin\kernels\Nextor-2.1.1-alpha2.SunriseIDE.ROM
# :okide
# cd ..


include rules.mk

