/*
**  conmgr.c -- Connection manager
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
#  ifdef __hpux
#    ifndef _HPUX_SOURCE
#      define _HPUX_SOURCE 1
#    endif
#    ifndef _POSIX_SOURCE
#      define _POSIX_SOURCE
#    endif
#  endif
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#endif
#include "vt3kglue.h"
#include "tty.h"
#include "rlogin.h"
#include "conmgr.h"
/*******************************************************************/
#define SHOW_RX_DATA 0
#define SHOW_TX_DATA 0
/*******************************************************************/
void conmgr_rxfunc (long refcon, char *buf, int nbuf) {
/*
**  Callback function which receives characters from the connection
*/
#if SHOW_RX_DATA
    {
	int ii,ch;
	for (ii=0; ii<nbuf; ii++) {
	    ch = buf[ii];
	    if (ch<32 || ch>126) {
	        printf ("<%d>", ch);
	    } else {
		printf ("<%c>", ch);
	    }
	    hpterm_rxfunc (0, &buf[ii], 1);
	}
	fflush (stdout);
    }
#else
    hpterm_rxfunc (0, buf, nbuf);
#endif
}
/*******************************************************************/
struct conmgr * conmgr_connect (enum e_contype type, char *hostname) {
/*
**  Establish a connection
*/
    void *ptr=0;
    int s=0;
    struct conmgr *out=0;

    switch (type) {
	case e_tty:
	    s = open_tty_connection (hostname);
	    if (s == -1)
	      return (0);
	    break;
        case e_rlogin:
	    s = open_rlogin_connection (hostname);
	    if (!s) return (0);
	    break;
        case e_vt3k:
	    ptr = open_vt3k_connection (hostname);
	    if (!ptr) return (0);
	    s = VTSocket(ptr);
	    break;
        default:
	    printf ("conmgr_connect: unknown contype %d\n", type);
	    return (0);
    }
    out = (struct conmgr*)calloc(1,sizeof(struct conmgr));
    out->type = type;
    out->hostname = strcpy(malloc(strlen(hostname)+1),hostname);
    out->ptr = ptr;
    out->socket = s;
    out->eof = 0;
    return (out);
}
/*********************************************************************/
void conmgr_read (struct conmgr *con) {
/*
**  Read from connection, send to term_rxchar
*/
    int stat=0;

    switch (con->type) {
	case e_tty:
	    stat = read_tty_data (con->socket);
	    break;
        case e_rlogin:
	    stat = read_rlogin_data (con->socket);
	    break;
        case e_vt3k:
	    stat = read_vt3k_data (con->ptr);
	    break;
        default:
	    printf ("conmgr_read: type=%d\n", con->type);
	    stat = 1;
	    break;
    }
    if (stat) con->eof=1;
}
/***********************************************************************/
void conmgr_send (struct conmgr *con, char *buf, int nbuf) {

    int stat;

#if SHOW_TX_DATA
    {
	int ii,ch;
	for (ii=0; ii<nbuf; ii++) {
	    ch = buf[ii];
	    if (ch<32 || ch>126) {
		printf ("{%d}", ch);
            } else {
		printf ("{%c}", ch);
            }
        }
	fflush (stdout);
    }
#endif

    switch (con->type) {
	case e_tty:
	    stat = send_tty_data (con->socket, buf, nbuf);
	    break;
        case e_rlogin:
	    stat = send_rlogin_data (con->socket, buf, nbuf);
	    break;
        case e_vt3k:
	    stat = send_vt3k_data (con->ptr, buf, nbuf);
	    break;
        default:
	    printf ("conmgr_send: type=%d\n", con->type);
	    stat = 1;
	    break;
    }
    if (stat) con->eof=1;
}
/*********************************************************************/
void conmgr_send_break (struct conmgr *con) {

    switch (con->type) {
        case e_tty:
	    break;
        case e_rlogin:
	    break;
        case e_vt3k:
	    send_vt3k_break (con->ptr);
	    break;
	default:
	    break;
    }
}
/*********************************************************************/
void conmgr_close (struct conmgr *con) {

    switch (con->type) {
	case e_tty:
	    break;
        case e_rlogin:
	    break;
        case e_vt3k:
	    close_vt3k (con->ptr);
	    break;
        default:
	    break;
    }
}
/********************************************************************/
