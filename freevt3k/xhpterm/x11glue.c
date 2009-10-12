/*
 *    $Log: x11glue.c,v $
 *    Revision 1.1  2003/03/14 16:28:44  randy
 *    First import
 *
 */

static char const rcsid[] = "$Id: x11glue.c,v 1.1 2003/03/14 16:28:44 randy Exp $";

/*
 * x11glue.c  --  X11 window interface
 *
 * Derived from O'Reilly and Associates example 'basicwin.c'
 * Copyright 1989 O'Reilly and Associates, Inc.
 * See ../Copyright for complete rights and liability information.
 */
#ifdef VMS
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
#include <iodef.h>
#include <stsdef.h>
#include <socket.h>
#include <in.h>
#include <netdb.h>
#include <inet.h>
#include <lib$routines.h>
#include <starlet.h>
#include <ucx$inetdef.h>
#else
#include "config.h"
#ifdef __hpux
#ifndef _HPUX_SOURCE
#define _HPUX_SOURCE 1
#endif
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#endif
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <stdlib.h>

#ifndef VMS
#include <stdio.h>
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <netinet/in.h>
#endif

#if !defined(MEMLOCK_2000)
#  define USE_9X15     1
#endif

/* i changed font names here for a more readable font.  
sorry courier just wasn't doing it for me - mwest */
#ifdef USE_9X15
#  if !defined(MEMLOCK_2000)
#    define CHAR_WIDTH	9
#    define CHAR_HEIGHT	15
#  endif
#  define FONT_NAME	"9x15"
#else
#  define FONT_NAME	"9x15"	/* changed from courier */
#  define FONT_NAME0	"9x15"
#endif

#include "x11glue.h"
#include "conmgr.h"
#include "hpterm.h"
#include "typedef.h"
#include "vtconn.h"
#include "terminal.bm"

#define DEBUG_KEYSYMS 0

#define BITMAPDEPTH 1

/* These are used as arguments to nearly every Xlib routine, so it saves
 * routine arguments to declare them global.  If there were
 * additional source files, they would be declared extern there. */
Display *display;
int screen_num;
Window win;

unsigned int width, height;	/* window size (pixels) */
GC gc_normal;
GC gc_inverse;
GC gc_halfbright;
GC gc_red;

XFontStruct *font_info;

struct conmgr *con = 0;
int logging = 0;

int must_logoff = 0;

char *termid = NULL;

char *version_str = "A.01.04";

/*******************************************************************/
int get_colors P ((int nb_colors, char **color_names, unsigned long *color_codes));
int getGC P ((Window win, GC * gc, XFontStruct * font_info));
int getGC_Inverse P ((Window win, GC * gc, XFontStruct * font_info));
int getGC_Halfbright P ((Window win, GC * gc, XFontStruct * font_info));
int getGC_Red P ((Window win, GC * gc, XFontStruct * font_info));
void Logit P ((int typ, char *ptr, int len, int special_dc1));
int LoadKeybdTable P ((char *file_name, int i_type));
/*******************************************************************/
#define GRAY_INDEX 0
#define RED_INDEX 1
#define NB_COLORS 2
unsigned long color_codes[NB_COLORS];
char *color_names[NB_COLORS] =
{"DimGray", "Red"};		/* changed "light grey", "red" */
char *progname;			/* name this program was invoked by */

