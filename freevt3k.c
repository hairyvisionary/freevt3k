/* Copyright (C) 1996 Office Products Technology, Inc.

   freevt3k: a VT emulator for Unix. 

   This file is distributed under the GNU General Public License.
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software Foundation,
   Inc., 675 Mass Ave, Cambridge MA 02139, USA. 

   Original: Bruce Toback, 22 MAR 96
   Additional: Dan Hollis <dhollis@pharmcomp.com>, 27 MAR 96
               Randy Medd <randy@telamon.com>, 28 MAR 96
   Platforms:	HP-UX 9.04 (9000/807)
   		HP-UX 9.10 (9000/425)
		AIX 4.1
		Solaris 2.4
		SunOS 4.1.3
		Linux 1.1.59, 1.3.83
		SCO 3.2
		AlphaOSF 3.2
		AlphaVMS 6.2
		VaxVMS 6.2
		IRIX 5.3
		FreeBSD 2.1.0
*/

#ifdef VMS
#  include <types.h>
#  include <stdio.h>
#  include <unixio.h>
#  include <string.h>
#  include <stdlib.h>
#  include <time.h>
#  include <timeb.h>
#  include <stdarg.h>
#  include <ctype.h>
#  include <errno.h>
#  include <limits.h>
#  include <file.h>
#  include <signal.h>
#  include <assert.h>
#  include <stat.h>
#  include <dcdef.h>
#  include <ssdef.h>
#  include <iodef.h>
#  include <msgdef.h>
#  include <ttdef.h>
#  include <tt2def.h>
#  include <stsdef.h>
#  include <descrip.h>
#  include <psldef.h>
#  include <syidef.h>
#  include <rms.h>
#  include <socket.h>
#  include <in.h>
#  include <netdb.h>
#  include <inet.h>
#  include <lib$routines.h>
#  include <starlet.h>
#  include <ucx$inetdef.h>
#else
#  ifdef __hpux
#    ifndef _HPUX_SOURCE
#      define _HPUX_SOURCE		(1)
#    endif
#    ifndef _POSIX_SOURCE
#      define _POSIX_SOURCE		(1)
#    endif
#  endif
#  ifdef _M_UNIX	/* SCO */
#    ifndef M_UNIX
#      define M_UNIX			(1)
#    endif
#  endif
#  include <sys/types.h>
#  include <unistd.h>
#  include <stdio.h>
#  include <stddef.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <ctype.h>
#  include <sys/types.h>
#  include <string.h>
#  include <errno.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <fcntl.h>
#  if defined(_AIX) || defined(AIX) || defined(aix)
#    define TERMIO_TYPE	termio
#    include <termio.h>
#    include <sys/select.h>
#  else
#    define HAVE_TERMIOS_H	(1)
#    define TERMIO_TYPE	termios
#    include <termios.h>
#  endif
#  if defined(_AIX) || defined(AIX) || defined(aix) || defined(M_UNIX)
#    include <time.h>
#  endif
#  include <sys/time.h>
#  include <signal.h>
#endif
#include "typedef.h"
#include "vt.h"
#include "freevt3k.h"
#include "hpvt100.h"
#include "vtconn.h"
#ifdef VMS
#  include "vmsutil.h"
#endif
#include "dumpbuf.h"

/* Useful macros */

#ifndef MAX
#  define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

/* Global variables */

#define DFLT_BREAK_MAX		(3)
#define DFLT_BREAK_TIMER	(1)
int
	break_max = DFLT_BREAK_MAX,
	break_sigs = DFLT_BREAK_MAX,
	break_timer = DFLT_BREAK_TIMER;
int
	stdin_fd = 0;
tBoolean
	done = FALSE,
	send_break = FALSE;
int
	debug = 0;
tBoolean
	stop_at_eof = FALSE,
	type_ahead = FALSE;
#ifdef VMS
unsigned int
	readMask = 0,
	returnMask = 0,
	efnMask = 0,
	sockBit = 0,
	sockEfn = 0,
	termEfn = 0,
	termBit = 0;
int
	termReadPending = 0,
	sockReadPending = 0;
#else
struct TERMIO_TYPE
	old_termios;
#endif

#define ASC_BS			(0x08)
#define ASC_LF			(0x0A)
#define ASC_CR			(0x0D)
#define ASC_DC1			(0x11)
#define ASC_DC2			(0x12)
#define ASC_CAN			(0x18)
#define ASC_EM			(0x19)
#define ASC_ESC			(0x1B)
#define ASC_RS			(0x1E)

/* Current line awaiting send to satisfy an FREAD request */
#define MAX_INPUT_REC		(kVT_MAX_BUFFER)
char
	input_rec[MAX_INPUT_REC];
int
	input_rec_len = 0;
/* Circular input queue parms */
#define MAX_INPUT_QUEUE		(kVT_MAX_BUFFER)
char
	input_queue[MAX_INPUT_QUEUE],
	*inq_rptr = input_queue,
	*inq_wptr = input_queue;
int
	input_queue_len = 0;

/* Logging stuff */
#define LOG_INPUT		(0x01)
#define LOG_OUTPUT		(0x02)
FILE
	*log = NULL;
int
	log_type = 0;

/* Miscellaneous stuff */
tBoolean
	generic = FALSE,
	vt100 = FALSE,
	vt52 = FALSE,
	translate = FALSE;
int
	term_type = 10;
tBoolean
	disable_xon_xoff = FALSE;
