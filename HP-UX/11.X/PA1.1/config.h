/* config.h.  Generated automatically by configure.  */
#ifndef _CONFIG_H_INCLUDED
/* Generated automatically from configure.in by autoheader.  DO NOT EDIT!  */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #  undef pid_t */
/* #  undef uid_t */

/* Define as the return type of signal handlers (int or void).  */
#  define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
#  ifndef __mpexl
/* #  undef size_t */
#  endif

/* Define if you have unistd.h.  */
#  define HAVE_UNISTD_H 1

/* Define if int is 16 bits instead of 32.  */
/* #  undef INT_16_BITS */

/* Define if you have memory.h, and string.h doesn't declare the
   mem* functions.  */
/* #  undef NEED_MEMORY_H */

/* Define if you have the ANSI C header files.  */
#  define STDC_HEADERS 1

/* Define if you do not have strings.h, index, bzero, etc..  */
/* #  undef USG */

/* Define if your processor stores words with the most significant
   byte first (like Motorola and Sparc, unlike Intel and VAX).  */
#  define WORDS_BIGENDIAN 1

/* If a program may include both `time.h' and `sys/time.h', define
   `TIME_WITH_SYS_TIME'.  On some older systems `sys/time.h' includes
   `time.h', but `time.h' is not protected against multiple
   inclusion, so programs should not explicitly include both files.
   This macro is useful in programs that use for example `struct
   timeval' or `struct timezone' as well as `struct tm'.  It is best
   used in conjunction with HAVE_SYS_TIME_H.  */
#  define TIME_WITH_SYS_TIME 1

/* Define if `struct timeval' is defined in `time.h' or `sys/time.h'  */
#  define HAVE_TIMEVAL 1

/* Define if `struct flock' is defined in `fcntl.h'  */
#  define HAVE_FLOCK 1

/* Define if 'struct sigaction' is defined in 'signal.h' */
#  define HAVE_SIGACTION 1

/* Define if you have gettimeofday.  */
#  define HAVE_GETTIMEOFDAY 1

/* Define if you have rename.  */
#  define HAVE_RENAME 1

/* Define if you have memmove.  */
#  define HAVE_MEMMOVE 1

/* Define if you have strftime.  */
#  define HAVE_STRFTIME 1

/* Define if you have strerror.  */
#  define HAVE_STRERROR 1

/* Define if you have strdup.  */
#  define HAVE_STRDUP 1

/* Define if you have setsockopt.  */
#  define HAVE_SETSOCKOPT 1

/* Define if you have getsockopt.  */
#  define HAVE_GETSOCKOPT 1

/* Define if you have strcasecmp.  */
#  define HAVE_STRCASECMP 1

/* Define if you have uname.  */
#  define HAVE_UNAME 1

/* Define if you have tcdrain.  */
#  define HAVE_TCDRAIN 1

/* Define if you have fsync.  */
#  define HAVE_FSYNC 1

/* Define if you have ftruncate.  */
#  define HAVE_FTRUNCATE 1

/* Define if you have tgetent.  */
#  define HAVE_TGETENT 1

/* Define if you have ttyslot.  */
#  define HAVE_TTYSLOT 1

/* Define if you have getutent.  */
#  define HAVE_GETUTENT 1

/* Define if you have syslog.  */
#  define HAVE_SYSLOG 1

/* Define if you have waitpid.  */
#  define HAVE_WAITPID 1

/* Define if you have tempnam.  */
#  define HAVE_TEMPNAM 1

/* Define if utmp.h defines ut_host.  */
#  define HAVE_UT_HOST 1

/* Define if utmpx.h defines ut_host.  */
#  define HAVE_UT_HOSTX 1

/* Define if sys/un.h defines sun_len.  */
/* #  undef HAVE_SUN_LEN */

/* Define if you have the <varargs.h> header file.  */
#  define HAVE_VARARGS_H 1

/* Define if you have the <stdarg.h> header file.  */
#  define HAVE_STDARG_H 1

/* Define if you have the <sys/timeb.h> header file.  */
#  define HAVE_SYS_TIMEB_H 1

/* Define if you have the <sys/time.h> header file.  */
#  define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/times.h> header file.  */
#  define HAVE_SYS_TIMES_H 1

/* Define if you have the <string.h> header file.  */
#  define HAVE_STRING_H 1

/* Define if you have the <sys/select.h> header file.  */
/* #  undef HAVE_SYS_SELECT_H */

/* Define if you have the <termios.h> header file.  */
#  define HAVE_TERMIOS_H 1

/* Define if you have the <sys/termiox.h> header file.  */
#  define HAVE_SYS_TERMIOX_H 1

/* Define if you have the <sys/modem.h> header file.  */
#  define HAVE_SYS_MODEM_H 1

/* Define if you have the <sys/socket.h> header file.  */
#  define HAVE_SYS_SOCKET_H 1

/* Define if you have the <sys/sockio.h> header file.  */
/* #  undef HAVE_SYS_SOCKIO_H */

/* Define if you have the <stdlib.h> header file.  */
#  define HAVE_STDLIB_H 1

/* Define if you have the <fcntl.h> header file.  */
#  define HAVE_FCNTL_H 1

/* Define if you have the <netinet/in.h> header file.  */
#  define HAVE_NETINET_IN_H 1

/* Define if you have the <arpa/inet.h> header file.  */
#  define HAVE_ARPA_INET_H 1

/* Define if you have the <netdb.h> header file.  */
#  define HAVE_NETDB_H 1

/* Define if you have the <sys/un.h> header file.  */
#  define HAVE_SYS_UN_H 1

/* Define if you have the <sys/wait.h> header file.  */
#  define HAVE_SYS_WAIT_H 1

/* Define if you have the <sys/mman.h> header file.  */
#  define HAVE_SYS_MMAN_H 1

/* Define if system calls automatically restart after interruption
   by a signal.  */
/* #  undef HAVE_RESTARTABLE_SYSCALLS */

/* Define if -lcurses is usable */
#  define HAVE_LIBCURSES 1

/* Define if -ltermcap is usable */
#  define HAVE_LIBTERMCAP 1

/* Define to indicate that this file has been seen. */
#  define _CONFIG_H_INCLUDED	(1)
#endif /* _CONFIG_H_INCLUDED */