/*******************************************************************/
#if defined(MEMLOCK_2000)
void init_disp (int argc, char **argv, char *hostname, char *font1)
{
  int x, y, i, j;		/* window position */
  unsigned int border_width = 4;	/* four pixels */
  unsigned int display_width, display_height;
  unsigned int icon_width, icon_height;
  char *window_name1 = "xHPTerm";
  char *icon_name = "freevt3k";
  Pixmap icon_pixmap;
  XSizeHints size_hints;
  XIconSize *size_list;
  int count;
  char *window_name;
  char title_name[50];
  char *display_name = NULL;
  int nbrows = 26;
  int nbcols = 80;
  char ch;
  sprintf (title_name, "%s : %s", window_name1, hostname);
  window_name = title_name;

  progname = argv[0];

  /* connect to X server */
  if ((display = XOpenDisplay (display_name)) == NULL)
  {
    (void) fprintf (stderr, "%s: cannot connect to X server %s\n",
		    progname, XDisplayName (display_name));
    exit (-1);
  }

  /* get screen size from display structure macro */
  screen_num = DefaultScreen (display);
  display_width = DisplayWidth (display, screen_num);
  display_height = DisplayHeight (display, screen_num);

  /* Note that in a real application, x and y would default to 0
   * but would be settable from the command line or resource database.
   */
  x = y = 0;

/*
 *  fetch font info to ensure metrics
 */

  load_font (&font_info, font1);

  width = nbcols * font_info->max_bounds.width;
  height = nbrows * (font_info->ascent + font_info->descent);

  /* create opaque window */
  win = XCreateSimpleWindow (display, RootWindow (display, screen_num),
		     x, y, width, height, border_width, BlackPixel (display,
			     screen_num), WhitePixel (display, screen_num));

  /* Get available icon sizes from Window manager */

  if (XGetIconSizes (display, RootWindow (display, screen_num),
		     &size_list, &count) == 0)
    (void) fprintf (stderr, "%s: Window manager didn't set icon sizes - using default.\n", progname);
  else
  {
    ;
    /* A real application would search through size_list
     * here to find an acceptable icon size, and then
     * create a pixmap of that size.  This requires
     * that the application have data for several sizes
     * of icons. */
  }

  /* Create pixmap of depth 1 (bitmap) for icon */

  icon_pixmap = XCreateBitmapFromData (display, win, (const char *) terminal_bits,
				       terminal_width, terminal_height);

  /* Set size hints for window manager.  The window manager may
   * override these settings.  Note that in a real
   * application if size or position were set by the user
   * the flags would be UPosition and USize, and these would
   * override the window manager's preferences for this window. */

#ifdef X11R3
  size_hints.flags = PPosition | PSize | PMinSize;
  size_hints.x = x;
  size_hints.y = y;
  size_hints.width = width;
  size_hints.height = height;
  size_hints.min_width = 300;
  size_hints.min_height = 200;
#else /* X11R4 or later */

  /* x, y, width, and height hints are now taken from
   * the actual settings of the window when mapped. Note
   * that PPosition and PSize must be specified anyway. */

  size_hints.flags = PPosition | PSize | PMinSize;
  size_hints.min_width = 300;
  size_hints.min_height = 200;

#endif

#ifdef X11R3
  /* set Properties for window manager (always before mapping) */
  XSetStandardProperties (display, win, window_name, icon_name,
			  icon_pixmap, argv, argc, &size_hints);

#else /* X11R4 or later */

  {
    XWMHints wm_hints;
    XClassHint class_hints;

    /* format of the window name and icon name
     * arguments has changed in R4 */
    XTextProperty windowName, iconName;

    /* These calls store window_name and icon_name into
     * XTextProperty structures and set their other
     * fields properly. */
    if (XStringListToTextProperty (&window_name, 1, &windowName) == 0)
    {
      (void) fprintf (stderr, "%s: structure allocation for windowName failed.\n",
		      progname);
      exit (-1);
    }

    if (XStringListToTextProperty (&icon_name, 1, &iconName) == 0)
    {
      (void) fprintf (stderr, "%s: structure allocation for iconName failed.\n",
		      progname);
      exit (-1);
    }

    wm_hints.initial_state = NormalState;
    wm_hints.input = True;
    wm_hints.icon_pixmap = icon_pixmap;
    wm_hints.flags = StateHint | IconPixmapHint | InputHint;

    class_hints.res_name = progname;
    class_hints.res_class = "Basicwin";

    XSetWMProperties (display, win, &windowName, &iconName,
		      argv, argc, &size_hints, &wm_hints,
		      &class_hints);
  }
#endif

  /* Select event types wanted */
  XSelectInput (display, win, ExposureMask | KeyPressMask |
		ButtonPressMask | StructureNotifyMask);

  /* get colors that we need */
  get_colors (NB_COLORS, color_names, color_codes);

  /* create GC for text and drawing */
  getGC (win, &gc_normal, font_info);

  /* create GC for Inverse video */
  getGC_Inverse (win, &gc_inverse, font_info);

  /* create GC for halfbright video */
  getGC_Halfbright (win, &gc_halfbright, font_info);

  /* create GC for red video */
  getGC_Red (win, &gc_red, font_info);

  /* Display window */
  XMapWindow (display, win);

}
#else
void init_disp(int argc, char **argv)
{
	int x, y; 	/* window position */
	unsigned int border_width = 4;	/* four pixels */
	unsigned int display_width, display_height;
	unsigned int icon_width, icon_height;
	char *window_name = "FREEVT3K Terminal Emulator";
	char *icon_name = "freevt3k";
	Pixmap icon_pixmap;
	XSizeHints size_hints;
	XIconSize *size_list;
	int count;
	char *display_name = NULL;
	int nbrows = 26;
	int nbcols = 80;
	char ch;

	progname = argv[0];

	/* connect to X server */
	if ( (display=XOpenDisplay(display_name)) == NULL )
	{
		(void) fprintf( stderr, "%s: cannot connect to X server %s\n",
				progname, XDisplayName(display_name));
		exit( -1 );
	}

	/* get screen size from display structure macro */
	screen_num = DefaultScreen(display);
	display_width = DisplayWidth(display, screen_num);
	display_height = DisplayHeight(display, screen_num);

	/* Note that in a real application, x and y would default to 0
	 * but would be settable from the command line or resource database.
	 */
	x = y = 0;

	width = nbcols * CHAR_WIDTH;          /* Should get the font first */
	height = nbrows * CHAR_HEIGHT;

	/* create opaque window */
	win = XCreateSimpleWindow(display, RootWindow(display,screen_num),
			x, y, width, height, border_width, BlackPixel(display,
			screen_num), WhitePixel(display,screen_num));

	/* Get available icon sizes from Window manager */

	if (XGetIconSizes(display, RootWindow(display,screen_num),
			&size_list, &count) == 0)
		(void) fprintf( stderr, "%s: Window manager didn't set icon sizes - using default.\n", progname);
	else {
		;
		/* A real application would search through size_list
		 * here to find an acceptable icon size, and then
		 * create a pixmap of that size.  This requires
		 * that the application have data for several sizes
		 * of icons. */
	}

	/* Create pixmap of depth 1 (bitmap) for icon */

	icon_pixmap = XCreateBitmapFromData(display, win, (const char*)terminal_bits,
			terminal_width, terminal_height);

	/* Set size hints for window manager.  The window manager may
	 * override these settings.  Note that in a real
	 * application if size or position were set by the user
	 * the flags would be UPosition and USize, and these would
	 * override the window manager's preferences for this window. */
#ifdef X11R3
	size_hints.flags = PPosition | PSize | PMinSize;
	size_hints.x = x;
	size_hints.y = y;
	size_hints.width = width;
	size_hints.height = height;
	size_hints.min_width = 300;
	size_hints.min_height = 200;
#else /* X11R4 or later */
	/* x, y, width, and height hints are now taken from
	 * the actual settings of the window when mapped. Note
	 * that PPosition and PSize must be specified anyway. */

	size_hints.flags = PPosition | PSize | PMinSize;
	size_hints.min_width = 300;
	size_hints.min_height = 200;
#endif

#ifdef X11R3
	/* set Properties for window manager (always before mapping) */
	XSetStandardProperties(display, win, window_name, icon_name,
			icon_pixmap, argv, argc, &size_hints);

#else /* X11R4 or later */
	{
	XWMHints wm_hints;
	XClassHint class_hints;

	/* format of the window name and icon name
	 * arguments has changed in R4 */
	XTextProperty windowName, iconName;

	/* These calls store window_name and icon_name into
	 * XTextProperty structures and set their other
	 * fields properly. */
	if (XStringListToTextProperty(&window_name, 1, &windowName) == 0) {
		(void) fprintf( stderr, "%s: structure allocation for windowName failed.\n",
				progname);
		exit(-1);
	}

	if (XStringListToTextProperty(&icon_name, 1, &iconName) == 0) {
		(void) fprintf( stderr, "%s: structure allocation for iconName failed.\n",
				progname);
		exit(-1);
	}

	wm_hints.initial_state = NormalState;
	wm_hints.input = True;
	wm_hints.icon_pixmap = icon_pixmap;
	wm_hints.flags = StateHint | IconPixmapHint | InputHint;

	class_hints.res_name = progname;
	class_hints.res_class = "Basicwin";

	XSetWMProperties(display, win, &windowName, &iconName,
			argv, argc, &size_hints, &wm_hints,
			&class_hints);
	}
#endif

	/* Select event types wanted */
	XSelectInput(display, win, ExposureMask | KeyPressMask |
			ButtonPressMask | StructureNotifyMask);

	load_font(&font_info);

	/* get colors that we need */
	get_colors (NB_COLORS, color_names, color_codes);

	/* create GC for text and drawing */
	getGC(win, &gc_normal, font_info);

	/* create GC for Inverse video */
	getGC_Inverse (win, &gc_inverse, font_info);

	/* create GC for halfbright video */
	getGC_Halfbright (win, &gc_halfbright, font_info);

	/* create GC for red video */
	getGC_Red (win, &gc_red, font_info);

	/* Display window */
	XMapWindow(display, win);

}
#endif