int32
	first_break_time = 0;

#include "timers.c"


#ifdef __STDC__
void PrintUsage(int detail)
#else
void PrintUsage(detail)
    int
    	detail;
#endif
{

    printf("Usage: freevt3k [-li|-lo|-lio] [-f file] [-x] [-tt n] [-t]\n");
    printf("                ");
#ifdef VMS
    printf("[-breaks count] [-breaktimer timer]\n");
#else
    printf("[-B count] [-T timer]\n");
#endif
    printf("                [-a|-I file] [-d[d]] host\n");
    if (!detail)
	return;
    printf("   -li|-lo|-lio    - specify input|output logging options\n");
    printf("   -f file         - destination for logging [stdout]\n");
    printf("   -x              - disable xon/xoff flow control\n");
    printf("   -tt n           - 'n'->10 (default) generates DC1 read triggers\n");
    printf("   -t              - enable type-ahead\n");
#ifdef VMS
    printf("   -breaks count   - change number of breaks for command mode [%d]\n",
	   DFLT_BREAK_MAX);
    printf("   -breaktimer timer - change -breaks time interval in seconds [%d]\n",
	   DFLT_BREAK_TIMER);
#else
    printf("   -B count        - change number of breaks for command mode [%d]\n",
	   DFLT_BREAK_MAX);
    printf("   -T timer        - change -B time interval in seconds [%d]\n",
	   DFLT_BREAK_TIMER);
#endif
    printf("   -vt100          - Emulate hp2392 on vt100 terminals.\n");
    printf("   -vt52           - Emulate hp2392 on vt52 terminals.\n");
    printf("   -generic        - Translate hp escape sequences to tokens\n");
    printf("   -a file         - read initial commands from file.\n");
    printf("   -I file         - like -a, but stops when end-of-file reached\n");
    printf("   -d[d]           - enable debug output to freevt3k.debug\n");
    printf("   host            - name/IP address of target HP 3000\n");

}

#ifndef VMS
#ifdef __STDC__
int SetTtyAttributes(int fd, struct TERMIO_TYPE *termio_buf)
#else
int SetTtyAttributes(fd, termio_buf)
    int
    	fd;
    struct TERMIO_TYPE
	*termio_buf;
#endif
{ /*SetTtyAttributes*/

#ifdef HAVE_TERMIOS_H
    if (tcsetattr(fd, TCSANOW, termio_buf) == -1)
	return(-1);
#else
    if (ioctl(fd, TCSETA, termio_buf) == -1)
	return(-1);
#endif
    return(0);

} /*SetTtyAttributes*/

#ifdef __STDC__
int GetTtyAttributes(int fd, struct TERMIO_TYPE *termio_buf)
#else
int GetTtyAttributes(fd, termio_buf)
    int
	fd;
    struct TERMIO_TYPE
    	*termio_buf;
#endif
{ /*GetTtyAttributes*/

#ifdef HAVE_TERMIOS_H
    if (tcgetattr(fd, termio_buf) == -1)
	return(-1);
#else
    if (ioctl(fd, TCGETA, termio_buf) == -1)
	return(-1);
#endif
    return(0);

} /*GetTtyAttributes*/
#endif /*~VMS*/

#ifdef __STDC__
void ProcessInterrupt(void)
#else
void ProcessInterrupt()
#endif
{/*ProcessInterrupt*/
    
#ifndef VMS
    struct TERMIO_TYPE
	curr_termios;
    struct TERMIO_TYPE
	temp_termios;
#endif
    char
	ans[32];
    
#ifdef VMS
    SetTTYNormal();
#else
    GetTtyAttributes(STDIN_FILENO, &curr_termios);
    temp_termios = old_termios;
    SetTtyAttributes(STDIN_FILENO, &temp_termios);
#endif
    printf("\n");
    for (;;)
	{
	printf("Please enter FREEVT3K command (Exit or Continue) : ");
	gets(ans);
	if (strlen(ans) == 0)
	    continue;
	if (islower(*ans))
	    *ans = (char)toupper(*ans);
	if (*ans == 'E')
	    {
	    done = TRUE;
	    printf("\r\nTerminating\r\n");
	    break;
	    }
	if (*ans == 'C')
	    {
	    break_sigs = break_max;
	    break;
	    }
	}
#ifdef VMS
    SetTTYRaw();
#else
    SetTtyAttributes(STDIN_FILENO, &curr_termios);
#endif

} /*ProcessInterrupt*/

#ifdef USE_CTLC_INTERRUPTS
typedef void (*SigfuncInt)(int);

#ifdef __STDC__
void CatchCtlC(int sig_type)
#else
void CatchCtlC(sig_type)
    int
    	sig_type;
#endif
{ /*CatchCtlC*/

    SigfuncInt
	    signalPtr;

#  ifdef BREAK_VIA_SIG
    if (!(--break_sigs))
	ProcessInterrupt();
#  endif
    signalPtr = (SigfuncInt)CatchCtlC;
    if (signal(SIGINT, signalPtr) == SIG_ERR)
        {
        perror("signal");
#  ifdef VMS
	exit(STS$K_WARNING);
#  else
        exit(1);
#  endif
        }
} /*CatchCtlC*/

#ifdef __STDC__
void RestoreCtlC(void)
#else
void RestoreCtlC()
#endif
{ /*RestoreCtlC*/

    SigfuncInt
	    signalPtr;

    signalPtr = (SigfuncInt)SIG_DFL;
    if (signal(SIGINT, signalPtr) == SIG_ERR)
        perror("signal");

} /*RestoreCtlC*/
#endif /* USE_CTLC_INTERRUPTS */

