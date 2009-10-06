/* Copyright (C) 1996 Office Products Technology, Inc.

   freevt3k: a VT emulator for Unix. 

   vt.h      VT packet definitions.

   This file is distributed under the GNU General Public License.
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software Foundation,
   Inc., 675 Mass Ave, Cambridge MA 02139, USA. 

   Original: Bruce Toback, 22 MAR 96
   Additional: Dan Hollis <dhollis@pharmcomp.com>, 27 MAR 96
               Randy Medd <randy@telamon.com>, 28 MAR 96
*/

#ifndef _VT_H
#define _VT_H

/* This information is primarily from the NS Message Formats Reference
   Manual, 12/92 edition, as modified by experience. The HP part number
   for the manual is 91790-61024. Modifications to the manual are noted
   in the file.
*/

#define kVTProtocolID		2

typedef enum etVTMessageType
{
    kvmtEnvCntlReq		= 0,	/* Environment control	*/
    kvmtEnvCntlResp		= 1,
    kvmtTerminalIOReq		= 2,	/* Terminal I/O */
    kvmtTerminalIOResp		= 3,
    kvmtTerminalCntlReq		= 4,	/* Terminal control */
    kvmtTerminalCntlResp	= 5,
    kvmtApplicationCntlReq	= 6,	/* Application control */
    kvmtApplicationCntlResp	= 7,
    kvmtMPECntlReq		= 8,	/* Messages specific to MPE */
    kvmtMPECntlResp		= 9,
    kvmtGenericFDCReq		= 10,
    kvmtGenericFDCResp		= 11,

    kvmtUnknown			= -1
} tVTMessageType;

/* Environment control primitives */

#define kvtpTMNegotiate		0
#define kvtpAMNegotiate		1
#define kvtpTerminate		2
#define kvtpLogonInfo		3

/* Terminal control primitives */

#define kvtpSetBreakInfo	0
#define kvtpSetDriverInfo	1

/* MPE Specific control primitives */

#define kvtpMPECntl		0
#define kvtpMPEGetInfo		1

/* Generic FDevicecontrol primitives */

#define kvtpDevSet		0
#define kvtpDevGetInfo		1

/* Generic Application primitives */

#define kvtpApplInvokeBreak	0

/* These values are used to fill in the response mask. */

#define kvtRespNoError		0
#define kvtRespFailed		0x8000
#define kvtRespBadMessageFormat	0x4000
#define kvtRespBadPrimitive	0x2000
#define kvtRespBadParameter	0x1000

/* Break control defines */

#define kVTSystemBreak	1
#define kVTSubsysBreak  0

/* Define the standard message header */

#define VT_MESSAGE_HEADER \
		unsigned16	fMessageLength; \
		unsigned8 	fProtocolID; \
		unsigned8	fMessageType; \
		unsigned8	fUnused; \
		unsigned8	fPrimitive

/* Define messages */

typedef struct stVTMHeader
{
    VT_MESSAGE_HEADER;
} tVTMHeader;

/* NOTE: These structures only work with compilers that will allow
   structure members to begin on their own alignment requirements,
   that is, on compilers for which a 16-bit member may be aligned
   on a 16-bit boundary. The structure will NOT work on compilers
   that require 16-bit values to live on 32-bit boundaries.
*/

/*=======================================================================
  AM Negotiation Request/Reply
 *=======================================================================*/

typedef struct stVTMAMNegotiationRequest
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    char		fVersionMask[4];
    unsigned16		fBufferSize;
    unsigned16		fOperatingSystem;	/* A code; see below	*/
    unsigned8		fEcho;
    unsigned8		fEchoControl;
    unsigned8		fCharacterDelete;
    unsigned8		fCharacterDeleteEcho;
    unsigned8		fLineDeleteCharacter;
    unsigned8		fNoBreakRead;
    unsigned16		fTypeAheadSize;
    unsigned8		fTypeAheadLines;
    unsigned8		fParity;
    unsigned16		fBreakOffset;
    unsigned16		fBreakIndexCount;
    unsigned16		fLogonIDOffset;
    unsigned16		fLogonIDLength;
    unsigned16		fDeviceIDOffset;
    unsigned16		fDeviceIDLength;
    unsigned16		fLineDeleteOffset;
    unsigned16		fLineDeleteLength;
    unsigned16		fAMMaxReceiveBurst;
    unsigned16		fAMMaxSendBurst;
    char		fVariable[1];		/* Variable-length	*/
} tVTMAMNegotiationRequest;

