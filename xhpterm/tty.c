/***************************************************************/
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
#  ifdef HPUX
#    ifndef _HPUX_SOURCE
#      define _HPUX_SOURCE (1)
#    endif
#    ifndef _POSIX_SOURCE
#      define _POSIX_SOURCE (1)
#    endif
#  endif
/***************************************************************/
#  include <errno.h>
#  include <fcntl.h>
#  include <sys/types.h>
#  include <unistd.h>
/***************************************************************/
#  include <stdio.h>
#  include <stddef.h>
#  include <stdlib.h>
#  include <string.h>
#  ifdef HAVE_TERMIOS_H
#    include <termios.h>
#  else
#    include <termio.h>
#  endif
#endif
/***************************************************************/
#include "conmgr.h"

#ifdef HAVE_TERMIOS_H
typedef struct termios TERMIO, *PTERMIO;
#else
typedef struct termio TERMIO, *PTERMIO;
#endif

static TERMIO
	curr_termio, 	/* tty termio block */
	prev_termio;	/* tty previous termio block */

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
int open_tty_connection (deviceinfo)
  /*
   **  Create tty connection, return file number
   */
  char *deviceinfo;

{

  char
    *ptr,
    parity,
    *devicename;
  int
    fd = 0,
    speed = 0,
    flags = 0;

  devicename = (char*)malloc(strlen(deviceinfo)+1);
  strcpy(devicename, deviceinfo);
  ptr = strchr(devicename, '|');
  if (ptr == (char*)NULL)
    {
      printf("Missing '|': [%s]\n", deviceinfo);
      return(-1);
    }
  *(ptr++) = '\0';
  speed = atoi(ptr);
  ptr = strchr(ptr, '|');
  if (ptr == (char*)NULL)
    {
      printf("Missing '|': [%s]\n", deviceinfo);
      return(-1);
    }
  *(ptr++) = '\0';
  parity = *ptr;
  fd = open (devicename, O_RDWR | O_NDELAY | O_NOCTTY, 0);
  if (fd < 0)
    {
      printf ("Error %d from open(%s)\n", errno, devicename);
      show_tty_error ("open()", errno);
      free(devicename);
      return(-1);
    }
  free(devicename);

  printf ("Got the connection!\n");
  fflush (stdout);
/*
**  Put termio stuff here
**  Baud rate, parity, etc.
*/
#ifdef HAVE_TERMIOS_H
  if (tcgetattr(fd, &prev_termio) == -1)
    {
      printf ("Error %d from tcgetattr(%s)\n", errno, deviceinfo);
      show_tty_error ("tcgetattr()", errno);
      return(-1);
    }
#else
  if (ioctl(fd, TCGETA, &prev_termio) == -1)
    {
      printf ("Error %d from ioctl(TCGETA):%s\n", errno, deviceinfo);
      show_tty_error ("ioctl()", errno);
      return(-1);
    }
#endif

  curr_termio = prev_termio;

  switch (speed)
    {
    case  300:
    case   30: speed =   B300; break;
    case 1200:
    case  120: speed =  B1200; break;
    case 2400:
    case  240: speed =  B2400; break;
    case 4800:
    case  480: speed =  B4800; break;
    case 9600:
    case  960: speed =  B9600; break;
    case 38400:
    case 3840: speed = B38400; break;
    default:   speed = B19200;
    }
#ifdef _POSIX_SOURCE
  if (cfsetispeed((PTERMIO)&curr_termio, speed) == -1)
    {
      printf ("Error %d from cfsetispeed(%s)\n", errno, deviceinfo);
      show_tty_error ("cfsetispeed()", errno);
      return(-1);
    }
  if (cfsetospeed((PTERMIO)&curr_termio, speed) == -1)
    {
      printf ("Error %d from cfsetospeed(%s)\n", errno, deviceinfo);
      show_tty_error ("cfsetospeed()", errno);
      return(-1);
    }
#else
  curr_termio.c_cflag &= ~(CBAUD);
  curr_termio.c_cflag |= speed;
#endif
  curr_termio.c_cflag &= ~(CSIZE);
  curr_termio.c_cflag |= (CREAD | CLOCAL);
  
  if (parity == 'N')
    {
      curr_termio.c_cflag &= ~(PARENB | PARODD);
      curr_termio.c_cflag |= (CS8);
    }
  else
    {
      curr_termio.c_cflag &= ~(PARODD);
      curr_termio.c_cflag |= (CS7 | PARENB);
      if (parity == 'O')
	curr_termio.c_cflag |= (PARODD);
    }

  curr_termio.c_iflag = (IGNBRK | IGNPAR);
  
  curr_termio.c_oflag = 0;
  
  curr_termio.c_lflag = 0;
  curr_termio.c_cc[VMIN] = 1;
  curr_termio.c_cc[VTIME] = 0;

#ifdef HAVE_TERMIOS_H
  if (tcsetattr(fd, TCSANOW, &curr_termio) == -1)
    {
      printf ("Error %d from tcsetattr(%s)\n", errno, deviceinfo);
      show_tty_error ("tcsetattr()", errno);
      return(-1);
    }
#else
  if (ioctl(fd, TCSETA, &curr_termio) == -1)
    {
      printf ("Error %d from ioctl(TCSETA):%s\n", errno, deviceinfo);
      show_tty_error ("ioctl()", errno);
      return(-1);
    }
#endif
  
  if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
    {
      printf ("Error %d from fcntl(%s)\n", errno, deviceinfo);
      show_tty_error ("fcntl()", errno);
      return(-1);
    }
#ifdef O_NONBLOCK
  flags &= ~O_NONBLOCK;
#endif
#ifdef O_NDELAY
  flags &= ~O_NDELAY;
#endif
  if (fcntl(fd, F_SETFL, flags) == -1)
    {
      printf ("Error %d from fcntl(%s)\n", errno, deviceinfo);
      show_tty_error ("fcntl()", errno);
      return(-1);
    }
  
  return (fd);

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
