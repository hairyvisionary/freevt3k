/*
 * @(#) tcpreader.c: 96/04/16-14:43:19
 */

#ifdef __hpux
#  ifndef _HPUX_SOURCE
#    define _HPUX_SOURCE		(1)
#  endif
#  ifndef _POSIX_SOURCE
#    define _POSIX_SOURCE		(1)
#  endif
#endif
#ifdef _M_UNIX	/* SCO */
#  ifndef M_UNIX
#    define M_UNIX			(1)
#  endif
#endif
#include <sys/types.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if defined(_AIX) || defined(AIX) || defined(aix) || defined(M_UNIX)
#  include <time.h>
#endif
#if defined(M_UNIX)
#  include <arpa/inet.h>
#endif

#include "../typedef.h"
#include "../vt.h"

#define SWAP_L(x)	if (need_swap) Swap(&x,4)
#define SWAP_S(x)	if (need_swap) Swap(&x,2)

typedef struct svtEntry
{
    VT_MESSAGE_HEADER;
    unsigned16		fRequestCount;
    unsigned8		fData[2];	/* Expands */
} vtEntry;

typedef struct stcpRecord
{
    unsigned char	misc1[26];
    unsigned char	ip_from[4];
    unsigned char	ip_to[4];
    unsigned char	ip_port_from[2];
    unsigned char	ip_port_to[2];
    unsigned char	misc2[16];
    unsigned16		len;
    unsigned8		data[2];	/* Expands */
} tcpRecord;

typedef struct stcpdumpHeader
{
    time_t		time;
    unsigned char	misc[4];
    int			actual_len;
    int			raw_len;
} tcpdumpHeader;

typedef struct stcpdumpControl
{
    unsigned int	magic;
    unsigned char	misc1[12];
    int			max;		/* tcpdump -s option */
    unsigned char	misc2[4];
} tcpdumpControl;

unsigned char
	tcpdump_rec[4096];
tcpdumpControl
	control;
tcpdumpHeader
	header;
tcpRecord
	*prefix;
vtEntry
	*vt;	
time_t
	start_time = 0;
char
	*progName = "tcpreader";
int
	need_swap = 0;
unsigned32
	ip1 = 0xFFFFFFFF,
	ip2 = 0xFFFFFFFF;
char
	*ip1_name = NULL,
	*ip2_name = NULL;

#ifdef __STDC__
void Swap(void *buf, int n_bytes)
#else
void Swap(buf, n_bytes)
  void *buf;
  int n_bytes;
#endif
{ /*Swap*/

  unsigned char
    c,
    *inPtr,
    *outPtr;
    
/* Only swap if even number, greater than zero */
  if ((n_bytes > 0) && (!(n_bytes & 0x01)))
    {
      inPtr = outPtr = (unsigned char*)buf;
      inPtr += n_bytes;
      while (n_bytes)
	{
	  c = *outPtr;
	  *(outPtr++) = *(--inPtr);
	  *inPtr = c;
	  n_bytes -= 2;
	}
    }

} /*Swap*/

#ifdef __STDC__
void DumpBuffer(void *buf, int buf_len, char *dump_id, int vt_entry)
#else
void DumpBuffer(buf, buf_len, dump_id, vt_entry)
  unsigned char *buf;
  int buf_len;
  char *dump_id;
  int vt_entry;
