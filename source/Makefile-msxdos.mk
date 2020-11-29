
all: nextor.sys nextork.sys
	@:

.ONESHELL:

nextor.hex: nokmsg.mac codes.rel data.rel reloc.rel ver.rel ref.rel sys.mac real.rel messages.rel end.rel
	@ln -sf nokmsg.mac usekmsg.mac
	m80.sh sys.mac SYS
	l80.sh nextor.hex /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,END,NEXTOR/n/x/y/e

nextor.sys: nextor.hex
	@hex2bin -s 100 nextor.hex
	mv nextor.bin nextor.sys

nextork.hex: yeskmsg.mac codes.rel data.rel reloc.rel ver.rel ref.rel sys.mac real.rel messages.rel kmsg.rel end.rel
	@ln -sf yeskmsg.mac usekmsg.mac
	m80.sh sys.mac SYS
	l80.sh nextork.hex /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,KMSG,END,NEXTORK/n/x/y/e

nextork.sys: nextork.hex
	@hex2bin -s 100 nextork.hex
	mv nextork.bin nextork.sys

include rules.mk

