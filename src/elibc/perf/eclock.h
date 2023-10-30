/*
    Time measurements tools
*/

#ifndef _ECLOCK_H_
#define _ECLOCK_H_

/*----------------------------------------------------------------------*/
/* time measurements */

#ifdef _ELIBC_ENABLE_PERFORMANCE_TOOLS

/* used processor time counters */
euint64_t  elibc_get_microseconds_used();   /* 1/1000000 sec */
euint64_t  elib_get_milliseconds_used();   /* 1/1000 sec */

/* total time counters */
euint64_t  elibc_get_microsecond_counter();   /* 1/1000000 sec */
euint64_t  elibc_get_millisecond_counter();   /* 1/1000 sec */

#endif /* _ELIBC_ENABLE_PERFORMANCE_TOOLS */

/*----------------------------------------------------------------------*/

#endif /* _ECLOCK_H_ */