#endif
{ /*DumpBuffer*/

#define CHAR_PER_LINE		(16)
  int
    nChars = CHAR_PER_LINE,
    nLines,
    iLine,
    iChar;
  unsigned char
    *charPtr,
    *ptr,
    *msgPtr,
    msg[81];
  unsigned16
    *to_port,
    *from_port;
  char
    *from_ptr,
    from_temp[32],
    *to_ptr,
    to_temp[32];
  int
    et = 0;
  struct tm
    *tm;
    
  if (vt_entry)
    {
      et = header.time - start_time;
      tm = localtime(&header.time);
      to_port = (unsigned16*)&prefix->ip_port_to;
      from_port = (unsigned16*)&prefix->ip_port_from;
      printf("%-24s%5d  %02d:%02d:%02d\n",
	     dump_id, et, tm->tm_hour, tm->tm_min, tm->tm_sec);
      if (ip1 == 0xFFFFFFFF)
	{
	  sprintf(from_temp, "%d.%d.%d.%d", 
		  prefix->ip_from[0], prefix->ip_from[1],
		  prefix->ip_from[2], prefix->ip_from[3]);
	  from_ptr = from_temp;
	  sprintf(to_temp, "%d.%d.%d.%d", 
		  prefix->ip_to[0], prefix->ip_to[1],
		  prefix->ip_to[2], prefix->ip_to[3]);
	  to_ptr = to_temp;
	}
      else if (!memcmp((void*)&ip1, (void*)&prefix->ip_from, 4))
	{
	  from_ptr = ip1_name;
	  to_ptr = ip2_name;
	}
      else
	{
	  from_ptr = ip2_name;
	  to_ptr = ip1_name;
	}
      printf("  %s[%d] -> %s[%d]\n",
	     from_ptr, ntohs(*from_port), to_ptr, ntohs(*to_port));
    }
  else
    printf("%s\n", dump_id);
  nLines = buf_len / CHAR_PER_LINE;
  charPtr = (unsigned char*)buf;
  for (iLine = 0; iLine <= nLines; iLine++)
    {
      if (iLine == nLines)
	nChars = buf_len % CHAR_PER_LINE;
      memset((char*)msg, ' ', sizeof(msg)-1);
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
	printf("%s\n", msg);
    }

} /*DumpBuffer*/

#ifdef __STDC__
void ExitProc(char *text1, char *text2, int exit_code)
#else
void ExitProc(text1, text2, exit_code)
  char *text1;
  char *text2;
  int exit_code;
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
	  strcat(buf, strerror(errno));
	}
      fprintf(stderr, "%s: %s\n", progName, buf);
    }
  exit(exit_code);

} /*ExitProc*/

