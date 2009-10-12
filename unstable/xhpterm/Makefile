#
# Makefile for xhpterm
#

APPSRCS  = ../freevt3k.c ../vtconn.c ../error.c vt3kglue.c x11glue.c hpterm.c conmgr.c tty.c rlogin.c getcolor.c
APPOBJS = freevt3k.o ../vtconn.o ../error.o vt3kglue.o x11glue.o hpterm.o conmgr.o tty.o rlogin.o getcolor.o
HEADERS =  ../vt.h ../typedef.h ../freevt3k.h ../vtconn.h ../error.h ../dumpbuf.h hpterm.h conmgr.h terminal.bm

CC=cc
SFX=
DEBUG_FLAGS=
COMPILE_FLAGS=
COPTS= 
LIBS=
CDEFINES=-DSHOW_DC1_COUNT=0 -DDEBUG_BREAK=0 -DIGNORE_KEYBOARD_LOCK=1 -DIGNORE_USER_SYSTEM_LOCK=1 -DDEBUG_BLOCK_MODE=0 -DMEMLOCK_2000

CFLAGS= $(COPTS) $(DEBUG_FLAGS) $(COMPILE_FLAGS) $(CDEFINES)
SOURCE=Makefile $(APPSRCS) $(HEADERS) 

dflt:
	@echo "Do one of:"
	@echo "  make aix"
	@echo "  make aix3 (untested)"
	@echo "  make alphaosf (untested)"
	@echo "  make clean"
	@echo "  make freebsd (untested)"
#	@echo "  make hpux_ansi"
#	@echo "  make hpux_posix"
	@echo "  make hpux11pa11 (HP-UX 11.X PARISC 1.1)"
	@echo "  make hpux11pa20 (HP-UX 11.X PARISC 2.0)"
	@echo "  make hpux11pa20w (HP-UX 10.X PARISC 2.0 64-bit)"
	@echo "  make hpux10pa11 (HP-UX 10.X PARISC 1.1)"
	@echo "  make hpux10pa20 (HP-UX 10.X PARISC 2.0)"
	@echo "  make hpux9pa11 (HP-UX 9.X PARISC 1.1)"
	@echo "  make hpux9m68k (HP-UX 9.X Motorola 68k)"
	@echo "  make irix (untested)"
	@echo "  make linux"
	@echo "  make macosx (MacOSX 10.3)"
	@echo "  make ncr (untested)"
	@echo "  make sco"
	@echo "  make solaris"
	@echo "  make sequent (untested)"
	@echo "  make sunos"
	@echo "  make tatung"

rebuild: clean all

all: xhpterm$(SFX)

xhpterm$(SFX): $(APPOBJS)
	$(CC) $(CFLAGS) $(APPOBJS) -o xhpterm$(SFX) $(LIBS)

freevt3k.o: ../freevt3k.c ../timers.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -DXHPTERM -c ../freevt3k.c

../vtconn.o: ../vtconn.c ../dumpbuf.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c ../vtconn.c -o ../vtconn.o

../error.o: ../error.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c ../error.c -o ../error.o

vt3kglue.o: vt3kglue.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c vt3kglue.c

x11glue.o: x11glue.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c x11glue.c

hpterm.o: hpterm.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c hpterm.c

conmgr.o: conmgr.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c conmgr.c

tty.o: tty.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c tty.c

rlogin.o: rlogin.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c rlogin.c

getcolor.o: getcolor.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c getcolor.c

clean.o:
	rm -f $(APPOBJS)

clean: clean.o
	rm -f xhpterm$(SFX)

aix: aix4

aix4:
	make -f Makefile CC=xlc COPTS="-DAIX -I../ -I../AIX/4" SFX=.aix4 LIBS="-L/usr/lib/X11R5 -lX11" all

aix3:
#	make -f Makefile CC=cc COPTS="-DAIX -I../ -I../AIX/3 -I/users/telalert/Motif/include" SFX=.aix3 LIBS="-L/usr/lib/X11R5 -lX11" all
	make -f Makefile CC=cc COPTS="-DAIX -I../ -I../AIX/3" SFX=.aix3 LIBS="-L/usr/lib/X11R5 -lX11" all

alphaosf:
	make -f Makefile CC=cc COPTS="-std -I../ -I../AlphaOSF/4" LIBS="-L/usr/lib/X11 -lX11" SFX=.alphaosf all

freebsd:
	make -f Makefile CC=gcc COPTS="-I../ -I../BSDI" SFX=.freebsd all

macosx:
	make -f Makefile CC=cc COPTS="-I../ -I../MacOSX -I/usr/include/X11 -I/usr/include/X11/extensions" LIBS="-L/usr/X11R6/lib -lX11" SFX=.macosx all