#ifdef __STDC__
void FlushQ(void)
#else
void FlushQ()
#endif
{ /*FlushQ*/

    input_queue_len = 0;
    inq_rptr = inq_wptr = input_queue;

} /*FlushQ*/

#ifdef __STDC__
int GetQ(void)
#else
int GetQ()
#endif
{ /*GetQ*/

    if (input_queue_len)
	{
	if (++inq_rptr == &input_queue[MAX_INPUT_QUEUE])
	    inq_rptr = input_queue;
	--input_queue_len;
	return(*inq_rptr);
	}
    return(-1);
    
} /*GetQ*/

#ifdef __STDC__
int PutQ(char ch)
#else
int PutQ(ch)
    char
	ch;
#endif
{ /*PutQ*/

    if (++inq_wptr == &input_queue[MAX_INPUT_QUEUE])
	inq_wptr = input_queue;
    if (inq_wptr == inq_rptr)
	{
	fprintf(stderr, "<queue overflow>\n");
	return(-1);
	}
    ++input_queue_len;
    *inq_wptr = ch;
    return(0);

} /*PutQ*/

#ifdef __STDC__
tBoolean AltEol(tVTConnection *conn, char ch)
#else
tBoolean AltEol(conn, ch)
    tVTConnection
    	*conn;
    char
    	ch;
#endif
{ /*AltEol*/

    if ((conn->fAltLineTerminationChar) &&
	(conn->fAltLineTerminationChar !=
	 conn->fLineTerminationChar) &&
	(ch == conn->fAltLineTerminationChar))
	return(TRUE);
    return(FALSE);

} /*AltEol*/

#ifdef __STDC__
tBoolean PrimEol(tVTConnection *conn, char ch)
#else
tBoolean PrimEol(conn, ch)
    tVTConnection
    	*conn;
    char
    	ch;
#endif
{ /*PrimEol*/

    if (ch == conn->fLineTerminationChar)
	return(TRUE);
    return(FALSE);

} /*PrimEol*/

#ifdef __STDC__
int ProcessQueueToHost(tVTConnection *conn, int len)
#else
int ProcessQueueToHost(conn, len)
    tVTConnection
    	*conn;
    int
    	len;
#endif
{/*ProcessQueueToHost*/

/*
#define TRANSLATE_INPUT	(1)
 */

    static char
	cr = '\r',
	lf = '\n';
    char
	ch;
    tBoolean
	vt_fkey = FALSE,
	alt = FALSE,
	prim = FALSE;
    int
	int_ch = 0,
	whichError = 0,
	send_index = -1,
	comp_mask = kVTIOCSuccessful;

    if (len == -2)
	{ /* Break - flush all queues */
	if (conn->fSysBreakEnabled)
	    {
	    send_index = kDTCSystemBreakIndex;
	    FlushQ();
	    }
	}
    else if (len == -1)
	comp_mask = kVTIOCTimeout;
    else if (len >= 0)
	{
	for (;;)
	    {
	    if ((int_ch = GetQ()) == -1)
		{
		if (stop_at_eof)
		    done = TRUE;
		return(0);	/* Ran out of characters */
		}
	    ch = (char)int_ch;
	    if ((!(conn->fUneditedMode)) && (!(conn->fBinaryMode)))
		{
		if ((ch == conn->fCharDeleteChar) ||
		    (ch == (char)127))
		    {
		    if (input_rec_len)
			{
			char	bs_buf[8];
			int	bs_len = 0;
			--input_rec_len;
			switch (conn->fCharDeleteEcho)
			    {
			case kAMEchoBackspace:
			    bs_buf[0] = ASC_BS;
			    bs_len = 1;
			    break;
			case kAMEchoBSSlash:
			    bs_buf[0] = '\\';
			    bs_buf[1] = ASC_LF;
			    bs_len = 2;
			    break;
			case kAMEchoBsSpBs:
			    bs_buf[0] = ASC_BS;
			    bs_buf[1] = ' ';
			    bs_buf[2] = ASC_BS;
			    bs_len = 3;
			    break;
			default:bs_len = 0;
			    }
			if ((bs_len) && (conn->fEchoControl != 1))
			    conn->fDataOutProc(conn->fDataOutRefCon,
					       bs_buf, bs_len);
			}
		    continue;
		    }
		if (ch == conn->fLineDeleteChar)
		    {
		    input_rec_len = 0;
/* Don't echo if line delete echo disabled */
		    if (conn->fDisableLineDeleteEcho)
			continue;
		    conn->fDataOutProc(conn->fDataOutRefCon,
				       conn->fLineDeleteEcho,
				       conn->fLineDeleteEchoLength);
		    continue;
		    }
		}
	    if (conn->fDriverMode == kDTCBlockMode)
		{
		if ((!input_rec_len) && (ch == ASC_DC2))
		    {
		    input_rec[0] = ASC_ESC;
		    input_rec[1] = 'h';
		    input_rec[2] = ASC_ESC;
		    input_rec[3] = 'c';
		    input_rec[4] = ASC_DC1;
		    conn->fDataOutProc(conn->fDataOutRefCon,
				       (void*)input_rec, 5);
		    while (GetQ() != -1);
		    return(0);
		    }
		}
	    input_rec[input_rec_len++] = ch;
	    if ((conn->fEchoControl != 1) &&
		(conn->fDriverMode == kDTCVanilla))
		conn->fDataOutProc(conn->fDataOutRefCon, (void*)&ch, 1);
	    if ((conn->fSubsysBreakEnabled) &&
		(ch == conn->fSubsysBreakChar))
		send_index = kDTCCntlYIndex;
#ifdef TRANSLATE_INPUT
	    if ((translate) && (ch == '~') && (input_rec[0] == ASC_ESC))
		vt_fkey = TRUE;
	    else
#endif
	      if (conn->fDriverMode == kDTCVanilla)   /* RM 960509 */
		prim = PrimEol(conn, ch);
	    alt = AltEol(conn, ch);
	    if ((send_index == kDTCCntlYIndex) ||
		(input_rec_len >= conn->fReadLength) ||
		(prim) || (alt) || (vt_fkey))
		{
		if (send_index == kDTCCntlYIndex)
		    --input_rec_len;
		else
		    {
		    if (alt)
			{
			comp_mask = kVTIOCBreakRead;
			if ((conn->fEchoControl != 1) &&
			    (conn->fDriverMode == kDTCVanilla))
			    conn->fDataOutProc(conn->fDataOutRefCon,
					       (void*)&cr, 1);
			}
		    else if (input_rec_len <= conn->fReadLength)
			{
			if (prim)
			    --input_rec_len;
			}
		    if ((conn->fEchoCRLFOnCR) &&
			(conn->fDriverMode == kDTCVanilla) &&
			(!(conn->fBinaryMode)))
			conn->fDataOutProc(conn->fDataOutRefCon,
					   (void*)&lf, 1);
		    }
		if (log_type & LOG_INPUT)
		    {
		    char *ptr = input_rec;
		    int len = input_rec_len;
		    while (len--)
			{
			putc((int)*ptr, log);
			++ptr;
			}
		    putc('\n', log);
		    }
		break;
		}
	    }
	}

    if (send_index == -1)
	{
#ifdef TRANSLATE_INPUT
	if (translate)
	    TranslateKeyboard(input_rec, &input_rec_len);
#endif
	whichError = VTSendData(conn, input_rec, input_rec_len, comp_mask);
	}
    else
	whichError = VTSendBreak(conn, send_index);
    if (whichError)
	{
	char	messageBuffer[128];
	printf("Unable to send to host:\n");
	VTErrorMessage(conn, whichError,
		       messageBuffer, sizeof(messageBuffer));
	printf("%s\n", messageBuffer);
	return(-1);
	}

    conn->fReadInProgress = FALSE;
    input_rec_len = 0;
    return(0);

}/*ProcessQueueToHost*/