#ifdef __STDC__
void FormatEnvCntl(void)
#else
void FormatEnvCntl()
#endif
{/*FormatEnvCntl*/

  tVTMAMNegotiationRequest
    *am_req = (tVTMAMNegotiationRequest *)vt;
  tVTMAMNegotiationReply
    *am_resp = (tVTMAMNegotiationReply *)vt;
  tVTMTMNegotiationRequest
    *tm_req = (tVTMTMNegotiationRequest *)vt;
  tVTMTMNegotiationReply
    *tm_resp = (tVTMTMNegotiationReply *)vt;
  tVTMLogonInfo
    *logon_req = (tVTMLogonInfo *)vt;
  tVTMLogonInfoResponse
    *logon_resp = (tVTMLogonInfoResponse *)vt;
  tVTMAMBreakInfo
    *break_info;
  unsigned16
    mask = 0;
  unsigned char
    *ptr;
  int
    len;

  switch (vt->fPrimitive)
    {
    case kvtpTMNegotiate:
      printf("TMNegotiate");
      if (vt->fMessageType == kvmtEnvCntlResp)
	{
	  mask = ntohs(tm_resp->fResponseCode);
	  if (mask == kTMNRSuccessful)
	    printf(" ok\n");
	  else
	    printf(" code=$%04x\n", mask);
	  break;
	}
      printf("\n");
      printf("  link_type=%d\n", tm_req->fLinkType);
      printf("  terminal_class=%d\n", ntohs(tm_req->fTerminalClass));
      printf("  session_id=");
      ptr = (unsigned char*)&tm_req->fSessionID;
      len = sizeof(tm_req->fSessionID);
      while (len)
	{
	  if (isprint(*ptr))
	    printf("%c", *ptr);
	  else
	    printf("<$%02x>", *ptr);
	  ++ptr;
	  --len;
	}
      printf("\n");
      len = ntohs(tm_req->fNodeLength);
      if (len > sizeof(tm_req->fNodeName))
	len = sizeof(tm_req->fNodeName);
      printf("  node_name=%*s\n", len, tm_req->fNodeName);
      break;
    case kvtpAMNegotiate:
      printf("AMNegotiate");
      if (vt->fMessageType == kvmtEnvCntlResp)
	{
	  mask = ntohs(am_resp->fResponseCode);
	  if (mask == kAMNRSuccessful)
	    printf(" ok\n");
	  else
	    printf(" code=$%04x\n", mask);
	  printf("  buffer_size=%d\n", ntohs(am_resp->fBufferSize));
	  printf("  op_sys=$%04x\n", ntohs(am_resp->fOperatingSystem));
	  printf("  hw_mask=$%04x\n", ntohs(am_resp->fHardwareControlCompletionMask));
	  printf("  max_burst: recv=%d, send=%d\n",
		 ntohs(am_resp->fTMMaxReceiveBurst),
		 ntohs(am_resp->fTMMaxSendBurst));
	  break;
	}
      printf("\n");
      printf("  buffer_size=%d\n", ntohs(am_req->fBufferSize));
      printf("  echo_resp=%d[", am_req->fEcho);
      switch (am_req->fEcho)
	{
	case kDTCEchoOffAll:		printf("Don't Care"); break;
	case kDTCEchoOffNext:		printf("Backspace"); break;
	case kDTCEchoOnAll:		printf("BSSlash"); break;
	case kDTCEchoOnNext:		printf("BsSpBs"); break;
	default:			printf("???"); break;
	}
      printf("], echo_cntl=%d[", am_req->fEchoControl);
      switch (am_req->fEchoControl)
	{
	case kAMEchoDontCare:		printf("Don't Care"); break;
	case kAMEchoDontAllow:		printf("Don't Allow"); break;
	case kAMEchoSwitchOK:		printf("SwitchOk"); break;
	case kAMEchoNotifyOnSwitch:	printf("NotifyOnSwitch"); break;
	default:			printf("???"); break;
	}
      printf("]\n");
      printf("  char_del=$%02x, char_del_echo=%d\n",
	     am_req->fCharacterDelete,
	     am_req->fCharacterDeleteEcho);
      printf("  line_del_offset=%d, length=%d, line_del=$%02x",
	     ntohs(am_req->fLineDeleteOffset),
	     ntohs(am_req->fLineDeleteLength),
	     am_req->fLineDeleteCharacter);
      if ((ntohs(am_req->fLineDeleteLength)) &&
	  (ntohs(am_req->fLineDeleteOffset)))
	{
	  int len = ntohs(am_req->fLineDeleteLength);
	  unsigned char *ptr;
	  printf(" [");
	  ptr = (void*)vt;
	  ptr += ntohs(am_req->fLineDeleteOffset);
	  while (len)
	    {
	      if (isprint(*ptr))
		printf("%c", *ptr);
	      else
		printf("<$%02x>", *ptr);
	      ++ptr;
	      --len;
	    }
	}
      printf("]\n");
      printf("  no_break=%d\n", am_req->fNoBreakRead);
      printf("  type_ahead_size=%d, type_ahead_lines=%d\n",
	     ntohs(am_req->fTypeAheadSize),
	     am_req->fTypeAheadLines);
      printf("  parity=%d\n", am_req->fParity);
      if (ntohs(am_req->fBreakOffset))
	{
	  printf("  break_offset=%d, break_index_count=%d\n",
		 ntohs(am_req->fBreakOffset),
		 ntohs(am_req->fBreakIndexCount));
	  if (ntohs(am_req->fBreakOffset))
	    {
	      break_info = (tVTMAMBreakInfo*)
		((unsigned16*)am_req + ntohs(am_req->fBreakOffset)/2);
	      printf("  sys_break_enabled=%d, subsys_break_enabled=%d\n",
		     ntohs(break_info->fSysBreakEnabled),
		     ntohs(break_info->fSubsysBreakEnabled));
	      printf("  sys_break_char=$%04x, subsys_break_char=$%04x\n",
		     ntohs(break_info->fSysBreakChar),
		     ntohs(break_info->fSubsysBreakChar));
	    }
	}
      printf("  logon_id_offset=%d, length=%d",
	     ntohs(am_req->fLogonIDOffset),
	     ntohs(am_req->fLogonIDLength));
      if ((ntohs(am_req->fLogonIDLength)) &&
	  (ntohs(am_req->fLogonIDOffset)))
	{
	  int len = ntohs(am_req->fLogonIDLength);
	  unsigned char *ptr;
	  printf(" [");
	  ptr = (void*)vt;
	  ptr += ntohs(am_req->fLogonIDOffset);
	  while (len)
	    {
	      if (isprint(*ptr))
		printf("%c", *ptr);
	      else
		printf("<$%02x>", *ptr);
	      ++ptr;
	      --len;
	    }
	}
      printf("\n");
      printf("  device_id_offset=%d, length=%d",
	     ntohs(am_req->fDeviceIDOffset),
	     ntohs(am_req->fDeviceIDLength));
      if ((ntohs(am_req->fDeviceIDLength)) &&
	  (ntohs(am_req->fDeviceIDOffset)))
	{
	  int len = ntohs(am_req->fDeviceIDLength);
	  unsigned char *ptr;
	  printf(" [");
	  ptr = (void*)vt;
	  ptr += ntohs(am_req->fDeviceIDOffset);
	  while (len)
	    {
	      if (isprint(*ptr))
		printf("%c", *ptr);
	      else
		printf("<$%02x>", *ptr);
	      ++ptr;
	      --len;
	    }
	}
      printf("\n");
      printf("  max_burst: recv=%d, send=%d\n",
	     ntohs(am_req->fAMMaxReceiveBurst),
	     ntohs(am_req->fAMMaxSendBurst));
      break;
    case kvtpTerminate:
      printf("Terminate\n");
      if (vt->fMessageType == kvmtEnvCntlResp)
	{
	  break;
	}
      break;
    case kvtpLogonInfo:
      printf("LogonInfo");
      if (vt->fMessageType == kvmtEnvCntlResp)
	{
	  mask = ntohs(logon_resp->fResponseMask);
	  if (mask == kVTIOCSuccessful)	/* Need different success code */
	    printf(" ok\n");
	  else
	    printf(" code=$%04x\n", mask);
	  break;
	}
      printf("  session_id=");
      ptr = (unsigned char*)&logon_req->fSessionID;
      len = sizeof(logon_req->fSessionID);
      while (len)
	{
	  if (isprint(*ptr))
	    printf("%c", *ptr);
	  else
	    printf("<$%02x>", *ptr);
	  ++ptr;
	  --len;
	}
      printf("\n");
      printf("  logon_string=%*s\n",
	     ntohs(logon_req->fLogonLength), logon_req->fLogonString);
      break;
    default:
      printf("Unknown: %d\n", vt->fPrimitive);
    }

} /*FormatEnvCntl*/

