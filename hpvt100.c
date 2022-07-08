/* Circular VT queue parms */
#define MAX_VT_QUEUE		(kVT_MAX_BUFFER)

char
    vt_ch = '\0',
    vt_queue[MAX_VT_QUEUE],
    *vtq_rptr = vt_queue,
    *vtq_rptr_hold = vt_queue,
    *vtq_wptr = vt_queue;
int
    vt_enhanced = 0,
    vt_queue_len_hold = 0,
    vt_queue_len = 0;

#include <stdarg.h>
int int_sprintf(char *buf, const char *fmt, ...)
{ /*int_sprintf*/

  va_list
    va_alist;

  va_start(va_alist, fmt);
  vsprintf(buf, fmt, va_alist);
  va_end(va_alist);
  return(strlen(buf));

} /*int_sprintf*/

static int GetVTQueue(void)
{ /*GetVTQueue*/

  if (!vt_queue_len)
    return(-1);
  if (++vtq_rptr == &vt_queue[MAX_VT_QUEUE])
    vtq_rptr = vt_queue;
  --vt_queue_len;
  vt_ch = *vtq_rptr;
  return(0);
    
} /*GetVTQueue*/

static int PutVTQueue(char ch)
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

static int GetNextChar(void)
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

static char *VT100DisplayEnhance(char ch)
{ /*VT100DisplayEnhance*/
/*                                      0x40+
@                                        0000
A                               Blink    0001
B                      Inverse           0010
C                      Inverse  Blink    0011
D            Underline                   0100
E            Underline          Blink    0101
F            Underline Inverse           0110
G            Underline Inverse  Blink    0111
H   Half                                 1000
I   Half                        Blink    1001
J   Half               Inverse           1010
K   Half               Inverse  Blink    1011
L   Half     Underline                   1100
M   Half     Underline          Blink    1101
N   Half     Underline Inverse           1110
O   Half     Underline Inverse  Blink    1111
 */

#define HPTERM_BLINK		(0x01)
#define HPTERM_INVERSE		(0x02)
#define HPTERM_UNDERLINE	(0x04)
#define HPTERM_BOLD		(0x08)
#define HPTERM_OPT_MASK		(HPTERM_BLINK | HPTERM_INVERSE | HPTERM_UNDERLINE | HPTERM_BOLD)

  static char
    buf[256];
#  define CSI			"\033["
  const char
    *blink = CSI "5m",
    *bold  = CSI "1m",
    *rev   = CSI "7m",	/* aka: smso */
    *smul  = CSI "4m",	/* set mode: underline */
    *sgr0  = CSI "m";	/* aka: rmso and rmul */

  if ((int)ch & HPTERM_OPT_MASK)
    {
      *buf = '\0';
      if ((int)ch & HPTERM_BLINK)
	strcat(buf, blink);
      if ((int)ch & HPTERM_INVERSE)
	strcat(buf, rev);
      if ((int)ch & HPTERM_UNDERLINE)
	strcat(buf, smul);
      if (!((int)ch & HPTERM_BOLD))
	strcat(buf, bold);
      vt_enhanced = 1;
    }
  else
    {
      strcpy(buf, sgr0);
      vt_enhanced = 0;
    }
  return(buf);

} /*VT100DisplayEnhance*/

static char *GenericDisplayEnhance(char ch)
{ /*GenericDisplayEnhance*/
/*                                      0x40+
@                                        0000
A                               Blink    0001
B                      Inverse           0010
C                      Inverse  Blink    0011
D            Underline                   0100
E            Underline          Blink    0101
F            Underline Inverse           0110
G            Underline Inverse  Blink    0111
H   Half                                 1000
I   Half                        Blink    1001
J   Half               Inverse           1010
K   Half               Inverse  Blink    1011
L   Half     Underline                   1100
M   Half     Underline          Blink    1101
N   Half     Underline Inverse           1110
O   Half     Underline Inverse  Blink    1111
 */

#define HPTERM_BLINK		(0x01)
#define HPTERM_INVERSE		(0x02)
#define HPTERM_UNDERLINE	(0x04)
#define HPTERM_BOLD		(0x08)
#define HPTERM_OPT_MASK		(HPTERM_BLINK | HPTERM_INVERSE | HPTERM_UNDERLINE | HPTERM_BOLD)

  static char
    buf[256];
  const char
    *blink = "<blink>",
    *bold  = "<bold>",
    *rev   = "<reverse>",
    *smul  = "<underline>",
    *sgr0  = "<dim>";

  if ((int)ch & HPTERM_OPT_MASK)
    {
      *buf = '\0';
      if ((int)ch & HPTERM_BLINK)
	strcat(buf, blink);
      if ((int)ch & HPTERM_INVERSE)
	strcat(buf, rev);
      if ((int)ch & HPTERM_UNDERLINE)
	strcat(buf, smul);
      if (!((int)ch & HPTERM_BOLD))
	strcat(buf, bold);
      vt_enhanced = 1;
    }
  else
    {
      strcpy(buf, sgr0);
      vt_enhanced = 0;
    }
  return(buf);

} /*GenericDisplayEnhance*/

