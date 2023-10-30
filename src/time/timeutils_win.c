/*
    Time utilities
*/

#include <windows.h>

#include "../elib_config.h"

#include "timeutils.h"

/*----------------------------------------------------------------------*/
/* timestamp helpers */

eint64_t timestamp_current()
{
    SYSTEMTIME st;
    FILETIME ft;

    /* get current time */
    GetSystemTime(&st);

    /* convert first to file time */
    if(!SystemTimeToFileTime(&st, &ft))
    {
        ETRACE_WERR_LAST("timestamp_current: failed to convert system time to file time");
        return 0;
    }

    /* convert to int64 */
    return (eint64_t)(((ULONGLONG) ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
}

eint64_t timestamp_append_sec(eint64_t timestamp, double seconds)
{
    /* append seconds */
    return timestamp + (ULONGLONG)(seconds * ((ULONGLONG) 10000000));
}

/* system independent timestamp in milliseconds */
eint64_t timestamp_current_ms()
{
    return timestamp_current() / 10000;
}

/*----------------------------------------------------------------------*/