#ifdef __STDC__
void FormatTerminalIO(void)
#else
void FormatTerminalIO()
#endif
{/*FormatTerminalIO*/

  tVTMTerminalIOResponse
    *io_resp;
  tVTMIORequest
    *io_req = (tVTMIORequest *)vt;
  unsigned16
    mask = 0;
    
  switch (vt->fPrimitive)
    {
    case kVTIOWriteRead:
    case kVTIOWrite:
      mask = ntohs(io_req->fWriteFlags);
      printf("VTIOWrite%s len=%d",
	     ((vt->fPrimitive == kVTIOWriteRead) ? "Read" : ""),
	     ntohs(io_req->fWriteByteCount));
      if (mask & kVTIOWUseCCTL)
	printf(" cctl[%s]", (mask & kVTIOWPrespace) ? "pre" : "post");
      if (mask & kVTIOWPrompt)
	printf(" prompt");
      if (mask & kVTIOWNeedsResponse)
	printf(" needs_response");
      if (mask & kVTIOWPreemptive)
	printf(" preemptive");
      if (mask & kVTIOWClearFlush)
	printf(" clear_flush");
      printf("\n");
      if (vt->fPrimitive == kVTIOWrite)
	break;
    case kVTIORead:
      printf("VTIORead");
      if (vt->fMessageType == kvmtTerminalIOResp)
	{
	  io_resp = (tVTMTerminalIOResponse*)vt;
	  printf(" len=%d", ntohs(io_resp->fBytesRead));
	  mask = ntohs(io_resp->fResponseCode);
	  if (mask & kVTIOCSuccessful)
	    {
	      printf(" ok\n");
	      break;
	    }
	  mask = ntohs(io_resp->fCompletionMask);
	  switch (mask)
	    {
	    case kVTIOCTimeout:
	      printf(" timeout");
	      break;
	    case kVTIOCBreakRead:
	      printf(" break");
	      break;
	    default:
	      printf(" other: %d", mask);
	    }
	  printf("\n");
	  break;
	}
      mask = ntohs(io_req->fReadFlags);
      printf(" len=%d timeout=%d",
	     ntohs(io_req->fReadByteCount),
	     ntohs(io_req->fTimeout));
      if (mask & kVTIORTypeAhead)
	printf(" type_ahead");
      if (mask & kVTIORPrompt)
	printf(" prompt");
      if (mask & kVTIORNoCRLF)
	printf(" no_cr_echo");
      if (mask & kVTIORBypassTypeAhead)
	printf(" bypass_type_ahead");
      if (mask & kVTIORFlushTypeAhead)
	printf(" flush_type_ahead");
      printf("\n");
      break;
    case kVTIOAbort:
      printf("VTIOAbort\n");
      break;
    default:
      printf("Unknown: %d\n", vt->fPrimitive);
    }
	
}/*FormatTerminalIO*/

