/* Copyright (C) 1996 Office Products Technology, Inc.

   freevt3k: a VT emulator for Unix. 

   This file is distributed under the GNU General Public License.
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software Foundation,
   Inc., 675 Mass Ave, Cambridge MA 02139, USA. 

   Original: Bruce Toback, 22 MAR 96
   Additional: Dan Hollis <dhollis@pharmcomp.com>, 27 MAR 96
               Randy Medd <randy@telamon.com>, 28 MAR 96
	       Stan Sieler <sieler@allegro.com>, 1997-01-07

   Platforms:	HP-UX 9.04 (9000/807)
   		HP-UX 9.10 (9000/425)
		HP-UX 10.10 (9000/859)
		AIX 3.2, 4.1
		Solaris 2.4
		SunOS 4.1.3
		Linux 1.1.59, 1.3.83, 2.0.0
		SCO 3.2
		AlphaOSF 3.2
		AlphaVMS 6.2
		VaxVMS 6.2
		IRIX 5.3
		FreeBSD 2.1.0
*/

char
	*Sccsid = "@(#) freevt3k.c: A.01.03";

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
#  include <iodef.h>
#  include <stsdef.h>
#  include <socket.h>
#  include <in.h>
#  include <netdb.h>
#  include <inet.h>
#  include <lib$routines.h>
#  include <starlet.h>
#  include <ucx$inetdef.h>
#else
#  include "config.h"
#  ifdef HPUX
#    ifndef _HPUX_SOURCE
#      define _HPUX_SOURCE		(1)
#    endif
#    ifndef _POSIX_SOURCE
#      define _POSIX_SOURCE		(1)
#    endif
#  endif
#  ifdef SCO
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
#  ifdef HAVE_TERMIOS_H
#    include <termios.h>
typedef struct termios TERMIO, *PTERMIO;
#  else
#    include <termio.h>
typedef struct termio TERMIO, *PTERMIO;
#  endif
#  ifdef HAVE_SYS_SELECT_H
#    include <sys/select.h>
#  endif
#  ifdef TIME_WITH_SYS_TIME
#    include <sys/time.h>
#    include <time.h>
#  else
#    ifdef HAVE_SYS_TIME_H
#      include <sys/time.h>
#    else
#      include <time.h>
#    endif
#  endif
#  if defined(WINNT)
#    include <sys/select.h>
#    define SHORT_GETTIMEOFDAY	(1)
#pragma warning (disable: 4706 4100 4101)
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
#  include "error.h"
#endif
#include "dumpbuf.h"

/* Useful macros */

#ifndef MAX
#  define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define DEBUG_PRINT_CH(x)	{ \
				if (isprint(x)) \
				   fprintf(debug_fd, "%c", x); \
				else \
				   fprintf(debug_fd, "\\0x%02x", x); \
				debug_need_crlf = 1; \
				}

/* Global variables */

#define DFLT_BREAK_MAX		(3)
#define DFLT_BREAK_TIMER	(1)
char
	*version_id = NULL;
int
	break_max = DFLT_BREAK_MAX,
	break_sigs = DFLT_BREAK_MAX,
	break_char = -1,
	break_timer = DFLT_BREAK_TIMER;
int
	stdin_fd = 0,
	stdin_tty = 0;
tBoolean
	done = FALSE,
	send_break = FALSE;
int
	debug = 0,
	debug_need_crlf = 0;
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
TERMIO
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

static char *asc_logvalue[] =
{
  "<nul>", "<soh>", "<stx>", "<etx>", "<eot>", "<enq>", "<ack>",
  "<bel>", "<bs>", "<ht>", "<lf>", "<vt>", "<ff>", "<cr>",
  "<so>", "<si>", "<dle>", "<dc1>", "<dc2>", "<dc3>", "<dc4>",
  "<nak>", "<syn>", "<etb>", "<can>", "<em>", "<sub>", "<esc>",
  "<fs>", "<gs>", "<rs>", "<us>", "<del>"
};

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

/* Immediate queue for things like status requests, etc. */
#define MAX_IMM_INPUT_QUEUE	(256)
char
	imm_input_queue[MAX_IMM_INPUT_QUEUE],
	*imm_inq_rptr = imm_input_queue,
	*imm_inq_wptr = imm_input_queue;
int
	imm_input_queue_len = 0;

