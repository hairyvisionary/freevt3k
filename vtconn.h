/* Copyright (C) 1996 Office Products Technology, Inc.

   freevt3k: a VT emulator for Unix. 

   vtconn.h      

   External and connection structure definitions.

   This file is distributed under the GNU General Public License.
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software Foundation,
   Inc., 675 Mass Ave, Cambridge MA 02139, USA. 

   Original: Bruce Toback, 22 MAR 96
*/

#ifndef _VTCONN_H
#define _VTCONN_H

/* Connection constants */

#define kVT_PORT	1570
#define kVT_MAX_BUFFER	24576

/* Connection structure. This structure manages everything to do with
   a single connection. It could easily be encapsulated into a C++ class,
   but for the moment, this is more portable. 
*/

#define kMaxLineDeleteEcho	23		/* Long enough, no? */

#ifdef __STDC__
typedef void tVTDataOutProc(int32 refCon, char * outBuffer, int bufferLength);
typedef tVTDataOutProc * tVTDataOutProcPtr;
#else
typedef void tVTDataOutProc();
typedef tVTDataOutProc * tVTDataOutProcPtr;
#endif

typedef enum etVTState
{
    kvtsUninitialized		= 0,
    kvtsClosed	        	= 1,
    kvtsWaitingForAM    	= 2, 
    kvtsWaitingForTMReply	= 3,
    kvtsOpen			= 4,
    kvtsWaitingForCloseReply	= 5
} tVTState;

typedef struct stVTConnection
{
    tVTState		fState;
    struct sockaddr_in	fTargetAddress;
    int			fSocket;
    int			fLastSocketError;	/* Set when error occurs. */

    char *		fSendBuffer;		/* Data to be sent */
    char *		fReceiveBuffer;		/* Data from VT host */
    int			fSendBufferSize;
    int			fReceiveBufferSize;
    int			fLengthToReceive;	/* Length left in cur rec */
    unsigned16		fLengthWord;		/* Length word being rec'd */
    int			fReceiveBufferOffset;	/* Where to put next block */
    int			fSendBufferOffset;	/* Where to put next in char */
    tBoolean		fReadInProgress;	/* TRUE when OK to read */
    tBoolean		fReadStarted;		/* TRUE when read initiated */
    tBoolean		fReadFlush;		/* Flush type-ahead? */
    unsigned16          fReadRequestCount;      /* From orignl read req */
    int			fReadBufferOffset;	/* Where to put next term char*/
    int			fReadLength;		/* Length of current read */
    unsigned16		fCurrentTMRequestCount;	/* For sequencing */

    /* Terminal state variables */

    char		fEcho;			/* What is this? */
    char		fLineDeleteChar;	/* Usu. ^X */
    char		fCharDeleteChar;	/* Usu. ^H, mapped to 0x7f */
    tBoolean		fDisableLineDeleteEcho; /* The FSETMODE, I think   */
    int			fLineDeleteEchoLength;	/* Len. of line del string */
    char		fLineDeleteEcho[kMaxLineDeleteEcho + 1];
    int			fCharDeleteEcho;	/* What to echo on char del */
    int			fEchoControl;		/* Type echo control */
    char                fLineTerminationChar;   /* Normally ^M */
    char                fAltLineTerminationChar;   /* Normally undef */
    int			fDriverMode;		/* Block/User/Char? */

    /* These control timed reads. When a timed read is started, the */
    /* current time is stored in fReadStartTime, and a calculated   */
    /* end time is stored in fReadEndTime. The difference between   */
    /* these is used to calculate a timeout value for the next      */
    /* single-character read.                                       */

    int			fReadTimeout;		/* In seconds, 0 for infinite*/
    int			fReadStartTime;		/* In seconds from base */
    int			fReadEndTime;		/* Time at timeout */

    /* Not sure how to handle system break at this point. Probably with */
    /* a command, but that seems clumsy. 				*/
    /* In any case, it has to be system-dependent because we can't be	*/
    /* sure whether we'll have a fancy keyboard or not.			*/

    int			fSysBreakChar;		/* -1 for none */
    int			fSubsysBreakChar;	/* -1 for none */
    tBoolean		fSysBreakEnabled;
    tBoolean		fSubsysBreakEnabled;
    char		fNoBreakRead;		/* What is this? */

    /* Per normal HP 3000 conventions. */

    tBoolean		fUneditedMode;
    tBoolean		fBinaryMode;

    /* Flags that get set on prompts and reads. */

    tBoolean		fEchoCRLFOnCR;  	/* For the current read */
    int			fPromptLength;		/* For current read.	*/

    /* Misc flags */

    char		fTermType;		/* Terminal type	*/
    char		fTypeAhead;		/* Typeahead enabled	*/
    tBoolean		fBlockModeSupported;	/* Ok for block mode forms ? */

    /* The data-out proc. The default just dumps stuff onto the terminal */

    tVTDataOutProcPtr	fDataOutProc;
    int32		fDataOutRefCon;
} tVTConnection;

/* Error codes returned from VT routines */

#define kVTCNoError			0
#define kVTCMemoryAllocationError	1
#define kVTCSocketError			2
#define kVTCCantAllocateSocket		3
#define kVTCNotInitialized		4
#define kVTCReceiveRecordLengthError	5
#define kVTCReceivedInvalidProtocolID   6
#define kVTCReceivedInvalidControlPrimitive 7
#define kVTCReceivedInvalidMessageType  8
#define kVTCSendError                   9
#define kVTCRejectedTMNegotiation       10
#define kVTCVTOpen			11
#define kVTCStartShutdown               12
#define kVTCReceivedInvalidIOPrimitive  13
#define kVTCReceivedUnexpectedIOResp    14
#define kVTCReceivedUnexpectedControlResp 15
#define kVTCReceivedUnexpectedAppCtlReq 16

/* Prototypes */

void FlushQ P((void));
int  GetQ P((void));
int  PutQ P((char));
void VTErrorMessage P((tVTConnection * conn, int code, char * msg, int maxLen));
int  VTInitConnection P((tVTConnection * conn, long ipAddress, int ipPort));
void VTCleanUpConnection P((tVTConnection * conn));
int  VTConnect P((tVTConnection * conn));
int  VTReceiveDataReady P((tVTConnection * conn));
int  VTProcessKeyBuffer P((tVTConnection * conn, char * buffer, int length));
int  VTSetDataOutProc P((tVTConnection * conn, tVTDataOutProcPtr dataOutProc));
int  VTCloseConnection P((tVTConnection * conn));
int  VTSocket P((tVTConnection * conn));
int  VTSendBreak P((tVTConnection * conn, int send_index));
int  VTSendData P((tVTConnection * conn, char * buffer, int length, int flags));
int  ProcessQueueToHost P((tVTConnection *conn, int len));
#endif

