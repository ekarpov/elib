/*
 * ELIBC debug helpers
 */

///// only in debug builds
#ifdef _DEBUG

#include <windows.h>
#include <strsafe.h>

#include "../elibc_config.h"
#include "etrace.h"
#include "eassert.h"

void _elibc_assert(const char* file, int line, const char* msg)
{
    char buff[_EMAX_DBG_MSG];

    /* format assertion message */
    if(msg)
        StringCbPrintfA(buff, _EMAX_DBG_MSG, "Assertion failed in %s at line %d: %s", file, line, msg);
    else
        StringCbPrintfA(buff, _EMAX_DBG_MSG, "Assertion failed in %s at line %d", file, line);

    /* just trace message, macro will trigger breakpoint */
    ETRACE(buff);
}

#endif // _DEBUG