void event_loop (void)
{

  int xsocket, nfds, i;		/* select stuff */
  int dsocket;
  fd_set readfds, readmask;
  int count;

  XComposeStatus compose;
  KeySym keysym;
  int bufsize = 20;
  int charcount;
  char buffer[20];
  int nbrows, nbcols;

  XEvent report;

  while (!con->eof)
  {
    term_update ();		/* flush deferred screen updates */
    XFlush (display);		/* and send them to the server */

    /* Use select() to wait for traffic from X or remote computer */
    xsocket = ConnectionNumber (display);
    dsocket = con->socket;
    nfds = (dsocket < xsocket) ? (1 + xsocket) : (1 + dsocket);

    FD_ZERO (&readmask);
    if (!(con->eof))
      FD_SET (dsocket, &readmask);
    FD_SET (xsocket, &readmask);
    readfds = readmask;
    i = select (nfds, (void *) &readfds, 0, 0, 0);

    if (i < 0)
    {
      perror ("select failed");
    }
    else if (i == 0)
    {
      printf ("select timed out\n");
    }
    else if ((FD_ISSET (dsocket, &readmask)) &&
	     (FD_ISSET (dsocket, &readfds)))
    {

      conmgr_read (con);

      /*
         **  Wait up to 10 ms for more data from host
         **  This should prevent excessive re-draws
       */
      i = 1;
      count = 0;
      while (i > 0 && count < 10 && !(con->eof))
      {
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 10000;

	FD_ZERO (&readfds);
	FD_SET (dsocket, &readfds);

	i = select (nfds, (void *) &readfds, 0, 0, (struct timeval *) &timeout);
	if (i < 0)
	{
	  perror ("select failed");
	}
	else if (i == 0)
	{
	  /* time out */

	}
	else if (FD_ISSET (dsocket, &readfds))
	{
	  /* more data */
	  conmgr_read (con);
	}
	count++;		/* Can't ignore X for too long */
      }
    }
    while (XPending (display))
    {
      XNextEvent (display, &report);
      switch (report.type)
      {
      case Expose:
	/* unless this is the last contiguous expose,
	 * don't draw the window */
	if (report.xexpose.count != 0)
	  break;

	/* redraw term emulator stuff */
	term_redraw ();
	break;
      case ConfigureNotify:
	/* window has been resized
	 * notify hpterm.c */
	width = report.xconfigure.width;
	height = report.xconfigure.height;
#if defined(MEMLOCK_2000)
	nbcols = width / font_info->max_bounds.width;
	nbrows = height / (font_info->ascent + font_info->descent);
#else
	nbcols = width / CHAR_WIDTH;    /* Needs work here */
	nbrows = height / CHAR_HEIGHT;
#endif
	hpterm_winsize (nbrows, nbcols);
	break;
      case ButtonPress:
	if (report.xbutton.button == 1)
	{
	  int r, c;
#if defined(MEMLOCK_2000)
	  c = report.xbutton.x / font_info->max_bounds.width;
	  r = report.xbutton.y / (font_info->ascent + font_info->descent);
#else
	  c = report.xbutton.x / CHAR_WIDTH;
	  r = report.xbutton.y / CHAR_HEIGHT;
#endif
	  hpterm_mouse_click (r, c);
	}
	/* right mouse button causes program to exit */
	if ((report.xbutton.button == 3) && (!must_logoff))
	{
	  return;
	}
	break;
      case KeyPress:
	charcount = XLookupString (&report.xkey, buffer,
				   bufsize, &keysym, &compose);

	if (DEBUG_KEYSYMS)
	{
	  int ii;
	  printf ("(%x)", keysym);
	  printf ("<");
	  for (ii = 0; ii < charcount; ii++)
	  {
	    if (ii)
	      printf (",");
	    printf ("%d", buffer[ii]);
	  }
	  printf (">");
#if defined(MEMLOCK_2000)
	  if (report.xkey.state)
	  {
	    printf ("[%x,%lx]\n", report.xkey.state, keysym);
	  }
	  else
	  {
	    printf ("[%lx]\n", keysym);
	  }
	  printf ("KeySym  [%s]\n", XKeysymToString (keysym));
#endif
	  fflush (stdout);
	}

#if defined(MEMLOCK_2000)
	if (!keymapper (keysym, report.xkey.state, buffer, charcount))
#else
	if (!keymapper (keysym, report.xkey.state))
#endif
	{
	  if (charcount == 1)
	  {
	    hpterm_kbd_ascii (buffer[0]);
	  }
	}
	break;
      default:
	/* all events selected by StructureNotifyMask
	 * except ConfigureNotify are thrown away here,
	 * since nothing is done with them */
	break;
      }				/* end switch */
    }
  }				/* end while */
}

