/* Circular VT queue parms */
#define MAX_VT_QUEUE		(kVT_MAX_BUFFER)
char
	vt_ch = 0,
	vt_queue[MAX_VT_QUEUE],
	*vtq_rptr = vt_queue,
	*vtq_rptr_hold = vt_queue,
	*vtq_wptr = vt_queue;
int
	int_vt_ch = 0,
	vt_queue_len_hold = 0,
	vt_queue_len = 0;

#ifdef __STDC__
static int GetVTQueue(void)
#else
static int GetVTQueue()
#endif
{ /*GetVTQueue*/

    if (!vt_queue_len)
	{
	int_vt_ch = -1;
	vt_ch = (char)int_vt_ch;
	return(-1);
	}
    if (++vtq_rptr == &vt_queue[MAX_VT_QUEUE])
	vtq_rptr = vt_queue;
    --vt_queue_len;
    vt_ch = *vtq_rptr;
    int_vt_ch = (int)vt_ch;
    return(0);
    
} /*GetVTQueue*/

#ifdef __STDC__
static int PutVTQueue(char ch)
#else
static int PutVTQueue(ch)
    char
	ch;
#endif
{ /*PutVTQueue*/

    if (++vtq_wptr == &vt_queue[MAX_VT_QUEUE])
	vtq_wptr = vt_queue;
    if (vtq_wptr == vtq_rptr)
	{
	fprintf(stderr, "<queue overflow>\n");
	return(-1);
	}
    ++vt_queue_len;
    *vtq_wptr = ch;
    return(0);

} /*PutVTQueue*/

#ifdef __STDC__
static int GetNextChar(void)
#else
static int GetNextChar()
#endif
{ /*GetNextChar*/

    if (GetVTQueue() == -1)
	{
/*
 * Ran out of data, reset the read pointer/length to the held value
 *   so we can resume with any escape sequence that we may have been
 *   in the middle of.
 */
	vtq_rptr = vtq_rptr_hold;
	vt_queue_len = vt_queue_len_hold;
	return(0);
	}
    return(1);

} /*GetNextChar*/

#ifdef __STDC__
static char TtyLineDraw(char ch)
#else
static char TtyLineDraw(ch)
    char
    	ch;
#endif
{ /*TtyLineDraw*/

    switch (toupper((int)ch))
	{
    case 'G':
    case 'P':
    case 'S':	return('+');	/* lower right corner */
	
    case 'H':
    case 'I':
    case 'K':
    case 'L':
    case 'O':
    case 'T':
    case 'W':
    case 'Y':	return('+');	/* upper right corner */
	
    case '=':
    case 'Q':
    case 'R':	return('+');	/* upper left corner */
	
    case '-':
    case 'A':
    case 'F':	return('+');	/* lower left corner */
	
    case '*':
    case '+':
    case '/':
    case '0':
    case '<':
    case '>':
    case '?':
    case 'B':
    case 'M':
    case 'N':
    case 'V':	return('+');	/* crossing lines */
	
    case ',':
    case '9':
    case ';':
    case 'Z':
    case '\\':
    case '^':
    case '|':
    case '~':	return('-');	/* horizontal line */
	
    case '!':
    case '%':
    case '1':
    case '5':
    case '@':
    case '`':	return('+');	/* left T */
	
    case '\"':
    case '&':
    case '2':
    case '6':
    case 'U':
    case '[':
    case '{':	return('+');	/* right T */
	
    case '#':
    case '3':
    case '7':
    case 'J':
    case '\'':
    case '_':	return('+');	/* top T */

    case '$':
    case '(':
    case '4':
    case '8':
    case ']':
    case '}':	return('+');	/* bottom T */

    case ')':
    case '.':
    case ':':
    case 'C':
    case 'D':
    case 'E':
    case 'X':	return('|');	/* vertical line */

    default:	return(ch);
		}

} /*TtyLineDraw*/

#ifdef __STDC__
static char VT100LineDraw(char ch)
#else
static char VT100LineDraw(ch)
    char
    	ch;
