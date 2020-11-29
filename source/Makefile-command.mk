
all: command2.com
	@:

.ONESHELL:

command2.hex: codes.rel data.rel start.rel cli.rel cmd.rel copy.rel dirs.rel files.rel io.rel jtext.rel messages.rel misc.rel var.rel ver.rel
	@l80.sh command2.hex /P:100,CODES,DATA,START,CLI,CMD,COPY,DIRS,FILES,IO,JTEXT,MESSAGES,MISC,VAR,VER,COMMAND2/n/x/y/e

include rules.mk
