/***************************************************************/
#ifdef __hpux
#  ifndef _HPUX_SOURCE
#    define _HPUX_SOURCE (1)
#  endif
#  ifndef _POSIX_SOURCE
#    define _POSIX_SOURCE (1)
#  endif
#endif
/***************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
/***************************************************************/
#include <stdio.h>
#include <string.h>
/***************************************************************/
#include "conmgr.h"
/***************************************************************/
static void show_tty_error (funcname, errnum)
/*
**  Show error condition
*/
    char *funcname;
    int   errnum;
{
    fprintf (stderr, "An I/O error has occurred\n");
    fprintf (stderr, "Function name: %s\n", funcname);
    fprintf (stderr, "Error number:  %d\n", errnum);
    perror  ("Error message");
    fflush (stderr);
}
/***************************************************************/
int open_tty_connection (devicename)
/*
**  Create tty connection, return file number
*/
    char *devicename;
{
    int s;
    int mode=0;

    s = open (devicename, O_RDWR | O_NDELAY | O_NOCTTY, mode);
    if (s < 0) {
        printf ("Error %d from open(%s)\n", errno, devicename);
        show_tty_error ("open()", errno);
        return (0);
    }

printf ("Got the connection!\n");
fflush (stdout);
/*
**  Put termio stuff here
**  Baud rate, parity, etc.
*/
    return (s);
}
/***************************************************************/
int read_tty_data (s)
/*
**  Read data from the tty port
**  Send it to the terminal emulator
**  Returns non-zero on port eof
*/
    int s;       /* File number */
{
    int flags,len;
    int bufsize = 2048;
    char buf[2048];

    len = read (s, buf, bufsize);
    if (len < 0) {
	show_tty_error ("read()", errno);
	return (1);
    } else if (!len) {
	printf ("read(): len=0\n");
	show_tty_error ("read()", errno);
	return (1);
    }

    conmgr_rxfunc (0, buf, len);

    return (0);
}
/***************************************************************/
int send_tty_data (s, buf, nbuf)
/*
**  Send data to the tty port
**  Return non-zero on port eof
*/
    int s;       /* file number */
    char *buf;   /* character buffer */
    int nbuf;    /* character count */
{
    int flags,len;

    flags = 0;
    len = write (s, buf, nbuf);
    if (len < nbuf) {
        show_tty_error ("write()", errno);
        return (1);
    }
    return (0);
}
/***************************************************************/
