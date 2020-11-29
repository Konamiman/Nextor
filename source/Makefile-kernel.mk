
all: codes.rel kvar.rel data.rel rel.rel chgbnk.rel drv.rel
	@:

dos250ba.dat: bank0/b0.bin bank1/b1.bin bank2/b2.bin bank3/b3.bin bank4/b4.bin bank5/b5.bin bank6/b6.bin bank5/fdisk.dat bank5/fdisk2.dat
	@cat bank0/b0.bin bank1/b1.bin bank2/b2.bin bank3/b3.bin bank4/b4.bin bank5/b5.bin bank6/b6.bin > dos250ba.dat
	@dd conv=notrunc if=dos250ba.dat of=doshead.bin bs=1 count=255
	@dd conv=notrunc if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=16k
	@dd conv=notrunc if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=32k
	@dd conv=notrunc if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=64k
	@dd conv=notrunc if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=96k
	@dd conv=notrunc if=bank5/fdisk.dat of=dos250ba.dat bs=1 count=16000 seek=82176
	@dd conv=notrunc if=bank5/fdisk2.dat of=dos250ba.dat bs=1 count=8000 seek=98560
	@dd conv=notrunc if=doshead.bin of=dos250ba.dat bs=1 count=255 seek=80k
	@dd conv=notrunc if=bank4/b4rd.bin of=dos250ba.dat bs=1 count=15 seek=65664

include rules.mk