#endif
{ /*VT100LineDraw*/

    switch (toupper((int)ch))
	{
    case 'G':
    case 'P':
    case 'S':	return('j');	/* lower right corner */
	
    case 'H':
    case 'I':
    case 'K':
    case 'L':
    case 'O':
    case 'T':
    case 'W':
    case 'Y':	return('k');	/* upper right corner */
	
    case '=':
    case 'Q':
    case 'R':	return('l');	/* upper left corner */
	
    case '-':
    case 'A':
    case 'F':	return('m');	/* lower left corner */
	
    case '*':
    case '+':
    case '/':
    case '0':
    case '<':
    case '>':
    case '?':
    case 'B':
    case 'M':
    case 'N':
    case 'V':	return('n');	/* crossing lines */
	
    case ',':
    case '9':
    case ';':
    case 'Z':
    case '\\':
    case '^':
    case '|':
    case '~':	return('q');	/* horizontal line: scan 5 */
	
    case '!':
    case '%':
    case '1':
    case '5':
    case '@':
    case '`':	return('t');	/* left T */
	
    case '\"':
    case '&':
    case '2':
    case '6':
    case 'U':
    case '[':
    case '{':	return('u');	/* right T */
	
    case '#':
    case '3':
    case '7':
    case 'J':
    case '\'':
    case '_':	return('w');	/* top T */

    case '$':
    case '(':
    case '4':
    case '8':
    case ']':
    case '}':	return('v');	/* bottom T */

    case ')':
    case '.':
    case ':':
    case 'C':
    case 'D':
    case 'E':
    case 'X':	return('x');	/* vertical line */

    default:	return(ch);
		}

} /*VT100LineDraw*/

#ifdef __STDC__
void vt3kHPtoVT100(int32 refCon, char *buf, int buf_len)
#else
void vt3kHPtoVT100(refCon, buf, buf_len)
    int32
    	refCon;
    char
    	*buf;
    int
    	buf_len;
