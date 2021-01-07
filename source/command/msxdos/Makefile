# Makefile for NEXTOR.SYS

# The generated NEXTOR.SYS and NEXTOR.SYS.japanese are copied to the bin/tools directory.

export X80_COMMAND_LINE=-t -nb
export M80_COMMAND_LINE=-8

define hex2bin
    objcopy -I ihex -O binary $(1) $(2)
endef

define copy_to_bin
	cp $(1) ../../../bin/tools/$(2)
endef

nextor-sys: NEXTOR.SYS NEXTORK.SYS

.phony: prerequisites

TOOLS := M80 L80 objcopy

prerequisites:
	@mkdir -p ../../../bin/tools
	$(foreach exec,$(TOOLS),\
		$(if $(shell which $(exec)),,$(error "ERROR: can't execute $(exec), is it installed/in PATH?")))

-include prerequisites

INCS := ../../kernel/macros.inc ../../kernel/const.inc ../../kernel/condasm.inc
RELS := KMSG.REL MESSAGES.REL REAL.REL REF.REL RELOC.REL VER.REL END.REL

NEXTOR.SYS: \
	$(RELS) \
	CODES.REL \
	DATA.REL \
	sys.mac \
	kmsg.mac \
	nokmsg.mac

	cp nokmsg.mac USEKMSG.MAC
	M80 -p ../../kernel =SYS
	L80 /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,END,NEXTOR/n/x/y/e
	$(call hex2bin,NEXTOR.HEX,NEXTOR.SYS)
	$(call copy_to_bin,NEXTOR.SYS)

NEXTORK.SYS: \
	$(RELS) \
	CODES.REL \
	DATA.REL \
	KMSG.REL \
	sys.mac \
	yeskmsg.mac

	cp yeskmsg.mac USEKMSG.MAC
	M80 -p ../../kernel =SYS
	L80 /P:100,CODES,DATA,RELOC,VER,REF,SYS,REAL,SYS,MESSAGES,KMSG,END,NEXTORK/n/x/y/e
	$(call hex2bin,NEXTORK.HEX,NEXTORK.SYS)
	$(call copy_to_bin,NEXTORK.SYS,NEXTOR.SYS.japanese)

lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,\
	$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,\
	$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

.SECONDEXPANSION:
$(RELS): \
	$$(patsubst %.rel,%.mac,$$(call lc, $$@)) \
	$(INCS)

	M80 =$(notdir $(patsubst %.REL,%,$@))

CODES.REL: \
	../../kernel/codes.mac \
	$(INCS)

	M80 -p ../../kernel =CODES

DATA.REL: \
	../../kernel/data.mac \
	$(INCS)

	M80 -p ../../kernel =DATA

.PHONY: clean

clean:
	for ext in BIN HEX REL SYM; do find . -type f -name "*.$$ext" -delete; done
	rm -f USEKMSG.MAC