static char TtyLineDraw(char ch)
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

static char VT100LineDraw(char ch)
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

void vt3kHPtoVT100(int32_t refCon, char *buf, size_t buf_len)
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
    hold_len;

  char
    out_buf[MAX_VT_QUEUE],
    num_buf[256],
    *num_ptr = num_buf,
    *out_ptr = out_buf;
#  define SI			"\017"
#  define ESC			"\033"
#  define CSI			"\033["
  const char
    *cup   = CSI "%d;%dH",	/* Cursor position */
    *cuu   = CSI "%dA",		/* Cursor up */
    *cud   = CSI "%dB",		/* Cursor down */
    *cuf   = CSI "%dC",		/* Cursor forward */
    *cub   = CSI "%dD",		/* Cursor back */
    *home  = CSI "1;1H",	/* Cursor home */
    *ed    = CSI "%dJ",		/* Erase to end-of-display */
    *el    = CSI "%dK",		/* Erase to end-of-line */
    *il    = CSI "%dL",		/* Insert line */
    *dl    = CSI "%dM",		/* Delete line */
    *dch   = CSI "%dP",		/* Delete char */
    *ris   = ESC "c",		/* Reset to initial */
    *sel_ascii  = ESC "G",	/* Select ASCII character set */
    *sel_graph  = ESC "F",	/* Select special graphics character set */
    *sel_g0     = SI,		/* Select G0 character sets from below */
    *g0_usascii = ESC "(B",	/* Specify USASCII set */
    *g0_graphic = ESC "(0";	/* Specify graphics/line drawing set */
  static int
    line_draw = 0;

  if (!buf_len)
    return;