#endif
{ /*vt3kHPtoVT100*/

#define	ASC_SO			(0x0E)
#define	ASC_SI			(0x0F)
#define	ASC_DC1			(0x11)
#define	ASC_ESC			(0x1B)
    int
	i = 0,
	row_position = 1,
	num_val = 0,
	row = 0,
	col = 0,
	move_relative = 0,
	display_enh = 0;
    char
	*csi = "\033[",
	out_buf[MAX_VT_QUEUE],
	num_buf[256],
	*num_ptr = num_buf,
	*out_ptr = out_buf;
    static int
	line_draw = 0;

    if (!buf_len)
	return;

/* Copy raw data to the queue */
    for (; i<buf_len; i++)
	PutVTQueue(buf[i]);

    if (debug)
	{
	int hold_len = vt_queue_len;
	char *ptr;
	ptr = vtq_rptr;
	while (GetVTQueue() != -1)
	    *(out_ptr++) = vt_ch;
	DumpBuffer(out_buf, out_ptr - out_buf, "hp");
	vtq_rptr = ptr;
	vt_queue_len = hold_len;
	}

    out_ptr = out_buf;
    for (;;)
	{
	vtq_rptr_hold = vtq_rptr;
	vt_queue_len_hold = vt_queue_len;
	if (GetVTQueue() == -1)
	    break;
	if (vt_ch == ASC_SI)
	    {
	    line_draw = 0;
	    out_ptr += sprintf(out_ptr, "%c(B%cG", ASC_ESC, ASC_ESC);
	    continue;
	    }
	if (vt_ch == ASC_SO)
	    {
	    line_draw = 1;
	    out_ptr += sprintf(out_ptr, "%cF%c(0%c", ASC_ESC, ASC_ESC, ASC_SI);
	    continue;
	    }
	if (vt_ch != ASC_ESC)
	    {
	    *(out_ptr++) = (line_draw) ? VT100LineDraw(vt_ch) : vt_ch;
	    continue;
	    }
	if (!GetNextChar())
	    goto Do_Write;
	if (isalpha(int_vt_ch))
	    { /* ESC+alpha */
	    switch (int_vt_ch)
		{
	    case 'A':	/* Cursor up */
		out_ptr += sprintf(out_ptr, "%s1A", csi);
		break;
	    case 'B':	/* Cursor down */
		out_ptr += sprintf(out_ptr, "%s1B", csi);
		break;
	    case 'b':	/* Keyboard unlock */
		out_ptr += sprintf(out_ptr, "%s2l", csi);
		break;
	    case 'C':	/* Cursor right */
		out_ptr += sprintf(out_ptr, "%s1C", csi);
		break;
	    case 'c':	/* Keyboard lock */
		out_ptr += sprintf(out_ptr, "%s2h", csi);
		break;
	    case 'D':	/* Cursor left */
		out_ptr += sprintf(out_ptr, "%s1D", csi);
		break;
	    case 'E':	/* Hard reset */
		out_ptr += sprintf(out_ptr, "%cc", ASC_ESC);
		break;
	    case 'F':	/* Home down */
		out_ptr += sprintf(out_ptr, "%s>1s", csi);
		break;
	    case 'G':	/* Move cursor to left margin ??? */
		out_ptr += sprintf(out_ptr, "%s;1H", csi);
		break;
	    case 'h':
	    case 'H':	/* Home cursor */
		out_ptr += sprintf(out_ptr, "%s1;1H", csi);
		break;
	    case 'i':	/* Back tab */
		out_ptr += sprintf(out_ptr, "%s1Z", csi);
		break;
	    case 'J':	/* Clear to end of memory */
		out_ptr += sprintf(out_ptr, "%s0J", csi);
		break;
	    case 'K':	/* Erase to end of line */
		out_ptr += sprintf(out_ptr, "%s0K", csi);
		break;
	    case 'L':	/* Insert line */
		out_ptr += sprintf(out_ptr, "%s1L", csi);
		break;
	    case 'M':	/* Delete line */
		out_ptr += sprintf(out_ptr, "%s1M", csi);
		break;
	    case 'P':	/* Delete char */
		out_ptr += sprintf(out_ptr, "%s1P", csi);
		break;
	    case 'Q':	/* Insert char on */
		out_ptr += sprintf(out_ptr, "%s4h", csi);
		break;
	    case 'R':	/* Insert char off */
		out_ptr += sprintf(out_ptr, "%s4l", csi);
		break;
	    case 'S':	/* Scroll up */
		out_ptr += sprintf(out_ptr, "%s1S", csi);
		break;
	    case 'T':	/* Scroll down */
		out_ptr += sprintf(out_ptr, "%s1T", csi);
		break;
	    case 'U':	/* Page down (next) */
		out_ptr += sprintf(out_ptr, "%s1U", csi);
		break;
	    case 'V':	/* Page up (prev) */
		out_ptr += sprintf(out_ptr, "%s1V", csi);
		break;
	    case 'Y':	/* Display fns on */
		out_ptr += sprintf(out_ptr, "%s3h", csi);
		break;
	    case 'Z':	/* Display fns off */
		out_ptr += sprintf(out_ptr, "%s3l", csi);
		break;
	    default:	/* ??? */
		break;
		}
	    continue;
	    }
	if (vt_ch != '&')
	    { /* ESC+anything_but_ampersand */
	    for (;;)
		{ /* Munch till upper case or DC1 - may go too far */
		if (!GetNextChar())
		    goto Do_Write;
		if ((isupper(int_vt_ch)) || (int_vt_ch == ASC_DC1))
		    break;
		}
	    continue;
	    }
/* ESC+"&" sequences */
	if (!GetNextChar())
	    goto Do_Write;
	if (vt_ch == 'd')
	    { /* ESC+"&dx": Display enhancements */
	    if (!GetNextChar())
		goto Do_Write;
/*
@                                                0000
A                                       Blink    0001
B                              Inverse           0010
C                              Inverse  Blink    0011
D                    Underline                   0100
E                    Underline          Blink    0101
F                    Underline Inverse           0110
G                    Underline Inverse  Blink    0111
H           Half                                 1000
I           Half                        Blink    1001
J           Half               Inverse           1010
K           Half               Inverse  Blink    1011
L           Half     Underline                   1100
M           Half     Underline          Blink    1101
N           Half     Underline Inverse           1110
O           Half     Underline Inverse  Blink    1111
 */
	    display_enh = int_vt_ch - '@';
	    out_ptr += sprintf(out_ptr, "%s", csi);
	    if (!display_enh)			/* Normal */
		out_ptr += sprintf(out_ptr, "0;");
	    else
		{
		if (display_enh & 0x01)		/* Blink */
		    out_ptr += sprintf(out_ptr, "5;");
		if (display_enh & 0x02)		/* Inverse */
		    out_ptr += sprintf(out_ptr, "7;");
		if (display_enh & 0x04)		/* Underline */
		    out_ptr += sprintf(out_ptr, "4;");
		if (!(display_enh & 0x08))	/* Half */
		    out_ptr += sprintf(out_ptr, "1;");
		}
	    out_ptr[-1] = 'm';
	    continue;
	    }
	if (vt_ch != 'a')
	    { /* Anything other than cursor address: munch till upper case */
	    for (;;)
		{
		if (!GetNextChar())
		    goto Do_Write;
		if (isupper(int_vt_ch))
		    break;
		}
	    continue;
	    }
/* ESC+"&a [[+|-]n{r|c|x|y}] [[+|-]n{R|C|X|Y}]" */
	if (!GetNextChar())
	    goto Do_Write;
/* If prefaced with '+|-', this is a cursor relative move */
	if ((vt_ch == '+') || (vt_ch == '-'))
	    move_relative = 1;
/* Get numeric row/column value */
	num_ptr = num_buf;
	*(num_ptr++) = vt_ch;
	for (;;)
	    {
	    if (!GetNextChar())
		goto Do_Write;
	    if (isalpha(int_vt_ch))
		break;
	    *(num_ptr++) = vt_ch;
	    }
	*num_ptr = '\0';
	num_val = atoi(num_buf);
	if ((toupper(int_vt_ch) == 'C') || (toupper(int_vt_ch) == 'X'))
	    {
	    row_position = 0;
	    col = ++num_val;
	    }
	else
	    row = ++num_val;
	if (isupper(int_vt_ch))
	    { /* End of sequence: just row or column position */
	    if (row_position)
		{
		if (move_relative)
		    {
		    if (num_val > 0)
			out_ptr += sprintf(out_ptr, "%s%dC", csi, num_val);
		    else
			out_ptr += sprintf(out_ptr, "%s%dD", csi, -num_val);
		    }
		else
		    out_ptr += sprintf(out_ptr, "%s%dH", csi, num_val);
		}
	    else
		{
		if (move_relative)
		    {
		    if (num_val > 0)
			out_ptr += sprintf(out_ptr, "%s%dB", csi, num_val);
		    else
			out_ptr += sprintf(out_ptr, "%s%dA", csi, -num_val);
		    }
		else
		    out_ptr += sprintf(out_ptr, "%s;%dH", csi, num_val);
		}
	    continue;
	    }
/* Get numeric row/column value */
	num_ptr = num_buf;
	for (;;)
	    {
	    if (!GetNextChar())
		goto Do_Write;
	    if (isalpha(int_vt_ch))
		break;
	    *(num_ptr++) = vt_ch;
	    }
	*num_ptr = '\0';
	num_val = atoi(num_buf);
	if ((vt_ch == 'C') || (vt_ch == 'X'))
	    col = ++num_val;
	else
	    row = ++num_val;
	out_ptr += sprintf(out_ptr, "%s%d;%dH", csi, row, col);
	} /* main for() loop */
 Do_Write:
    buf_len = out_ptr - out_buf;
    vt3kDataOutProc(refCon, out_buf, buf_len);
    if (debug)
	DumpBuffer(out_buf, buf_len, "vt100");

} /*vt3kHPtoVT100*/

