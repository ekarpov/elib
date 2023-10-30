/*
 * ELIBC debug helpers
 */

#include <stdlib.h>
#include <stdio.h>

#include "../elibc_config.h"
#include "etrace.h"
#include "eassert.h"

///// only in debug builds
#ifdef _DEBUG

void _elibc_assert(const char* file, int line, const char* msg)
{
    char buff[_EMAX_DBG_MSG];

    /* TODO: handle buffer overflow */

    /* format assertion message */
    if(msg)
        sprintf(buff, "Assertion failed in %s at line %d: %s", file, line, msg);
    else
        sprintf(buff, "Assertion failed in %s at line %d", file, line);

    /* trace message first */
    ETRACE(buff);

    /* stop */
    abort();
}

#endif // _DEBUG