/* Copy raw data to the queue */
  for (; i<buf_len; i++)
    PutVTQueue(buf[i]);

  if (debug)
    {
      hold_len = vt_queue_len;
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
	  out_ptr += int_sprintf(out_ptr, g0_usascii);
	  out_ptr += int_sprintf(out_ptr, sel_ascii);
	  continue;
	}
      if (vt_ch == ASC_SO)
	{
	  line_draw = 1;
	  out_ptr += int_sprintf(out_ptr, sel_graph);
	  out_ptr += int_sprintf(out_ptr, g0_graphic);
	  out_ptr += int_sprintf(out_ptr, sel_g0);
	  continue;
	}
      if (vt_ch != ASC_ESC)
	{
	  *(out_ptr++) = (char)((line_draw) ? VT100LineDraw(vt_ch) : vt_ch);
	  if ((vt_enhanced) && ((vt_ch == '\r') || (vt_ch == '\n')))
	    out_ptr += int_sprintf(out_ptr, VT100DisplayEnhance('@'));
	  continue;
	}
      if (!GetNextChar())
	goto Do_Write;
      if (isdigit((int)vt_ch))
	{ /* ESC+digit */
	  switch ((int)vt_ch)
	    {
	    case '1':	/* Set tab */
	      out_ptr += int_sprintf(out_ptr, "%cH", ASC_ESC);
	      break;
	    case '2':	/* Clear tab */
	      out_ptr += int_sprintf(out_ptr, CSI "0G");
	      break;
	    case '3':	/* Clear all tabs */
	      out_ptr += int_sprintf(out_ptr, CSI "3G");
	      break;
	    case '4':	/* Set Left Margin */
	      break;
	    case '5':	/* Set Right Margin */
	      break;
	    case '9':	/* Clear all margins */
	      break;
	    default:	/* ??? */
	      break;
	    }
	  continue;
	}
      if (isalpha((int)vt_ch))
	{ /* ESC+alpha */
	  switch ((int)vt_ch)
	    {
	    case 'A':	/* Cursor up */
	      out_ptr += int_sprintf(out_ptr, cuu, 1);
	      break;
	    case 'B':	/* Cursor down */
	      out_ptr += int_sprintf(out_ptr, cud, 1);
	      break;
	    case 'b':	/* Keyboard unlock */
	      out_ptr += int_sprintf(out_ptr, CSI "2l");
	      break;
	    case 'C':	/* Cursor forward */
	      out_ptr += int_sprintf(out_ptr, cuf, 1);
	      break;
	    case 'c':	/* Keyboard lock */
	      out_ptr += int_sprintf(out_ptr, CSI "2h");
	      break;
	    case 'D':	/* Cursor back */
	      out_ptr += int_sprintf(out_ptr, cub, 1);
	      break;
	    case 'E':	/* Hard reset */
	      out_ptr += int_sprintf(out_ptr, ris);
	      break;
	    case 'F':	/* Home down */
	      out_ptr += int_sprintf(out_ptr, CSI ">1s");
	      break;
	    case 'h':
	    case 'H':	/* Home cursor */
	      out_ptr += int_sprintf(out_ptr, home);
	      break;
	    case 'i':	/* Back tab */
	      out_ptr += int_sprintf(out_ptr, CSI "1Z");
	      break;
	    case 'J':	/* Clear to end of memory */
	      out_ptr += int_sprintf(out_ptr, ed, 0);
	      break;
	    case 'K':	/* Erase to end of line */
	      out_ptr += int_sprintf(out_ptr, el, 0);
	      break;
	    case 'L':	/* Insert line */
	      out_ptr += int_sprintf(out_ptr, il, 1);
	      break;
	    case 'M':	/* Delete line */
	      out_ptr += int_sprintf(out_ptr, dl, 1);
	      break;
	    case 'P':	/* Delete char */
	      out_ptr += int_sprintf(out_ptr, dch, 1);
	      break;
	    case 'Q':	/* Insert char on */
	      out_ptr += int_sprintf(out_ptr, CSI "4h");
	      break;
	    case 'R':	/* Insert char off */
	      out_ptr += int_sprintf(out_ptr, CSI "4l");
	      break;
	    case 'S':	/* Scroll up */
	      out_ptr += int_sprintf(out_ptr, CSI "1S");
	      break;
	    case 'T':	/* Scroll down */
	      out_ptr += int_sprintf(out_ptr, CSI "1T");
	      break;
	    case 'U':	/* Page down (next) */
	      out_ptr += int_sprintf(out_ptr, CSI "1U");
	      break;
	    case 'V':	/* Page up (prev) */
	      out_ptr += int_sprintf(out_ptr, CSI "1V");
	      break;
	    case 'Y':	/* Display fns on */
	      out_ptr += int_sprintf(out_ptr, CSI "3h");
	      break;
	    case 'Z':	/* Display fns off */
	      out_ptr += int_sprintf(out_ptr, CSI "3l");
	      break;
	    default:	/* ??? */
	      break;
	    }
	  continue;
	}
      if (vt_ch == '[')
	{ /* Special case to handle echo of outbound PF keys */
	  if (!GetNextChar())
	    goto Do_Write;
	  if (isdigit((int)vt_ch))
	    {
	      for (;;)
		{ /* Munch till '~' */
		  if (!GetNextChar())
		    goto Do_Write;
		  if (vt_ch == '~')
		    break;
		}
	      continue;
	    }
	  continue;
	}
      if ((vt_ch == '*') || (vt_ch == '^') || (vt_ch == '~'))
	{
	  int do_terminal_id = 0;
	  if (vt_ch == '^')
	    {
	      char *prim = ESC "\\?008000\r", *ptr = prim;
	      while (*ptr)
		{
		  if (PutImmediateQ(*ptr) == -1)
		    return;
		  ++ptr;
		}
	      continue;
	    }
	  if (vt_ch == '~')
	    {
	      char *sec = ESC "|0400000\r", *ptr = sec;
	      while (*ptr)
		{
		  if (PutImmediateQ(*ptr) == -1)
		    return;
		  ++ptr;
		}
	      continue;
	    }
	  if (!GetNextChar())
	    goto Do_Write;
	  if (vt_ch == 's')
	    do_terminal_id = 1;
	  for (;;)
	    {
	      if (!GetNextChar())
		goto Do_Write;
	      if (vt_ch == '^')
		break;
	    }
	  if (do_terminal_id)
	    {
	      char *id = "2392A\r", *ptr = id;
	      while (*ptr)
		{
		  if (PutImmediateQ(*ptr) == -1)
		    return;
		  ++ptr;
		}
	    }
	    continue;
	}
      if (vt_ch != '&')
	{ /* ESC+anything_but_ampersand */
	  for (;;)
	    { /* Munch till upper case or DC1 - may go too far */
	      if (!GetNextChar())
		goto Do_Write;
/*	      if ((isupper((int)vt_ch)) || (vt_ch == ASC_DC1)) */
	      if (isupper((int)vt_ch))
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
	  out_ptr += int_sprintf(out_ptr, VT100DisplayEnhance(vt_ch));
	  continue;
	}
      if (vt_ch != 'a')
	{ /* Anything other than cursor address: munch till upper case */
	  for (;;)
	    {
	      if (!GetNextChar())
		goto Do_Write;
	      if (isupper((int)vt_ch))
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
	  if (isalpha((int)vt_ch))
	    break;
	  *(num_ptr++) = vt_ch;
	}
      *num_ptr = '\0';
      num_val = atoi(num_buf);
      if ((toupper((int)vt_ch) == 'C') || (toupper((int)vt_ch) == 'X'))
	{
	  row_position = 0;
	  col = (move_relative) ? num_val : ++num_val;
	}
      else
	row = (move_relative) ? num_val : ++num_val;
      if (isupper((int)vt_ch))
	{ /* End of sequence: just row or column position */
	  if (row_position)
	    {
	      if (move_relative)
		{
		  if (num_val > 0)
		    out_ptr += int_sprintf(out_ptr, cud, num_val);
		  else
		    out_ptr += int_sprintf(out_ptr, cuu, -num_val);
		}
	      else
		{
		  out_ptr += int_sprintf(out_ptr, cuu, 25);
		  out_ptr += int_sprintf(out_ptr, cud, num_val-1);
/*
		  out_ptr += int_sprintf(out_ptr, CSI "%dH", num_val);
 */
		}
	    }
	  else
	    {
	      if (move_relative)
		{
		  if (num_val > 0)
		    out_ptr += int_sprintf(out_ptr, cuf, num_val);
		  else
		    out_ptr += int_sprintf(out_ptr, cub, -num_val);
		}
	      else
		{
		  out_ptr += int_sprintf(out_ptr, cub, 80);
		  out_ptr += int_sprintf(out_ptr, cuf, num_val-1);
/*
		  out_ptr += int_sprintf(out_ptr, CSI ";%dH", num_val);
 */
		}
	    }
	  continue;
	}
/* Get numeric row/column value */
      num_ptr = num_buf;
      for (;;)
	{
	  if (!GetNextChar())
	    goto Do_Write;
	  if (isalpha((int)vt_ch))
	    break;
	  *(num_ptr++) = vt_ch;
	}
      *num_ptr = '\0';
      num_val = atoi(num_buf);
      if ((vt_ch == 'C') || (vt_ch == 'X'))
	col = ++num_val;
      else
	row = ++num_val;
      out_ptr += int_sprintf(out_ptr, cup, row, col);
    } /* main for() loop */
 Do_Write:
  buf_len = out_ptr - out_buf;
  vt3kDataOutProc(refCon, out_buf, buf_len);
  if (debug)
    DumpBuffer(out_buf, buf_len, "vt100");

} /*vt3kHPtoVT100*/