#ifdef __STDC__
int ProcessSocket(tVTConnection * conn)
#else
int ProcessSocket(conn)
    tVTConnection
    	*conn;
#endif
{/*ProcessSocket*/

    int
	whichError = 0;
    static char trigger[] = { ASC_DC1 };               /* JCM 040296 */
    
    whichError = VTReceiveDataReady(conn);
    if (whichError == kVTCVTOpen)
	{
/*
 * The connection is open now, so initialize for
 * terminal operations. This means setting up
 * the TTY for "raw" operation. (Or, it will
 * once we get that set up.)
 */
/*     	vtOpen = TRUE; */
	}
    else if (whichError != kVTCNoError)
	{
	char	messageBuffer[128];
	if (whichError == kVTCStartShutdown)
	    return(1);
	printf("VT error:\r\n");
	VTErrorMessage(conn, whichError,
		       messageBuffer, sizeof(messageBuffer));
	printf("%s\r\n", messageBuffer);
	return(-1);
	}
    if (conn->fReadStarted)
	{
	conn->fReadStarted = FALSE;
	if (conn->fReadFlush)	/* RM 960403 */
	    {
	    conn->fReadFlush = FALSE;
	    FlushQ();
	    }
	if (term_type == 10)
	    conn->fDataOutProc(conn->fDataOutRefCon,
			       trigger, sizeof(trigger));
/*
 * As we just got a read request, check for any typed-ahead data and
 *   process it now.
 */
	ProcessQueueToHost(conn, 0);
	}
    return(0);

}/*ProcessSocket*/

#ifdef __STDC__
int ProcessTTY(tVTConnection * conn, char *buf, int len)
#else
int ProcessTTY(conn, buf, len)
    tVTConnection
    	*conn;
    char
    	*buf;
    int
    	len;
