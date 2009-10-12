/************************************************************
 * Compiler-dependent typedefs
 *
 * typedef.h
 ************************************************************/

#ifndef _TYPEDEF_H
#define _TYPEDEF_H

typedef short int16;
typedef unsigned short unsigned16;
typedef long int32;
typedef unsigned long unsigned32;
typedef unsigned char unsigned8;
typedef char tBoolean;

#  ifdef VMS
#    include "vmstypes.h"
#  endif

#  ifdef __STDC__
#    define P(x)			x
#  else
#    define P(x)			()
#  endif

#  ifndef STDIN_FILENO
#    define STDIN_FILENO		(0)
#  endif
#  ifndef STDOUT_FILENO
#    define STDOUT_FILENO		(1)
#  endif
#  ifndef STDERR_FILENO
#    define STDERR_FILENO		(2)
#  endif

#  ifndef INADDR_NONE
#    define INADDR_NONE	(-1)
#  endif

#endif

