/*
 * x11glue.h  --  X11 window interface
 *
 * Derived from O'Reilly and Associates example 'basicwin.c'
 * Copyright 1989 O'Reilly and Associates, Inc.
 * See ../Copyright for complete rights and liability information.
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <stdio.h>

void init_disp(int argc, char **argv);
void event_loop (void);
int getGC(Window win, GC *gc, XFontStruct *font_info);
int load_font(XFontStruct **font_info);
int keymapper (KeySym keysym, unsigned int state);
int disp_drawtext (int style, int row, int col, char *buf, int nbuf);
int disp_erasetext (int row, int col, int nchar);
int disp_drawcursor (int style, int row, int col);