#endif
{/*ProcessTTY*/
    
    extern FILE
	*debug_fd;
    struct timeval
	timeout;
    int
	readCount = 1;
#ifndef VMS
    fd_set
	readfds;
#endif

    if (len > 0)
	{
	if (debug > 1)
	    fprintf(debug_fd, "read: ");
#ifdef VMS
#  ifndef BREAK_VIA_SIG
	if (*buf == (conn->fSysBreakChar & 0xFF))
	    { /* Break */
	    send_break = TRUE;
/* Check for consecutive breaks - 'break_max'-in-a-row to get out */
	    if (debug > 1)
		fprintf(debug_fd, "%02x ", *buf);
	    if (break_sigs == break_max)
		first_break_time = MyGettimeofday();
	    if (ElapsedTime(first_break_time) > break_timer)
		{
		break_sigs = break_max;
		first_break_time = MyGettimeofday();
		}
	    if (!(--break_sigs))
		ProcessInterrupt();
	    if (send_break)
		{
		if (conn->fSysBreakEnabled)
		    ProcessQueueToHost(conn, -2);
		send_break = FALSE;
		}
	    }
	else
#  endif
	    {
	    if (debug > 1)
		fprintf(debug_fd, "%02x ", *buf);
	    break_sigs = break_max;
	    if ((type_ahead) || (conn->fReadInProgress))
		{
		if (PutQ(*buf) == -1)
		    return(-1);
		}
	    }
#else
/*
 * Once we get the signal that at least one byte is ready, sit and read
 *   bytes from stdin until the select timer goes off after 10000 microsecs
 */
	for (;;)
	    {
	    if (!readCount)
		{
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
		FD_ZERO(&readfds);
		FD_SET(stdin_fd, &readfds);
		switch (select(stdin_fd+1, (void*)&readfds, NULL, NULL, (struct timeval *)&timeout))
		    {
		case -1:	/* Error */
		    if (errno == EINTR)
			{
			errno = 0;
			continue;
			}
		    fprintf(stderr, "Error on select: %d.\n", errno);
		    return(-1);
		case 0:		/* Timeout */
		    readCount = -1;
		    if (debug > 1)
			fprintf(debug_fd, "\n");
		    break;
		default:
		    if (FD_ISSET(stdin_fd, &readfds))
			{
			if ((readCount = read(stdin_fd, buf, 1)) != 1)
			    {
			    fprintf(stderr, "Error on read: %d.\n", errno);
			    return(-1);
			    }
			}
		    }
		if (readCount == -1)
		    break;
		}
#ifndef BREAK_VIA_SIG
	    if (*buf == (conn->fSysBreakChar & 0xFF))
		{ /* Break */
		send_break = TRUE;
/* Check for consecutive breaks - 'break_max'-in-a-row to get out */
		if (debug > 1)
		    fprintf(debug_fd, "%02x ", *buf);
		if (break_sigs == break_max)
		    first_break_time = MyGettimeofday();
		if (ElapsedTime(first_break_time) > break_timer)
		    {
		    break_sigs = break_max;
		    first_break_time = MyGettimeofday();
		    }
		if (!(--break_sigs))
		    ProcessInterrupt();
		if (send_break)
		    {
		    if (conn->fSysBreakEnabled)
			ProcessQueueToHost(conn, -2);
		    send_break = FALSE;
		    }
		readCount = 0;
		continue;
		}
#endif
	    if (debug > 1)
		fprintf(debug_fd, "%02x ", *buf);
	    break_sigs = break_max;
	    if ((type_ahead) || (conn->fReadInProgress))
		{
		if (PutQ(*buf) == -1)
		    return(-1);
		}
/*
 * If a read is in progress and we've gathered enough data to satisfy it,
 *    get out of the loop.
 */
	    if ((conn->fReadInProgress) &&
		((input_rec_len + input_queue_len) >= conn->fReadLength))
		{
		if (debug > 1)
		    fprintf(debug_fd, "len\n");
		break;
		}
	    readCount = 0;
	    } /* for (;;) */
#endif
	} /* if (len > 0) */
    if (conn->fReadInProgress)
	ProcessQueueToHost(conn, len);
    return(0);

} /*ProcessTTY*/

#ifndef VMS
#ifdef __STDC__
int OpenTTY(struct TERMIO_TYPE *new_termio, struct TERMIO_TYPE *old_termio)
#else
int OpenTTY(new_termio, old_termio)
    struct TERMIO_TYPE
    	*new_termio,
    	*old_termio;
#endif
{ /*OpenTTY*/

    int
	posix_vdisable = 0,
	fd = 0,
	whichError = 0;

    fd = STDIN_FILENO;
    whichError = GetTtyAttributes(fd, old_termio);
    if (whichError)
        {
        printf("Unable to get terminal attributes.\n");
	return(-1);
        }

    *new_termio = *old_termio;

/* Raw mode */
    new_termio->c_lflag = 0;
/* Setup for raw single char I/O */
    new_termio->c_cc[VMIN] = 1;
    new_termio->c_cc[VTIME] = 0;
/* Don't do output post-processing */
    new_termio->c_oflag = 0;
/* Don't convert CR to NL */
    new_termio->c_iflag &= ~(ICRNL);
/* Character formats */
#ifdef do_we_care
    new_termio->c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    new_termio->c_cflag |= (CS8 | CREAD);
#endif
/* Break handling */
    new_termio->c_iflag &= ~IGNBRK;
    new_termio->c_iflag |= BRKINT;
#ifdef _POSIX_VDISABLE
#  if (_POSIX_VDISABLE == -1)
#    undef _POSIX_VDISABLE
#  endif
#endif
#ifndef _POSIX_VDISABLE
#  ifdef _PC_VDISABLE
    if ((posix_vdisable = fpathconf(fd, _PC_VDISABLE)) == -1)
	{
	errno = 0;
	posix_vdisable = 0377;
	}
#  else
    posix_vdisable = 0377;
#  endif
#endif
    if (disable_xon_xoff)
	{
#ifdef VSTART
	new_termio->c_cc[VSTART]= (unsigned char)posix_vdisable;
#endif
#ifdef VSTOP
	new_termio->c_cc[VSTOP]	= (unsigned char)posix_vdisable;
#endif
	}
#ifdef BREAK_VIA_SIG
    new_termio->c_lflag |= ISIG;
#  ifdef VSUSP
    new_termio->c_cc[VSUSP]	= (unsigned char)posix_vdisable;
#  endif
#  ifdef VDSUSP
    new_termio->c_cc[VDSUSP]	= (unsigned char)posix_vdisable;
#  endif
    new_termio->c_cc[VINTR]	= (unsigned char)posix_vdisable;
    new_termio->c_cc[VQUIT]	= (unsigned char)posix_vdisable;
    new_termio->c_cc[VERASE]	= (unsigned char)posix_vdisable;
    new_termio->c_cc[VKILL]	= (unsigned char)posix_vdisable;
#  ifdef VEOF
    new_termio->c_cc[VEOF]	= (unsigned char)posix_vdisable;
#  endif
#  ifdef VSWTCH
    new_termio->c_cc[VSWTCH]	= (unsigned char)posix_vdisable;
#  endif
#endif /*BREAK_VIA_SIG*/

    SetTtyAttributes(fd, new_termio);

    return(fd);

} /*OpenTTY*/

