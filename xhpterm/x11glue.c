/*
 * x11glue.c  --  X11 window interface
 *
 * Derived from O'Reilly and Associates example 'basicwin.c'
 * Copyright 1989 O'Reilly and Associates, Inc.
 * See ../Copyright for complete rights and liability information.
 */
#ifdef __hpux
#  ifndef _HPUX_SOURCE
#    define _HPUX_SOURCE 1
#  endif
#  ifndef _POSIX_SOURCE
#    define _POSIX_SOURCE
#  endif
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <stdio.h>
#include <sys/time.h>
#if defined(_AIX) || defined(AIX) || defined(aix)
#  include <sys/select.h>
#endif

#include "x11glue.h"
#include "conmgr.h"
#include "hpterm.h"
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

struct conmgr *con=0;

/*******************************************************************/
#define GRAY_INDEX 0
#define RED_INDEX 1
#define NB_COLORS 2
unsigned long color_codes[NB_COLORS];
char *color_names[NB_COLORS] = { "light gray", "red" };
int got_colors=0;
extern int get_colors();
/*******************************************************************/
char *progname; /* name this program was invoked by */

void init_disp(argc, argv)
int argc;
char **argv;
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

	width = nbcols * 9;          /* Should get the font first */
	height = nbrows * 15;

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
	got_colors = get_colors (NB_COLORS, color_names, color_codes);
	if (!got_colors) {
	    color_codes[GRAY_INDEX] = BlackPixel(display,screen_num);
	    color_codes[RED_INDEX] = BlackPixel(display,screen_num);
        }

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

void event_loop () {

	int xsocket,nfds,i;  /* select stuff */
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

	while (!con->eof)  {
                term_update();      /* flush deferred screen updates */
		XFlush(display);    /* and send them to the server */

		/* Use select() to wait for traffic from X or remote computer */
		xsocket = ConnectionNumber(display);
		dsocket = con->socket;
		nfds = (dsocket < xsocket) ? (1 + xsocket) : (1 + dsocket);

		FD_ZERO(&readmask);
		if (!(con->eof)) FD_SET(dsocket, &readmask);
		FD_SET(xsocket, &readmask);
		readfds = readmask;
		i = select (nfds, (void*)&readfds, 0, 0, 0);

		if (i < 0) {
			perror ("select failed");
		} else if (i == 0) {
			printf ("select timed out\n");
		} else if ((FD_ISSET(dsocket, &readmask)) && 
                           (FD_ISSET(dsocket, &readfds))) {

			conmgr_read (con);

			/*
			**  Wait up to 50 ms for more data from host
			**  This should prevent excessive re-draws
			*/
                        i = 1;
                        count = 0;
                        while (i > 0 && count < 10 && !(con->eof)) { 
			    struct timeval timeout;
			    timeout.tv_sec = 0;
			    timeout.tv_usec = 50000;

			    FD_ZERO(&readfds);
			    FD_SET(dsocket, &readfds);

			    i = select (nfds, (void*)&readfds, 0, 0, &timeout);
			    if (i < 0) {
				perror ("select failed");
                            } else if (i == 0) {
				/* time out */

			    } else if (FD_ISSET(dsocket, &readfds)) {
                                /* more data */
				conmgr_read (con);
                            }
                            count++;  /* Can't ignore X for too long */
                        }
		}
		while (XPending (display)) {
		XNextEvent(display, &report);
		switch  (report.type) {
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
			nbcols = width / 9;    /* Needs work here */
			nbrows = height / 15;
			hpterm_winsize (nbrows, nbcols);
			break;
		case ButtonPress:
			if (report.xbutton.button == 1) {
			    int r,c;
			    c = report.xbutton.x / 9;
			    r = report.xbutton.y / 15;
			    hpterm_mouse_click (r, c);
                        }
			/* right mouse button causes program to exit */
			if (report.xbutton.button == 3) {
                            return;
                        }
			break;
		case KeyPress:
			charcount = XLookupString(&report.xkey, buffer,
					bufsize, &keysym, &compose);
    if (0) {
      int ii;
      printf ("(%x)", keysym);
      printf ("<");
      for (ii=0; ii<charcount; ii++) {
	if (ii) printf (",");
	printf ("%d", buffer[ii]);
      }
      printf (">");
      fflush(stdout);
    }

			if (!keymapper(keysym, report.xkey.state)) {
			    if (charcount == 1) {
				hpterm_kbd_ascii (buffer[0]);
                            }
			}
			break;
		default:
			/* all events selected by StructureNotifyMask
			 * except ConfigureNotify are thrown away here,
			 * since nothing is done with them */
			break;
		} /* end switch */
	    }
	} /* end while */
}

