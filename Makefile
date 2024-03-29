#
# Makefile for freevt3k
#

APPSRCS  = freevt3k.c vtconn.c error.c
APPOBJS  = freevt3k.o vtconn.o error.o
HEADERS =  vt.h typedef.h freevt3k.h vtconn.h

CC=cc
SFX=
DEBUG_FLAGS=
COMPILE_FLAGS=
COPTS= 
#LIBS=-lcurses
LIBS=

CFLAGS= $(COPTS) $(DEBUG_FLAGS) $(COMPILE_FLAGS)
SOURCE= Makefile $(APPSRCS) $(HEADERS) 

dflt:
	@echo "Do one of:"
	@echo "  make aix"
	@echo "  make aix3"
	@echo "  make aux"
	@echo "  make alphaosf"
	@echo "  make clean"
	@echo "  make freebsd"
#	@echo "  make hpux_ansi"
#	@echo "  make hpux_posix"
	@echo "  make hpux11pa11 (HP-UX 11.X PARISC 1.1)"
	@echo "  make hpux11pa20 (HP-UX 11.X PARISC 2.0)"
	@echo "  make hpux11pa20w (HP-UX 11.X PARISC 2.0 64-bit)"
	@echo "  make hpux10pa11 (HP-UX 10.X PARISC 1.1)"
	@echo "  make hpux10pa20 (HP-UX 10.X PARISC 2.0)"
	@echo "  make hpux9pa10 (HP-UX 9.X PARISC 1.0)"
	@echo "  make hpux9pa11 (HP-UX 9.X PARISC 1.1)"
	@echo "  make hpux9m68k (HP-UX 9.X Motorola 68k)"
	@echo "  make irix"
	@echo "  make linux"
	@echo "  make ncr"
	@echo "  make sco"
	@echo "  make sequent"
	@echo "  make solaris"
	@echo "  make sunos"
	@echo "  make tatung"
	@echo "  make generic (if all else fails)"

all: freevt3k$(SFX)

freevt3k$(SFX): $(APPOBJS)
	$(CC) $(CFLAGS) $(APPOBJS) -o freevt3k$(SFX) $(LIBS)

freevt3k.o: freevt3k.c hpvt100.c $(HEADERS)
	$(CC) $(CFLAGS) -c freevt3k.c

vtconn.o: vtconn.c dumpbuf.c $(HEADERS)
	$(CC) $(CFLAGS) -c vtconn.c

error.o: error.c
	$(CC) $(CFLAGS) -c error.c

clean.o:
	rm -f $(APPOBJS)

clean: clean.o
	rm -f freevt3k$(SFX)

generic:
	make -f Makefile CC=cc COPTS="-DGENERIC" LIBS="" all

nt:
	make -f Makefile CC=cc COPTS="-DWINNT -O -W/W4" LIBS="-lsocket" SFX=.exe all

aix: aix4

aix4:
	make -f Makefile CC=xlc COPTS="-DAIX -IAIX/4" SFX=.aix4 all

aix3:
	make -f Makefile CC=cc COPTS="-DAIX -IAIX/3" SFX=.aix3 all

aux:
	make -f Makefile CC=cc COPTS="-DAUX -IAUX" SFX=.aux all

alphaosf:
	make -f Makefile CC=cc COPTS="-std -IAlphaOSF/4" SFX=.alphaosf all

freebsd:
	make -f Makefile CC=cc COPTS="-IBSDI" SFX=.freebsd all

macosx:
	make -f Makefile CC=cc COPTS="-IMacOSX" SFX=.macosx all

hpux11pa11:
	make -f Makefile CC=c89 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DD32 -DHPUX -IHP-UX/11.X/PA1.1" SFX=.hpux11pa11 all

hpux11pa20:
	make -f Makefile CC=c89 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA2.0N -DHPUX -IHP-UX/11.X/PA2.0" SFX=.hpux11pa20 all

hpux11pa20w:
	make -f Makefile CC=c89 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA2.0W -DHPUX -IHP-UX/11.X/PA2.0W" SFX=.hpux11pa20w all

hpux10pa11:
	make -f Makefile CC=c89 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA1.1 +DS1.1 -DHPUX -IHP-UX/10.X/PA1.1" SFX=.hpux10pa11 all

hpux10pa20:
	make -f Makefile CC=c89 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA2.0 +DS2.0 -DHPUX -IHP-UX/10.X/PA2.0" SFX=.hpux10pa20 all

hpux9pa10:
	make -f Makefile CC=c89 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA1.0 +DS1.0 -DHPUX -IHP-UX/9.X/PA1.0" SFX=.hpux9pa10 all

hpux9pa11:
	make -f Makefile CC=c89 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA1.1 +DS1.1 -DHPUX -IHP-UX/9.X/PA1.1" SFX=.hpux9pa11 all

hpux9m68k:
	make -f Makefile CC=c89 COPTS="-O -DHPUX -IHP-UX/9.X/S300" SFX=.hpux9m68k all

hpux_ansi:
	make -f Makefile CC=cc COPTS="-Aa -DHPUX -IHP-UX"  all

hpux_posix:
	make -f Makefile CC=cc COPTS="-Ae -DHPUX -IHP-UX"  all

irix:
	make -f Makefile CC=cc COPTS="-ISGI" SFX=.irix all

linux20:
	make -f Makefile CC=i386-glibc20-linux-gcc COPTS="-ILinux" SFX=.linux20 all

linux21:
	make -f Makefile CC=gcc COPTS="-ILinux" SFX=.linux21 all

ncr:
	make -f Makefile CC=cc COPTS="-Xa -Hnocopyr -INCR" LIBS="-lnsl -lsocket -lnet" SFX=.ncr all

sequent:
	make -f Makefile CC=cc COPTS="-Xa -O -W0,-xstring -DSHORT_GETTIMEOFDAY -ISequent" LIBS="-lnsl -lsocket" SFX=.sequent all

sco:
	make -f Makefile CC=cc COPTS="-Xa -a xpg4plus -w3 -DSCO -ISCO/5" LIBS="-lintl -lsocket -lnsl" SFX=.sco all

solaris:
	make -f Makefile CC=cc COPTS="-Xa -ISolaris" LIBS="-lsocket -lnsl" SFX=.solaris all

sunos:
	make -f Makefile CC=acc COPTS="-sys5 -Xa -Bstatic -ISunOS" LIBS="-lnsl" SFX=.sunos all

tatung:
	make -f Makefile CC=gcc SFX=.tatung all