#ifdef __STDC__
void CloseTTY(int fd, struct TERMIO_TYPE *old_termio)
#else
void CloseTTY(fd, old_termio)
    int
    	fd;
    struct TERMIO_TYPE
    	*old_termio;
#endif
{ /*CloseTTY*/

    SetTtyAttributes(fd, old_termio);
    if (fd != STDIN_FILENO)
	close(fd);

} /*CloseTTY*/
#endif /*~VMS*/

#ifdef __STDC__
int DoMessageLoop(tVTConnection * conn)
#else
int DoMessageLoop(conn)
    tVTConnection
    	*conn;
#endif
{ /*DoMessageLoop*/
    int         whichError;
    int		readCount;
    int		returnValue = 0;
#ifndef VMS
    struct timeval timeout;
    struct timeval *time_ptr;
    fd_set	readfds;
    struct TERMIO_TYPE new_termios;
    tBoolean    oldTermiosValid = FALSE;
    int		nfds = 0;
    char        termBuffer[2];
#else
    int32	timeout = 0;
    char        termBuffer[2048];
#endif
/*  tBoolean    vtOpen; */
    int32	start_time = 0;
    int32	read_timer = 0;
    int32      	time_remaining = 0;
    tBoolean	timed_read = FALSE;
    char        messageBuffer[128];
    int		vtSocket;
    extern FILE
	*debug_fd;

#ifdef VMS
    OpenTTY();
#else
    if ((stdin_fd = OpenTTY(&new_termios, &old_termios)) == -1)
	{
	returnValue = 1;
	goto Last;
	}
    oldTermiosValid = TRUE;    /* We can clean up now. */
#endif

/*
 * Setup a read loop waiting for I/O on either fd.  For connection I/O,
 *   process the data using VTReceiveDataReady.  For tty data, add the
 *   data to the outbound queue and call ProcessQueueToHost if a read is
 *   in progress.
 */
    
#ifdef USE_CTLC_INTERRUPTS
/* Dummy call up front to prime the pump */
    break_sigs = break_max;
    CatchCtlC(0);
#endif
    break_sigs = break_max;

    vtSocket = VTSocket(conn);
#ifndef VMS
    nfds = 1 + MAX(stdin_fd, vtSocket);
#endif
    while (! done)
	{
#ifdef VMS
	if (!sockReadPending)
	    VTReceiveDataReady(conn);
/*
 * If a read timer has been specified, use it in the read
 */
	if ((conn->fReadInProgress) && (conn->fReadTimeout))
	    {
	    if (!timed_read)
		{ /* First time timer was specified */
		timed_read = TRUE;
		start_time = MyGettimeofday();
		read_timer = conn->fReadTimeout;
		time_remaining = read_timer;
		}
	    timeout = time_remaining;
	    if (debug)
		fprintf(debug_fd, "timer: %d\n", timeout);
	    }
	else
	    {
	    timed_read = FALSE;
	    timeout = 0;
	    }
	StartTTYRead((unsigned char*)termBuffer,
		     (conn->fEchoControl != 1),
		     timeout);
	
	if (WaitForCompletion(&returnMask) == -1)
	    ExitProc("WaitForCompletion", "", 1);
	if (returnMask & sockBit)
	    {
	    if (timed_read)
		time_remaining = read_timer - ElapsedTime(start_time)/1000;
	    switch (ProcessSocket(conn))
		{
	    case -1:	returnValue = 1;	/* fall through */
	    case 1:	done = TRUE;
		}
	    }
	if ((!done) && (returnMask & termBit))
	    {
#  ifdef USE_CTLC_INTERRUPTS
	    break_sigs = break_max;
#  endif
	    readCount = CompleteTTYRead((unsigned char*)termBuffer);
	    if (readCount == -2)
		{
		if (ProcessTTY(conn, termBuffer, -1) == -1)
		    {
		    returnValue = 1;
		    goto Last;
		    }
		timed_read = FALSE;
		continue;
		}
	    if (timed_read)
		time_remaining = read_timer - ElapsedTime(start_time)/1000;
	    if (readCount >= 0)
		{
		if (ProcessTTY(conn, termBuffer, readCount) == -1)
		    {
		    returnValue = 1;
		    goto Last;
		    }
		}
	    }
#else
	FD_ZERO(&readfds);
	FD_SET(stdin_fd, &readfds);
	FD_SET(vtSocket, &readfds);
/*
 * If a read timer has been specified, use it in the select()
 *   call.
 */
	if ((conn->fReadInProgress) && (conn->fReadTimeout))
	    {
	    if (!timed_read)
		{ /* First time timer was specified */
		timed_read = TRUE;
		start_time = MyGettimeofday();
		read_timer = conn->fReadTimeout * 1000;
		time_remaining = read_timer;
		}
	    timeout.tv_sec = time_remaining / 1000;
	    timeout.tv_usec = (time_remaining % 1000) * 1000;
	    time_ptr = (struct timeval*)&timeout;
	    if (debug)
		fprintf(debug_fd, "timer: %d.%06d\n", timeout.tv_sec, timeout.tv_usec);
	    }
	else
	    {
	    timed_read = FALSE;
	    time_ptr = (struct timeval*)NULL;
	    }

	switch (select(nfds, (void*)&readfds, NULL, NULL, time_ptr))
	    {
	case -1:	/* Error */
	    if (errno == EINTR)
		{
#  ifdef BREAK_VIA_SIG
		if (send_break)
		    {
		    ProcessQueueToHost(conn, -2);
		    send_break = FALSE;
		    }
#  endif
		errno = 0;
		continue;
		}
	    printf("Error on select: %d.\n", errno);
	    returnValue = 1;
	    goto Last;
	case 0:		/* Timeout */
	    if (ProcessTTY(conn, termBuffer, -1) == -1)
		{
		returnValue = 1;
		goto Last;
		}
	    timed_read = FALSE;
	    continue;
	default:
	    if (timed_read)
		time_remaining = read_timer - ElapsedTime(start_time);
	    if (FD_ISSET(vtSocket, &readfds))
		{
		switch (ProcessSocket(conn))
		    {
		case -1: returnValue = 1;	/* fall through */
		case 1:  done = TRUE;
		    }
		}
	    if ((!done) && (FD_ISSET(stdin_fd, &readfds)))
		{
		if ((readCount = read(stdin_fd, termBuffer, 1)) != 1)
		    {
		    returnValue = 1;
		    goto Last;
		    }
		if (ProcessTTY(conn, termBuffer, readCount) == -1)
		    {
		    returnValue = 1;
		    goto Last;
		    }
		}
	    } /* switch */
#endif /*~VMS*/
        }  /* End read loop */

Last:
#ifdef USE_CTLC_INTERRUPTS
    RestoreCtlC();
#endif
#ifdef VMS
    CloseTTY();
#else
    if (oldTermiosValid)
	CloseTTY(stdin_fd, &old_termios);
#endif
    return(returnValue);
} /*DoMessageLoop*/

