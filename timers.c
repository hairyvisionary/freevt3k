/*
 * @(#) timers.c: 94/04/12-07:17:58
 */

int32 MyGettimeofday(void)
{ /*MyGettimeofday*/


    static int
	baseSec = 0,
	baseSet = 0;
    int
	sec,
	ms;
#ifdef VMS
#  include <timeb.h>

    struct timeb
	time_pointer;
    
    ftime(&time_pointer);
    sec = time_pointer.time;
    ms = time_pointer.millitm;
#else
    struct timeval
	tp;
#  ifdef SHORT_GETTIMEOFDAY
    (void)gettimeofday(&tp);
#  else
    struct timezone
	tzp;

    (void)gettimeofday(&tp, &tzp);
#  endif
    sec = tp.tv_sec;
    ms = tp.tv_usec/1000;
#endif
    if (!baseSet)
	{
	baseSet = 1;
	baseSec = sec;
	sec = 0;
	}
    else
	sec = sec - baseSec;
    return((int32)((sec*1000)+ms));

} /*MyGettimeofday*/

int32 ElapsedTime(int32 start_time)
{ /*ElapsedTime*/

    return(MyGettimeofday() - start_time);

} /*ElapsedTime*/
#ifdef INCLUDE_WALLTIME

void WallTime(void)
{ /*WallTime*/

    time_t
	ltime;
    struct tm
	*tmNow;
    int
	ms;
    extern FILE
	*debug_fd;

    time(&ltime);
    tmNow = localtime(&ltime);
    ms = (int)(MyGettimeofday() % 1000);
    fprintf(debug_fd, "%02d:%02d:%02d.%03d: ",
	    tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec, ms);
    
} /*WallTime*/
#endif /*INCLUDE_WALLTIME*/