void vt3kHPtoGeneric(int32_t refCon, char *buf, size_t buf_len)
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
    hold_len;
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
      hold_len = vt_queue_len;
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
	  out_ptr += int_sprintf(out_ptr, "<line_draw_off>");
	  continue;
	}
      if (vt_ch == ASC_SO)
	{
	  out_ptr += int_sprintf(out_ptr, "<line_draw_on>");
	  continue;
	}
      if (vt_ch != ASC_ESC)
	{
	  if (!vt_ch)
	    out_ptr += int_sprintf(out_ptr, "<nul>");
	  else
	    {
	      *(out_ptr++) = vt_ch;
	      if ((vt_enhanced) && ((vt_ch == '\r') || (vt_ch == '\n')))
		out_ptr += int_sprintf(out_ptr, GenericDisplayEnhance('@'));
	    }
	  continue;
	}
      if (!GetNextChar())
	goto Do_Write;
      if (vt_ch == '[')
	{
	  out_ptr += int_sprintf(out_ptr, "<protect_on>");
	  continue;
	}
      if (vt_ch == ']')
	{
	  out_ptr += int_sprintf(out_ptr, "<protect_off>");
	  continue;
	}
      if (vt_ch == '@')
	{
	  out_ptr += int_sprintf(out_ptr, "<pause>");
	  continue;
	}
      if (isalpha((int)vt_ch))
	{				/* ESC+alpha */
	  switch ((int)vt_ch)
	    {
	    case 'A':			/* Cursor up */
	      out_ptr += int_sprintf(out_ptr, "<move:up>");
	      break;
	    case 'a':			/* Cursor sense */
	      out_ptr += int_sprintf(out_ptr, "<cursor_sense>");
	      break;
	    case 'B':			/* Cursor down */
	      out_ptr += int_sprintf(out_ptr, "<move:down>");
	      break;
	    case 'b':			/* Keyboard unlock */
	      out_ptr += int_sprintf(out_ptr, "<unlock_keyboard>");
	      break;
	    case 'C':			/* Cursor right */
	      out_ptr += int_sprintf(out_ptr, "<move:right>");
	      break;
	    case 'c':			/* Keyboard lock */
	      out_ptr += int_sprintf(out_ptr, "<lock_keyboard>");
	      break;
	    case 'D':			/* Cursor left */
	      out_ptr += int_sprintf(out_ptr, "<move:left>");
	      break;
	    case 'd':			/* Read */
	      out_ptr += int_sprintf(out_ptr, "<read>");
	      break;
	    case 'E':			/* Hard reset */
	      out_ptr += int_sprintf(out_ptr, "<hard_reset>");
	      break;
	    case 'F':			/* Home bottom */
	      out_ptr += int_sprintf(out_ptr, "<home_bottom>");
	      break;
	    case 'g':			/* Soft reset */
	      out_ptr += int_sprintf(out_ptr, "<soft_reset>");
	      break;
	    case 'G':			/* Move cursor to left margin */
	      out_ptr += int_sprintf(out_ptr, "<move:left_margin>");
	      break;
	    case 'h':
	    case 'H':			/* Home cursor */
	      out_ptr += int_sprintf(out_ptr, "<home>");
	      break;
	    case 'J':			/* Clear to end of memory */
	      out_ptr += int_sprintf(out_ptr, "<clear_eos>");
	      break;
	    case 'K':			/* Erase to end of line */
	      out_ptr += int_sprintf(out_ptr, "<clear_eol>");
	      break;
	    case 'L':			/* Insert line */
	      out_ptr += int_sprintf(out_ptr, "<insert_line>");
	      break;
	    case 'M':			/* Delete line */
	      out_ptr += int_sprintf(out_ptr, "<delete_line>");
	      break;
	    case 'P':			/* Delete char */
	      out_ptr += int_sprintf(out_ptr, "<delete_char>");
	      break;
	    case 'Q':			/* Insert char on */
	      out_ptr += int_sprintf(out_ptr, "<insert_char_on>");
	      break;
	    case 'R':			/* Insert char off */
	      out_ptr += int_sprintf(out_ptr, "<insert_char_off>");
	      break;
	    case 'S':			/* Scroll up */
	      out_ptr += int_sprintf(out_ptr, "<scroll_up>");
	      break;
	    case 'T':			/* Scroll down */
	      out_ptr += int_sprintf(out_ptr, "<scroll_down>");
	      break;
	    case 'U':			/* Page down (next) */
	      out_ptr += int_sprintf(out_ptr, "<page_down>");
	      break;
	    case 'V':			/* Page up (prev) */
	      out_ptr += int_sprintf(out_ptr, "<page_up>");
	      break;
	    case 'W':			/* Format mode on */
	      out_ptr += int_sprintf(out_ptr, "<format_on>");
	      break;
	    case 'X':			/* Format mode off */
	      out_ptr += int_sprintf(out_ptr, "<format_off>");
	      break;
	    case 'Y':			/* Display fns on */
	      out_ptr += int_sprintf(out_ptr, "<disp_fns_on>");
	      break;
	    case 'Z':			/* Display fns off */
	      out_ptr += int_sprintf(out_ptr, "<disp_fns_off>");
	      break;
	    default:			/* ??? */
	      out_ptr += int_sprintf(out_ptr, "<esc+\"%c\">", vt_ch);
	      break;
	    }
	  continue;
	}
      if ((vt_ch == '*') || (vt_ch == '^') || (vt_ch == '~'))
	{
	  int do_terminal_id = 0;
	  if (vt_ch == '^')
	    {
	      out_ptr += int_sprintf(out_ptr, "<prim_status>");
	      continue;
	    }
	  if (vt_ch == '~')
	    {
	      out_ptr += int_sprintf(out_ptr, "<sec_status>");
	      continue;
	    }
	  if (!GetNextChar())
	    goto Do_Write;
	  if (vt_ch == 's')
	    do_terminal_id = 1;
	  for (;;)
	    {
	      if (!GetNextChar())
		goto Do_Write;
	      if (vt_ch == '^')
		break;
	    }
	  if (do_terminal_id)
	    out_ptr += int_sprintf(out_ptr, "<term_id>");
	  continue;
	}
      if (vt_ch != '&')
	{ /* ESC+anything_but_ampersand */
	  char temp[256], *ptr = temp;
	  ptr += int_sprintf(ptr, "<esc+\"%c", vt_ch);
	  for (;;)
	    { /* Munch till upper case or DC1 - may go too far */
	      if (!GetNextChar())
		goto Do_Write;
	      *(ptr++) = vt_ch;
/*	      if ((isupper((int)vt_ch)) || (vt_ch == '^') || (vt_ch == '~') || (vt_ch == ASC_DC1))*/
	      if ((isupper((int)vt_ch)) || (vt_ch == '^') || (vt_ch == '~'))
		break;
	    }
	  ptr += int_sprintf(ptr, "\">");
	  out_ptr += int_sprintf(out_ptr, "%s", temp);
	  continue;
	}
