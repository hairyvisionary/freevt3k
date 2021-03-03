#include "vtconn.c"

/* globale from freevt3k.c */
int debug = 0;
int debug_need_crlf = 0;
int table_spec = 0;
unsigned char out_table[256];

int test_VTAllocConnection()
{
	tVTConnection *cxn = NULL;

	cxn = VTAllocConnection();
	return (cxn == NULL);
}

struct testEntry {
	int (*testEntry)();
	char * testDescription;
};

typedef struct testEntry tTestEntry;

tTestEntry tests[] = {
	{
		&(test_VTAllocConnection),
		"VTAllocConnection allocates"
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
				"test %d (%s) returns %d\n",
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