getGC (win, gc, font_info)
     Window win;
     GC *gc;
     XFontStruct *font_info;
{
  unsigned long valuemask = 0;	/* ignore XGCvalues and use defaults */
  XGCValues values;
  unsigned int line_width = 6;
  int line_style = LineOnOffDash;
  int cap_style = CapRound;
  int join_style = JoinRound;
  int dash_offset = 0;
  static char dash_list[] =
  {12, 24};
  int list_length = 2;

  /* Create default Graphics Context */
  *gc = XCreateGC (display, win, valuemask, &values);

  /* specify font */
  XSetFont (display, *gc, font_info->fid);

  /* specify black foreground since default window background is
   * white and default foreground is undefined. */
  XSetForeground (display, *gc, BlackPixel (display, screen_num));

  /* set line attributes */
  XSetLineAttributes (display, *gc, line_width, line_style,
		      cap_style, join_style);

  /* set dashes */
  XSetDashes (display, *gc, dash_offset, dash_list, list_length);
}

getGC_Inverse (win, gc, font_info)
     Window win;
     GC *gc;
     XFontStruct *font_info;
{
  unsigned long valuemask = 0;	/* ignore XGCvalues and use defaults */
  XGCValues values;

  /* Create default Graphics Context */
  *gc = XCreateGC (display, win, valuemask, &values);

  /* specify font */
  XSetFont (display, *gc, font_info->fid);

  /* Set foreground and background swapped for Inverse Video */
  XSetForeground (display, *gc, WhitePixel (display, screen_num));
  XSetBackground (display, *gc, BlackPixel (display, screen_num));
}

getGC_Halfbright (win, gc, font_info)
     Window win;
     GC *gc;
     XFontStruct *font_info;
{
  unsigned long valuemask = 0;	/* ignore XGCValues and use defaults */
  XGCValues values;

  /* Create default Graphics Context */
  *gc = XCreateGC (display, win, valuemask, &values);

  /* specify font */
  XSetFont (display, *gc, font_info->fid);

  /* Set foreground gray and background white for halfbright video */
  XSetForeground (display, *gc, color_codes[GRAY_INDEX]);
  XSetBackground (display, *gc, WhitePixel (display, screen_num));
}

getGC_Red (win, gc, font_info)
     Window win;
     GC *gc;
     XFontStruct *font_info;
{
  unsigned long valuemask = 0;	/* ignore XGCValues and use defaults */
  XGCValues values;

  /* Create default Graphics Context */
  *gc = XCreateGC (display, win, valuemask, &values);

  /* specify font */
  XSetFont (display, *gc, font_info->fid);

  /* Set foreground red and background white */
  XSetForeground (display, *gc, color_codes[RED_INDEX]);
  XSetBackground (display, *gc, WhitePixel (display, screen_num));
}