/* ESC+"&" sequences */
      if (!GetNextChar())
	goto Do_Write;
      if (vt_ch == 'd')
	{ /* ESC+"&dx": Display enhancements */
	  if (!GetNextChar())
	    goto Do_Write;
	  out_ptr += int_sprintf(out_ptr, GenericDisplayEnhance(vt_ch));
	  continue;
	}
      if (vt_ch != 'a')
	{ /* Anything other than cursor address: munch till upper case */
	  char temp[256], *ptr = temp;
	  ptr += int_sprintf(ptr, "<esc+\"&%c", vt_ch);
	  for (;;)
	    {
	      if (!GetNextChar())
		goto Do_Write;
	      *(ptr++) = vt_ch;
	      if ((isupper((int)vt_ch)) || (vt_ch == '@'))
		break;
	    }
	  ptr += int_sprintf(ptr, "\">");
	  out_ptr += int_sprintf(out_ptr, "%s", temp);
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
	  if (isalpha((int)vt_ch))
	    break;
	  *(num_ptr++) = vt_ch;
	}
      *num_ptr = '\0';
      num_val = atoi(num_buf);
      if ((toupper((int)vt_ch) == 'C') || (toupper((int)vt_ch) == 'X'))
	{
	  row_position = 0;
	  col = num_val;
	}
      else
	row = num_val;
      if (isupper((int)vt_ch))
	{ /* End of sequence: just row or column position */
	  if (row_position)
	    {
	      if (move_relative)
		out_ptr += int_sprintf(out_ptr, "<move:r=%+d>", row);
	      else
		out_ptr += int_sprintf(out_ptr, "<move:r=%d>", row);
	    }
	  else
	    {
	      if (move_relative)
		out_ptr += int_sprintf(out_ptr, "<move:c=%+d>", col);
	      else
		out_ptr += int_sprintf(out_ptr, "<move:c=%d>", col);
	    }
	  continue;
	}
