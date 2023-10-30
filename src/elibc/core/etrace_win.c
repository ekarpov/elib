/*
 * ELIBC debug helpers
 */

#include <windows.h>
#include <stdio.h>
#include <strsafe.h>

#include "../elibc_config.h"
#include "etrace.h"

///// only if traces are defined
#ifdef _ELIBC_ENABLE_TRACES

// callback settings if any
static _elibc_trace_callback_t  _elibc_trace_callback_ptr = 0;
static void*                    _elibc_trace_callback_param = 0;

// trace options
void    elibc_init_trace_callback(_elibc_trace_callback_t callback_ptr, void* callback_param)
{
    // set pointers
    _elibc_trace_callback_ptr = callback_ptr;
    _elibc_trace_callback_param = callback_param;
}

// tracing
void _elibc_trace(const char* format_str, ...)
{
    char buff[_EMAX_DBG_MSG];

    va_list args;
    va_start(args, format_str);

    /* format message */
    StringCbVPrintfA(buff, _EMAX_DBG_MSG, format_str, args);

    va_end(args); 

    /* output to debugger  */
    OutputDebugStringA(buff);
    OutputDebugStringA("\n");

    /* output also to callback if set */
    if(_elibc_trace_callback_ptr)
    {
        _elibc_trace_callback_ptr(buff, _elibc_trace_callback_param);

    } else if(GetConsoleWindow())
    {
        /* if no callback, output to console */
        fprintf(stderr, "%s\n", buff);
    }
}

// trace string buffer
void _elibc_trace_sbuff(const char* msg, const char* str, size_t slen)
{
    size_t idx;
    static char strBuf[_EMAX_DBG_MSG];

    /* text length */
    size_t text_len = slen < sizeof(strBuf) ? slen : sizeof(strBuf);
    
    /* zero length if string is null */
    if(str == 0) text_len = 0;

    /* copy string */
    for(idx = 0; idx < text_len; ++idx)
    {
        strBuf[idx] = str[idx];
    }
    strBuf[text_len] = 0;

    /* output */
    _elibc_trace(msg, strBuf);
}

// text error for windows error code
const char* _elibc_error_text(unsigned long err_code)
{
    static char lpMsgBuf[_EMAX_DBG_MSG];

    DWORD retVal = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_MAX_WIDTH_MASK, // ignore line breaks
        NULL,
        err_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&lpMsgBuf,
        sizeof(lpMsgBuf), NULL );

    /* check size */
    if(retVal >= sizeof(lpMsgBuf)) retVal = sizeof(lpMsgBuf) - 1;

    /* make string */
    lpMsgBuf[retVal] = 0;

    return lpMsgBuf;
}

void _elibc_trace_werr(const char* text, unsigned long err)
{
    // trace error
    _elibc_trace("%s: (Windows error: %d - \"%s\")", text, err, _elibc_error_text(err));
}

void _elibc_trace_werr_last(const char* text)
{
    // last error 
    _elibc_trace_werr(text, GetLastError());
}

void _elibc_trace_hres(const char* text, unsigned long res)
{
    // trace error
    _elibc_trace("%s: (HRESULT: %X - \"%s\")", text, res, _elibc_error_text(res));
}

#endif // _ELIBC_ENABLE_TRACES