#if defined(MEMLOCK_2000)
int load_font (XFontStruct **font_info, char *font1)
{
  char *fontname = FONT_NAME;
  if(font1 != NULL)
  {
    /* Load font and get font information structure. */
    if ((*font_info = XLoadQueryFont (display, font1)) == NULL)
	{
	    (void) fprintf (stderr, "%s: Cannot open %s font, switching to DEFAULT\n",
			    progname, font1);
	    if ((*font_info = XLoadQueryFont (display, fontname)) == NULL)
	    {
	      (void) fprintf (stderr, "%s: Cannot open %s font\n",
			    progname, fontname);
	      exit (-1);
	    }
	}
  }
  else
  {
	  if ((*font_info = XLoadQueryFont (display, fontname)) == NULL)
	  {
	    (void) fprintf (stderr, "%s: Cannot open %s font\n",
			    progname, fontname);
	    exit (-1);
	  }
  }
}
#else
int load_font(XFontStruct **font_info)
{
	char *fontname = FONT_NAME;

	/* Load font and get font information structure. */
	if ((*font_info = XLoadQueryFont(display,fontname)) == NULL)
	{
		(void) fprintf( stderr, "%s: Cannot open %s font\n",
				progname, FONT_NAME);
		exit( -1 );
	}
}
#endif

static struct km
{
  char *keyname;
  KeySym keysym;
  void (*keyfunc) ();
}
keymap[] =
{
    "Break",     XK_Break,    hpterm_kbd_Break,
    "Menu",      XK_Menu,     hpterm_kbd_Menu,
    "F1",        XK_F1,       hpterm_kbd_F1,
    "F2",        XK_F2,       hpterm_kbd_F2,
    "F3",        XK_F3,       hpterm_kbd_F3,
    "F4",        XK_F4,       hpterm_kbd_F4,
    "F5",        XK_F5,       hpterm_kbd_F5,
    "F6",        XK_F6,       hpterm_kbd_F6,
    "F7",        XK_F7,       hpterm_kbd_F7,
    "F8",        XK_F8,       hpterm_kbd_F8,
    "Home",      XK_Home,     hpterm_kbd_Home,
    "Left",      XK_Left,     hpterm_kbd_Left,
    "Right",     XK_Right,    hpterm_kbd_Right,
    "Down",      XK_Down,     hpterm_kbd_Down,
    "Up",        XK_Up,       hpterm_kbd_Up,
    "Prev",      XK_Prior,    hpterm_kbd_Prev,
    "Next",      XK_Next,     hpterm_kbd_Next,
    "Select",    XK_Select,   hpterm_kbd_Select,
    "KP_Enter",  XK_KP_Enter, hpterm_kbd_KP_Enter,
    "Enter",     XK_Execute,  hpterm_kbd_Enter,
#if defined(MEMLOCK_2000)
    "Clear",     XK_F12,      hpterm_kbd_Clear,
    "PrintScrn", XK_Print,    dump_display,
#endif
/*
   **  Following group needed for HP 715 workstation
 */
#ifdef hpXK_Reset
    "Reset",      hpXK_Reset,      hpterm_kbd_Reset,
    "User",       hpXK_User,       hpterm_kbd_User,
    "System",     hpXK_System,     hpterm_kbd_System,
    "ClearLine",  hpXK_ClearLine,  hpterm_kbd_ClearLine,
    "InsertLine", hpXK_InsertLine, hpterm_kbd_InsertLine,
    "DeleteLine", hpXK_DeleteLine, hpterm_kbd_DeleteLine,
    "InsertChar", hpXK_InsertChar, hpterm_kbd_InsertChar,
    "DeleteChar", hpXK_DeleteChar, hpterm_kbd_DeleteChar,
    "BackTab",    hpXK_BackTab,    hpterm_kbd_BackTab,
    "KP_BackTab", hpXK_KP_BackTab, hpterm_kbd_KP_BackTab,
#endif
/*
   **  Following group needed for Tatung Mariner 4i with AT keyboard
 */
    "F9",         XK_F9,     hpterm_kbd_Menu,
    "F10",        XK_F10,    hpterm_kbd_User,
#if defined(MEMLOCK_2000)
    "F11",        XK_F11,    hpterm_kbd_System,
    "Break",      XK_Pause,  hpterm_kbd_Break,
#else
    "F11", 	  0x1000FF10,hpterm_kbd_System,
    "Break",      XK_F23,    hpterm_kbd_Break,
#endif
    "PageUp",     XK_F29,    hpterm_kbd_Prev,
    "PageDown",   XK_F35,    hpterm_kbd_Next,
    "Home",       XK_F27,    hpterm_kbd_Home,
    "End",        XK_F33,    hpterm_kbd_HomeDown,
#if defined(MEMLOCK_2000)
    "InsertChar", XK_Insert, hpterm_kbd_InsertChar,
    "DeleteChar", XK_Delete, hpterm_kbd_DeleteChar,
#endif
     0,           0,         0
};


