
all: chkdsk.com
	@:

.ONESHELL:

chkdsk.hex: main.rel codes.rel text.rel jtext.rel var.rel end.rel misc.rel nodebug.rel ver.rel dir.rel
	@l80.sh chkdsk.hex /P:100,CODES,TEXT,JTEXT,DIR,VAR,END,MAIN,MISC,NODEBUG,VER,CHKDSK/N/X/Y/E

include rules.mk

