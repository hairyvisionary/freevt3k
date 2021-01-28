/*
 * Copyright 1989 O'Reilly and Associates, Inc.
 * See ../Copyright for complete rights and liability information.
 */
#ifdef VMS
#  include <types.h>
#  include <stdio.h>
#  include <unixio.h>
#  include <string.h>
#  include <stdlib.h>
#  include <time.h>
#  include <timeb.h>
#  include <stdarg.h>
#  include <ctype.h>
#  include <errno.h>
#  include <limits.h>
#  include <file.h>
#  include <signal.h>
#  include <assert.h>
#  include <iodef.h>
#  include <stsdef.h>
#  include <socket.h>
#  include <in.h>
#  include <netdb.h>
#  include <inet.h>
#  include <lib$routines.h>
#  include <starlet.h>
#  include <ucx$inetdef.h>
#else
#  include "config.h"
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <X11/Xos.h>
#  include <stdio.h>
#  include <stdlib.h>
#endif

#define DEBUG_GET_COLORS 0 
#define BLACKWHITE_OK 0
#define GRAYSCALE_OK 0

extern Display *display;
extern int screen_num;
extern Screen *screen_ptr;
extern char *progname;


static char *visual_class[] = {
"StaticGray",
"GrayScale",
"StaticColor",
"PseudoColor",
"TrueColor",
"DirectColor"
};

get_colors(nb_colors, color_names, color_codes)

    int nb_colors;
    char **color_names;
    unsigned long *color_codes;
{
        int default_depth;
        Visual *default_visual;
	XColor exact_def;
	Colormap default_cmap;
	int ncolors = 0;
	int i = 5;
	XVisualInfo visual_info;
	
	/* Try to allocate colors for PseudoColor, TrueColor, 
	 * DirectColor, and StaticColor.  Use black and white
	 * for StaticGray and GrayScale */

	default_depth = DefaultDepth(display, screen_num);
        default_visual = DefaultVisual(display, screen_num);
	default_cmap   = DefaultColormap(display, screen_num);
	if (default_depth == 1) {
		/* must be StaticGray, use black and white */
#if BLACKWHITE_OK
		border_pixel = BlackPixel(display, screen_num);
		background_pixel = WhitePixel(display, screen_num);
		foreground_pixel = BlackPixel(display, screen_num);
		return(0);
#else
                fprintf (stderr,"Need a color monitor\n");
                exit(0);
#endif
	}

	while (!XMatchVisualInfo(display, screen_num, default_depth, /* visual class */i--, &visual_info)) ;
#if DEBUG_GET_COLORS		
	printf("%s: found a %s class visual at default_depth.\n", 
                progname, visual_class[++i]);
#endif	
	if (i < 2) {
		/* No color visual available at default_depth.
		 * Some applications might call XMatchVisualInfo
		 * here to try for a GrayScale visual 
		 * if they can use gray to advantage, before 
		 * giving up and using black and white.
		 */
#if GRAYSCALE_OK
		border_pixel = BlackPixel(display, screen_num);
		background_pixel = WhitePixel(display, screen_num);
		foreground_pixel = BlackPixel(display, screen_num);
		return(0);
#else
                fprintf (stderr,"Need a color monitor\n");
                exit (0);
#endif
	}

	/* otherwise, got a color visual at default_depth */

	/* The visual we found is not necessarily the 
	 * default visual, and therefore it is not necessarily
	 * the one we used to create our window.  However,
	 * we now know for sure that color is supported, so the
	 * following code will work (or fail in a controlled way).
	 * Let's check just out of curiosity: */
	if (visual_info.visual != default_visual)
		printf("%s: PseudoColor visual at default depth is not default visual!\nContinuing anyway...\n", progname);

	for (i = 0; i < nb_colors; i++) {
#if DEBUG_GET_COLORS
		printf("allocating %s\n", color_names[i]);
#endif
		if (!XParseColor (display, default_cmap, color_names[i],
                                 &exact_def)) {
			fprintf(stderr, "%s: color name %s not in database",
                                 progname, color_names[i]);
			exit(0);
		}
#if DEBUG_GET_COLORS
		printf("The RGB values from the database are %d, %d, %d\n", exact_def.red, exact_def.green, exact_def.blue);
#endif
   		if (!XAllocColor(display, default_cmap, &exact_def)) {
			fprintf(stderr, "%s: can't allocate color: all colorcells allocated and no matching cell found.\n", progname);
		exit(0);
		}
#if DEBUG_GET_COLORS
		printf("The RGB values actually allocated are %d, %d, %d\n", exact_def.red, exact_def.green, exact_def.blue);
#endif
		color_codes[i] = exact_def.pixel;
		ncolors++;
	}
#if DEBUG_GET_COLORS
	printf("%s: allocated %d read-only color cells\n", progname, ncolors);
#endif
	return(1);
}