/* Keyboard translate tables */
int
	table_spec = 0;
unsigned char
	in_table[256],
	out_table[256];

/* Logging stuff */
FILE
	*logFd = NULL;
int
	log_type = 0;

/* Miscellaneous stuff */
tBoolean
	generic = FALSE,
	vt100 = FALSE,
	vt52 = FALSE,
	eight_none = FALSE,
	translate = FALSE;
int
	term_type = 10;
tBoolean
	disable_xon_xoff = FALSE;
int32
	first_break_time = 0;

#include "timers.c"

#ifdef DEBUG_TRANSLATE_TABLE
#ifdef __STDC__
void DisplayHex(void *buf, int buf_len, char *dump_id)
#else
void DisplayHex(buf, buf_len, dump_id)
  unsigned char *buf;
  int buf_len;
  char *dump_id;
#endif
{ /*DisplayHex*/

#define CHAR_PER_LINE		(16)
  int
    printOffset = 0,
    offset = 0,
    nChars = CHAR_PER_LINE,
    nLines,
    iLine,
    iChar;
  unsigned char
    *charPtr,
    *ptr,
    *msgPtr,
    msg[81];

  fprintf(stderr, "[ %s ]\n", dump_id);
  if (buf_len < 0)
    {
      buf_len = -buf_len;
      printOffset = 1;
    }
  nLines = buf_len / CHAR_PER_LINE;
#ifdef __STDC__
  charPtr = (unsigned char*)buf;
#else
  charPtr = buf;
#endif
  for (iLine = 0; iLine <= nLines; iLine++)
    {
      if (iLine == nLines)
	nChars = buf_len % CHAR_PER_LINE;
      memset((void*)msg, ' ', 80);
      ptr = &msg[(CHAR_PER_LINE * 3) + 2];
      msgPtr = msg;
      for (iChar = 0; iChar < nChars; iChar++)
	{
	  sprintf((char*)msgPtr, "%02x ", *charPtr);
	  msgPtr += 3;
	  *msgPtr = ' ';
	  *ptr = (unsigned char)
	    ((isprint(*charPtr) && isascii(*charPtr)) ? *charPtr : '#');
	  ++charPtr;
	  *(++ptr) = '\0';
	}
      if (nChars > 0)
	{
	  if (printOffset)
	    {
	      fprintf(stderr, "%04X: ", offset);
	      offset += CHAR_PER_LINE;
	    }
	  fprintf(stderr, "%s\n", msg);
	}
    }
    
} /*DisplayHex*/
#endif /*DEBUG_TRANSLATE_TABLE*/

#ifdef __STDC__
void Logit (int typ, char *ptr, int len, int special_dc1)
#else
void Logit (typ, ptr, len, special_dc1)
  int typ;
  char *ptr;
  int len;
  int special_dc1;
#endif
{ /*Logit*/

#ifdef XHPTERM
  extern int
    logging;

  if (!logging)
    return;
#endif /*XHPTERM*/

  if (log_type & LOG_PREFIX)
    {
      if (typ == LOG_INPUT)
	fprintf (logFd, "in:  ");

      else if (typ == LOG_OUTPUT)
	fprintf (logFd, "out: ");

      else 
	fprintf (logFd, "???: ");
    } 

  while (len--)
    {
      if (((int) *ptr < 32) || ((int) *ptr == 127))
	{
	  int index = (int) *ptr;
	  if (index == 127)
	    index = 33;
	  fprintf (logFd, asc_logvalue[index]);
	  if (index == ASC_LF)
	    putc ('\n', logFd);
	}
      else
	putc ((int)*ptr, logFd);

      if (special_dc1 && (*ptr == ASC_DC1))	/* Ugh */
	putc ('\n', logFd);

      ++ptr;
    }

  putc ('\n', logFd);

  fflush (logFd);

} /* Logit */

#ifndef XHPTERM
#ifdef __STDC__
void PrintUsage(int detail)
#else
void PrintUsage(detail)
  int detail;
