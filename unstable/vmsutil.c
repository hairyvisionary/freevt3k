/*
 * @(#) vmsutil.c: 94/10/13-08:47:00
 */

#include <types.h>
#include <stdio.h>
#include <unixio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <timeb.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <file.h>
#include <signal.h>
#include <assert.h>
#include <stat.h>
#include <dcdef.h>
#include <ssdef.h>
#include <iodef.h>
#include <msgdef.h>
#include <ttdef.h>
#include <tt2def.h>
#include <stsdef.h>
#include <descrip.h>
#include <setjmp.h>
#include <psldef.h>
#include <prvdef.h>
#include <syidef.h>
#include <rms.h>
#include <socket.h>
#include <in.h>
#include <netdb.h>
#include <inet.h>
#include <lib$routines.h>
#include <starlet.h>
#include <ucx$inetdef.h>
#include "vmstypes.h"

#define P(x)			x

#include "error.h"

TT_READ_IOSB
	term_iosb;
TT_READ_IOSB
	sock_iosb;
TRM_CHR
	old_sc_buf;
TRM_CHR
	new_sc_buf;
unsigned short
	termNum = 0;
extern unsigned int
	readMask,
	efnMask,
	sockBit,
	sockEfn,
	termEfn,
	termBit;
extern int
	eight_none,
	termReadPending,
	sockReadPending;

int GetEventFlag(unsigned int *efn, unsigned int *bit, unsigned int *mask)

{ /*GetEventFlag*/
/*
 * Allocate an event flag and set bit in mask
 */
    if (VMSerror(lib$get_ef(efn)))
	return(-1);
    if (bit != 0)
	{
	*bit = (1 << (*efn % 32));
	if (mask != 0)
	    *mask |= *bit;
	}
    return(0);
    
} /*GetEventFlag*/

int FreeEventFlag(unsigned int *efn, unsigned int *bit, unsigned int *mask)

{ /*FreeEventFlag*/

/*
 * Free an event flag and clear bit in mask
 */
    if (*efn)
	{
	if (VMSerror(lib$free_ef(efn)))
	    return(-1);
	}
    if (bit != 0)
	{
	if (mask != 0)
	    *mask &= ~(*bit);
	*bit = 0;
	}
    return(0);

} /*FreeEventFlag*/

#ifdef __STDC__
void ExitProc(char *text1, char *text2, int exit_code)
#else
void ExitProc(text1, text2, exit_code)

    char
	*text1,
	*text2;
    int
	exit_code;
#endif

{ /*ExitProc*/

    char
	buf[256];

    if (*text1)
	{
	strcpy(buf, text1);
	if (*text2)
	    {
	    strcat(buf, ": ");
	    strcat(buf, text2);
	    }
	if (errno)
	    {
	    strcat(buf, ": ");
	    strcat(buf, PortableStrerror(errno));
	    }
	fprintf(stderr, "freevt3k: %s\n", buf);
	}
    
    exit((exit_code) ? STS$K_SUCCESS : STS$K_WARNING);

} /*ExitProc*/

void SetTTYNormal(void)

{ /*SetTTYNormal*/

#ifdef ONE_AT_A_TIME
    TT_SENSE_IOSB
	sense_iosb;
    int
	io_status = 0;

    io_status = sys$qiow(termEfn,	/* event flag */
			 termNum,	/* channel */
			 IO$_SETMODE,	/* function */
			 &sense_iosb,	/* i/o status block */
			 0,		/* astadr */
			 0,		/* astprm */
			 &old_sc_buf,	/* P1, char buffer */
			 sizeof(old_sc_buf),/* P2, length */
			 0,		/* P3, baud rate */
			 0,		/* P4, fill */
			 0,		/* P5, parity */
			 0);		/* P6, fill */
    if ((VMSerror(io_status)) || (VMSerror(sense_iosb.status)))
	ExitProc("sys$qiow", "IO$_SETMODE", 1);
#endif

} /* SetTTYNormal */