hpux11pa11:
	make -f Makefile CC=c89 SFX=.hpux11pa11 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DD32 -I/usr/include/X11R5 -DHPUX -I../ -I../HP-UX/11.X/PA1.1" LIBS="-L/usr/lib/X11R5 -lX11" all

hpux11pa20:
	make -f Makefile CC=c89 SFX=.hpux11pa20 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA2.0N -I/usr/include/X11R5 -DHPUX -I../ -I../HP-UX/11.X/PA2.0" LIBS="-L/usr/lib/X11R5 -lX11" all

hpux11pa20w:
	make -f Makefile CC=c89 SFX=.hpux11pa20w COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA2.0W -I/usr/include/X11R5 -DHPUX -I../ -I../HP-UX/11.X/PA2.0W" LIBS="-L/usr/lib/X11R5 -lX11" all

hpux10pa20:
	make -f Makefile CC=c89 SFX=.hpux10pa20 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA2.0 +DS2.0 -I/usr/include/X11R5 -DHPUX -I../ -I../HP-UX/10.X/PA2.0" LIBS="-L/usr/lib/X11R5 -lX11" all

hpux10pa11:
	make -f Makefile CC=c89 SFX=.hpux10pa11 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA1.1 +DS1.1 -I/usr/include/X11R5 -DHPUX -I../ -I../HP-UX/10.X/PA1.1" LIBS="-L/usr/lib/X11R5 -lX11" all

hpux9pa10:
	make -f Makefile CC=c89 SFX=.hpux9pa10 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA1.0 +DS1.0 -I/usr/include/X11R5 -DHPUX -I../ -I../HP-UX/9.X/PA1.0" LIBS="-L/usr/lib/X11R5 -lX11" all

hpux9pa11:
	make -f Makefile CC=c89 SFX=.hpux9pa11 COPTS="-Wc,-w1 +e -z -O +Onolimit +ESlit +DA1.1 +DS1.1 -I/usr/include/X11R5 -DHPUX -I../ -I../HP-UX/9.X/PA1.1" LIBS="-L/usr/lib/X11R5 -lX11" all

hpux9m68k:
	make -f Makefile CC=c89 SFX=.hpux9m68k COPTS="-O -I/usr/include/X11R5 -DHPUX -I../ -I../HP-UX/9.X/S300" LIBS="-L/usr/lib/X11R5 -lX11" all

hpux_ansi:
	make -f Makefile CC=cc SFX=.hpux_ansi COPTS="-Aa -I/usr/include/X11R5 -DHPUX -I../ -I../HP-UX" LIBS="-L/usr/lib/X11R5 -lX11" all

hpux_posix:
	make -f Makefile CC=cc SFX=.hpux_posix COPTS="-Ae -I/usr/include/X11R5 -DHPUX -I../ -I../HP-UX" LIBS="-L/usr/lib/X11R5 -lX11" all
#	make -f Makefile CC=cc COPTS="-Ae -DHPUX" all

irix:
	make -f Makefile CC=cc SFX=.irix COPTS="-I../ -I../SGI" LIBS="-L/usr/lib/X11R5 -lX11" all

linux20:
	make -f Makefile CC=i386-glibc20-linux-gcc SFX=.linux20 COPTS="-I../ -I../Linux" LIBS="-L/usr/i386-glibc20-linux/lib -lg -L/usr/X11R6/lib -lX11" all

linux:
	make -f Makefile CC=gcc COPTS="-I../ -I../Linux" LIBS="-L/usr/X11R6/lib -lX11" all

ncr:
	make -f Makefile CC=cc COPTS="-Xa -Hnocopyr -I../ -I../NCR" LIBS="-lnsl -lsocket -lnet" SFX=.ncr all

sequent:
	make -f Makefile CC=cc COPTS="-Xa -O -W0,-xstring -DSHORT_GETTIMEOFDAY -I../ -I../Sequent" LIBS="-lX11 -lnsl -lsocket" SFX=.sequent all

sco:
	make -f Makefile CC=cc COPTS="-Xa -a xpg4plus -w3 -DSCO -I../ -I../SCO/5" LIBS="-lX11 -lintl -lsocket -lnsl" SFX=.sco all

solaris:
	make -f Makefile CC=cc COPTS="-Xa -I/usr/openwin/share/include -I../ -I../Solaris" LIBS="-lsocket -lnsl -L/usr/openwin/lib -lX11" SFX=.solaris all

sunos:
	make -f Makefile CC=acc COPTS="-I. -I/usr/openwin/share/include -sys5 -Xa -Bstatic -I../ -I../SunOS" LIBS="-lnsl -L/usr/openwin/lib -lX11" SFX=.sunos all

tatung:
	make -f Makefile CC=gcc COPTS="-DTATUNG" LIBS="-lX11" SFX=.tatung all

