/*
 *  Windows specific snprintf implementation (for VS version prior to 2015)
*/

#include "../elibc_config.h"
#include "../core/eassert.h"

#include <stdarg.h>

#include "estdlib.h"

/*
    NOTE: implementation adopted from http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
*/

#if defined(_MSC_VER) && _MSC_VER < 1900

int _evsnprintf(char* str_out, size_t size, const char* format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(str_out, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

int _esnprintf(char* str_out, size_t size, const char* format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = _evsnprintf(str_out, size, format, ap);
    va_end(ap);

    return count;
}

#endif

