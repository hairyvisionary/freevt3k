/* Copyright (C) 1996 Office Products Technology, Inc.

   vt3kglue.h  --  Interface between character based terminal
		   emulator and record based vt3k protocol

   freevt3k: a VT emulator for Unix.

   This file is distributed under the GNU General Public License.
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software Foundation,
   Inc., 675 Mass Ave, Cambridge MA 02139, USA.

   Based on freevt3k.c

   Original: Bruce Toback, 22 MAR 96
   Additional: Dan Hollis <dhollis@pharmcomp.com>, 27 MAR 96
               Randy Medd <randy@telamon.com>, 28 MAR 96
	       Jeff Moffatt <jeff@taurus.com>, 31 MAR 96
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
#      define _HPUX_SOURCE (1)
#    endif
#    ifndef _POSIX_SOURCE
#      define _POSIX_SOURCE (1)
#    endif
#  endif

#  include <sys/types.h>
#  include <unistd.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <string.h>
#  include <errno.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <fcntl.h>
#  include <termios.h>
#  include <sys/types.h>
#  include <sys/time.h>
#  include <signal.h>
#endif
#include "typedef.h"
#include "vt.h"
#include "freevt3k.h"
#include "vtconn.h"

#include "hpterm.h"

void myDataOutProc (int32 refCon, char *buf, int nbuf);
tVTConnection * open_vt3k_connection (char *hostname);
int read_vt3k_data (tVTConnection * theConnection);
int send_vt3k_data (tVTConnection * theConnection, char *buf, int nbuf);
void send_vt3k_break (tVTConnection * theConnection);
void close_vt3k (tVTConnection * theConnection);