getGC(win, gc, font_info)
Window win;
GC *gc;
XFontStruct *font_info;
{
	unsigned long valuemask = 0; /* ignore XGCvalues and use defaults */
	XGCValues values;
	unsigned int line_width = 6;
	int line_style = LineOnOffDash;
	int cap_style = CapRound;
	int join_style = JoinRound;
	int dash_offset = 0;
	static char dash_list[] = {12, 24};
	int list_length = 2;

	/* Create default Graphics Context */
	*gc = XCreateGC(display, win, valuemask, &values);

	/* specify font */
	XSetFont(display, *gc, font_info->fid);

	/* specify black foreground since default window background is
	 * white and default foreground is undefined. */
	XSetForeground(display, *gc, BlackPixel(display,screen_num));

	/* set line attributes */
	XSetLineAttributes(display, *gc, line_width, line_style,
			cap_style, join_style);

	/* set dashes */
	XSetDashes(display, *gc, dash_offset, dash_list, list_length);
}

getGC_Inverse(win, gc, font_info)
Window win;
GC *gc;
XFontStruct *font_info;
{
	unsigned long valuemask = 0; /* ignore XGCvalues and use defaults */
	XGCValues values;

	/* Create default Graphics Context */
	*gc = XCreateGC(display, win, valuemask, &values);

	/* specify font */
	XSetFont(display, *gc, font_info->fid);

	/* Set foreground and background swapped for Inverse Video */
	XSetForeground(display, *gc, WhitePixel(display,screen_num));
	XSetBackground(display, *gc, BlackPixel(display,screen_num));
}

getGC_Halfbright(win, gc, font_info)
Window win;
GC *gc;
XFontStruct *font_info;
{
	unsigned long valuemask = 0; /* ignore XGCValues and use defaults */
	XGCValues values;

	/* Create default Graphics Context */
	*gc = XCreateGC(display, win, valuemask, &values);

	/* specify font */
	XSetFont(display, *gc, font_info->fid);

	/* Set foreground gray and background white for halfbright video */
	XSetForeground(display, *gc, color_codes[GRAY_INDEX]);
	XSetBackground(display, *gc, WhitePixel(display,screen_num));
}

getGC_Red(win, gc, font_info)
Window win;
GC *gc;
XFontStruct *font_info;
{
	unsigned long valuemask = 0; /* ignore XGCValues and use defaults */
	XGCValues values;

	/* Create default Graphics Context */
	*gc = XCreateGC(display, win, valuemask, &values);

	/* specify font */
	XSetFont(display, *gc, font_info->fid);

	/* Set foreground red and background white */
	XSetForeground(display, *gc, color_codes[RED_INDEX]);
	XSetBackground(display, *gc, WhitePixel(display,screen_num));
}

load_font(font_info)
XFontStruct **font_info;
{
	char *fontname = "9x15";

	/* Load font and get font information structure. */
	if ((*font_info = XLoadQueryFont(display,fontname)) == NULL)
	{
		(void) fprintf( stderr, "%s: Cannot open 9x15 font\n",
				progname);
		exit( -1 );
	}
}

static struct km {
    char     *keyname;
    KeySym    keysym;
    void      (*keyfunc)();
} keymap[] = {
    "Break"        , XK_Break         , hpterm_kbd_Break,
    "Menu"         , XK_Menu          , hpterm_kbd_Menu,
    "F1"           , XK_F1            , hpterm_kbd_F1,
    "F2"           , XK_F2            , hpterm_kbd_F2,
    "F3"           , XK_F3            , hpterm_kbd_F3,
    "F4"           , XK_F4            , hpterm_kbd_F4,
    "F5"           , XK_F5            , hpterm_kbd_F5,
    "F6"           , XK_F6            , hpterm_kbd_F6,
    "F7"           , XK_F7            , hpterm_kbd_F7,
    "F8"           , XK_F8            , hpterm_kbd_F8,
    "Home"         , XK_Home          , hpterm_kbd_Home,
    "Left"         , XK_Left          , hpterm_kbd_Left,
    "Right"        , XK_Right         , hpterm_kbd_Right,
    "Down"         , XK_Down          , hpterm_kbd_Down,
    "Up"           , XK_Up            , hpterm_kbd_Up,
    "Prev"         , XK_Prior         , hpterm_kbd_Prev,
    "Next"         , XK_Next          , hpterm_kbd_Next,
    "Select"       , XK_Select        , hpterm_kbd_Select,
    "KP_Enter"     , XK_KP_Enter      , hpterm_kbd_KP_Enter,
    "Enter"        , XK_Execute       , hpterm_kbd_Enter,
/*
**  Following group needed for HP 715 workstation
*/
#ifdef hpXK_Reset
    "Reset"        , hpXK_Reset       , hpterm_kbd_Reset,
    "User"         , hpXK_User        , hpterm_kbd_User,
    "System"       , hpXK_System      , hpterm_kbd_System,
    "ClearLine"    , hpXK_ClearLine   , hpterm_kbd_ClearLine,
    "InsertLine"   , hpXK_InsertLine  , hpterm_kbd_InsertLine,
    "DeleteLine"   , hpXK_DeleteLine  , hpterm_kbd_DeleteLine,
    "ClearDisplay" , XK_Clear         , hpterm_kbd_ClearDisplay,
    "InsertChar"   , hpXK_InsertChar  , hpterm_kbd_InsertChar,
    "DeleteChar"   , hpXK_DeleteChar  , hpterm_kbd_DeleteChar,
    "BackTab"      , hpXK_BackTab     , hpterm_kbd_BackTab,
    "KP_BackTab"   , hpXK_KP_BackTab  , hpterm_kbd_KP_BackTab,
#endif
/*
**  Following group needed for Tatung Mariner 4i with AT keyboard
*/
    "F9"           , XK_F9            , hpterm_kbd_Menu,
    "F10"          , XK_F10           , hpterm_kbd_User,
    "F11"          , 0x1000FF10       , hpterm_kbd_System,
    "Break"        , XK_F23           , hpterm_kbd_Break,
    "PageUp"       , XK_F29           , hpterm_kbd_Prev,
    "PageDown"     , XK_F35           , hpterm_kbd_Next,
    "Home"         , XK_F27           , hpterm_kbd_Home,
    "End"          , XK_F33           , hpterm_kbd_HomeDown,
    0              , 0                , 0
};


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


