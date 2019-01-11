#
#	@(#)Makefile for standalone test ROM
#

### DOSKIT

KIT2=	../bank0/dos2kit.rel
HEX1=	../bank1/b1.hex
HEX2=	../bank2/b2.hex
KIT1=	../bank3/dos1kit.rel

### OEM modules

OEM2=	drv.rel
BNK2=	chgbnk2.rel
OEM1=	drv.rel
BNK1=	chgbnk1.rel
TARGET=	dos2rom.bin

### Ruleset

XL80=	xl80

.SUFFIXES: .mac .rel
.mac.rel:
	m80 =$*.mac

all:	$(TARGET)

b0.hex: $(KIT2) $(OEM2) $(BNK2)
	@echo Loading $@
	@$(XL80) -p4000 $(KIT2) -p7405 $(OEM2) -p7fd0 $(BNK2) -P -x$@

b3.hex:	$(KIT1) $(OEM1) $(BNK1)
	@echo Loading $@
	@$(XL80) -p4000 -df237 $(KIT1) $(OEM1) -p7fd0 $(BNK1) -P -x$@

$(TARGET): b0.hex $(HEX1) $(HEX2) b3.hex
	@echo Making $@
	@h2b	-o -4000 b0.hex \
		-o     0 b0.hex -f 4100,7fcf,ff -o    0 $(HEX1) \
		-o  4000 b0.hex -f 8100,bfcf,ff -o 4000 $(HEX2) \
		-o  8000 b3.hex >$@

### Verify compatibility

verify:
	APPEND=:../bank0 \
	l80 /p:4000,dos2kit,/p:7405,dskdrv2,format1,/p:7fd0,chgbnk2,v0/n/x/e
	diff b0.hex v0.hex
	APPEND=:../bank3 \
	l80 /p:4000,/d:f237,dos1kit,dskdrv1,format1,/p:7fd0,chgbnk1,v3/n:p/x/e
	diff b3.hex v3.hex
	rm -f v?.hex

### Download for testing

WRITER=	sypc20

install: $(TARGET)
	@(	echo "open $(WRITER)"; \
		echo "user $(USER)"; \
		echo "ascii"; \
		echo "put $(TARGET) prn" ) | ftp -v