void SetTTYRaw(void)

{ /*SetTTYRaw*/

#ifdef ONE_AT_A_TIME
    TT_SENSE_IOSB
	sense_iosb;
    int
	io_status = 0;

    io_status = sys$qiow(termEfn,	/* event flag */
			 termNum,	/* channel */
			 IO$_SETMODE,	/* function */
			 &sense_iosb,	/* i/o status block */
			 0,		/* astadr */
			 0,		/* astprm */
			 &new_sc_buf,	/* P1, char buffer */
			 sizeof(new_sc_buf),/* P2, length */
			 0,		/* P3, baud rate */
			 0,		/* P4, fill */
			 0,		/* P5, parity */
			 0);		/* P6, fill */
    if ((VMSerror(io_status)) || (VMSerror(sense_iosb.status)))
	ExitProc("sys$qiow", "IO$_SETMODE", 1);
#endif

} /* SetTTYRaw */

#ifdef __STDC__
void OpenTTY(void)
#else
void OpenTTY()
#endif

{ /*OpenTTY*/

    struct dsc$descriptor_s
	devDesc;
    TT_SENSE_IOSB
	sense_iosb;
    int
	io_status = 0;
    unsigned int
	efnMask = 0;

/*
 * Assign an I/O channel to the terminal port device
 */
    devDesc.dsc$w_length	= strlen("TT:");
    devDesc.dsc$b_dtype	= DSC$K_DTYPE_T;
    devDesc.dsc$b_class	= DSC$K_CLASS_S;
    devDesc.dsc$a_pointer	= "TT:";
    if (VMSerror(sys$assign(&devDesc, &termNum, PSL$C_USER, 0)))
	ExitProc("sys$assign", "", 1);
/*
 * Read and save the current characteristics.  These should be restored
 *    on process or image exit by the program.
 */
    io_status = sys$qiow(termEfn,		/* event flag */
			 termNum,		/* channel */
			 IO$_SENSEMODE,		/* function */
			 &sense_iosb,		/* i/o status block */
			 0,			/* astadr */
			 0,			/* astprm */
			 &old_sc_buf,		/* P1, char buffer */
			 sizeof(old_sc_buf),	/* P2, length */
			 0,			/* P3, fill */
			 0,			/* P4, fill */
			 0,			/* P5, fill */
			 0);			/* P6, fill */
    if ((VMSerror(io_status)) || (VMSerror(sense_iosb.status)))
	ExitProc("sys$qiow", "IO$_SENSEMODE", 1);
    new_sc_buf = old_sc_buf;
/*
 * Allocate an event flag for TTY
 */
    if (GetEventFlag(&termEfn, &termBit, &efnMask) == -1)
	ExitProc("lib$get_ef", "", 1);
#ifdef ONE_AT_A_TIME
/*
 * Configure TTY port
 */
/*
 * Setup the terminal port for PASTHRU, EIGHTBIT (no parity), HOSTSYNC
 *    and NOBRDCST
 */
    new_sc_buf.extended |= TT2$M_PASTHRU | TT2$M_ALTYPEAHD;
    new_sc_buf.basic &= ~TT$M_ESCAPE;		/* Disable escape-seq processing */
    new_sc_buf.extended &= ~TT2$M_LOCALECHO;	/* and local echoing */
    new_sc_buf.basic |= TT$M_NOECHO;
    if (eight_none)
	new_sc_buf.basic |= TT$M_EIGHTBIT;
    SetTTYRaw();
#endif

    if (VMSerror(sys$clref(termEfn)))
	ExitProc("sys$clref", "", 1);
    readMask |= termBit;

} /*OpenTTY*/

void CloseTTY(void)