#endif
{ /*PrintUsage*/

  printf("freevt3k - version %s", version_id);
#if defined(__DATE__) && defined(__TIME__)
  printf(" (%s-%s)", __DATE__, __TIME__);
#endif
  printf("\n\n");
    
  printf("Usage: freevt3k [-li|-lo|-lio] [-f file] [-x] [-tt n] [-t]\n");
  printf("                [-C breakchar] ");
#ifdef VMS
  printf("[-breaks count] [-breaktimer timer]\n");
  printf("                [-table file] [-a|-I file] [-d[d]] host\n");
#else
  printf("[-B count] [-T timer]\n");
  printf("                [-X file] [-a|-I file] [-d[d]] host\n");
#endif
  if (!detail)
    return;
  printf("   -li|-lo|-lio    - specify input|output logging options\n");
  printf("   -lp             - put a prefix on logging output\n");
  printf("   -f file         - destination for logging [stdout]\n");
  printf("   -x              - disable xon/xoff flow control\n");
  printf("   -tt n           - 'n'->10 (default) generates DC1 read triggers\n");
  printf("   -t              - enable type-ahead\n");
  printf("   -C breakchar    - use 'breakchar' (integer) as break trigger [BREAK or nul]\n");
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
  printf("   -vt100          - emulate hp2392 on vt100 terminals.\n");
  printf("   -vt52           - emulate hp2392 on vt52 terminals.\n");
  printf("   -generic        - translate hp escape sequences to tokens\n");
#ifdef VMS
  printf("   -table file     - specify 256-byte translation table.\n");
#else
  printf("   -X file         - specify 256-byte translation table.\n");
#endif
  printf("   -a file         - read initial commands from file.\n");
  printf("   -I file         - like -a, but stops when end-of-file reached\n");
  printf("   -d[d]           - enable debug output to freevt3k.debug\n");
  printf("   host            - name/IP address of target HP 3000\n");

} /*PrintUsage*/
#endif /*XHPTERM*/

#ifdef __STDC__
int LoadKeybdTable(char *file_name, int i_type)
#else
int LoadKeybdTable(file_name, i_type)
  char *file_name;
  int i_type;
#endif
{ /*LoadKeybdTable*/

  char
    charSpec[256];
  int
    i = -1,
    idx = 0,
    file_num = 0;

  memset((void*)charSpec,0,256);
  if ((file_num = open(file_name, O_RDONLY, 0)) == -1)
    {
      perror("open");
      return(1);
    }
  if (read(file_num, in_table, 256) != 256)
    {
      perror("read");
      return(1);
    }
  close(file_num);
#ifndef DEBUG_TRANSLATE_TABLE
  for (i=0; i<32; i++)
    if (in_table[i] != (unsigned char)i)
      {
	fprintf(stderr, "Cannot change the first 32 values\n");
	return(1);
      }
#endif

  if (i_type == 1)
    {
      for (i=0; i<256; i++)
	{
	  idx = ((int)in_table[i]) & 0x00FF;
	  if (charSpec[idx])
	    {
	      fprintf(stderr, "Translate table contains duplicate entries\n");
	      return(1);
	    }
	  out_table[idx] = (unsigned char)i;
	  charSpec[idx] = 1;
	}
    }
  else
    memcpy(out_table, in_table, 256);
#ifdef DEBUG_TRANSLATE_TABLE
  DisplayHex(in_table, -256, "in");
  DisplayHex(out_table, -256, "out");
#endif
  table_spec = i_type;
  return(0);
    
} /*LoadKeybdTable*/

#ifndef XHPTERM
#ifndef VMS
#ifdef __STDC__
int SetTtyAttributes(int fd, PTERMIO termio_buf)
#else
int SetTtyAttributes(fd, termio_buf)
  int fd;
  PTERMIO termio_buf;
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
int GetTtyAttributes(int fd, PTERMIO termio_buf)
#else
int GetTtyAttributes(fd, termio_buf)
  int fd;
  PTERMIO termio_buf;
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
  TERMIO
    curr_termios,
    temp_termios;
#endif
  char
    ans[32];
    
#ifdef VMS
  SetTTYNormal();
#else
  if (stdin_tty)
    {
      GetTtyAttributes(STDIN_FILENO, &curr_termios);
      temp_termios = old_termios;
      SetTtyAttributes(STDIN_FILENO, &temp_termios);
    }
#endif
  printf("\n");
  for (;;)
    {
      printf("Please enter FREEVT3K command (Exit or Continue) : ");
      fgets(ans, sizeof(ans), stdin);
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
  if (stdin_tty)
    SetTtyAttributes(STDIN_FILENO, &curr_termios);
#endif

} /*ProcessInterrupt*/