/* Get numeric row/column value */
      num_ptr = num_buf;
      for (;;)
	{
	  if (!GetNextChar())
	    goto Do_Write;
	  if (isalpha((int)vt_ch))
	    break;
	  *(num_ptr++) = vt_ch;
	}
      *num_ptr = '\0';
      num_val = atoi(num_buf);
      if ((vt_ch == 'C') || (vt_ch == 'X'))
	col = num_val;
      else
	row = num_val;
      out_ptr += int_sprintf(out_ptr, "<move:r=%d,c=%d>", row, col);
    } /* main for() loop */
 Do_Write:
  buf_len = out_ptr - out_buf;
  vt3kDataOutProc(refCon, out_buf, buf_len);
  if (debug)
    DumpBuffer(out_buf, buf_len, "Generic");

} /*vt3kHPtoGeneric*/

void vt3kHPtoVT52(int32_t refCon, char *buf, size_t buf_len)
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
    hold_len;
  char
    out_buf[MAX_VT_QUEUE],
    num_buf[256],
    *num_ptr = num_buf,
    *out_ptr = out_buf;
#  define ESC			"\033"
  const char
    *cup   = ESC "Y%c%c",	/* Cursor position */
    *cuu1  = ESC "A",		/* Cursor up */
    *cud1  = ESC "B",		/* Cursor down */
    *cuf1  = ESC "C",		/* Cursor forward */
    *cub1  = ESC "D",		/* Cursor back */
    *home  = ESC "H",		/* Cursor home */
    *ed    = ESC "J",		/* Erase to end-of-display */
    *el    = ESC "K";		/* Erase to end-of-line */
  static int
    line_draw = 0;

  if (!buf_len)
    return;