#ifdef __STDC__
void FormatTerminalCntl(void)
#else
void FormatTerminalCntl()
#endif
{/*FormatTerminalCntl*/

  tVTMSetBreakRequest
    *break_req = (tVTMSetBreakRequest*)vt;
  tVTMSetBreakResponse
    *break_resp = (tVTMSetBreakResponse*)vt;
  tVTMTerminalDriverControlRequest
    *driver_req = (tVTMTerminalDriverControlRequest*)vt;
  tVTMTerminalDriverControlResponse
    *driver_resp = (tVTMTerminalDriverControlResponse*)vt;
  unsigned16
    mask = 0;
    
  switch (vt->fPrimitive)
    {
    case kvtpSetBreakInfo:
      printf("SetBreakInfo");
      if (vt->fMessageType == kvmtTerminalCntlResp)
	{
	  mask = ntohs(break_resp->fResponseCode);
	  if (mask & kVTIOCSuccessful)
	    printf(" ok\n");
	  else
	    printf(" code=$%04x\n", mask);
	  break;
	}
      if (break_req->fIndex == kVTSystemBreak)
	printf(" sys_break_enabled=%d", break_req->fState);
      else if (break_req->fIndex == kVTSubsysBreak)
	printf(" subsys_break_enabled=%d", break_req->fState);
      printf("\n");
      break;
    case kvtpSetDriverInfo:
      mask = ntohs(driver_req->fRequestMask);
      printf("SetDriverInfo");
      if (vt->fMessageType == kvmtTerminalCntlResp)
	{
	  mask = ntohs(driver_resp->fResponseCode);
	  if (mask & kVTIOCSuccessful)
	    printf(" ok\n");
	  else
	    printf(" code=$%04x\n", mask);
	  break;
	}
      if (mask & kTDCMEcho)
	{
	  printf(" echo_cntl=%d[", driver_req->fEcho);
	  switch (driver_req->fEcho)
	    {
	    case kDTCEchoOffAll:	printf("Don't Care"); break;
	    case kDTCEchoOffNext:	printf("Backspace"); break;
	    case kDTCEchoOnAll:		printf("BSSlash"); break;
	    case kDTCEchoOnNext:	printf("BsSpBs"); break;
	    default:			printf("???"); break;
	    }
	  printf("]");
	}

      if (mask & kTDCMEditMode)
	{
	  printf(" edit_mode=%d[", driver_req->fEditMode);
	  switch (driver_req->fEditMode)
	    {
	    case kDTCEditedMode:	printf("edited_mode"); break;
	    case kDTCUneditedMode:	printf("unedited_mode"); break;
	    case kDTCBinaryMode:	printf("binary_mode_on"); break;
	    case kDTCDisableBinary:	printf("binary_mode_off"); break;
	    default:			printf("???");
	    }
	  printf("]");
	  if (driver_req->fEditMode == kDTCUneditedMode)
	    printf(" term=$%02x", driver_req->fLineTermCharacter);
	}
      if (mask & kTDCMDriverMode)
	{
	  printf(" driver_mode=%d[", driver_req->fDriverMode);
	  switch (driver_req->fDriverMode)
	    {
	    case kDTCVanilla:		printf("vanilla"); break;
	    case kDTCBlockMode:		printf("block_mode"); break;
	    case kDTCUserBlock:		printf("user_block"); break;
	    default:			printf("???");
	    }
	  printf("]");
	}
      if (mask & kTDCMTermChar)
	printf(" alt_eol=$%02x", driver_req->fLineTermCharacter);
      if (mask & kTDCMDataStream)
	printf(" data_stream=%d", driver_req->fDataStream);
      if (mask & kTDCMEchoLine)
	printf(" echo_line=%d", driver_req->fEchoLineDelete);
      printf("\n");
      break;
    default:
      printf("Unknown: %d\n", vt->fPrimitive);
    }
	
}/*FormatTerminalCntl*/