typedef struct stVTMAMBreakInfo
{
	unsigned16	fSysBreakEnabled;
	unsigned16	fSubsysBreakEnabled;
	unsigned16	fSysBreakChar;
	unsigned16	fSubsysBreakChar;
} tVTMAMBreakInfo;

/* Character deletion echo values */

#define kAMEchoDontCare		0
#define kAMEchoBackspace	1
#define kAMEchoBSSlash		2
#define kAMEchoBsSpBs		3


/* Echo control */

#define kAMEchoDontCare		0
#define kAMEchoDontAllow	1
#define kAMEchoSwitchOK		2
#define kAMEchoNotifyOnSwitch	3

typedef struct stVTMAMNegotiationReply
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fResponseCode;
    unsigned16		fCompletionMask;
    char		fVersionMask[4];
    unsigned16		fBufferSize;
    unsigned16		fOperatingSystem;
    unsigned16		fHardwareControlCompletionMask;
    unsigned16		fTMMaxReceiveBurst;
    unsigned16		fTMMaxSendBurst;
} tVTMAMNegotiationReply;

#define kReturnOSType   0x0408		/* We are Lunix! :-) */

/* Completion mask bits */

#define kAMNRSuccessful		0x8000
#define kAMNREchoFailed		0x4000
#define kAMNRBreakFailed	0x2000
#define kAMNRNoDevice		0x1000

/* Hardware control completion mask: bits for supported features */

#define kHWCTLEchoControl	0x8000
#define kHWCTLCharacterDelete	0x4000
#define kHWCTLCharacterEcho	0x2000
#define kHWCTLLineDelete	0x1000
#define kHWCTLLineChar  	0x0800   
#define kHWCTLTypeAhead		0x0400
#define kHWCTLNoBreakRead	0x0200
#define kHWCTLParity		0x0100

/*=======================================================================
  TM Negotiation Request/Reply
 *=======================================================================*/

typedef struct stVTMTMNegotiationRequest
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned8		fLinkType;
    unsigned8		fUnused2;
    unsigned16		fTerminalClass;
    char		fSessionID[6];
    unsigned16		fNodeLength;
    char		fNodeName[51];
} tVTMTMNegotiationRequest;


#define kVTDefaultCommLinkType		1	/* What does this mean? */
#define kVTDefaultTerminalClass		3	/* What does this mean? */

typedef struct stVTMTMNegotiationReply
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fResponseCode;
    unsigned8		fCommLinkAcceptReject;
    unsigned8		fUnused2;
    unsigned16		fTerminalClass;
} tVTMTMNegotiationReply;

#define kTMNRSuccessful		0x8000

/*=======================================================================
  Terminate Message Request
 *=======================================================================*/

typedef struct stVTMTerminationRequest
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned8		fTerminationType;
    unsigned8		fUnused2;
    unsigned16		fTerminationReason;
} tVTMTerminationRequest;

typedef struct stVTMTerminationResponse
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fResponseCode;
} tVTMTerminationResponse;

/* Termination type codes */

#define kVTTerminationAgreed	0
#define kVTTerminationForced	1

/* Termination reason codes */

#define kVTTerminateNormal	0
#define kVTTerminateUserAbort	1
#define kVTTerminateControlReq	2
#define kVTTerminatePgmError	3
#define kVTTerminateBadNegotiate 4
#define kVTTerminateVTSError	5
#define kVTTerminateLogonFail	6
#define kVTTerminateInitFail	7
#define kVTTerminateNoDevice	8

/*=======================================================================
  Logon Info Message/Response
 *=======================================================================*/

typedef struct stVTMLogonInfo
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    char		fSessionID[6];
    unsigned16		fLogonLength;
    char		fLogonString[1];  /* Extended by length */
} tVTMLogonInfo;

