#
# Makefile for xhpterm
#

APPSRCS  = ../freevt3k.c ../vtconn.c vt3kglue.c x11glue.c hpterm.c conmgr.c tty.c rlogin.c getcolor.c
APPOBJS = freevt3k.o ../vtconn.o vt3kglue.o x11glue.o hpterm.o conmgr.o tty.o rlogin.o getcolor.o
HEADERS =  ../vt.h ../typedef.h ../freevt3k.h ../vtconn.h ../dumpbuf.h hpterm.h conmgr.h terminal.bm

CC=cc
SFX=
DEBUG_FLAGS=
COMPILE_FLAGS=
COPTS= 
LIBS=

CFLAGS= $(COPTS) $(DEBUG_FLAGS) $(COMPILE_FLAGS)
SOURCE= Makefile $(APPSRCS) $(HEADERS) 

dflt:
	@echo "Do one of:"
	@echo "  make aix"
	@echo "  make alphaosf (untested)"
	@echo "  make clean"
	@echo "  make freebsd (untested)"
	@echo "  make hpux_ansi"
	@echo "  make hpux_posix (untested)"
	@echo "  make irix (untested)"
	@echo "  make linux"
	@echo "  make ncr (untested)"
	@echo "  make sco"
	@echo "  make solaris"
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

aix:
	make -f Makefile CC=xlc SFX=.aix LIBS="-L /usr/lib/X11R5 -lX11" all

alphaosf:
	make -f Makefile CC=cc COPTS="-std" SFX=.alphaosf all

freebsd:
	make -f Makefile CC=gcc SFX=.freebsd all

hpux_ansi:
	make -f Makefile CC=cc COPTS="-Aa -I /usr/include/X11R5" LIBS="-L /usr/lib/X11R5 -lX11" all

hpux_posix:
	make -f Makefile CC=cc COPTS="-Ae" all

irix:
	make -f Makefile CC=cc SFX=.irix all

linux:
	make -f Makefile CC=gcc SFX=.linux LIBS="-L /usr/X11/lib -lX11" all

ncr:
	make -f Makefile CC=cc COPTS="-Xa -Hnocopyr" LIBS="-lnsl -lsocket -lnet" SFX=.ncr all

sco:
	make -f Makefile CC=cc COPTS="-Xa -a xpg4plus -w3" LIBS="-lX11 -lintl -lsocket -lnsl" SFX=.sco all

solaris:
	make -f Makefile CC=cc COPTS="-Xa -I /usr/openwin/share/include" LIBS="-lsocket -lnsl -L /usr/openwin/lib -lX11" SFX=.solaris all

sunos:
	make -f Makefile CC=acc COPTS="-I. -I/usr/openwin/share/include -sys5 -Xa -Bstatic" LIBS="-lnsl -L /usr/openwin/lib -lX11" SFX=.sunos all

tatung:
	make -f Makefile CC=gcc COPTS="-DTATUNG" LIBS="-lX11" SFX=.tatung all

