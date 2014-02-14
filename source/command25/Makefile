include	Makefile.def

MAKE=	make -f ../Makefile.def
MASTER=	MASTER

all:	msxlib.rel clibmath.rel
#	@cd loader;	$(MAKE) -f ../Makefile.sys msxdos.sys
	@cd msxdos;	$(MAKE) -f ../Makefile.sys msxdos2.sys
	@cd command;	$(MAKE) -f ../Makefile.sys command2.com
	@cd diskcopy;	$(MAKE) -f ../Makefile.utl diskcopy.com
	@cd xcopy;	$(MAKE) -f ../Makefile.utl xcopy.com
	@cd xdir;	$(MAKE) -f ../Makefile.utl xdir.com
	@cd chkdsk;	$(MAKE) -f ../Makefile.utl chkdsk.com
	@cd undel;	$(MAKE) -f ../Makefile.utl undel.com
	@cd fixdisk;	$(MAKE) -f ../Makefile.utl fixdisk.com
	@cd kmode;	$(MAKE) -f ../Makefile.utl kmode.com

testdisk:
	put -b msxdos/msxdos2.sys	\\b\\msxdos2.sys
	put -b command/command2.com	\\b\\command2.com
	put -b diskcopy/diskcopy.com	\\b\\utils\\diskcopy.com
	put -b xcopy/xcopy.com		\\b\\utils\\xcopy.com
	put -b xdir/xdir.com		\\b\\utils\\xdir.com
	put -b chkdsk/chkdsk.com	\\b\\utils\\chkdsk.com
	put -b undel/undel.com		\\b\\utils\\undel.com
	put -b fixdisk/fixdisk.com	\\b\\utils\\fixdisk.com
	put -b kmode/kmode.com		\\b\\utils\\kmode.com

master:
	rm -rf $(MASTER)
	mkdir $(MASTER)