#ifdef __STDC__
void vt3kHPtoGeneric(int32 refCon, char *buf, int buf_len)
#else
void vt3kHPtoGeneric(refCon, buf, buf_len)
    int32
    	refCon;
    char
    	*buf;
    int
    	buf_len;
#endif
{ /*vt3kHPtoGeneric*/

#define	ASC_SO			(0x0E)
#define	ASC_SI			(0x0F)
#define	ASC_DC1			(0x11)
#define	ASC_ESC			(0x1B)
    int
	i = 0,
	row_position = 1,
	num_val = 0,
	row = 0,
	col = 0,
	move_relative = 0,
	display_enh = 0;
    char
	out_buf[MAX_VT_QUEUE],
	num_buf[256],
	*num_ptr = num_buf,
	*out_ptr = out_buf;

    if (!buf_len)
	return;

/* Copy raw data to the queue */
    for (; i<buf_len; i++)
	PutVTQueue(buf[i]);

    if (debug)
	{
	int hold_len = vt_queue_len;
	char *ptr;
	ptr = vtq_rptr;
	while (GetVTQueue() != -1)
	    *(out_ptr++) = vt_ch;
	DumpBuffer(out_buf, out_ptr - out_buf, "hp");
	vtq_rptr = ptr;
	vt_queue_len = hold_len;
	}

    out_ptr = out_buf;
    for (;;)
	{
	vtq_rptr_hold = vtq_rptr;
	vt_queue_len_hold = vt_queue_len;
	if (GetVTQueue() == -1)
	    break;
	if (vt_ch == ASC_SI)
	    {
	    out_ptr += sprintf(out_ptr, "<line_draw_off>");
	    continue;
	    }
	if (vt_ch == ASC_SO)
	    {
	    out_ptr += sprintf(out_ptr, "<line_draw_on>");
	    continue;
	    }
	if (vt_ch != ASC_ESC)
	    {
	    if (!int_vt_ch)
		out_ptr += sprintf(out_ptr, "<nul>");
	    else
		*(out_ptr++) = vt_ch;
	    continue;
	    }
	if (!GetNextChar())
	    goto Do_Write;
	if (vt_ch == '[')
	    {
	    out_ptr += sprintf(out_ptr, "<protect_on>");
	    continue;
	    }
	if (vt_ch == ']')
	    {
	    out_ptr += sprintf(out_ptr, "<protect_off>");
	    continue;
	    }
	if (vt_ch == '@')
	    {
	    out_ptr += sprintf(out_ptr, "<pause>");
	    continue;
	    }
	if (isalpha(int_vt_ch))
	    { /* ESC+alpha */
	    switch (int_vt_ch)
		{
	    case 'A':	/* Cursor up */
		out_ptr += sprintf(out_ptr, "<move:up>");
		break;
	    case 'a':	/* Cursor sense */
		out_ptr += sprintf(out_ptr, "<cursor_sense>");
		break;
	    case 'B':	/* Cursor down */
		out_ptr += sprintf(out_ptr, "<move:down>");
		break;
	    case 'b':	/* Keyboard unlock */
		out_ptr += sprintf(out_ptr, "<unlock_keyboard>");
		break;
	    case 'C':	/* Cursor right */
		out_ptr += sprintf(out_ptr, "<move:right>");
		break;
	    case 'c':	/* Keyboard lock */
		out_ptr += sprintf(out_ptr, "<lock_keyboard>");
		break;
	    case 'D':	/* Cursor left */
		out_ptr += sprintf(out_ptr, "<move:left>");
		break;
	    case 'd':	/* Read */
		out_ptr += sprintf(out_ptr, "<read>");
		break;
	    case 'E':	/* Hard reset */
		out_ptr += sprintf(out_ptr, "<hard_reset>");
		break;
	    case 'F':	/* Home bottom */
		out_ptr += sprintf(out_ptr, "<home_bottom>");
		break;
	    case 'g':	/* Soft reset */
		out_ptr += sprintf(out_ptr, "<soft_reset>");
		break;
	    case 'G':	/* Move cursor to left margin */
		out_ptr += sprintf(out_ptr, "<move:left_margin>");
		break;
	    case 'h':
	    case 'H':	/* Home cursor */
		out_ptr += sprintf(out_ptr, "<home>");
		break;
	    case 'J':	/* Clear to end of memory */
		out_ptr += sprintf(out_ptr, "<clear_eos>");
		break;
	    case 'K':	/* Erase to end of line */
		out_ptr += sprintf(out_ptr, "<clear_eol>");
		break;
	    case 'L':	/* Insert line */
		out_ptr += sprintf(out_ptr, "<insert_line>");
		break;
	    case 'M':	/* Delete line */
		out_ptr += sprintf(out_ptr, "<delete_line>");
		break;
	    case 'P':	/* Delete char */
		out_ptr += sprintf(out_ptr, "<delete_char>");
		break;
	    case 'Q':	/* Insert char on */
		out_ptr += sprintf(out_ptr, "<insert_char_on>");
		break;
	    case 'R':	/* Insert char off */
		out_ptr += sprintf(out_ptr, "<insert_char_off>");
		break;
	    case 'S':	/* Scroll up */
		out_ptr += sprintf(out_ptr, "<scroll_up>");
		break;
	    case 'T':	/* Scroll down */
		out_ptr += sprintf(out_ptr, "<scroll_down>");
		break;
	    case 'U':	/* Page down (next) */
		out_ptr += sprintf(out_ptr, "<page_down>");
		break;
	    case 'V':	/* Page up (prev) */
		out_ptr += sprintf(out_ptr, "<page_up>");
		break;
	    case 'W':	/* Format mode on */
		out_ptr += sprintf(out_ptr, "<format_on>");
		break;
	    case 'X':	/* Format mode off */
		out_ptr += sprintf(out_ptr, "<format_off>");
		break;
	    case 'Y':	/* Display fns on */
		out_ptr += sprintf(out_ptr, "<disp_fns_on>");
		break;
	    case 'Z':	/* Display fns off */
		out_ptr += sprintf(out_ptr, "<disp_fns_off>");
		break;
	    default:	/* ??? */
		out_ptr += sprintf(out_ptr, "<esc+\"%c\">", vt_ch);
		break;
		}
	    continue;
	    }
	if (vt_ch != '&')
	    { /* ESC+anything_but_ampersand */
	    char temp[256], *ptr = temp;
	    ptr += sprintf(ptr, "<esc+\"%c", vt_ch);
	    for (;;)
		{ /* Munch till upper case or DC1 - may go too far */
		if (!GetNextChar())
		    goto Do_Write;
		*(ptr++) = vt_ch;
		if ((isupper(int_vt_ch)) || (vt_ch == '^') || (vt_ch == '~') || (vt_ch == ASC_DC1))
		    break;
		}
	    ptr += sprintf(ptr, "\">");
	    out_ptr += sprintf(out_ptr, "%s", temp);
	    continue;
	    }
/* ESC+"&" sequences */
	if (!GetNextChar())
	    goto Do_Write;
	if (vt_ch == 'd')
	    { /* ESC+"&dx": Display enhancements */
	    if (!GetNextChar())
		goto Do_Write;
/*
@                                                0000
A                                       Blink    0001
B                              Inverse           0010
C                              Inverse  Blink    0011
D                    Underline                   0100
E                    Underline          Blink    0101
F                    Underline Inverse           0110
G                    Underline Inverse  Blink    0111
H           Half                                 1000
I           Half                        Blink    1001
J           Half               Inverse           1010
K           Half               Inverse  Blink    1011
L           Half     Underline                   1100
M           Half     Underline          Blink    1101
N           Half     Underline Inverse           1110
O           Half     Underline Inverse  Blink    1111
 */
	    display_enh = int_vt_ch - '@';
	    if (!display_enh)		/* Normal */
		out_ptr += sprintf(out_ptr, "<normal>");
	    else
		{ /* These can be added - need to do multiples */
		if (display_enh & 0x01)	/* Blink */
		    out_ptr += sprintf(out_ptr, "<blink>");
		if (display_enh & 0x02)	/* Inverse */
		    out_ptr += sprintf(out_ptr, "<reverse>");
		if (display_enh & 0x04)	/* Underline */
		    out_ptr += sprintf(out_ptr, "<underline>");
		if (display_enh & 0x08)	/* Half */
		    out_ptr += sprintf(out_ptr, "<dim>");
		}
	    continue;
	    }
	if (vt_ch != 'a')
	    { /* Anything other than cursor address: munch till upper case */
	    char temp[256], *ptr = temp;
	    ptr += sprintf(ptr, "<esc+\"&%c", vt_ch);
	    for (;;)
		{
		if (!GetNextChar())
		    goto Do_Write;
		*(ptr++) = vt_ch;
		if ((isupper(int_vt_ch)) || (vt_ch == '@'))
		    break;
		}
	    ptr += sprintf(ptr, "\">");
	    out_ptr += sprintf(out_ptr, "%s", temp);
	    continue;
	    }
/* ESC+"&a [[+|-]n{r|c|x|y}] [[+|-]n{R|C|X|Y}]" */
	if (!GetNextChar())
	    goto Do_Write;
/* If prefaced with '+|-', this is a cursor relative move */
	if ((vt_ch == '+') || (vt_ch == '-'))
	    move_relative = 1;
/* Get numeric row/column value */
	num_ptr = num_buf;
	*(num_ptr++) = vt_ch;
	for (;;)
	    {
	    if (!GetNextChar())
		goto Do_Write;
	    if (isalpha(int_vt_ch))
		break;
	    *(num_ptr++) = vt_ch;
	    }
	*num_ptr = '\0';
	num_val = atoi(num_buf);
	if ((toupper(int_vt_ch) == 'C') || (toupper(int_vt_ch) == 'X'))
	    {
	    row_position = 0;
	    col = num_val;
	    }
	else
	    row = num_val;
	if (isupper(int_vt_ch))
	    { /* End of sequence: just row or column position */
	    if (row_position)
		{
		if (move_relative)
		    out_ptr += sprintf(out_ptr, "<move:r=%+d>", row);
		else
		    out_ptr += sprintf(out_ptr, "<move:r=%d>", row);
		}
	    else
		{
		if (move_relative)
		    out_ptr += sprintf(out_ptr, "<move:c=%+d>", col);
		else
		    out_ptr += sprintf(out_ptr, "<move:c=%d>", col);
		}
	    continue;
	    }
/* Get numeric row/column value */
	num_ptr = num_buf;
	for (;;)
	    {
	    if (!GetNextChar())
		goto Do_Write;
	    if (isalpha(int_vt_ch))
		break;
	    *(num_ptr++) = vt_ch;
	    }
	*num_ptr = '\0';
	num_val = atoi(num_buf);
	if ((vt_ch == 'C') || (vt_ch == 'X'))
	    col = num_val;
	else
	    row = num_val;
	out_ptr += sprintf(out_ptr, "<move:r=%d,c=%d>", row, col);
	} /* main for() loop */
 Do_Write:
    buf_len = out_ptr - out_buf;
    vt3kDataOutProc(refCon, out_buf, buf_len);
    if (debug)
	DumpBuffer(out_buf, buf_len, "Generic");

} /*vt3kHPtoGeneric*/

