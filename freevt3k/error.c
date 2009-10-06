/*
 * @(#) error.c: 95/03/06-08:39:31
 */

#ifndef VMS
#  include "config.h"
#  include <sys/types.h>
#  include <unistd.h>
#  include <errno.h>
#  include <stdio.h>
#  include <string.h>
#  if defined(WINNT)
#    define HAVE_STRERROR	(1)
#pragma warning (disable: 4706 4100 4101)
#  endif
#else
#  include types
#  include unixio
#  include descrip
#  include errno
#  include starlet
#  include stddef
#  include stdio
#  include string
#  include stsdef
#  include "vmstypes.h"
#endif

#ifdef VMS
int VMSerror(int vms_stat)

{ /*VMSerror*/

    if (vms_stat & STS$M_SUCCESS)
	return(0);
    errno = EVMSERR;
    vaxc$errno = vms_stat;
    return(1);

} /*VMSerror*/

char *VMSstrerror(int io_status)

{ /*VMSstrerror*/

    struct dsc$descriptor_s
	msgDesc;
    short
	len;
    static char
	buf[257];

    msgDesc.dsc$w_length  = sizeof buf;
    msgDesc.dsc$b_dtype   = DSC$K_DTYPE_T;
    msgDesc.dsc$b_class   = DSC$K_CLASS_S;
    msgDesc.dsc$a_pointer = buf;
    sys$getmsg(io_status, &len, &msgDesc, 0x03, 0);
    buf[len] = '\0';
    return(buf);

} /*VMSstrerror*/
#endif /* VMS */

#ifdef __STDC__
int PortableErrno(int err)
#else
int PortableErrno(err)

    int
	err;
#endif

{ /*PortableErrno*/

#ifdef VMS
    return((err == EVMSERR) ? vaxc$errno : err);
#else
    return(err);
#endif

} /*PortableErrno*/

#ifdef __STDC__
char *PortableStrerror(int err)
#else
char *PortableStrerror(err)

    int
	err;
#endif

{ /*PortableStrerror*/

#if !defined(VMS) && !defined(HAVE_STRERROR)
    extern char
	*sys_errlist[];
    extern int
	sys_nerr;
    static char
	errorMsg[80];
#endif

#ifdef VMS
    if (err == EVMSERR)
	return(VMSstrerror(vaxc$errno));
    return(strerror(err));
#else
#  ifdef HAVE_STRERROR
    return(strerror(err));
#  else
    if ((err > 0) && (err < sys_nerr))
	return(sys_errlist[err]);
    sprintf(errorMsg, "Unknown errno = %d", err);
    return(errorMsg);
#  endif
#endif

} /*PortableStrerror*/

#ifdef __STDC__
void PortablePerror(char *text)
#else
void PortablePerror(text)

    char
	*text;
#endif

{ /*PortablePerror*/

    fprintf(stderr, "%s: %s\n", text, PortableStrerror(errno));

} /*PortablePerror*/