#if defined(MEMLOCK_2000)
int keymapper (KeySym keysym, unsigned int state, char *buffer, int charcount)
{
/*
   **  Attempt to map the key to a special function
   **  If key maps, call the function and return 1
   **  If key does not map, return 0
 */
  int ii;

/*
   **  Following group needed for HP 715 workstation
 */
  if (state & ShiftMask)
  {
    if (keysym == XK_Up)
    {
      hpterm_kbd_RollUp ();
      return (1);
    }
    if (keysym == XK_Down)
    {
      hpterm_kbd_RollDown ();
      return (1);
    }
    if (keysym == XK_Home)
    {
      hpterm_kbd_HomeDown ();
      return (1);
    }
  }
/*
   **  Following group needed for Tatung Mariner 4i with AT keyboard
 */
  if (state & ShiftMask)
  {
    if (keysym == XK_KP_8)
    {
      hpterm_kbd_RollUp ();
      return (1);
    }
    if (keysym == XK_KP_2)
    {
      hpterm_kbd_RollDown ();
      return (1);
    }
    if (keysym == XK_Tab)
    {
      hpterm_kbd_BackTab ();
      return (1);
    }
  }
/*
 **  Following to allow Reflections compatible ALT accelerators
 **   ...but is doesn't work...  fixed!
 */
  if (state & Mod1Mask)
  {
    if (keysym == XK_1)
    {
      hpterm_kbd_F1 ();
      return (1);
    }
    else if (keysym == XK_2)
    {
      hpterm_kbd_F2 ();
      return (1);
    }
    else if (keysym == XK_U || keysym == XK_u)
    {
      hpterm_kbd_Menu ();
      return (1);
    }
    else if (keysym == XK_M || keysym == XK_m)
    {
      hpterm_kbd_Modes ();
      return (1);
    }
    else if (keysym == XK_S || keysym == XK_s)
    {
      hpterm_kbd_System ();
      return (1);
    }
    else if (keysym == XK_J || keysym == XK_j)
    {
      hpterm_kbd_Clear ();
      return (1);
    }
    else if (keysym == XK_R || keysym == XK_r)
    {
      hpterm_kbd_Reset ();
      return (1);
    }
    else if (keysym == XK_D || keysym == XK_d)
    {
      hpterm_kbd_DeleteLine ();
      return (1);
    }
    else if (keysym == XK_I || keysym == XK_i)
    {
      hpterm_kbd_InsertLine ();
      return (1);
    }
    else if (keysym == XK_P || keysym == XK_p)
    {
      dump_display ();
      return (1);
    }
  }
/*
 **  No special cases - now search the table
 */
  for (ii = 0; keymap[ii].keyname; ii++)
  {
    if (keymap[ii].keysym == keysym)
    {
      (*(keymap[ii].keyfunc)) ();
      return (1);
    }
  }
/*
   **  Following to allow use of lower case letters with CapsLock on
   **  and using the Shift key in combination with a letter 
 */
  if ((state & LockMask) && (state & ShiftMask) && (charcount == 1))
  {
    if ((keysym >= XK_A) && (keysym <= XK_Z))
      buffer[0] = tolower (buffer[0]);
  }
  return (0);
}
#else
int keymapper (KeySym keysym, unsigned int state) {
/*
**  Attempt to map the key to a special function
**  If key maps, call the function and return 1
**  If key does not map, return 0
*/
    int ii;

    if (DEBUG_KEYSYMS) {
	if (state) {
	    printf ("[%x,%lx]", state, keysym);
        } else {
            printf ("[%lx]", keysym);
        }
        fflush (stdout);
    }
/*
**  Following group needed for HP 715 workstation
*/
    if (state & ShiftMask) {
	if (keysym == XK_Up) {
	    hpterm_kbd_RollUp();
	    return (1);
        }
	if (keysym == XK_Down) {
	    hpterm_kbd_RollDown();
	    return (1);
        }
	if (keysym == XK_Home) {
	    hpterm_kbd_HomeDown();
	    return (1);
        }
    }
/*
**  Following group needed for Tatung Mariner 4i with AT keyboard
*/
    if (state & ShiftMask) {
        if (keysym == XK_KP_8) {
            hpterm_kbd_RollUp();
            return (1);
        }
        if (keysym == XK_KP_2) {
            hpterm_kbd_RollDown();
            return (1);
        }
        if (keysym == XK_Tab) {
            hpterm_kbd_BackTab();
            return (1);
        }
    }
/*
**  Following to allow Reflections compatible ALT accelerators
**   ...but is doesn't work...
*/
    if (state & Mod2Mask) {
        if (keysym == XK_1) {
            hpterm_kbd_F1();
            return (1);
        } else if (keysym == XK_2) {
            hpterm_kbd_F2();
            return (1);
        } else if (keysym == XK_M || keysym == XK_m) {
            hpterm_kbd_Menu();
            return (1);
        }
    }
/*
**  No special cases - now search the table
*/
    for (ii=0; keymap[ii].keyname; ii++) {
	if (keymap[ii].keysym == keysym) {
	   (*(keymap[ii].keyfunc))();
	   return (1);
        }
    }
    return (0);
}
#endif