typedef struct stVTMLogonInfoResponse
{
    VT_MESSAGE_HEADER;
    unsigned16		fResponseCount;
    unsigned16		fResponseMask;
} tVTMLogonInfoResponse;

/*=======================================================================
  Terminal I/O request/response
 *=======================================================================*/

typedef struct stVTMIORequest
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fReadFlags;
    unsigned16		fReadByteCount;
    unsigned16		fTimeout;
    unsigned16		fWriteFlags;
    unsigned16		fWriteByteCount;
    char		fWriteData[1];	/* Extended by WriteByteCount */
} tVTMIORequest;

/* These make up the primitive. They're listed as bitmapped in the */
/* manual, but they're really just codes.                          */

#define kVTIORead		0
#define kVTIOWrite		1
#define kVTIOWriteRead		2
#define kVTIOAbort		4

/* These are the read flag bits */

#define kVTIORTypeAhead		0x8000
#define kVTIORPrompt		0x4000
#define kVTIORNoCRLF		0x2000
#define kVTIORBypassTypeAhead	0x1000
#define kVTIORFlushTypeAhead	0x0800

/* And the write flag bits */

#define kVTIOWUseCCTL		0x8000
#define kVTIOWPrespace		0x4000
#define kVTIOWPrompt		0x2000
#define kVTIOWNeedsResponse	0x1000
#define kVTIOWPreemptive	0x0800
#define kVTIOWClearFlush	0x0400

typedef struct stVTMTerminalIOResponse
{
    VT_MESSAGE_HEADER;
    unsigned16			fRequestCount;
    unsigned16			fResponseCode;
    unsigned16			fCompletionMask;
    unsigned16			fBytesRead;
    char 			fBytes[2];	/* Expands as necessary */
} tVTMTerminalIOResponse;

#define kVTIOCSuccessful	0x8000
#define kVTIOCEOF		0x4000
#define kVTIOCEOFError		0x2000		/* What's this? */
#define kVTIOCBadOp		0x1000
#define kVTIOCTimeout		0x0800
#define kVTIOCAborted		0x0400
#define kVTIOCDataLost		0x0200
#define kVTIOCParityError	0x0100
#define kVTIOCDeviceUnavailable 0x0080
#define kVTIOCUnitFailure	0x0040
#define kVTIOCBuffShort		0x0020
#define kVTIOCundefined		0x0010
#define kVTIOCBreakRead		0x0008
#define kVTIOCEORRead		0x0004
#define kVTIOCExtra1		0x0002
#define kVTIOCExtra2		0x0001

/*=======================================================================
  Abort I/O request/response
 *=======================================================================*/

typedef struct stVTMAbortIORequest
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fRequestMask;
} tVTMAbortIORequest;

typedef struct stVTMAbortIOResponse
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fResponseCode;
} tVTMAbortIOResponse;

/*=======================================================================
  Set Break request/response
 *=======================================================================*/

typedef struct stVTMSetBreakRequest
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned8		fIndex;
    unsigned8		fState;
} tVTMSetBreakRequest;

typedef struct stVTMSetBreakResponse
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16          fResponseCode;
} tVTMSetBreakResponse;

/*=======================================================================
  Terminal Driver Control request/response
 *=======================================================================*/

typedef struct stVTMTerminalDriverControlRequest
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fRequestMask;
    unsigned8		fEcho;
    unsigned8		fEditMode;
    unsigned8		fDriverMode;
    unsigned8		fDataStream;
    unsigned8		fLineTermCharacter;
    unsigned8		fEchoLineDelete;
    unsigned16		fBaudRate;
    unsigned8		fParity;
    unsigned8		fParityChecking;
} tVTMTerminalDriverControlRequest;

/* Bitmap for request mask */

#define kTDCMEcho		0x8000
#define kTDCMEditMode		0x4000
#define kTDCMDriverMode		0x2000
#define kTDCMTermChar		0x1000
#define kTDCMDataStream		0x0800
#define kTDCMEchoLine		0x0400
#define kTDCMBaudRate		0x0200
#define kTDCMParity		0x0100
#define kTDCMParityCheck	0x0080

/* Values for "edit mode" */

#define kDTCEditedMode		1
#define kDTCUneditedMode	2
#define kDTCBinaryMode		3
#define kDTCDisableBinary	4