/* Copy raw data to the queue */
  for (; i<buf_len; i++)
    PutVTQueue(buf[i]);

  if (debug)
    {
      hold_len = vt_queue_len;
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
	  continue;
	}
      if (vt_ch == ASC_SO)
	{
	  line_draw = 1;
	  continue;
	}
      if (vt_ch != ASC_ESC)
	{
	  *(out_ptr++) = (char)((line_draw) ? TtyLineDraw(vt_ch) : vt_ch);
	  if ((vt_enhanced) && ((vt_ch == '\r') || (vt_ch == '\n')))
	    out_ptr += int_sprintf(out_ptr, VT100DisplayEnhance('@'));
	  continue;
	}
      if (!GetNextChar())
	goto Do_Write;
      if (isalpha((int)vt_ch))
	{ /* ESC+alpha */
	  switch ((int)vt_ch)
	    {
	    case 'A': /* Cursor up */
	      out_ptr += int_sprintf(out_ptr, cuu1);
	      break;
	    case 'B': /* Cursor down */
	      out_ptr += int_sprintf(out_ptr, cud1);
	      break;
	    case 'C': /* Cursor right */
	      out_ptr += int_sprintf(out_ptr, cuf1);
	      break;
	    case 'D': /* Cursor left */
	      out_ptr += int_sprintf(out_ptr, cub1);
	      break;
	    case 'h':
	    case 'H': /* Home cursor */
	      out_ptr += int_sprintf(out_ptr, home);
	      break;
	    case 'J': /* Clear to end of memory */
	      out_ptr += int_sprintf(out_ptr, ed);
	      break;
	    case 'K': /* Erase to end of line */
	      out_ptr += int_sprintf(out_ptr, el);
	      break;
	    default: /* ??? */
	      break;
	    }
	  continue;
	}
      if ((vt_ch == '*') || (vt_ch == '^') || (vt_ch == '~'))
	{
	  int do_terminal_id = 0;
	  if (vt_ch == '^')
	    {
	      char *prim = ESC "\\?008000\r", *ptr = prim;
	      while (*ptr)
		{
		  if (PutImmediateQ(*ptr) == -1)
		    return;
		  ++ptr;
		}
	      continue;
	    }
	  if (vt_ch == '~')
	    {
	      char *sec = ESC "|0400000\r", *ptr = sec;
	      while (*ptr)
		{
		  if (PutImmediateQ(*ptr) == -1)
		    return;
		  ++ptr;
		}
	      continue;
	    }
	  if (!GetNextChar())
	    goto Do_Write;
	  if (vt_ch == 's')
	    do_terminal_id = 1;
	  for (;;)
	    {
	      if (!GetNextChar())
		goto Do_Write;
	      if (vt_ch == '^')
		break;
	    }
	  if (do_terminal_id)
	    {
	      char *id = "2392A\r", *ptr = id;
	      while (*ptr)
		{
		  if (PutImmediateQ(*ptr) == -1)
		    return;
		  ++ptr;
		}
	    }
	  continue;
	}
      if (vt_ch != '&')
	{ /* ESC+anything_but_ampersand */
	  for (;;)
	    { /* Munch till upper case or DC1 - may go too far */
	      if (!GetNextChar())
		goto Do_Write;
/*	      if ((isupper((int)vt_ch)) || ((int)vt_ch == ASC_DC1)) */
	      if (isupper((int)vt_ch))
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
	  out_ptr += int_sprintf(out_ptr, VT100DisplayEnhance(vt_ch));
	  continue;
	}
      if (vt_ch != 'a')
	{ /* Anything other than cursor address: munch till upper case */
	  for (;;)
	    {
	      if (!GetNextChar())
		goto Do_Write;
	      if (isupper((int)vt_ch))
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
	  if (isalpha((int)vt_ch))
	    break;
	  *(num_ptr++) = vt_ch;
	}
      *num_ptr = '\0';
      num_val = atoi(num_buf);
      if ((toupper((int)vt_ch) == 'C') || (toupper((int)vt_ch) == 'X'))
	{
	  row_position = 0;
	  col = (move_relative) ? num_val : ++num_val;
	}
      else
	row = (move_relative) ? num_val : ++num_val;
      if (isupper((int)vt_ch))
	{ /* End of sequence: just row or column position */
/* No can do in VT52 mode */
	  continue;
	}
/* Get numeric row/column value */
      num_ptr = num_buf;
      for (;;)
	{
	  if (!GetNextChar())
	    goto Do_Write;
	  if (isalpha((int)vt_ch))
	    break;
	  *(num_ptr++) = vt_ch;
	}
      *num_ptr = '\0';
      num_val = atoi(num_buf);
      if ((vt_ch == 'C') || (vt_ch == 'X'))
	col = ++num_val;
      else
	row = ++num_val;
      out_ptr += int_sprintf(out_ptr, cup, (char)(037+row), (char)(037+col));
    } /* main for() loop */
 Do_Write:
  buf_len = out_ptr - out_buf;
  vt3kDataOutProc(refCon, out_buf, buf_len);
  if (debug)
    DumpBuffer(out_buf, buf_len, "vt52");

} /*vt3kHPtoVT52*/