int disp_drawtext (style, row, col, buf, nbuf)
     int style;			/* Low order 4 bits of display enhancements escape code */
     int row;			/* Row number of 1st char of string, 0..23 (or more) */
     int col;			/* Column number of 1st char of string, 0..79 (or more) */
     char *buf;			/* String to display */
     int nbuf;			/* Number of chars to display */
{
#if defined(MEMLOCK_2000)
  int font_height, font_width;
#else
  int font_height, font_width = CHAR_WIDTH;
#endif
  int ii;
  GC gc;

  font_height = font_info->ascent + font_info->descent;
#if defined(MEMLOCK_2000)
  font_width = font_info->max_bounds.width;
#endif

  if (style & HPTERM_BLINK_MASK)
  {
    gc = gc_red;
  }
  else if (style & HPTERM_HALFBRIGHT_MASK)
  {
    gc = gc_halfbright;
  }
  else
  {
    gc = gc_normal;
  }
  if (style & HPTERM_INVERSE_MASK)
  {
    XFillRectangle (display, win, gc,
		    col * font_width,
		    row * font_height,
		    nbuf * font_width,
		    1 * font_height);
    gc = gc_inverse;
  }

  XDrawString (display, win, gc, col * font_width,
	       font_info->ascent + row * font_height, buf, nbuf);

  if (style & HPTERM_UNDERLINE_MASK)
  {
    XFillRectangle (display, win, gc,
		    col * font_width,
		    row * font_height + font_info->ascent + 1,
		    nbuf * font_width, 1);
  }
/*
   **      Simulate half-bright attribute by re-drawing string one pixel
   **      to the right -- This creates a phony 'bold' effect
 */
#if NO_COLOR
  if (style & HPTERM_HALFBRIGHT_MASK)
  {
    XDrawString (display, win, gc, col * font_width + 1,
		 font_info->ascent + row * font_height, buf, nbuf);
  }
#endif
}


disp_erasetext (row, col, nchar)
     int row;
     int col;
     int nchar;
{
  int font_height;
#if defined(MEMLOCK_2000)
  int font_width;
#else
  int font_width = CHAR_WIDTH;
#endif

#if defined(MEMLOCK_2000)
  font_width = (font_info->max_bounds.width);
#endif
  font_height = font_info->ascent + font_info->descent;

  XFillRectangle (display, win, gc_inverse,
		  col * font_width, row * font_height,
		  nchar * font_width, font_height);
}

disp_drawcursor (style, row, col)
     int style;
     int row;
     int col;
{
  int font_height;
#if defined(MEMLOCK_2000)
  int font_width;
#else
  int font_width = CHAR_WIDTH;
#endif

  font_height = font_info->ascent + font_info->descent;
#if defined(MEMLOCK_2000)
  font_width = font_info->max_bounds.width;
#endif
  if (style & HPTERM_INVERSE_MASK)
  {

    XFillRectangle (display, win, gc_inverse, col * font_width,
		    font_info->ascent + (row * font_height) + 1,
		    font_width, 2);
  }
  else
  {

    XFillRectangle (display, win, gc_normal, col * font_width,
		    font_info->ascent + (row * font_height) + 1,
		    font_width, 2);
  }
}

void  doXBell (void)
{
  int strength = 50;
  XBell (display, strength);
}



void Usage (void)
{ /*Usage */

  printf ("xhpterm version %s\n\n",version_str);
  printf ("Usage: xhpterm [opts] [-rlogin] <hostname>\n");
  printf ("   or: xhpterm [opts] -tty <devicefile> -speed <cps> -parity {E|O|N}\n");
  printf ("opts:\n");
  printf ("   -li|-lo|-lio    - specify input|output logging options\n");
  printf ("   -lp             - logging output has a prefix\n");
  printf ("   -termid str     - override terminal ID [X-hpterm]\n");
  printf ("   -clean          - right-click exit disabled\n");
  printf ("   -f file         - destination for logging (default: stdout)\n");
  printf ("   -a file         - read initial commands from file.\n");
  printf ("   -df             - start with Display Functions enabled.\n");
#if defined(MEMLOCK_2000)
  printf ("   -font fontname  - override default font with fontname.\n");
#endif

} /*Usage */