#ifdef USE_CTLC_INTERRUPTS
typedef void (*SigfuncInt)(int);

#ifdef __STDC__
void CatchCtlC(int sig_type)
#else
void CatchCtlC(sig_type)
  int sig_type;
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
#endif /*~XHPTERM*/

#ifdef __STDC__
void FlushQ(void)
#else
void FlushQ()
#endif
{ /*FlushQ*/

    input_queue_len = 0;
    inq_rptr = inq_wptr = input_queue;
    imm_input_queue_len = 0;
    imm_inq_rptr = imm_inq_wptr = imm_input_queue;

} /*FlushQ*/

#ifdef __STDC__
int GetQ(void)
#else
int GetQ()
#endif
{ /*GetQ*/

/*
 * Get a byte from the immediate queue if one's present, else
 *   get it from the normal circular queue.
 */
  if (imm_input_queue_len)
    {
      if (++imm_inq_rptr == &imm_input_queue[MAX_IMM_INPUT_QUEUE])
	imm_inq_rptr = imm_input_queue;
      --imm_input_queue_len;
      return(*imm_inq_rptr);
    }
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
  char ch;
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
int PutImmediateQ(char ch)
#else
int PutImmediateQ(ch)
  char ch;
#endif
{ /*PutImmediateQ*/

  if (++imm_inq_wptr == &imm_input_queue[MAX_IMM_INPUT_QUEUE])
    imm_inq_wptr = imm_input_queue;
  if (imm_inq_wptr == imm_inq_rptr)
    {
      fprintf(stderr, "<immediate queue overflow>\n");
      return(-1);
    }
  ++imm_input_queue_len;
  *imm_inq_wptr = ch;
  return(0);

} /*PutImmediateQ*/

#ifdef __STDC__
tBoolean AltEol(tVTConnection *conn, char ch)
#else
tBoolean AltEol(conn, ch)
  tVTConnection *conn;
  char ch;
#endif
{ /*AltEol*/

/*
 * 961126: Don't check for alt eol if in unedited mode
 */
  if ((conn->fUneditedMode) || (conn->fBinaryMode))
    return(FALSE);
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
  tVTConnection *conn;
  char ch;
#endif
{ /*PrimEol*/

/*
 * 961126: Don't check for prim eol if in binary mode
 */
  if (conn->fBinaryMode)
    return(FALSE);
  if (ch == conn->fLineTerminationChar)
    return(TRUE);
  return(FALSE);

} /*PrimEol*/

#ifdef __STDC__
int ProcessQueueToHost(tVTConnection *conn, int len)
#else
int ProcessQueueToHost(conn, len)
  tVTConnection *conn;
  int len;
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
	    {
	      char ch1 = ch;
	      if (table_spec == 1)
		ch1 = in_table[((int)ch1) & 0x00FF];
	      conn->fDataOutProc(conn->fDataOutRefCon, (void*)&ch1, 1);
	    }
	  if ((conn->fSubsysBreakEnabled) &&
/*
 * 961126: Don't check for ctl-y if in binary mode
 */
	      (!(conn->fBinaryMode)) &&
	      (ch == conn->fSubsysBreakChar))
	    send_index = kDTCCntlYIndex;
#ifdef TRANSLATE_INPUT
	  if ((translate) && (ch == '~') && (input_rec[0] == ASC_ESC))
	    vt_fkey = TRUE;
	  else
#endif
	  if (conn->fDriverMode != kDTCBlockMode)
	    prim = PrimEol(conn, ch);
	  alt = AltEol(conn, ch);
/*
	    if (debug)
		{
		extern FILE *debug_fd;
		fprintf(debug_fd,
			"ch=%02x, alt=%d, prim=%d, mode=%d, UneditedMode=%d, LTC=%02x, ALTC=%02x\n",
			ch, alt, prim,
			conn->fDriverMode,
			conn->fUneditedMode,
			conn->fLineTerminationChar,
			conn->fAltLineTerminationChar);
		debug_need_crlf = 0;
		}
 */
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
		    {
		      if (!prim) /* Echo cr if read didn't include one */
			conn->fDataOutProc(conn->fDataOutRefCon,
					   (void*)&cr, 1);
		      conn->fDataOutProc(conn->fDataOutRefCon,
					 (void*)&lf, 1);
		    }
		}

	      if (log_type & LOG_INPUT)
		Logit (LOG_INPUT, input_rec, input_rec_len, FALSE);
		    
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
/*
 * Do input translation here
 */
      if (table_spec == 1)
	for (send_index=0; send_index<input_rec_len; send_index++)
	  input_rec[send_index] = in_table[((int)input_rec[send_index]) & 0x00FF];
      whichError = VTSendData(conn, input_rec, input_rec_len, comp_mask);
    }
  else
    whichError = VTSendBreak(conn, send_index);
  if (whichError)
    {
      char	messageBuffer[128];
      VTErrorMessage(conn, whichError,
		     messageBuffer, sizeof(messageBuffer));
      fprintf(stderr, "Unable to send to host:\n%s\n", messageBuffer);
      return(-1);
    }

  conn->fReadInProgress = FALSE;
  input_rec_len = 0;
  return(0);

}/*ProcessQueueToHost*/

