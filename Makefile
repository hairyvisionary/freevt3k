#
# Makefile for freevt3k
#

APPSRCS  = freevt3k.c vtconn.c
APPOBJS  = freevt3k.o vtconn.o
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
	@echo "  make alphaosf"
	@echo "  make clean"
	@echo "  make freebsd"
	@echo "  make hpux_ansi"
	@echo "  make hpux_posix"
	@echo "  make irix"
	@echo "  make linux"
	@echo "  make ncr"
	@echo "  make sco"
	@echo "  make solaris"
	@echo "  make sunos"
	@echo "  make tatung"

all: freevt3k$(SFX)

freevt3k$(SFX): $(APPOBJS)
	$(CC) $(CFLAGS) $(APPOBJS) -o freevt3k$(SFX) $(LIBS)

freevt3k.o: freevt3k.c hpvt100.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c freevt3k.c

vtconn.o: vtconn.c dumpbuf.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c vtconn.c

clean.o:
	rm -f $(APPOBJS)

clean: clean.o
	rm -f freevt3k$(SFX)

aix:
	make -f Makefile CC=xlc SFX=.aix all

alphaosf:
	make -f Makefile CC=cc COPTS="-std" SFX=.alphaosf all

freebsd:
	make -f Makefile CC=gcc SFX=.freebsd all

hpux_ansi:
	make -f Makefile CC=cc COPTS="-Aa" all

hpux_posix:
	make -f Makefile CC=cc COPTS="-Ae" all

irix:
	make -f Makefile CC=cc SFX=.irix all

linux:
	make -f Makefile CC=gcc SFX=.linux all

ncr:
	make -f Makefile CC=cc COPTS="-Xa -Hnocopyr" LIBS="-lnsl -lsocket -lnet" SFX=.ncr all

sco:
	make -f Makefile CC=cc COPTS="-Xa -a xpg4plus -w3" LIBS="-lintl -lsocket -lnsl" SFX=.sco all

solaris:
	make -f Makefile CC=cc COPTS="-Xa" LIBS="-lsocket -lnsl" SFX=.solaris all

sunos:
	make -f Makefile CC=acc COPTS="-sys5 -Xa -Bstatic" LIBS="-lnsl" SFX=.sunos all

tatung:
	make -f Makefile CC=gcc SFC=.tatung all