#	cp -p loader/msxdos.sys		$(MASTER)
	cp -p msxdos/msxdos2.sys	$(MASTER)
	cp -p command/command2.com	$(MASTER)
	mkdir $(MASTER)/utils
	cp -p diskcopy/diskcopy.com	$(MASTER)/utils
	cp -p xcopy/xcopy.com		$(MASTER)/utils
	cp -p xdir/xdir.com		$(MASTER)/utils
	cp -p chkdsk/chkdsk.com		$(MASTER)/utils
	cp -p undel/undel.com		$(MASTER)/utils
	cp -p fixdisk/fixdisk.com	$(MASTER)/utils
	cp -p kmode/kmode.com		$(MASTER)/utils
	mkdir $(MASTER)/help
	../u2d ../help/assign.hlp	$(MASTER)/help/assign.hlp
	../u2d ../help/atdir.hlp	$(MASTER)/help/atdir.hlp
	../u2d ../help/attrib.hlp	$(MASTER)/help/attrib.hlp
	../u2d ../help/basic.hlp	$(MASTER)/help/basic.hlp
	../u2d ../help/buffers.hlp	$(MASTER)/help/buffers.hlp
	../u2d ../help/cd.hlp		$(MASTER)/help/cd.hlp
	../u2d ../help/chdir.hlp	$(MASTER)/help/chdir.hlp
	../u2d ../help/chkdsk.hlp	$(MASTER)/help/chkdsk.hlp
	../u2d ../help/cls.hlp		$(MASTER)/help/cls.hlp
	../u2d ../help/command2.hlp	$(MASTER)/help/command2.hlp
	../u2d ../help/concat.hlp	$(MASTER)/help/concat.hlp
	../u2d ../help/copy.hlp		$(MASTER)/help/copy.hlp
	../u2d ../help/date.hlp		$(MASTER)/help/date.hlp
	../u2d ../help/del.hlp		$(MASTER)/help/del.hlp
	../u2d ../help/dir.hlp		$(MASTER)/help/dir.hlp
	../u2d ../help/diskcopy.hlp	$(MASTER)/help/diskcopy.hlp
	../u2d ../help/echo.hlp		$(MASTER)/help/echo.hlp
	../u2d ../help/era.hlp		$(MASTER)/help/era.hlp
	../u2d ../help/erase.hlp	$(MASTER)/help/erase.hlp
	../u2d ../help/exit.hlp		$(MASTER)/help/exit.hlp
	../u2d ../help/fixdisk.hlp	$(MASTER)/help/fixdisk.hlp
	../u2d ../help/format.hlp	$(MASTER)/help/format.hlp
	../u2d ../help/help.hlp		$(MASTER)/help/help.hlp
	../u2d ../help/kmode.hlp	$(MASTER)/help/kmode.hlp
	../u2d ../help/md.hlp		$(MASTER)/help/md.hlp
	../u2d ../help/mkdir.hlp	$(MASTER)/help/mkdir.hlp
	../u2d ../help/mode.hlp		$(MASTER)/help/mode.hlp
	../u2d ../help/move.hlp		$(MASTER)/help/move.hlp
	../u2d ../help/mvdir.hlp	$(MASTER)/help/mvdir.hlp
	../u2d ../help/path.hlp		$(MASTER)/help/path.hlp
	../u2d ../help/pause.hlp	$(MASTER)/help/pause.hlp
	../u2d ../help/ramdisk.hlp	$(MASTER)/help/ramdisk.hlp
	../u2d ../help/rd.hlp		$(MASTER)/help/rd.hlp
	../u2d ../help/rem.hlp		$(MASTER)/help/rem.hlp
	../u2d ../help/ren.hlp		$(MASTER)/help/ren.hlp
	../u2d ../help/rename.hlp	$(MASTER)/help/rename.hlp
	../u2d ../help/rmdir.hlp	$(MASTER)/help/rmdir.hlp
	../u2d ../help/rndir.hlp	$(MASTER)/help/rndir.hlp
	../u2d ../help/set.hlp		$(MASTER)/help/set.hlp
	../u2d ../help/time.hlp		$(MASTER)/help/time.hlp
	../u2d ../help/type.hlp		$(MASTER)/help/type.hlp
	../u2d ../help/undel.hlp	$(MASTER)/help/undel.hlp
	../u2d ../help/ver.hlp		$(MASTER)/help/ver.hlp
	../u2d ../help/verify.hlp	$(MASTER)/help/verify.hlp
	../u2d ../help/vol.hlp		$(MASTER)/help/vol.hlp
	../u2d ../help/xcopy.hlp	$(MASTER)/help/xcopy.hlp
	../u2d ../help/xdir.hlp		$(MASTER)/help/xdir.hlp
	../u2d ../help/batch.hlp	$(MASTER)/help/batch.hlp
	../u2d ../help/editing.hlp	$(MASTER)/help/editing.hlp
	../u2d ../help/env.hlp		$(MASTER)/help/env.hlp
	../u2d ../help/errors.hlp	$(MASTER)/help/errors.hlp
	../u2d ../help/io.hlp		$(MASTER)/help/io.hlp
	../u2d ../help/syntax.hlp	$(MASTER)/help/syntax.hlp
	mkdir $(MASTER)/khelp
	../u2d ../khelp/assign.hlp	$(MASTER)/khelp/assign.hlp
	../u2d ../khelp/atdir.hlp	$(MASTER)/khelp/atdir.hlp
	../u2d ../khelp/attrib.hlp	$(MASTER)/khelp/attrib.hlp
	../u2d ../khelp/basic.hlp	$(MASTER)/khelp/basic.hlp
	../u2d ../khelp/buffers.hlp	$(MASTER)/khelp/buffers.hlp
	../u2d ../khelp/chdir.hlp	$(MASTER)/khelp/cd.hlp
	../u2d ../khelp/chdir.hlp	$(MASTER)/khelp/chdir.hlp
	../u2d ../khelp/chkdsk.hlp	$(MASTER)/khelp/chkdsk.hlp
	../u2d ../khelp/cls.hlp		$(MASTER)/khelp/cls.hlp
	../u2d ../khelp/command2.hlp	$(MASTER)/khelp/command2.hlp
	../u2d ../khelp/concat.hlp	$(MASTER)/khelp/concat.hlp
	../u2d ../khelp/copy.hlp	$(MASTER)/khelp/copy.hlp
	../u2d ../khelp/date.hlp	$(MASTER)/khelp/date.hlp
	../u2d ../khelp/erase.hlp	$(MASTER)/khelp/del.hlp
	../u2d ../khelp/dir.hlp		$(MASTER)/khelp/dir.hlp
	../u2d ../khelp/diskcopy.hlp	$(MASTER)/khelp/diskcopy.hlp
	../u2d ../khelp/echo.hlp	$(MASTER)/khelp/echo.hlp
	../u2d ../khelp/erase.hlp	$(MASTER)/khelp/era.hlp
	../u2d ../khelp/erase.hlp	$(MASTER)/khelp/erase.hlp
	../u2d ../khelp/exit.hlp	$(MASTER)/khelp/exit.hlp
	../u2d ../khelp/fixdisk.hlp	$(MASTER)/khelp/fixdisk.hlp
	../u2d ../khelp/format.hlp	$(MASTER)/khelp/format.hlp
	../u2d ../khelp/help.hlp	$(MASTER)/khelp/help.hlp
	../u2d ../khelp/kmode.hlp	$(MASTER)/khelp/kmode.hlp
	../u2d ../khelp/mkdir.hlp	$(MASTER)/khelp/md.hlp
	../u2d ../khelp/mkdir.hlp	$(MASTER)/khelp/mkdir.hlp
	../u2d ../khelp/mode.hlp	$(MASTER)/khelp/mode.hlp
	../u2d ../khelp/move.hlp	$(MASTER)/khelp/move.hlp
	../u2d ../khelp/mvdir.hlp	$(MASTER)/khelp/mvdir.hlp
	../u2d ../khelp/path.hlp	$(MASTER)/khelp/path.hlp
	../u2d ../khelp/pause.hlp	$(MASTER)/khelp/pause.hlp
	../u2d ../khelp/ramdisk.hlp	$(MASTER)/khelp/ramdisk.hlp
	../u2d ../khelp/rmdir.hlp	$(MASTER)/khelp/rd.hlp
	../u2d ../khelp/rem.hlp		$(MASTER)/khelp/rem.hlp
	../u2d ../khelp/rename.hlp	$(MASTER)/khelp/ren.hlp
	../u2d ../khelp/rename.hlp	$(MASTER)/khelp/rename.hlp
	../u2d ../khelp/rmdir.hlp	$(MASTER)/khelp/rmdir.hlp
	../u2d ../khelp/rndir.hlp	$(MASTER)/khelp/rndir.hlp
	../u2d ../khelp/set.hlp		$(MASTER)/khelp/set.hlp
	../u2d ../khelp/time.hlp	$(MASTER)/khelp/time.hlp
	../u2d ../khelp/type.hlp	$(MASTER)/khelp/type.hlp
	../u2d ../khelp/undel.hlp	$(MASTER)/khelp/undel.hlp
	../u2d ../khelp/ver.hlp		$(MASTER)/khelp/ver.hlp
	../u2d ../khelp/verify.hlp	$(MASTER)/khelp/verify.hlp
	../u2d ../khelp/vol.hlp		$(MASTER)/khelp/vol.hlp
	../u2d ../khelp/xcopy.hlp	$(MASTER)/khelp/xcopy.hlp
	../u2d ../khelp/xdir.hlp	$(MASTER)/khelp/xdir.hlp
	../u2d ../khelp/batch.hlp	$(MASTER)/khelp/batch.hlp
	../u2d ../khelp/editing.hlp	$(MASTER)/khelp/editing.hlp
	../u2d ../khelp/env.hlp		$(MASTER)/khelp/env.hlp
	../u2d ../khelp/errors.hlp	$(MASTER)/khelp/errors.hlp
	../u2d ../khelp/io.hlp		$(MASTER)/khelp/io.hlp
	../u2d ../khelp/syntax.hlp	$(MASTER)/khelp/syntax.hlp
	cd $(MASTER); tar cf ../$(MASTER).tar .
	rm -r $(MASTER)
