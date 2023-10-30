/*
    Time measurements tools for Windows
*/

#include "../elibc_config.h"

/* check if enabled */
#ifdef _ELIBC_ENABLE_PERFORMANCE_TOOLS

#include <windows.h>
#include "eclock.h"

/*----------------------------------------------------------------------*/
/* time counters */

ULONGLONG   elibc_get_thread_time_used()
{
    FILETIME fCreationTime, fExitTime, fKernelTime, fUserTime;

    /* get current thread time counter */
    if(GetThreadTimes(GetCurrentThread(), &fCreationTime, &fExitTime, &fKernelTime, &fUserTime))
    {
        ULARGE_INTEGER uiKernel, uiUser;

        // copy times
        uiKernel.LowPart = fKernelTime.dwLowDateTime;
        uiKernel.HighPart = fKernelTime.dwHighDateTime;
        uiUser.LowPart = fUserTime.dwLowDateTime;
        uiUser.HighPart = fUserTime.dwHighDateTime;

        return (uiKernel.QuadPart + uiUser.QuadPart);
    }

    return 0;
}

euint64_t elibc_get_microseconds_used()
{
    /* convert from 100 nanoseconds */
    return elibc_get_thread_time_used() / 10;
}

euint64_t elib_get_milliseconds_used()
{
    /* convert from 100 nanoseconds */
    return elibc_get_thread_time_used() / 10000;
}

/*----------------------------------------------------------------------*/
/* total time counters */
ULONGLONG elibc_get_system_counter_microsec()
{
    LARGE_INTEGER liCount, liFreq;

    if(QueryPerformanceCounter(&liCount) && QueryPerformanceFrequency(&liFreq))
    {
        return 1000000 * liCount.QuadPart / liFreq.QuadPart;
    }

    return 0;
}

euint64_t elibc_get_microsecond_counter()
{
    return elibc_get_system_counter_microsec();
}

euint64_t elibc_get_millisecond_counter()
{
    return elibc_get_system_counter_microsec() / 1000;
}

/*----------------------------------------------------------------------*/

#endif /* _ELIBC_ENABLE_PERFORMANCE_TOOLS */

