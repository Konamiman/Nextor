#
#	@(#)Makefile for panasonic A1-ST disk ROM
#

### DOSKITs

KIT2=	../bank0/dos2kit.rel
HEX1=	../bank1/b1.hex
HEX2=	../bank2/b2.hex
KIT1=	../bank3/dos1kit.rel

### MSX-DOS2 drivers

OEM2=	dskdrv2.rel format2.rel
BNK2=	chgbnk2.rel

### MSX-DOS1 drivers

OEM1=	dskdrv1.rel format1.rel
BNK1=	chgbnk1.rel

### Ruleset

XL80=	xl80

.SUFFIXES: .mac .rel
.mac.rel:
	cpm32 m80 =$*.mac

all:	kernelrom.bin

clean:
	rm -f *.REL *.BAK

b0.hex: $(KIT2) $(OEM2) $(BNK2)
	@echo Loading $@
	cp $(KIT2) .
	cpm32 L80 /P:4000,DOS2KIT,/P:7000,DSKDRV2,FORMAT2,/P:7FD0,CHGBNK2,B0/X/Y/N/E
#	@$(XL80) -p 4000 $(KIT2) -p 7000 $(OEM2) -p 7fd0 $(BNK2) -P -x$@ -y b0.sym

b3.hex:	$(KIT1) $(OEM1) $(BNK1)
	@echo Loading $@
	cp $(KIT1) .
	cpm32 L80 /P:4000,/D:F237,DOS1KIT,DSKDRV1,FORMAT1,/P:7FD0,CHGBNK1,B3/X/Y/N/E
#	@$(XL80) -p4000 -df237 $(KIT1) $(OEM1) -p7fd0 $(BNK1) -P -x$@ -y b3.sym

kernelrom.bin kernelrom.hex: b0.hex $(HEX1) $(HEX2) b3.hex
	@echo Making $@
	@./h2b	\
		-o  8000 b3.hex \
		-o -4000 b0.hex \
		-o     0 b0.hex -f 4100,7fcf,ff -o    0 $(HEX1) \
		-o  4000 b0.hex -f 8100,bfcf,ff -o 4000 $(HEX2) \
		>kernelrom.bin
	@./BIN2HEX /y kernelrom.bin kernelrom.hex
#	@b2h <kernelrom.bin >kernelrom.hex

### Verify L80/XL80 compatibility

verify:
	APPEND=:../bank0 \
	l80 /p:4000,dos2kit,/p:7000,dskdrv2,format2,/p:7fd0,chgbnk2,v0/n/x/e
	diff b0.hex v0.hex
	APPEND=:../bank3 \
	l80 /p:4000,/d:f237,dos1kit,dskdrv1,format1,/p:7fd0,chgbnk1,v3/n:p/x/e
	diff b3.hex v3.hex
	rm -f v?.hex

### Download for testing
#
# Requires some PC is connected to ROM-Writer thru printer port, and
# FTPD.EXE is running there.
#
# OOPS that is aki-m's PC!!
#
WRITER=	sypc20

download: kernelrom.hex
	@(	echo "open $(WRITER)"; \
		echo "user $(USER)"; \
		echo "ascii"; \
		echo "put kernelrom.hex prn" ) | ftp -n