#ifdef __STDC__
void FormatApplicationCntl(void)
#else
void FormatApplicationCntl()
#endif
{/*FormatApplicationCntl*/

  tVTMApplCntlReq
    *appl_req = (tVTMApplCntlReq *)vt;
    
  switch (vt->fPrimitive)
    {
    case kvtpApplInvokeBreak:
      printf("ApplInvokeBreak Index=$%04x\n", ntohs(appl_req->fApplIndex));
      break;
    default:
      printf("Unknown: %d\n", vt->fPrimitive);
    }

}/*FormatApplicationCntl*/

#ifdef __STDC__
void FormatMPECntl(void)
#else
void FormatMPECntl()
#endif
{/*FormatMPECntl*/

  tVTMMPECntlReq
    *cntl_req = (tVTMMPECntlReq *)vt;
  unsigned16
    mask = 0;

  mask = ntohs(cntl_req->fRequestMask);
  printf("MPE mask=$%04x", mask);
  if (mask & kDTCMSetTermType)
    printf(" term_type=%d", cntl_req->fTermType);
  if (mask & kDTCMTypeAhead)
    printf(" type_ahead=%d", cntl_req->fTypeAhead);
  printf("\n");

}/*FormatMPECntl*/

#ifdef __STDC__
void FormatFDCCntl(void)
#else
void FormatFDCCntl()
#endif
{/*FormatFDCCntl*/

  tVTMFDCCntlReq
    *fdc_req = (tVTMFDCCntlReq *)vt;

  printf("FDC func=%d, len=%d\n",
	 ntohl(fdc_req->fFDCFunc),
	 ntohs(fdc_req->fFDCLength));

}/*FormatFDCCntl*/

#ifdef __STDC__
void main(int argc, char *argv[])
#else
void main(argc, argv)
  int argc;
  char *argv[];
