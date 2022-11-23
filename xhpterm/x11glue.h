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

#ifndef VMS
#include <stdio.h>
#endif

#define LOG_INPUT               (0x01)
#define LOG_OUTPUT              (0x02)
#define LOG_PREFIX              (0x04)

extern FILE
* logFd;

extern int
  log_type;

#if defined(MEMLOCK_2000)
void init_disp (int argc, char **argv, char *hostname, char *font1);
#else
void init_disp(int argc, char **argv);
#endif
void event_loop (void);
void getGC (Window win, GC * gc, XFontStruct * font_info);
#if defined(MEMLOCK_2000)
void load_font (XFontStruct ** font_info, char *font1);
int keymapper (KeySym keysym, unsigned int state, char *buffer, int charcount);
#else
int load_font(XFontStruct **font_info);
int keymapper (KeySym keysym, unsigned int state);
#endif
void disp_drawtext (int style, int row, int col, char *buf, int nbuf);
void disp_erasetext (int row, int col, int nchar);
void disp_drawcursor (int style, int row, int col);
void Logit (int typ, char *ptr, size_t len, bool special_dc1);
void doXBell (void);