#ifdef __STDC__
void vt3kDataOutProc(int32 refCon, char * buffer, int bufferLength)
#else
void vt3kDataOutProc(refCon, buffer, bufferLength)
    int32
    	refCon;
    char
    	*buffer;
    int
    	bufferLength;
#endif
{ /*vt3kDataOutProc*/

#ifdef VMS
    TT_WRITE_IOSB
	iosb;
    int
	io_status = 0;
    extern unsigned short
	termNum;
#endif

    if (log_type & LOG_OUTPUT)
	{
	char *ptr = buffer;
	int len = bufferLength;
	while (len--)
	    {
	    putc((int)*ptr, log);
	    if (*ptr == ASC_DC1)	/* Ugh */
		putc('\n', log);
	    ++ptr;
	    }
	}
#ifdef VMS
    io_status = SYS$QIOW(0,			/* event flag */
			 termNum,		/* channel */
			 IO$_WRITEVBLK,		/* function */
			 &iosb,			/* i/o status block */
			 0,			/* astadr */
			 0,			/* astprm */
			 buffer,		/* P1, char buffer */
			 bufferLength,		/* P2, length */
			 0,			/* P3, fill */
			 0,			/* P4, fill */
			 0,			/* P5, fill */
			 0);			/* P6, fill */
    if ((VMSerror(io_status)) || (VMSerror(iosb.status)))
	ExitProc("SYS$QIOW", "IO$_WRITEVBLK", 1);
#else
    write(STDOUT_FILENO, buffer, bufferLength);
#endif

} /*vt3kDataOutProc*/

#ifndef XHPTERM
#  include "hpvt100.c"

#ifdef __STDC__
int main(int argc, char ** argv)
#else
int main(argc, argv)
    int
    	argc;
    char
    	**argv;