{ /*CloseTTY*/

/* Cancel I/O requests */
    if (VMSerror(sys$cancel(termNum)))
	ExitProc("sys$cancel", "", 1);
#ifdef ONE_AT_A_TIME
    SetTTYNormal();
    readMask &= ~termBit;
#endif
/*
 * Free event flag
 */
    if (FreeEventFlag(&termEfn, &termBit, &readMask) == -1)
	ExitProc("lib$free_ef", "", 1);

} /* CloseTTY */

#ifdef ONE_AT_A_TIME
void StartTTYRead(unsigned char *buf, int echo, int timeout)

{ /*StartTTYRead*/

#define READ_LEN			(1)
    TERMINATOR_MASK
	termMask;
    static int
	trmmsk[2] = {0,0};
    static int
	trmlong[8] = {0,0,0,0,0,0,0,0};
    int
	ioMask = (IO$_READVBLK /*| IO$M_PURGE*/ | IO$M_NOFILTR | IO$M_NOECHO),
	io_status;

    if (termReadPending)
	return;

    if (VMSerror(sys$clref(termEfn)))
	ExitProc("sys$clref", "", 1);

    if (timeout)
	ioMask |= IO$M_TIMED;

    termMask.nil = 0;
    termMask.ctrl_char = 0;
    trmmsk[0] = sizeof(trmlong);	/* No terminators */
    trmmsk[1] = (int)&trmlong;		/* Keep all characters */
/*
 * Read a record
 */
    if (!termReadPending)
	{
	io_status = sys$qio(termEfn,	/* event flag */
			    termNum,	/* channel */
			    ioMask,	/* function */
			    &term_iosb,	/* i/o status block */
			    0,		/* astadr */
			    0,		/* astprm */
			    buf,	/* P1, char buffer */
			    READ_LEN,	/* P2, length */
			    timeout,	/* P3, timeout */
			    &trmmsk,	/* P4, terminator mask */
			    0,		/* P5, prompt string (DC1) */
			    0);		/* P6, length of prompt string */
	if (VMSerror(io_status))
	    ExitProc("sys$qio_READ", "term", 1);
	termReadPending = 1;
	}

} /*StartTTYRead*/

int CompleteTTYRead(unsigned char *buf)

{ /*CompleteTTYRead*/

    if (VMSerror(sys$clref(termEfn)))
	ExitProc("sys$clref", "", 1);
/*
 * Process a byte
 */
    termReadPending = 0;
    if (term_iosb.status == SS$_TIMEOUT)
	return(-2);
    return(term_iosb.terminator_offset);

} /*CompleteTTYRead*/
#else
void StartTTYRead(unsigned char *buf, int echo, int timeout)

{ /*StartTTYRead*/

#define READ_LEN			(256)
#define ASC_CR				(0x0D)
    TERMINATOR_MASK
	termMask;
    int
	ioMask = (IO$_READVBLK | IO$M_PURGE),
	io_status;

    if (termReadPending)
	{
	if (echo)
	    return;
/* Cancel I/O requests */
	if (VMSerror(sys$cancel(termNum)))
	    ExitProc("sys$cancel", "", 1);
/* Wait for cancellations to complete */
	if (VMSerror(sys$wflor(termEfn, termBit)))
	    ExitProc("sys$wflor", "", 1);
	termReadPending = 0;
	}

    termMask.nil = 0;
    termMask.ctrl_char = (1 << ASC_CR);
    
    if (echo == 0)
	ioMask |= IO$M_NOECHO;

    if (timeout)
	ioMask |= IO$M_TIMED;
    
    if (VMSerror(sys$clref(termEfn)))
	ExitProc("sys$clref", "", 1);
/*
 * Read a record
 */
    if (!termReadPending)
	{
	io_status = sys$qio(termEfn,	/* event flag */
			    termNum,	/* channel */
			    ioMask,	/* function */
			    &term_iosb,	/* i/o status block */
			    0,		/* astadr */
			    0,		/* astprm */
			    buf,	/* P1, char buffer */
			    READ_LEN,	/* P2, length */
			    timeout,	/* P3, timeout */
			    &termMask,	/* P4, terminator mask */
			    0,		/* P5, prompt string (DC1) */
			    0);		/* P6, length of prompt string */
	if (VMSerror(io_status))
	    ExitProc("sys$qio_READ", "", 1);
	termReadPending = 1;
	}

} /*StartTTYRead*/

