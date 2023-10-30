/*
    Memory usage tools
*/

#ifndef _EMEMUSE_H_
#define _EMEMUSE_H_

/*----------------------------------------------------------------------*/
/* memory usage */

#ifdef _ELIBC_ENABLE_PERFORMANCE_TOOLS

    /* system heap */
    size_t   elibc_get_memory_usage();

    /* standard library heap */
    size_t   elibc_get_stdlib_memory_usage();

    /* check if there is some memory left */
    int     elibc_check_memory_leaks();

#endif /* _ELIBC_ENABLE_PERFORMANCE_TOOLS */

/*----------------------------------------------------------------------*/

#endif /* _EMEMUSE_H_ */

