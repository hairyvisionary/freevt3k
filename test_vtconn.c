#include <arpa/inet.h>

#include "vtconn.c"

/* globale from freevt3k.c */
int debug = 0;
int debug_need_crlf = 0;
int table_spec = 0;
unsigned char out_table[256];

int test_VTAllocConnection_1()
{
	tVTConnection *cxn = NULL;

	cxn = VTAllocConnection();
	return (cxn == NULL);
}

int test_VTCleanUpConnection_1()
{
	tVTConnection *cxn = NULL;

	cxn = VTAllocConnection();
	
	VTCleanUpConnection(cxn);
	if (cxn->fSocket != -1) {
		return 1;
	}
	if (cxn->fSendBuffer != NULL) {
		return 2;
	}
	if (cxn->fReceiveBuffer != NULL) {
		return 3;
	}
	return 0;
}

int test_VTDeallocConnection_1()
{
	tVTConnection *cxn = NULL;

	cxn = VTAllocConnection();

	VTDeallocConnection(cxn);
	return 0;
}

int test_VTInitConnection_1()
{
	char hostname[] = "127.0.0.1";
	long ipAddress;
	struct hostent * theHost;
	int ipPort = kVT_PORT;
	tVTConnection * cxn;
	int rc;
	
	ipAddress = inet_addr(hostname);
	if (ipAddress == INADDR_NONE) {
		theHost = gethostbyname(hostname);
		if (theHost == NULL)
		{
			fprintf(stderr, "Unable to resolve %s.\n", hostname);
			return 1;
		}
		memcpy((char *) &ipAddress, theHost->h_addr, sizeof(ipAddress));
	}

	cxn = VTAllocConnection();
	if (cxn == NULL) {
		return 2;
	}

	rc = VTInitConnection(cxn, ipAddress, ipPort);
	if (rc != kVTCNoError) {
		return 3;
	}

	/* check connection fields to see if they are properly initialized */
	if (cxn->fLineTerminationChar != '\r') {
		return 4;
	}
	if (cxn->fSendBufferSize != kVT_MAX_BUFFER) {
		return 5;
	}
	if (cxn->fReceiveBufferSize != kVT_MAX_BUFFER) {
		return 6;
	}
	if (cxn->fTargetAddress.sin_family != AF_INET) {
		return 7;
	}
	if (cxn->fTargetAddress.sin_port != htons(ipPort)) {
		return 8;
	}
	if (cxn->fTargetAddress.sin_addr.s_addr != ipAddress) {
		return 9;
	}
	if (cxn->fSocket == -1) {
		return 10;
	}
	if (cxn->fDataOutProc != DefaultDataOutProc) {
		return 11;
	}
	if (cxn->fState != kvtsClosed) {
		return 12;
	}
	if (cxn->fDriverMode != kDTCVanilla) {
		return 13;
	}
	if (cxn->fBlockModeSupported != FALSE) {
		return 14;
	}

	VTDeallocConnection(cxn);
	return 0;
}

int test_VTConnect_1()
{
	char hostname[] = "127.0.0.1";
	long ipAddress;
	struct hostent * theHost;
	int ipPort = kVT_PORT;
	tVTConnection * cxn;
	int rc;
	
	ipAddress = inet_addr(hostname);
	if (ipAddress == INADDR_NONE) {
		theHost = gethostbyname(hostname);
		if (theHost == NULL)
		{
			fprintf(stderr, "Unable to resolve %s.\n", hostname);
			return 1;
		}
		memcpy((char *) &ipAddress, theHost->h_addr, sizeof(ipAddress));
	}

	cxn = VTAllocConnection();
	if (cxn == NULL) {
		return 1;
	}

	rc = VTInitConnection(cxn, ipAddress, ipPort);
	if (rc != kVTCNoError) {
		return 2;
	}

	/* force fState to state(s) other than kvtsClosed to provoke VTConnect() */
	cxn->fState = kvtsUninitialized;
	if (VTConnect(cxn) != kVTCNotInitialized) {
		return 3;
	}
	cxn->fState = kvtsWaitingForAM;
	if (VTConnect(cxn) != kVTCNotInitialized) {
		return 4;
	}
	cxn->fState = kvtsWaitingForTMReply;
	if (VTConnect(cxn) != kVTCNotInitialized) {
		return 5;
	}
	cxn->fState = kvtsOpen;
	if (VTConnect(cxn) != kVTCNotInitialized) {
		return 6;
	}
	cxn->fState = kvtsWaitingForCloseReply;
	if (VTConnect(cxn) != kVTCNotInitialized) {
		return 7;
	}

	/* can't really test VTConnect() beyond this without something to connect() to */
	return 0;
}

struct testEntry {
	int (*testEntry)();
	char * testDescription;
};

typedef struct testEntry tTestEntry;

tTestEntry tests[] = {
	{
		&(test_VTAllocConnection_1),
		"VTAllocConnection allocates"
	},
	{
		&(test_VTCleanUpConnection_1),
		"VTCleanUpConnection cleans up"
	},
	{
		&(test_VTDeallocConnection_1),
		"VTDeallocConnection deallocates"
	},
	{
		&(test_VTInitConnection_1),
		"VTInitConnection initializes"
	},
	{
		&(test_VTConnect_1),
		"VTConnect fails if state not kvtsClosed"
	}
};

int main(int argc, char ** argv)
{
	int numTests = sizeof(tests) / sizeof(tests[0]);
	int failedTests = 0;
	int i;
	int rc;
	
	for (i = 0; i < numTests; i++) {
		if ((rc = tests[i].testEntry())) {
			fprintf(stderr,
				"test %d (%s) returned error %d\n",
				i, tests[i].testDescription, rc);
			failedTests++;
		}
	}
	if (failedTests) {
		exit(1);
	}
	exit(0);
}

/* Local Variables: */
/* c-indent-level: 0 */
/* c-continued-statement-offset: 4 */
/* c-brace-offset: 0 */
/* c-argdecl-indent: 4 */
/* c-label-offset: -4 */
/* End: */