int CompleteTTYRead(unsigned char *buf)

{ /*CompleteTTYRead*/

#define READ_LEN			(256)
#define ASC_CR				(0x0D)
    TERMINATOR_MASK
	termMask;
    int
	ioMask = IO$_READVBLK,
	io_status;
    unsigned char
	*termPtr;
    int
	retLen,
	termLen = 0,
	done = 0;
    
    termMask.nil = 0;
    termMask.ctrl_char = (1 << ASC_CR);
    
    if (VMSerror(sys$clref(termEfn)))
	ExitProc("sys$clref", "", 1);
/*
 * Read a record
 */
    termReadPending = 0;
    if (term_iosb.status == SS$_TIMEOUT)
	return(-2);
    termLen = term_iosb.terminator_offset;
    termPtr = &buf[termLen];
    if (termLen < READ_LEN)
	{
	*termPtr = '\0';
	done = 1;
	}
    while (!done)
	{
	io_status = sys$qiow(termEfn,	/* event flag */
			     termNum,	/* channel */
			     ioMask,	/* function */
			     &term_iosb,/* i/o status block */
			     0,		/* astadr */
			     0,		/* astprm */
			     termPtr,	/* P1, char buffer */
			     READ_LEN,	/* P2, length */
			     0,		/* P3, timeout */
			     &termMask,	/* P4, terminator mask */
			     0,		/* P5, prompt string */
			     0);	/* P6, length of prompt string */
	if (VMSerror(io_status))
	    ExitProc("sys$qiow_READ", "", 1);
	if (VMSerror(term_iosb.status))
	    ExitProc("sys$qiow_READ", "", 1);
	retLen = term_iosb.terminator_offset;
	termLen += retLen;
	termPtr += retLen;
	if (retLen < READ_LEN)
	    {
	    *termPtr = '\0';
	    done = 1;
	    }
	}
    return(termLen);

} /*CompleteTTYRead*/
#endif

int StartReadSocket(int fd, unsigned char *buf, int len)

{ /*StartReadSocket*/

    short int
	chan = 0;
    int
	options = 0,
	io_status = 0;
    
    chan = vaxc$get_sdc(fd);
    io_status = sys$qio(sockEfn,	/* event flag */
			chan,		/* channel */
			IO$_READVBLK,	/* function */
			&sock_iosb,	/* i/o status block */
			0,		/* astadr */
			0,		/* astprm */
			buf,		/* P1, char buffer */
			len,		/* P2, length */
			0,		/* P3, ... */
			0,		/* P4, ... */
			0,		/* P5, ... */
			0);		/* P6, ... */
    if (VMSerror(io_status))
	return(-1);
    sockReadPending = 1;
    return(0);

} /*StartReadSocket*/

int CompleteReadSocket(void)
{ /*CompleteReadSocket*/

    if (VMSerror(sock_iosb.status))
	return(-1);
    sockReadPending = 0;
    return(sock_iosb.terminator_offset);

} /*CompleteReadSocket*/

int WaitForCompletion(unsigned int *returnMask)
{ /*WaitForCompletion*/

    if (VMSerror(sys$wflor(sockEfn, readMask)))
	return(-1);
    if (VMSerror(sys$readef(sockEfn, returnMask)))
	return(-1);
    return(0);

} /*WaitForCompletion*/

/* Local Variables: */
/* c-indent-level: 0 */
/* c-continued-statement-offset: 4 */
/* c-brace-offset: 0 */
/* c-argdecl-indent: 4 */
/* c-label-offset: -4 */
/* End: */