/* Values for "driver mode" */

#define kDTCVanilla		1
#define kDTCBlockMode		2
#define kDTCUserBlock		3

/* Values for "echo" */

#define kDTCEchoOffAll		1
#define kDTCEchoOffNext		2
#define kDTCEchoOnAll		3
#define kDTCEchoOnNext		4

/* Values for "break" */

#define kDTCSystemBreakIndex	0
#define kDTCCntlYIndex		1
#define kDTCConsoleIndex	2

typedef struct stVTMTerminalDriverControlResponse
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fResponseCode;
    unsigned16		fStatusMask;
} tVTMTerminalDriverControlResponse;

#define kDTCRSuccess		0x8000
#define kDTCREchoFailed		0x4000
#define kDTCREditModeFailed	0x2000
#define kDTCRDriverModeFailed	0x1000
#define kDTCRLineTermFailed	0x0800
#define kDTCRStreamFailed	0x0400
#define kDTCREchoLineDeleteFailed 0x0200
#define kDTCRBaudRateFailed	0x0100
#define kDTCRParityFailed	0x0080
#define kDTCRParityCheckFailed	0x0040

/*=======================================================================
  Invoke Break request/response
 *=======================================================================*/

typedef struct stVTMInvokeBreakRequest
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fIndex;
} tVTMInvokeBreakRequest;

typedef struct stVTMInvokeBreakResponse
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16          fResponseCode;
} tVTMInvokeBreakResponse;

/*=======================================================================
  MPE Specific Message/Response
 *=======================================================================*/

typedef struct stVTMMPECntlReq
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fRequestMask;
    unsigned8		fTermType;
    unsigned8		fTypeAhead;
} tVTMMPECntlReq;

/* Request mask */
#define kDTCMSetTermType	0x8000
#define kDTCMDoNoOp		0x4000
#define kDTCMClearFlush		0x2000	/* MPE/V only */
#define kDTCMTypeAhead		0x1000

typedef struct stVTMMPECntlResp
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fResponseCode;
    unsigned16		fCompletionMask;
} tVTMMPECntlResp;

/* Completion mask */
#define kDTCCSuccessful		0x8000
#define kDTCCTermTypeFail	0x4000
#define kDTCCNoOpFail		0x2000
#define kDTCCTypeAheadFail	0x1000

typedef struct stVTMMPEGetInfoReq
{
     VT_MESSAGE_HEADER;
     unsigned16		fRequestCount;
     unsigned16		fRequestMask;
} tVTMMPEGetInfoReq;

typedef struct stVTMMPEGetInfoResp
{
     VT_MESSAGE_HEADER;
     unsigned16		fRequestCount;
     unsigned16		fCompletionMask;
     unsigned16		fInfoByteCount;
     unsigned16		fBlockModeType;
     unsigned16		fTermType;
} tVTMMPEGetInfoResp;

#define kDTCBNoBlock		0
#define kDTCBLineBlock		1
#define kDTCBATPPageBlock	2
#define kDTCBATPPageLine	3	/* Both 1 and 2 */
#define kDTCAVESTAPageBlock	6
#define kDTCAVESTAPageLine	7	/* Both 1 and 7 */
#define kDTCIBM3270Block	8	/* SNA DHCF/XL, VPLUS */


/*=======================================================================
  FDEVICECONTROL Specific Message/Response
 *=======================================================================*/

typedef struct stVTMFDCCntlReq
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned32		fFDCFunc;
    unsigned16		fFDCLength;
    unsigned16		fFDCExtra;
    char		fFDCBuffer[26];
} tVTMFDCCntlReq;

typedef struct stVTMFDCCntlResp
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned32		fFDCFunc;
    unsigned16		fFDCLength;
    unsigned16		fFDCErrorCode;
    char		fFDCBuffer[26];
} tVTMFDCCntlResp;


/*=======================================================================
  Application Specific Message/Response
 *=======================================================================*/

typedef struct stVTMApplCntlReq
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fApplIndex;
} tVTMApplCntlReq;

typedef struct stVTMApplCntlResp
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned16		fApplIndex;
} tVTMApplCntlResp;

#endif