#ifdef TRANSLATE_INPUT
void TranslateKeyboard(char *buf, int *buf_len)
{ /*TranslateKeyboard*/

  int
    key = -1,
    hold_len = *buf_len;
  char
    hold_buf[MAX_VT_QUEUE],
    *in_ptr = hold_buf,
    *out_ptr = buf,
    *key_codes = "pqrst uvw";
  const char
    *kpp    = ESC "V",		/* Page Up (prev) */
    *knp    = ESC "U",		/* Page Down (next) */
    *kcuu1  = ESC "A",		/* Cursor up */
    *kcud1  = ESC "B",		/* Cursor down */
    *kcuf1  = ESC "C",		/* Cursor forward */
    *kcub1  = ESC "D",		/* Cursor back */
    *kfn    = ESC "%c\r";	/* Function key */

/*
 * Work in progress
 */

/*
 * ESC+"[11~" -> PF1 -> ESC+"p"
 * ESC+"[12~" -> PF2 -> ESC+"q"
 * ESC+"[13~" -> PF3 -> ESC+"r"
 * ESC+"[14~" -> PF4 -> ESC+"s"
 * ESC+"[15~" -> PF5 -> ESC+"t"
 * ESC+"[17~" -> PF6 -> ESC+"u"
 * ESC+"[18~" -> PF7 -> ESC+"v"
 * ESC+"[19~" -> PF8 -> ESC+"w"
 * ESC+"[4~"  -> Select    -> ESC+"???"
 * ESC+"[5~"  -> Prev page -> ESC+"V"
 * ESC+"[6~"  -> Next page -> ESC+"U"
 * ESC+"[A"   -> Crsr Up   -> ESC+"A"
 * ESC+"[B"   -> Crsr Down -> ESC+"B"
 * ESC+"[C"   -> Crsr Right-> ESC+"C"
 * ESC+"[D"   -> Crsr Left -> ESC+"D"
 */
/*
  if (*buf_len < 5)
    return;
 */
  if (!*buf_len)
    return;
  memcpy(hold_buf, buf, *buf_len);
  for (;;)
    {
      if (*in_ptr != ASC_ESC)
	{
	  *(out_ptr++) = *(in_ptr++);
	  if (--hold_len == 0)
	    break;
	  continue;
	}
      ++in_ptr;
      if (--hold_len == 0)
	break;
      if (*in_ptr != '[')
	continue;
      ++in_ptr;
      if (--hold_len == 0)
	break;
      if (isdigit(*in_ptr))
	{
	  key = atoi(in_ptr);
	  while (isdigit(*in_ptr))
	    {
	      ++in_ptr;
	      if (--hold_len == 0)
		break;
	    }
	}
      if (*in_ptr == '~')
	{
	  ++in_ptr;
	  if (--hold_len == 0)
	    break;
	  if ((key >= 11) && (key <= 19) && (key != 16))
	    { /* PF1 - PF8 */
	      out_ptr += int_sprintf(out_ptr, kfn, key_codes[(key-11)]);
	      continue;
	    }
	  if (key == 4)
	    { /* Select??? */
/*	      out_ptr += int_sprintf(out_ptr, ESC "V"); */
	      continue;
	    }
	  if (key == 5)
	    { /* Previous page */
	      out_ptr += int_sprintf(out_ptr, kpp);
	      continue;
	    }
	  if (key == 6)
	    { /* Next page */
	      out_ptr += int_sprintf(out_ptr, knp);
	      continue;
	    }
	  continue;
	}
      if ((*in_ptr >= 'A') && (*in_ptr <= 'D'))
	{
	  switch ((int)*in_ptr)
	    {
	    case 'A':
	      out_ptr += int_sprintf(out_ptr, kcuu1);
	      break;
	    case 'B':
	      out_ptr += int_sprintf(out_ptr, kcud1);
	      break;
	    case 'C':
	      out_ptr += int_sprintf(out_ptr, kcuf1);
	      break;
	    case 'D':
	      out_ptr += int_sprintf(out_ptr, kcub1);
	      break;
	    }
	  ++in_ptr;
	  if (--hold_len == 0)
	    break;
	}
    }
  *buf_len = out_ptr - buf;

} /*TranslateKeyboard*/
#endif /*TRANSLATE_INPUT*/
