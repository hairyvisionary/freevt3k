/* Copyright (C) 1996 Office Products Technology, Inc.

   vt3kglue.c  --  Interface between character based terminal
		   emulator and record based vt3k protocol

   freevt3k: a VT emulator for Unix.

   This file is distributed under the GNU General Public License.
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software Foundation,
   Inc., 675 Mass Ave, Cambridge MA 02139, USA.

   Based on freevt3k.c

   Original: Bruce Toback, 22 MAR 96
   Additional: Dan Hollis <dhollis@pharmcomp.com>, 27 MAR 96
               Randy Medd <randy@telamon.com>, 28 MAR 96
	       Jeff Moffatt <jeff@taurus.com>, 31 MAR 96
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
#  ifdef __hpux
#    ifndef _HPUX_SOURCE
#      define _HPUX_SOURCE (1)
#    endif
#    ifndef _POSIX_SOURCE
#      define _POSIX_SOURCE (1)
#    endif
#  endif

#  include <sys/types.h>
#  include <unistd.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <string.h>
#  include <errno.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <fcntl.h>
#  include <termios.h>
#  include <sys/types.h>
#  include <sys/time.h>
#  include <signal.h>
#endif
#include "typedef.h"
#include "vt.h"
#include "freevt3k.h"
#include "vtconn.h"

#include "conmgr.h"

/* Useful macros */

#ifndef MAX
#  define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

/* Global variables */



tVTConnection * open_vt3k_connection (char *hostname)
{
    long  ipAddress;
    int   ipPort = kVT_PORT;
    struct hostent * theHost;
    tVTConnection * theConnection;
    int             vtError;
    char	messageBuffer[128];
    int         term_type = 10;

    /* First, validate the destination. If the destination can be	*/
    /* validated, create a connection structure and try to open the     */
    /* connection.							*/

    ipAddress = inet_addr(hostname);
    if (ipAddress == INADDR_NONE)
	{
	theHost = gethostbyname(hostname);
	if (theHost == NULL)
	    {
            printf("Unable to resolve %s.\n", hostname);
	    theConnection = NULL;
	    goto Last;
            }
        memcpy((char *) &ipAddress, theHost->h_addr, sizeof(ipAddress));
	}

    theConnection = (tVTConnection *) calloc(1, sizeof(tVTConnection));
    if (theConnection == NULL)
        {
        printf("Unable to allocate a connection.\n");
        goto Last;
        }

    if (vtError = VTInitConnection(theConnection, ipAddress, ipPort))
        {
        printf("Unable to initialize the connection.\n");
        VTErrorMessage(theConnection, vtError,
				messageBuffer, sizeof(messageBuffer));
        printf("%s\n", messageBuffer);
	free (theConnection);
	theConnection = NULL;
        goto Last;
        }

    printf("Connection initialized.\n");

    if (term_type == 10) {
	theConnection->fBlockModeSupported = TRUE;      /* RM 960411 */
    }

    theConnection->fDataOutProc = conmgr_rxfunc;

    if (vtError = VTConnect(theConnection))
	{
        printf("Unable to connect to host.\n");
        VTErrorMessage(theConnection, vtError,
				messageBuffer, sizeof(messageBuffer));
        printf("%s\n", messageBuffer);
	VTCleanUpConnection(theConnection);
	free (theConnection);
	theConnection = NULL;
        goto Last;
	}
Last:
    return (theConnection);
}


int read_vt3k_data (tVTConnection * theConnection) {

    int whichError;
    tBoolean vtOpen=0;
    tBoolean done=0;
    char messageBuffer[128];
    static char trigger[] = { 17 };
    int eof=0;


    whichError = VTReceiveDataReady(theConnection);
    if (whichError == kVTCVTOpen)
	{
	/* Now the connection is _really_ open */
	vtOpen=1;
	}
    else if (whichError != kVTCNoError) {
	{
	if (whichError == kVTCStartShutdown) {
            done=TRUE;
            eof=1;
	} else
	    {
	    printf ("VT error!:\n");
	    VTErrorMessage(theConnection, whichError,
			  messageBuffer, sizeof(messageBuffer));
            printf ("%s\n", messageBuffer);
	    done=TRUE;
	    }
        }
    }
/*
 *  Check for start of a new read
 */
    if (!done && theConnection->fReadStarted) {
	theConnection->fReadStarted = FALSE;
/*
 *      Check for need to flush the type-ahead buffer
 */
	if (theConnection->fReadFlush)  /* RM 960403 */
	    {
	    theConnection->fReadFlush = FALSE;
	    FlushQ();
	    }
/*
 *      Send read trigger that is needed by hpterm
 */
	theConnection->fDataOutProc (theConnection->fDataOutRefCon,
				     trigger, sizeof(trigger));
/*
 *      As we just got a read request, check for any typed-ahead data and
 *      process it now.
 */
	ProcessQueueToHost(theConnection, 0);
    }
    return (eof);
}
/**********************************************************************/
int send_vt3k_data (tVTConnection * theConnection, char *buf, int nbuf) {

    int ii;
    char ch;

    for (ii=0; ii<nbuf; ii++) {
	ch = buf[ii];
	if (PutQ(ch) == -1) break;
    }

    if (theConnection->fReadInProgress) {
        ProcessQueueToHost(theConnection, 0);
    }

    return (0);
}
/*********************************************************************/
void send_vt3k_break (tVTConnection * theConnection) {

    theConnection->fSysBreakEnabled = 1;   /* Should not be needed */
    ProcessQueueToHost(theConnection, -2);
}
/*********************************************************************/
void close_vt3k (tVTConnection * theConnection) {

    VTCleanUpConnection(theConnection);
    free (theConnection);
}
/*********************************************************************/
