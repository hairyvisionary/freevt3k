/*
 * @(#) dumpbuf.c: 93/11/12-15:35:00
 */

FILE
	*debug_fd = NULL;

#ifdef __STDC__
void DumpBuffer(void *buf, int buf_len, char *dump_id)
#else
void DumpBuffer(buf, buf_len, dump_id)

    unsigned char
	*buf;
    int
	buf_len;
    char
	*dump_id;
#endif

{ /*DumpBuffer*/

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
    extern int
	debug_need_crlf;

    if (debug_fd == (FILE*)NULL)
	{
	debug_fd = fopen("freevt3k.debug", "w");
	if (debug_fd == (FILE*)NULL)
	    debug_fd = stderr;
	}

    if (debug_need_crlf)
	{
	debug_need_crlf = 0;
	fprintf(debug_fd, "\n");
	}
    fprintf(debug_fd, "[ %s ]\n", dump_id);
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
		fprintf(debug_fd, "%04X: ", offset);
		offset += CHAR_PER_LINE;
		}
	    fprintf(debug_fd, "%s\n", msg);
	    }
	}
    fflush(debug_fd);
    
} /*DumpBuffer*/