#ifndef XHPTERM
#ifdef __STDC__
int ProcessSocket(tVTConnection * conn)
#else
int ProcessSocket(conn)
  tVTConnection *conn;
#endif
{/*ProcessSocket*/

  int
    whichError = 0;
  static char trigger[] = { ASC_DC1 };                                   /* JCM 040296 */
    
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
      VTErrorMessage(conn, whichError,
		     messageBuffer, sizeof(messageBuffer));
      fprintf(stderr, "VT error:\r\n%s\r\n", messageBuffer);
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
  tVTConnection *conn;
  char *buf;
  int len;
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
	{
	  fprintf(debug_fd, "read: ");
	  debug_need_crlf = 1;
	}
#ifdef VMS
#  ifndef BREAK_VIA_SIG
      if (*buf == (conn->fSysBreakChar & 0xFF))
	{ /* Break */
	  send_break = TRUE;
/* Check for consecutive breaks - 'break_max'-in-a-row to get out */
	  if (debug > 1)
	    DEBUG_PRINT_CH(*buf);
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
	    DEBUG_PRINT_CH(*buf);
	  break_sigs = break_max;
	  if ((type_ahead) || (conn->fReadInProgress))
	    {
	      if (PutQ(*buf) == -1)
		return(-1);
	    }
	}
#else /* VMS */
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
		    {
		      if (debug_need_crlf)
			{
			  fprintf(debug_fd, "\n");
			  debug_need_crlf = 0;
			}
		    }
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
#  ifndef BREAK_VIA_SIG
	  if (((break_char != -1) && (*buf == (char)break_char)) ||
	      ((break_char == -1) && (*buf == (conn->fSysBreakChar & 0xFF))))
	    { /* Break */
	      send_break = TRUE;
/* Check for consecutive breaks - 'break_max'-in-a-row to get out */
	      if (debug > 1)
		{
		  if (debug_need_crlf)
		    fprintf(debug_fd, "\n");
		  fprintf(debug_fd, "break: ");
		  DEBUG_PRINT_CH(*buf);
		}
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
#  endif
	  if (debug > 1)
	    DEBUG_PRINT_CH(*buf);
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
		{
		  fprintf(debug_fd, " len\n");
		  debug_need_crlf = 0;
		}
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
int OpenTTY(PTERMIO new_termio, PTERMIO old_termio)
#else
int OpenTTY(new_termio, old_termio)
  PTERMIO new_termio;
  PTERMIO old_termio;
#endif
{ /*OpenTTY*/

  int
    posix_vdisable = 0,
    fd = 0;

  if (isatty(STDIN_FILENO))
    stdin_tty = 1;

  fd = STDIN_FILENO;
  if (!stdin_tty)
    return(fd);

  if (GetTtyAttributes(fd, old_termio))
    {
      fprintf(stderr, "Unable to get terminal attributes.\n");
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
  if (eight_none)
    {
      new_termio->c_cflag &= ~(CSIZE | PARENB | CSTOPB);
      new_termio->c_cflag |= (CS8 | CREAD);
    }
#ifdef BREAK_VIA_SIG
/* Break handling */
  new_termio->c_iflag &= ~IGNBRK;
  new_termio->c_iflag |= BRKINT;
#endif /*BREAK_VIA_SIG*/
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
void CloseTTY(int fd, PTERMIO old_termio)
#else
void CloseTTY(fd, old_termio)
  int fd;
  PTERMIO old_termio;
#endif
{ /*CloseTTY*/

  if (stdin_tty)
    SetTtyAttributes(fd, old_termio);
  if (fd != STDIN_FILENO)
    close(fd);

} /*CloseTTY*/
#endif /*~VMS*/

#ifdef __STDC__
int DoMessageLoop(tVTConnection * conn)
#else
int DoMessageLoop(conn)
  tVTConnection *conn;
#endif
{ /*DoMessageLoop*/
  int
    whichError,
    readCount,
    returnValue = 0;
#ifndef VMS
  struct timeval
    timeout,
    *time_ptr;
  fd_set
    readfds;
  TERMIO
    new_termios;
  tBoolean
    oldTermiosValid = FALSE;
  int
    nfds = 0;
  char
    termBuffer[2];
#else
  int32
    timeout = 0;
  char
    termBuffer[2048];
#endif
  /*  tBoolean    vtOpen; */
  int32
    start_time = 0,
    read_timer = 0,
    time_remaining = 0;
  tBoolean
    timed_read = FALSE;
  char
    messageBuffer[128];
  int
    vtSocket;
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
  if (stdin_tty)
    nfds = 1 + MAX(stdin_fd, vtSocket);
  else
    nfds = 1 + vtSocket;
#endif
  while (!done)
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
	    {
	      fprintf(debug_fd, "timer: %d\n", timeout);
	      debug_need_crlf = 0;
	    }
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
      if (stdin_tty)
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
	    {
	      fprintf(debug_fd, "timer: %d.%06d\n", timeout.tv_sec, timeout.tv_usec);
	      debug_need_crlf = 0;
	    }
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
	  fprintf(stderr, "Error on select: %d.\n", errno);
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
  int32 refCon;
  char *buffer;
  int bufferLength;
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
    Logit (LOG_OUTPUT, buffer, bufferLength, TRUE);

#ifdef VMS
  io_status = sys$qiow(0,		/* event flag */
		       termNum,		/* channel */
		       IO$_WRITEVBLK,	/* function */
		       &iosb,		/* i/o status block */
		       0,		/* astadr */
		       0,		/* astprm */
		       buffer,		/* P1, char buffer */
		       bufferLength,	/* P2, length */
		       0,		/* P3, fill */
		       0,		/* P4, fill */
		       0,		/* P5, fill */
		       0);		/* P6, fill */
  if ((VMSerror(io_status)) || (VMSerror(iosb.status)))
    ExitProc("sys$qiow", "IO$_WRITEVBLK", 1);
#else
  write(STDOUT_FILENO, buffer, bufferLength);
#endif

} /*vt3kDataOutProc*/

#  include "hpvt100.c"

#ifdef __STDC__
int main(int argc, char *argv[])
#else
int main(argc, argv)
  int argc;
  char *argv[];
#endif
{ /*main*/
  long
    ipAddress;
  int
    ipPort = kVT_PORT;
  struct hostent
    *theHost;
  tVTConnection
    *conn;
  tBoolean
    parm_error = FALSE;
  int
    vtError,
    returnValue = 0;
  char
    messageBuffer[128],
    *hostname = NULL,
    *input_file = NULL,
    *log_file = NULL,
    *ptr;

  version_id = strpbrk(Sccsid, ":")+2;

  if (argc < 2)
    {
      PrintUsage(1);
#ifdef VMS
      exit(STS$K_WARNING);
#else
      return(2);
#endif
    }

#ifdef VMS
  eight_none = TRUE;
#endif
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
	  stop_at_eof = (tBoolean)(!strcmp(*argv, "-I"));
	  if (--argc)
	    {
	      ++argv;
	      if (*argv[0] == '-')
		parm_error = TRUE;
	      else
		input_file = *argv;
	    }
	  else
	    parm_error = TRUE;
	}
      else if (!strcmp(*argv, "-8"))
	eight_none = TRUE;
      else if (!strcmp(*argv, "-7"))
	eight_none = FALSE;
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
	      if (*argv[0] == '-')
		parm_error = TRUE;
	      else
		log_file = *argv;
	    }
	  else
	    parm_error = TRUE;
	}
      else if ((strcmp(*argv, "-X") == 0) ||
	       (strcmp(*argv, "-otable") == 0) ||
	       (strcmp(*argv, "-table") == 0))
	{
	  char *file_name;
	  int i_type = 1;
	  if (strcmp(*argv, "-otable") == 0)
	    ++i_type;
	  if (--argc)
	    {
	      ++argv;
	      if (*argv[0] == '-')
		parm_error = TRUE;
	      else
		{
		  file_name = *argv;
		  if (LoadKeybdTable(file_name, i_type))
		    return(1);
		}
	    }
	  else
	    parm_error = TRUE;
	}
      else if (!strcmp(*argv, "-p"))
	{
	  if (--argc)
	    {
	      ++argv;
	      if (*argv[0] == '-')
		parm_error = TRUE;
	      else
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
	      if (*argv[0] == '-')
		parm_error = TRUE;
	      else
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
	      else if (*ptr == 'p')
		log_type |= LOG_PREFIX;
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
	      if (*argv[0] == '-')
		parm_error = TRUE;
	      else
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
	      if (*argv[0] == '-')
		parm_error = TRUE;
	      else
		break_timer = atoi(*argv);
	    }
	  else
	    parm_error = TRUE;
	}
      else if ((!strcmp(*argv, "-C")) ||
	       (!strcmp(*argv, "-breakchar")))
	{
	  if (--argc)
	    {
	      ++argv;
	      if (*argv[0] == '-')
		parm_error = TRUE;
	      else
		{
		  break_char = atoi(*argv) & 0x00FF;
		  if (!break_char)
		    break_char = -1;
		}
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

  if (!hostname)
    {
      PrintUsage(0);
      return(2);
    }

  if (log_file)
    {
      if ((logFd = fopen(log_file, "w")) == (FILE*)NULL)
	{
	  perror("fopen");
	  return(1);
	}
    }
/* vt3k doesn't work this way, although documented to do so */
  else if (log_type != 0)
    logFd = stdout;

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
	  fprintf(stderr, "Unable to resolve %s.\n", hostname);
	  return(1);
	}
      memcpy((char *) &ipAddress, theHost->h_addr, sizeof(ipAddress));
    }

  conn = (tVTConnection *) calloc(1, sizeof(tVTConnection));
  if (conn == NULL)
    {
      fprintf(stderr, "Unable to allocate a connection.\n");
      return(1);
    }

  if (vtError = VTInitConnection(conn, ipAddress, ipPort))
    {
      VTErrorMessage(conn, vtError,
		     messageBuffer, sizeof(messageBuffer));
      fprintf(stderr, "Unable to initialize the connection.\n%s\n", messageBuffer);
      VTCleanUpConnection(conn);
      return(1);
    }

  if (term_type == 10)
    conn->fBlockModeSupported = TRUE;	/* RM 960411 */
  
  conn->fDataOutProc =
    ((vt100) ? vt3kHPtoVT100 :
     ((vt52) ? vt3kHPtoVT52 :
      ((generic) ? vt3kHPtoGeneric: vt3kDataOutProc)));

  if (vtError = VTConnect(conn))
    {
      VTErrorMessage(conn, vtError,
		     messageBuffer, sizeof(messageBuffer));
      fprintf(stderr, "Unable to connect to host.\n%s\n", messageBuffer);
      VTCleanUpConnection(conn);
      return(1);
    }

  if (stdin_tty)
    {
      char break_desc[32];
      
      if (break_char == -1)
	sprintf(break_desc, "break");
      else if (isprint((char)break_char))
	sprintf(break_desc, "%c", break_char);
      else if (break_char < ' ')
	sprintf(break_desc, "ctl-%c", break_char+'@');
      else
	sprintf(break_desc, "0x%02X", break_char);
      printf("To suspend to FREEVT3K command mode press '%s' %d times in a %d second period.\n",
	     break_desc, break_max, break_timer);
      printf("To send a Break, press '%s' once.\n\n", break_desc);
    }

  break_timer *= 1000;	/* Convert to ms */

  returnValue = DoMessageLoop(conn);

  VTCleanUpConnection(conn);

#ifdef VMS
  exit((returnValue) ? STS$K_WARNING : STS$K_SUCCESS);
#else
  return(returnValue);
#endif

} /*main*/
#endif /*~XHPTERM*/