#ifdef __STDC__
void vt3kHPtoVT52(int32 refCon, char *buf, int buf_len)
#else
void vt3kHPtoVT52(refCon, buf, buf_len)
    int32
    	refCon;
    char
    	*buf;
    int
    	buf_len;
#endif
{ /*vt3kHPtoVT52*/

#define	ASC_SO			(0x0E)
#define	ASC_SI			(0x0F)
#define	ASC_DC1			(0x11)
#define	ASC_ESC			(0x1B)
    int
	i = 0,
	row_position = 1,
	num_val = 0,
	row = 0,
	col = 0,
	move_relative = 0,
	display_enh = 0;
    char
	*csi = "\033[",
	out_buf[MAX_VT_QUEUE],
	num_buf[256],
	*num_ptr = num_buf,
	*out_ptr = out_buf;
    static int
	line_draw = 0;

    if (!buf_len)
	return;

/* Copy raw data to the queue */
    for (; i<buf_len; i++)
	PutVTQueue(buf[i]);

    if (debug)
	{
	int hold_len = vt_queue_len;
	char *ptr;
	ptr = vtq_rptr;
	while (GetVTQueue() != -1)
	    *(out_ptr++) = vt_ch;
	DumpBuffer(out_buf, out_ptr - out_buf, "hp");
	vtq_rptr = ptr;
	vt_queue_len = hold_len;
	}

    out_ptr = out_buf;
    for (;;)
	{
	vtq_rptr_hold = vtq_rptr;
	vt_queue_len_hold = vt_queue_len;
	if (GetVTQueue() == -1)
	    break;
	if (vt_ch == ASC_SI)
	    {
	    line_draw = 0;
/*	    out_ptr += sprintf(out_ptr, "%c(B%cG", ASC_ESC, ASC_ESC); */
	    continue;
	    }
	if (vt_ch == ASC_SO)
	    {
	    line_draw = 1;
/*	    out_ptr += sprintf(out_ptr, "%cF%c(0%c", ASC_ESC, ASC_ESC, ASC_SI); */
	    continue;
	    }
	if (vt_ch != ASC_ESC)
	    {
	    *(out_ptr++) = (line_draw) ? TtyLineDraw(vt_ch) : vt_ch;
	    continue;
	    }
	if (!GetNextChar())
	    goto Do_Write;
	if (isalpha(int_vt_ch))
	    { /* ESC+alpha */
	    switch (int_vt_ch)
		{
	    case 'A':	/* Cursor up */
		out_ptr += sprintf(out_ptr, "%cA", ASC_ESC);
		break;
	    case 'B':	/* Cursor down */
		out_ptr += sprintf(out_ptr, "%cB", ASC_ESC);
		break;
	    case 'C':	/* Cursor right */
		out_ptr += sprintf(out_ptr, "%cC", ASC_ESC);
		break;
	    case 'D':	/* Cursor left */
		out_ptr += sprintf(out_ptr, "%cD", ASC_ESC);
		break;
	    case 'E':
	    case 'g':	/* Hard|soft reset */
		out_ptr += sprintf(out_ptr, "%cc", ASC_ESC);
		break;
	    case 'h':
	    case 'H':	/* Home cursor */
		out_ptr += sprintf(out_ptr, "%cH", ASC_ESC);
		break;
	    case 'J':	/* Clear to end of memory */
		out_ptr += sprintf(out_ptr, "%cJ", ASC_ESC);
		break;
	    case 'K':	/* Erase to end of line */
		out_ptr += sprintf(out_ptr, "%cK", ASC_ESC);
		break;
	    default:	/* ??? */
		break;
		}
	    continue;
	    }
	if (vt_ch != '&')
	    { /* ESC+anything_but_ampersand */
	    for (;;)
		{ /* Munch till upper case or DC1 - may go too far */
		if (!GetNextChar())
		    goto Do_Write;
		if ((isupper(int_vt_ch)) || (int_vt_ch == ASC_DC1))
		    break;
		}
	    continue;
	    }
/* ESC+"&" sequences */
	if (!GetNextChar())
	    goto Do_Write;
	if (vt_ch == 'd')
	    { /* ESC+"&dx": Display enhancements */
	    if (!GetNextChar())
		goto Do_Write;
/*
@                                                0000
A                                       Blink    0001
B                              Inverse           0010
C                              Inverse  Blink    0011
D                    Underline                   0100
E                    Underline          Blink    0101
F                    Underline Inverse           0110
G                    Underline Inverse  Blink    0111
H           Half                                 1000
I           Half                        Blink    1001
J           Half               Inverse           1010
K           Half               Inverse  Blink    1011
L           Half     Underline                   1100
M           Half     Underline          Blink    1101
N           Half     Underline Inverse           1110
O           Half     Underline Inverse  Blink    1111
 */
	    display_enh = int_vt_ch - '@';
	    out_ptr += sprintf(out_ptr, "%s", csi);
	    if (!display_enh)			/* Normal */
		out_ptr += sprintf(out_ptr, "0;");
	    else
		{
		if (display_enh & 0x01)		/* Blink */
		    out_ptr += sprintf(out_ptr, "5;");
		if (display_enh & 0x02)		/* Inverse */
		    out_ptr += sprintf(out_ptr, "7;");
		if (display_enh & 0x04)		/* Underline */
		    out_ptr += sprintf(out_ptr, "4;");
		if (!(display_enh & 0x08))	/* Half */
		    out_ptr += sprintf(out_ptr, "1;");
		}
	    out_ptr[-1] = 'm';
	    continue;
	    }
	if (vt_ch != 'a')
	    { /* Anything other than cursor address: munch till upper case */
	    for (;;)
		{
		if (!GetNextChar())
		    goto Do_Write;
		if (isupper(int_vt_ch))
		    break;
		}
	    continue;
	    }
/* ESC+"&a [[+|-]n{r|c|x|y}] [[+|-]n{R|C|X|Y}]" */
	if (!GetNextChar())
	    goto Do_Write;
/* If prefaced with '+|-', this is a cursor relative move */
	if ((vt_ch == '+') || (vt_ch == '-'))
	    move_relative = 1;
/* Get numeric row/column value */
	num_ptr = num_buf;
	*(num_ptr++) = vt_ch;
	for (;;)
	    {
	    if (!GetNextChar())
		goto Do_Write;
	    if (isalpha(int_vt_ch))
		break;
	    *(num_ptr++) = vt_ch;
	    }
	*num_ptr = '\0';
	num_val = atoi(num_buf);
	if ((toupper(int_vt_ch) == 'C') || (toupper(int_vt_ch) == 'X'))
	    {
	    row_position = 0;
	    col = ++num_val;
	    }
	else
	    row = ++num_val;
	if (isupper(int_vt_ch))
	    { /* End of sequence: just row or column position */
	    if (row_position)
		{
		if (move_relative)
		    {
		    if (num_val > 0)
			out_ptr += sprintf(out_ptr, "%s%dC", csi, num_val);
		    else
			out_ptr += sprintf(out_ptr, "%s%dD", csi, -num_val);
		    }
		else
		    out_ptr += sprintf(out_ptr, "%s%dH", csi, num_val);
		}
	    else
		{
		if (move_relative)
		    {
		    if (num_val > 0)
			out_ptr += sprintf(out_ptr, "%s%dB", csi, num_val);
		    else
			out_ptr += sprintf(out_ptr, "%s%dA", csi, -num_val);
		    }
		else
		    out_ptr += sprintf(out_ptr, "%s;%dH", csi, num_val);
		}
	    continue;
	    }
/* Get numeric row/column value */
	num_ptr = num_buf;
	for (;;)
	    {
	    if (!GetNextChar())
		goto Do_Write;
	    if (isalpha(int_vt_ch))
		break;
	    *(num_ptr++) = vt_ch;
	    }
	*num_ptr = '\0';
	num_val = atoi(num_buf);
	if ((vt_ch == 'C') || (vt_ch == 'X'))
	    col = ++num_val;
	else
	    row = ++num_val;
	out_ptr += sprintf(out_ptr, "%s%d;%dH", csi, row, col);
	} /* main for() loop */
 Do_Write:
    buf_len = out_ptr - out_buf;
    vt3kDataOutProc(refCon, out_buf, buf_len);
    if (debug)
	DumpBuffer(out_buf, buf_len, "vt52");

} /*vt3kHPtoVT52*/

#ifdef __STDC__
void TranslateKeyboard(char *buf, int *buf_len)
#else
void TranslateKeyboard(buf, buf_len)
    char
    	*buf;
    int
    	*buf_len;
#endif
{ /*TranslateKeyboard*/

    int
	key = 0;
    char
	*key_codes = "pqrst uvw";

    if (*buf_len < 5)
	return;
    if (memcmp(buf, "\033[1", 3))
	return;
    if (buf[4] != '~')
	return;
    key = atoi(&buf[2]);
    if ((key >= 11) && (key <= 19) && (key != 16))
	*buf_len = sprintf(buf, "\033%c", key_codes[(key-11)]);

} /*TranslateKeyboard*/

/* Local Variables: */
/* c-indent-level: 0 */
/* c-continued-statement-offset: 4 */
/* c-brace-offset: 0 */
/* c-argdecl-indent: 4 */
/* c-label-offset: -4 */
/* End: */