#endif
{ /*main*/

  int
    fd = 0,
    detail = 0,
    len = 0,
    msg_len = 0;
  char
    *fileName = NULL;
  struct tm
    *tm;
  struct hostent
    *hostentptr;
  unsigned char
    *ptr;
  char
    type_buf[32],
    *types[] =
      {
	"EnvCntlReq",
	"EnvCntlResp",
	"TermIOReq",
	"TermIOResp",
	"TermCntlReq",
	"TermCntlResp",
	"AppCntlReq",
	"AppCntlResp",
	"MPECntlReq",
	"MPECntlResp",
	"FDCCntlReq",
	"FDCCntlResp"
      };

  if (argc < 2)
    {
      printf("Usage: tcpreader [-d] filename\n");
      printf("  filename created using something like:\n");
      printf("    'tcpdump -D -s 4096 -w filename tcp \n");
      printf("        port ${PORT} and src host ${CLIENT} and dst host ${HOST}'\n");
      printf("  ${PORT} is 1537 for vt3k and 1570 for freevt3k\n");
      printf("  ${CLIENT} is the address of the machine running [free]vt3k\n");
      printf("  ${HOST} is the address of the target HP3000\n");
      printf("  '-d' will cause additional tcpdump info to be displayed (debug)\n");
      exit(0);
    }
  ++argv;
  --argc;
  while ((argc > 0) && (*argv[0] == '-'))
    {
      if (!strcmp(*argv,"-d"))
	detail = 1;
      else if (!strcmp(*argv, "-ip"))
	{
	  if (--argc)
	    if (ip1 == 0xFFFFFFFF)
	      ip1 = inet_addr(*(++argv));
	    else if (ip2 == 0xFFFFFFFF)
	      ip2 = inet_addr(*(++argv));
	    else
	      {
		fprintf(stderr, "too many ip addresses\n");
		exit(1);
	      }
	}
      --argc;
      ++argv;
    }
  if (!argc)
    exit(1);
  fileName = *argv;

  if (ip1 != 0xFFFFFFFF)
    {
      hostentptr = (struct hostent*)gethostbyaddr((char*)&ip1, sizeof (struct in_addr), AF_INET);
      if ((hostentptr != (struct hostent*)NULL))
	{
	  ip1_name = (char*)malloc(strlen((char*)hostentptr->h_name)+1);
	  strcpy(ip1_name, (char*)hostentptr->h_name);
	}
      if (ip1_name == (char*)NULL)
	{
	  char *ptr, temp[32];
	  ptr = (char*)&ip1;
	  sprintf(temp, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
	  ip1_name = (char*)malloc(strlen(temp)+1);
	  strcpy(ip1_name, temp);
	}
    }

  if (ip2 != 1)
    {
      hostentptr = (struct hostent*)gethostbyaddr((char*)&ip2, sizeof (struct in_addr), AF_INET);
      if ((hostentptr != (struct hostent*)NULL))
	{
	  ip2_name = (char*)malloc(strlen((char*)hostentptr->h_name)+1);
	  strcpy(ip2_name, (char*)hostentptr->h_name);
	}
      if (ip2_name == (char*)NULL)
	{
	  char *ptr, temp[32];
	  ptr = (char*)&ip2;
	  sprintf(temp, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
	  ip2_name = (char*)malloc(strlen(temp)+1);
	  strcpy(ip2_name, temp);
	}
    }

  if ((fd = open(fileName, O_RDONLY)) == -1)
    ExitProc("open", fileName, 1);
    
  if (read(fd, &control, sizeof(control)) != sizeof(control))
    ExitProc("read", "control", 1);
  if (detail)
    DumpBuffer((void*)&control, sizeof(control), "control", 0);
/*
 * Check first 4 bytes for magic number.  This will clue us into the
 *   byte-ordering rule in effect on the platform the dumpfile was
 *   created upon.
 */
  len = 0xA1B2C3D4;
  if (detail)
    {
      DumpBuffer((void*)&ip1, 4, "ip1", 0);
      DumpBuffer((void*)&ip2, 4, "ip2", 0);
      DumpBuffer((void*)&len, 4, "len", 0);
      DumpBuffer((void*)&control.magic, 4, "magic", 0);
    }
  if (memcmp((void*)&len, (void*)&control.magic, 4))
    need_swap = 1;	/* tcpdump platform different than me! */
  SWAP_L(control.max);
  for (;;)
    {
      if ((len = read(fd, &header, sizeof(header))) != sizeof(header))
	{
	  if (!len)
	    break;
	  ExitProc("read", "header", 1);
	}
      if (detail)
	DumpBuffer((void*)&header, sizeof(header), "header", 0);
      SWAP_L(header.raw_len);
      SWAP_L(header.actual_len);
      memset((char*)tcpdump_rec, '\0', sizeof(tcpdump_rec));
      if (read(fd, tcpdump_rec, header.actual_len) != header.actual_len)
	ExitProc("read", "data", 1);
      if (detail)
	DumpBuffer(tcpdump_rec, header.actual_len, "raw", 0);
      SWAP_L(header.time);
      if (start_time == 0)
	start_time = header.time;
      prefix = (struct stcpRecord*)tcpdump_rec;
      vt = (struct svtEntry *)&prefix->len;
      if (!prefix->len)
	continue;
      if (ip1 != 0xFFFFFFFF)
	{
	  if ((memcmp((void*)&ip1, (void*)&prefix->ip_from, 4)) &&
	      (memcmp((void*)&ip1, (void*)&prefix->ip_to, 4)))
	    continue;
	  if (ip2 != 0xFFFFFFFF)
	    {
	      if ((memcmp((void*)&ip2, (void*)&prefix->ip_from, 4)) &&
		  (memcmp((void*)&ip2, (void*)&prefix->ip_to, 4)))
		continue;
	    }
	}
      len = header.actual_len - offsetof(tcpRecord, len);
      for (;;)
	{
	  if (vt->fProtocolID != kVTProtocolID)
	    break;
/* No need to check for unsigned8 >= 0
	  if ((kvmtEnvCntlReq <= (int)vt->fMessageType) &&
	      ((int)vt->fMessageType <= kvmtGenericFDCResp))
 */
	  if ((int)vt->fMessageType <= kvmtGenericFDCResp)
	    {
	      if (msg_len = (int)ntohs(vt->fMessageLength))
		{
/*
 * If the length field indicates data that overflows the buffer, quit
 *   this record
 */
		  if ((len) && (msg_len > len))
		    {
		      len = 0;
		      break;
		    }
		  sprintf(type_buf, "%s[%d]",
			  types[vt->fMessageType], vt->fMessageType);
		  DumpBuffer((void*)vt, msg_len, type_buf, 1);
		  printf("Counter=$%04X, Primitive=%d\n",
			 ntohs(vt->fRequestCount),
			 vt->fPrimitive);
		  switch (vt->fMessageType)
		    {
		    case kvmtEnvCntlReq:
		    case kvmtEnvCntlResp:
		      FormatEnvCntl();
		      break;
		    case kvmtTerminalIOReq:
		    case kvmtTerminalIOResp:
		      FormatTerminalIO();
		      break;
		    case kvmtTerminalCntlReq:
		    case kvmtTerminalCntlResp:
		      FormatTerminalCntl();
		      break;
		    case kvmtApplicationCntlReq:
		    case kvmtApplicationCntlResp:
		      FormatApplicationCntl();
		      break;
		    case kvmtMPECntlReq:
		    case kvmtMPECntlResp:
		      FormatMPECntl();
		      break;
		    case kvmtGenericFDCReq:
		    case kvmtGenericFDCResp:
		      FormatFDCCntl();
		      break;
		    }
		  printf("\n");
		  len -= msg_len;
		}
	    }
	  if (len <= 0)
	    break;
/*
 * Ugh.  Need to bump vt pointer to next entry.
 *   Do it by copying.
 */
	  ptr = (void*)vt;
	  ptr += msg_len;
	  memcpy((void*)vt, (void*)ptr, len);
	}
    }
  close(fd);
    
  exit(0);

} /*main*/

/* Local Variables: */
/* c-indent-level: 2 */
/* c-continued-statement-offset: 2 */
/* c-brace-offset: 0 */
/* c-argdecl-indent: 2 */
/* c-label-offset: -2 */
/* End: */
