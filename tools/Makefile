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
	@echo "  make alphaosf"
	@echo "  make freebsd"
	@echo "  make hpux_ansi"
	@echo "  make hpux_posix"
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
