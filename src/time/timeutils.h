/*
    Time utilities
*/

#ifndef _TIME_UTILS_H_
#define _TIME_UTILS_H_

/*----------------------------------------------------------------------*/
/* timestamp helpers */

/*
    NOTE: timestamp value is system dependent
            - on Windows it is FILETIME (100-nanoseconds intervals since 1601)
*/

eint64_t timestamp_current();
eint64_t timestamp_append_sec(eint64_t timestamp, double seconds);

/* system independent timestamp in milliseconds */
eint64_t timestamp_current_ms();

/*----------------------------------------------------------------------*/

#endif /* _TIME_UTILS_H_*/

