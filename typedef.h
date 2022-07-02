/************************************************************
 * Compiler-dependent typedefs
 *
 * typedef.h
 ************************************************************/

#ifndef _TYPEDEF_H
#define _TYPEDEF_H

/* C99 standard integer types */
#include <stdint.h>
/* type int16_t */
/* type uint16_t */
/* type int32_t */
/* type uint32_t */
/* type uint8_t */

/* C99 standard boolean type */
#include <stdbool.h>
/* type bool */
/* value false */
/* value true */

typedef short int16;
typedef unsigned short unsigned16;
typedef long int32;
typedef unsigned long unsigned32;
typedef unsigned char unsigned8;
typedef char tBoolean;

#  ifdef VMS
#    include "vmstypes.h"
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