#endif
{ /*main*/
    long  ipAddress;
    int	  ipPort = kVT_PORT;
    struct hostent * theHost;
    tVTConnection * conn;
    tBoolean	parm_error = FALSE;
    int		vtError;
    int		returnValue = 0;
    char	messageBuffer[128];
    char	*hostname;
    char	*input_file = NULL;
    char	*log_file = NULL;
    char	*ptr;

    if (argc < 2)
        {
	PrintUsage(1);
#ifdef VMS
	exit(STS$K_WARNING);
#else
        return(2);
#endif
	}
    
    ++argv;
    --argc;
    while ((argc > 0) && (*argv[0] == '-'))
	{
	if (!strncmp(*argv, "-d", 2))
	    {
	    ++debug;
	    ptr = *argv;
	    ptr += 2;
	    while (*ptr == 'd')
		{
		++debug;
		++ptr;
		}
	    }
	else if (!strcmp(*argv, "-t"))
	    type_ahead = TRUE;
	else if ((!strcmp(*argv, "-a")) ||
		 (!strcmp(*argv, "-I")))
	    {
	    stop_at_eof = (!strcmp(*argv, "-I"));
	    if (--argc)
		{
		++argv;
		input_file = *argv;
		}
	    else
		parm_error = TRUE;
	    }
	else if (!strcmp(*argv, "-generic"))
	    translate = generic = TRUE;
	else if (!strcmp(*argv, "-vt100"))
	    translate = vt100 = TRUE;
	else if (!strcmp(*argv, "-vt52"))
	    translate = vt52 = TRUE;
	else if (!strcmp(*argv, "-x"))
	    disable_xon_xoff = TRUE;
	else if (!strcmp(*argv, "-f"))
	    {
	    if (--argc)
		{
		++argv;
		log_file = *argv;
		}
	    else
		parm_error = TRUE;
	    }
	else if (!strcmp(*argv, "-p"))
	    {
	    if (--argc)
		{
		++argv;
		ipPort = atoi(*argv);
		}
	    else
		parm_error = TRUE;
	    }
	else if (!strcmp(*argv, "-tt"))
	    {
	    if (--argc)
		{
		++argv;
		term_type = atoi(*argv);
		}
	    else
		parm_error = TRUE;
	    }
	else if (!strncmp(*argv, "-l", 2))
	    {
	    ptr = *argv;
	    ptr += 2;
	    while (*ptr)
		{
		if (*ptr == 'i')
		    log_type |= LOG_INPUT;
		else if (*ptr == 'o')
		    log_type |= LOG_OUTPUT;
		else
		    {
		    parm_error = TRUE;
		    break;
		    }
		++ptr;
		}
	    }
	else if ((!strcmp(*argv, "-B")) ||
		 (!strcmp(*argv, "-breaks")))
	    {
	    if (--argc)
		{
		++argv;
		break_max = atoi(*argv);
		}
	    else
		parm_error = TRUE;
	    }
	else if ((!strcmp(*argv, "-T")) ||
		 (!strcmp(*argv, "-breaktimer")))
	    {
	    if (--argc)
		{
		++argv;
		break_timer = atoi(*argv);
		}
	    else
		parm_error = TRUE;
	    }
	else
	    parm_error = TRUE;
	if (parm_error)
	    {
	    PrintUsage(0);
	    return(2);
	    }
	--argc;
	++argv;
	}
    if (argc > 0)
	hostname = *argv;

    if (log_file)
	{
	if ((log = fopen(log_file, "w")) == (FILE*)NULL)
	    {
	    perror("fopen");
	    return(1);
	    }
	}
    else if (log_type != 0)
	log = stdout;

    if (input_file)
	{
	FILE *input;
	char buf[128], *ptr;
	if ((input = fopen(input_file, "r")) == (FILE*)NULL)
	    {
	    perror("fopen");
	    return(1);
	    }
	for (;;)
	    {
	    if (fgets(buf, sizeof(buf)-1, input) == NULL)
		break;
	    ptr = buf;
	    while (*ptr)
		{
		if (*ptr == '\n')
		    PutQ(ASC_CR);
		else
		    PutQ(*ptr);
		++ptr;
		}
	    }
	fclose(input);
	}

    /* First, validate the destination. If the destination can be	*/
    /* validated, create a connection structure and try to open the     */
    /* connection.							*/

    ipAddress = (long)inet_addr(hostname);
    if (ipAddress == INADDR_NONE)
	{
	theHost = gethostbyname(hostname);
	if (theHost == NULL)
	    {
            printf("Unable to resolve %s.\n", hostname);
	    return(1);
            }
        memcpy((char *) &ipAddress, theHost->h_addr, sizeof(ipAddress));
	}

    conn = (tVTConnection *) calloc(1, sizeof(tVTConnection));
    if (conn == NULL)
        {
        printf("Unable to allocate a connection.\n");
        return(1);
        }

    if (vtError = VTInitConnection(conn, ipAddress, ipPort))
        {
        printf("Unable to initialize the connection.\n");
        VTErrorMessage(conn, vtError,
				messageBuffer, sizeof(messageBuffer));
        printf("%s\n", messageBuffer);
	VTCleanUpConnection(conn);
        return(1);
        }

    if (term_type == 10)
	conn->fBlockModeSupported = TRUE;	/* RM 960411 */

    conn->fDataOutProc =
	(vt100) ? vt3kHPtoVT100 :
	(vt52) ? vt3kHPtoVT52 :
	(generic) ? vt3kHPtoGeneric: vt3kDataOutProc;

    if (vtError = VTConnect(conn))
	{
        printf("Unable to connect to host.\n");
        VTErrorMessage(conn, vtError,
				messageBuffer, sizeof(messageBuffer));
        printf("%s\n", messageBuffer);
	VTCleanUpConnection(conn);
        return(1);
	}

    printf("To suspend to FREEVT3K command mode press 'break' %d times in a %d second period.\n\n",
	   break_max, break_timer);
    break_timer *= 1000;	/* Convert to ms */

    returnValue = DoMessageLoop(conn);

    VTCleanUpConnection(conn);

#ifdef VMS
    exit((returnValue) ? STS$K_WARNING : STS$K_SUCCESS);
#else
    return(returnValue);
#endif
} /*main*/
#endif /* ifndef XHPTERM */

/* Local Variables: */
/* c-indent-level: 0 */
/* c-continued-statement-offset: 4 */
/* c-brace-offset: 0 */
/* c-argdecl-indent: 4 */
/* c-label-offset: -4 */
/* End: */