disp_drawtext (style, row, col, buf, nbuf)
int style;    /* Low order 4 bits of display enhancements escape code */
int row;      /* Row number of 1st char of string, 0..23 (or more) */
int col;      /* Column number of 1st char of string, 0..79 (or more) */
char *buf;    /* String to display */
int nbuf;     /* Number of chars to display */
{
	int font_height;
	int font_width = 9;   /* Yikes! */
	int ii;
	GC gc;

	font_height = font_info->ascent + font_info->descent;

	if (style & HPTERM_BLINK_MASK) {
	    gc = gc_red;
	} else if (style & HPTERM_HALFBRIGHT_MASK) {
	    gc = gc_halfbright;
        } else {
	    gc = gc_normal;
        }
	if (style & HPTERM_INVERSE_MASK) {
            XFillRectangle (display, win, gc,
                            col * font_width,
                            row * font_height,
                            nbuf * font_width,
                            1 * font_height);
	    gc = gc_inverse;
	}

        XDrawString (display, win, gc, col * font_width,
		     font_info->ascent + row * font_height, buf, nbuf);
            
        if (style & HPTERM_UNDERLINE_MASK) {
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
	if (style & HPTERM_HALFBRIGHT_MASK) {
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
	int font_width = 9;  /* Yikes! */

	font_height = font_info->ascent + font_info->descent;

	XFillRectangle (display, win, gc_inverse,
			col * font_width, row * font_height,
			nchar * font_width, font_height);
}

disp_drawcursor (style,row,col)
int style;
int row;
int col;
{
	int font_height;
	int font_width = 9;

	font_height = font_info->ascent + font_info->descent;

        if (style & HPTERM_INVERSE_MASK) {

            XFillRectangle (display, win, gc_inverse, col * font_width,
                            font_info->ascent + (row * font_height) + 1,
                            font_width, 2);
        } else {

	    XFillRectangle (display, win, gc_normal, col * font_width,
			    font_info->ascent + (row * font_height) +1,
			    font_width, 2);
        }
}




int main(argc,argv)
int argc;
char **argv;
{
    /* Start the X11 driver */
    init_disp(argc,argv);

    /* Start the terminal emulator */
    init_hpterm ();

    /* Start the datacomm module */
    if (argc == 2) {
        con = conmgr_connect (e_vt3k, argv[1]);
    } else if (argc==3 && !strcmp(argv[1],"-rlogin")) {
	con = conmgr_connect (e_rlogin, argv[2]);
    } else if (argc==3 && !strcmp(argv[1],"-tty")) {
	con = conmgr_connect (e_tty, argv[2]);
    } else {
	printf ("Usage: xhpterm <hostname>\n");
	printf ("   or: xhpterm -rlogin <hostname>\n");
	printf ("   or: xhpterm -tty <devicefile>\n");
	printf ("Press right mouse button to exit\n");
	con=0;
    }
    if (!con) exit(-1);

    event_loop();

    XUnloadFont(display, font_info->fid);
    XFreeGC(display, gc_normal);
    XFreeGC(display, gc_inverse);
    XFreeGC(display, gc_halfbright);
    XFreeGC(display, gc_red);
    XCloseDisplay(display);

    conmgr_close (con);
}
