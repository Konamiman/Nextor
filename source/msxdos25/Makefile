.SUFFIXES: .mac .rel .hex .sym

.mac.rel:
	cpm32 m80 =$*.mac

#########################################
#	Top Level Makefile.		#
#########################################
COMINC=	macros.inc const.inc condasm.inc
COMREL=	codes.rel kvar.rel data.rel

all:	pana megasd ocmsx msxpl

clean:
	rm -f *.REL *.BAK
	rm -f bank0/*.REL bank0/*.BAK
	rm -f bank1/*.REL bank1/*.BAK
	rm -f bank2/*.REL bank2/*.BAK
	@cd megasdrom; $(MAKE) clean
	@cd ocmsxrom; $(MAKE) clean
	@cd mplayerrom; $(MAKE) clean

prod:	banks		;@cd prodrom; $(MAKE)
test:	banks		;@cd testrom; $(MAKE)
pana:	banks		;@cd panarom; $(MAKE)
megasd:	banks		;@cd megasdrom; $(MAKE)
ocmsx:	banks		;@cd ocmsxrom; $(MAKE)
msxpl:	banks		;@cd mplayerrom; $(MAKE)
files:	codes.rel	;@cd ../sys; $(MAKE)

banks:	$(COMREL) rel.rel
#	@cd bank0;	APPEND=:.. $(MAKE) -f ../Makefile bank0
#	@cd bank1;	APPEND=:.. $(MAKE) -f ../Makefile bank1
#	@cd bank2;	APPEND=:.. $(MAKE) -f ../Makefile bank2
#	@cd bank3;	APPEND=:.. $(MAKE) -f ../Makefile bank3
##	@cd bankk;	APPEND=:.. $(MAKE) -f ../Makefile bankk
	@cd bank0; cp ../*.inc .; $(MAKE) -f ../Makefile bank0
	@cd bank1; cp ../*.inc .; $(MAKE) -f ../Makefile bank1
	@cd bank2; cp ../*.inc .; $(MAKE) -f ../Makefile bank2
	@cd bank3; cp ../*.inc .; $(MAKE) -f ../Makefile bank3
#	@cd bankk; cp ../*.inc .; $(MAKE) -f ../Makefile bankk

$(COMREL): $(COMINC)

#########################################
#	Release date			#
#########################################
release:
	./reldate >rel.mac

#########################################
#	Common to all Banks		#
#########################################
INCS=	../macros.inc ../const.inc ../condasm.inc
RELS=	../codes.rel ../kvar.rel ../data.rel

#########################################
#	Ruleset for BANK0.		#
#########################################
bank0:	dos2kit.rel

B0RELS=	init.rel alloc.rel dskbasic.rel dosboot.rel bdos.rel ramdrv.rel

b0.rel: ../bank.inc
init.rel: ../bank.inc
$(B0RELS): $(INCS)

dos2kit.rel: $(RELS) ../rel.rel doshead.rel 40ff.rel b0.rel $(B0RELS)
	cp ../codes.rel .
	cp ../kvar.rel .
	cp ../data.rel .
	cp ../rel.rel .
	cpm32 LIB80 DOS2KIT=CODES,KVAR,DATA,REL,DOSHEAD,40FF,B0,INIT,ALLOC,DSKBASIC,DOSBOOT,BDOS,RAMDRV/E

#########################################
#	Ruleset for BANK1.		#
#########################################
bank1: b1.hex

B1REL1=	dosinit.rel mapinit.rel
B1REL2=	msg.rel
B1RELS=	$(B1REL1) $(B1REL2)

b1.rel: ../bank.inc
$(B1RELS): $(INCS)

b1.hex: $(RELS) b1.rel $(B1REL1) ../bank0/alloc.rel $(B1REL2)
	cp ../codes.rel .
	cp ../kvar.rel .
	cp ../data.rel .
	cp ../bank0/alloc.rel .
	cpm32 L80 /P:40FF,CODES,KVAR,DATA,B1,DOSINIT,MAPINIT,ALLOC,MSG,B1/X/Y/N/E

#########################################
#	Ruleset for BANK2.		#
#########################################
bank2:	b2.hex

B2RELS=	kinit.rel \
	char.rel dev.rel kbios.rel misc.rel env.rel time.rel seg.rel \
	path.rel find.rel dir.rel handles.rel del.rel rw.rel files.rel \
	buf.rel fat.rel val.rel err.rel cpm.rel
B21RELSC=	char.rel,dev.rel,kbios.rel,misc.rel,env.rel,time.rel,seg.rel
B22RELSC=	path.rel,find.rel,dir.rel,handles.rel,del.rel,rw.rel,files.rel
B23RELSC=	buf.rel,fat.rel,val.rel,err.rel,cpm.rel

b2.rel:	../bank.inc
$(B2RELS): $(INCS)

b2.hex: $(RELS) b2.rel $(B2RELS)
	cp ../codes.rel .
	cp ../kvar.rel .
	cp ../data.rel .
	cpm32 LIB80 TEMP21=$(B21RELSC)/E
	cpm32 LIB80 TEMP22=$(B22RELSC)/E
	cpm32 LIB80 TEMP23=$(B23RELSC)/E
	cpm32 L80 /P:40FF,CODES,KVAR,DATA,B2,KINIT,TEMP21,TEMP22,TEMP23,B2/X/Y/N/E
	rm TEMP21.REL
	rm TEMP22.REL
	rm TEMP23.REL

#########################################
#	Ruleset for BANK3.		#
#########################################
bank3:	dos1kit.rel

B3RELS=	

#dos1kit.rel:	$(B3RELS)
#	cpm32 lib80 dos1kit=/e

#########################################
DOSKIT=	DOSKIT
doskit:
	rm -rf $(DOSKIT); mkdir $(DOSKIT)
	cp -p bank0/dos2kit.rel		$(DOSKIT)/dos2kit.rel
	../u2d bank1/b1.hex		$(DOSKIT)/b1.hex
	../u2d bank2/b2.hex		$(DOSKIT)/b2.hex
	cp -p bank3/dos1kit.rel 	$(DOSKIT)/dos1kit.rel
	../u2d kitsrc/makebin.bat	$(DOSKIT)/makebin.bat
	cd $(DOSKIT); tar cf ../$(DOSKIT).tar .
	rm -rf $(DOSKIT)
