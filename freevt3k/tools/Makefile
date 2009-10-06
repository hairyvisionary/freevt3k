#
# Makefile for tcpreader
#

APPSRCS  = tcpreader.c
APPOBJS  = tcpreader.o
HEADERS =  ../vt.h ../typedef.h

CC=cc
SFX=
COPTS= 
LIBS=

CFLAGS= $(COPTS)
SOURCE= Makefile $(APPSRCS) $(HEADERS) 

dflt:
	@echo "Do one of:"
	@echo "  make aix"
	@echo "  make aix3"
	@echo "  make alphaosf"
	@echo "  make freebsd"
#	@echo "  make hpux_ansi"
#	@echo "  make hpux_posix"
	@echo "  make hpux10xpa11 (HP-UX 10.X PARISC 1.1)"
	@echo "  make hpux9xpa11 (HP-UX 9.X PARISC 1.1)"
	@echo "  make hpux9xm68k (HP-UX 9.X Motorola 68k)"
	@echo "  make irix"
	@echo "  make linux"
	@echo "  make ncr"
	@echo "  make sco"
	@echo "  make solaris"
	@echo "  make sunos"

all: tcpreader$(SFX)

tcpreader$(SFX): $(APPSRCS) $(HEADERS) Makefile
	$(CC) $(CFLAGS) tcpreader.c -o tcpreader$(SFX) $(LIBS)

clean.o:
	rm -f $(APPOBJS)

clean: clean.o
	rm -f tcpreader$(SFX)

aix: aix4

aix4:
	make -f Makefile CC=xlc COPTS="-DAIX -I../AIX/4" SFX=.aix4 all

aix3:
	make -f Makefile CC=cc COPTS="-DAIX -I../AIX/3" SFX=.aix3 all

alphaosf:
	make -f Makefile CC=cc COPTS="-std -I../AlphaOSF/4" SFX=.alphaosf all

freebsd:
	make -f Makefile CC=gcc COPTS="-I../BSDI" SFX=.freebsd all

hpux_ansi:
	make -f Makefile CC=cc COPTS="-Aa" all

hpux_posix:
	make -f Makefile CC=cc COPTS="-Ae" all

hpux10xpa11:
	make -f Makefile CC=cc COPTS="-Aa -DHPUX -I../HP-UX/10.X/PA1.1" SFX=.hpux10xpa11 all

hpux9xpa11:
	make -f Makefile CC=cc COPTS="-Aa -DHPUX -I../HP-UX/9.X/PA1.1" SFX=.hpux9xpa11 all

hpux9xm68k:
	make -f Makefile CC=cc COPTS="-Aa -DHPUX -I../HP-UX/9.X/S300" SFX=.hpux9xm68k all

irix:
	make -f Makefile CC=cc COPTS="-I../SGI" SFX=.irix all

linux:
	make -f Makefile CC=gcc COPTS="-I../Linux" SFX=.linux all

ncr:
	make -f Makefile CC=cc COPTS="-Xa -Hnocopyr -I../NCR" LIBS="-lnsl -lsocket -lnet" SFX=.ncr all

sco:
	make -f Makefile CC=cc COPTS="-Xa -a xpg4plus -w3 -DSCO -I../SCO/5" LIBS="-lintl -lsocket -lnsl" SFX=.sco all

solaris:
	make -f Makefile CC=cc COPTS="-Xa -I../Solaris" LIBS="-lsocket -lnsl" SFX=.solaris all

sunos:
	make -f Makefile CC=acc COPTS="-sys5 -Xa -Bstatic -I../SunOS" LIBS="-lnsl" SFX=.sunos all

#>>>>>>>>>>>>>>>>>
REBUILD=/usr/telamon/randy/freevt3k/tools/makeit.sh

aix4.rebuild:
	remsh rs6000 -l randy -n "/hp807s$(REBUILD) aix4 x"

alphaosf.rebuild:
	remsh alphaosf -l randy -n "/hp807s$(REBUILD) alphaosf"

aux.rebuild:
	remsh mac -l randy -n "/hp807s$(REBUILD) aux"

hpux9xm68k.rebuild:
	remsh hp425t -l randy -n "/hp807s$(REBUILD) hpux9xm68k x"

hpux10xpa11.rebuild:
	remsh hp71260 -l randy -n "/hp807s$(REBUILD) hpux10xpa11 x"

hpux9xpa11.rebuild:
	$(REBUILD) hpux9xpa11 x"

irix.rebuild:
	remsh iris -l randy -n "/hp807s$(REBUILD) irix x"

sunos.rebuild:
	remsh sunos -l randy -n "/hp807s$(REBUILD) sunos x"

solaris.rebuild:
	remsh sparc -l randy -n "/hp807s$(REBUILD) solaris x"

sco.rebuild:
	remsh scosysv -l randy -n "/hp807s$(REBUILD) sco"

ncr.rebuild:
	remsh ncr3350 -l randy -n "/hp807s$(REBUILD) ncr"

linux.rebuild:
	remsh linux -l randy -n "/hp807s$(REBUILD) linux x"

rebuild: aix4.rebuild alphaosf.rebuild hpux9xm68k.rebuild \
	 hpux10xpa11.rebuild sunos.rebuild solaris.rebuild sco.rebuild \
	 ncr.rebuild linux.rebuild hpux9xpa11.rebuild irix.rebuild

#<<<<<<<<<<<<<<<<<