int main (int argc, char **argv)
{
  int
    use_rlogin = 0, display_fns = 0, speed = 9600, parm_error = 0;
  char
   parity = 'N', *input_file = NULL, *hostname = NULL, *log_file = NULL, *ttyname = NULL;
#if defined(MEMLOCK_2000)
  char *font1 = NULL;
#endif
  
  /* init the logging stuff... *//* 970107 */

  logFd = stdout;		/* 970107 */
  log_type = 0;			/* 970107 */
  logging = 0;			/* 970107 */

#if !defined(MEMLOCK_2000)
  /* Start the X11 driver */
  init_disp(argc,argv);
#endif

  /* Start the datacomm module */
  ++argv;
  while ((--argc) && ((*argv)[0] == '-'))
  {
    if (!strcmp (*argv, "-help"))
    {
      Usage ();
      return (0);
    }
    if (!strcmp (*argv, "-rlogin"))
    {
      if ((--argc) && (argv[1][0] != '-'))
      {
	use_rlogin = 1;
	hostname = *(++argv);
      }
      else
	++parm_error;
    }
    else if (!strcmp (*argv, "-clean"))
      must_logoff = 1;
    else if (!strcmp (*argv, "-df"))
      display_fns = 1;
   else if (!strcmp (*argv, "-tty"))
    {
      if ((--argc) && (argv[1][0] != '-'))
	ttyname = *(++argv);
      else
	++parm_error;
    }
    else if (!strcmp (*argv, "-speed"))
    {
      if ((--argc) && (argv[1][0] != '-'))
      {
	++argv;
	speed = atoi (*argv);
      }
      else
	++parm_error;
    }
    else if (!strcmp (*argv, "-parity"))
    {
      if ((--argc) && (argv[1][0] != '-'))
      {
	char temp[32];
	++argv;
	parity = *argv[0];
	if (islower (parity))
	  parity = (char) toupper (parity);
	strcpy (temp, "EON");
	if (strchr (temp, parity) == (char *) NULL)
	  ++parm_error;
      }
      else
	++parm_error;
    }
    else if (!strcmp (*argv, "-termid"))
    {
      if ((--argc) && (argv[1][0] != '-'))
	termid = *(++argv);
      else
	++parm_error;
    }
#if defined(MEMLOCK_2000)
    else if (!strcmp (*argv, "-font"))
    {
      if ((--argc))
	font1 = *(++argv);
      else
	++parm_error;
    }
#endif
    else if (!strcmp (*argv, "-a"))
    {
      if ((--argc) && (argv[1][0] != '-'))
	input_file = *(++argv);
      else
	++parm_error;
    }
    else if (!strcmp (*argv, "-f"))
    {
      if ((--argc) && (argv[1][0] != '-'))
	log_file = *(++argv);
      else
	++parm_error;
    }
    else if (!strncmp (*argv, "-l", 2))
    {
      char *ptr;
      ptr = *argv;
      ptr += 2;
      while (*ptr)
      {
	if (*ptr == 'i')
	  log_type |= LOG_INPUT;
	else if (*ptr == 'o')
	  log_type |= LOG_OUTPUT;
	else if (*ptr == 'p')
	  log_type |= LOG_PREFIX;
	else
	{
	  ++parm_error;
	  break;
	}
	logging = 1;
	++ptr;
      }
    }
    else if ((strcmp(*argv, "-X") == 0) ||
	     (strcmp(*argv, "-otable") == 0) ||
	     (strcmp(*argv, "-table") == 0))
      {
	extern int table_spec;
	int i_type = 1;
	if (strcmp(*argv, "-otable") == 0)
	  ++i_type;
	if (--argc)
	  {
	    ++argv;
	    if (*argv[0] == '-')
	      ++parm_error;
	    else if (LoadKeybdTable(*argv, i_type))
	      ++parm_error;
	  }
	else
	  ++parm_error;
      }
    else
      ++parm_error;
    if (parm_error)
      break;
    ++argv;
  }

  if (parm_error)
  {
    fprintf (stderr, "Invalid parm [%s]\n", *argv);
    Usage ();
    return (1);
  }

  if (argc)
  {
    hostname = *argv;
#if defined(MEMLOCK_2000)
/* Start the X11 driver */
    init_disp (argc, argv, hostname, font1);
#endif
  }
#if defined(MEMLOCK_2000)
  else
  {
/* Start the X11 driver */
    init_disp (argc, argv, ttyname, font1);
  }
#endif
  /* Start the terminal emulator */
  init_hpterm ();
  if (display_fns)
    set_display_functions ();

  if (input_file)
  {
    FILE *input;
    char buf[128], *ptr;
    if ((input = fopen (input_file, "r")) == (FILE *) NULL)
    {
      char buf[128];
      sprintf (buf, "fopen [%s]:", input_file);
      perror (buf);
      return (1);
    }
    for (;;)
    {
      if (fgets (buf, sizeof (buf) - 1, input) == NULL)
	break;
      ptr = buf;
      while (*ptr)
      {
	if (*ptr == '\n')
	  PutQ ('\r');
	else
	  PutQ (*ptr);
	++ptr;
      }
    }
    fclose (input);
  }

  if (log_file == NULL)
    logFd = stdout;
  else if ((logFd = fopen (log_file, "w")) == (FILE *) NULL)
  {
    perror ("fopen of log_file");
    return (1);
  }

  if (ttyname)
  {
    char ttyinfo[256];
    sprintf (ttyinfo, "%s|%d|%c", ttyname, speed, parity);
    con = conmgr_connect (e_tty, ttyinfo);
  }
  else if (hostname)
  {
    if (use_rlogin)
      con = conmgr_connect (e_rlogin, hostname);
    else
      con = conmgr_connect (e_vt3k, hostname);
  }
  else
  {
    fprintf (stderr, "Missing hostname\n");
    Usage ();
    con = 0;
  }

  if (!con)
    return (1);

  event_loop ();

  XUnloadFont (display, font_info->fid);
  XFreeGC (display, gc_normal);
  XFreeGC (display, gc_inverse);
  XFreeGC (display, gc_halfbright);
  XFreeGC (display, gc_red);
  XCloseDisplay (display);

  conmgr_close (con);
}				/* main */
