/*
 * ELIBC debug helpers
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "../elibc_config.h"
#include "etrace.h"

///// only if traceces defined
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
    vsprintf(buff, format_str, args);

    va_end(args);

    /* output to callback if set */
    if(_elibc_trace_callback_ptr)
    {
        _elibc_trace_callback_ptr(buff, _elibc_trace_callback_param);

    } else
    {
        /* if not, output to stderr */
        fprintf(stderr, "[ELIBC] %s\n", buff);
    }
}

/* trace string buffer */
void _elibc_trace_sbuff(const char* msg, const char* str, size_t slen)
{
    size_t idx;
    static char strBuf[_EMAX_DBG_MSG];

    /* text length */
    size_t text_len = slen < sizeof(strBuf) ? slen : sizeof(strBuf);
    
    /* copy string */
    for(idx = 0; idx < text_len; ++idx)
    {
        strBuf[idx] = str[idx];
    }
    strBuf[text_len] = 0;

    /* output */
    _elibc_trace(msg, strBuf);
}

const char* _elibc_errno_to_text(unsigned long err_code)
{
    /*
     TODO: http://www.virtsync.com/c-error-codes-include-errno
    */

    switch(err_code)
    {
    case EPERM:     return "Operation not permitted";
    case ENOENT:    return "No such file or directory";
    case ESRCH:     return "No such process";
    case EINTR:     return "Interrupted system call";
    case EIO:       return "I/O error";
    case ENXIO:     return "No such device or address";
    case E2BIG:     return "Argument list too long";
    case ENOEXEC:   return "Exec format error";
    case EBADF:     return "Bad file number";
    case ECHILD:    return "No child processes";
    case EAGAIN:    return "Try again";
    case ENOMEM:    return "Out of memory";
    case EACCES:    return "Permission denied";
    case EFAULT:    return "Bad address";
    case ENOTBLK:   return "Block device required";
    case EBUSY:     return "Device or resource busy";
    case EEXIST:    return "File exists";
    case EXDEV:     return "Cross-device link";
    case ENODEV:    return "No such device";
    case ENOTDIR:   return "Not a directory";
    case EISDIR:    return "Is a directory";
    case EINVAL:    return "Invalid argument";
    case ENFILE:    return "File table overflow";
    case EMFILE:    return "Too many open files";
    case ENOTTY:    return "Not a typewriter";
    case ETXTBSY:   return "Text file busy";
    case EFBIG:     return "File too large";
    case ENOSPC:    return "No space left on device";
    case ESPIPE:    return "Illegal seek";
    case EROFS:     return "Read-only file system";
    case EMLINK:    return "Too many links";
    case EPIPE:     return "Broken pipe";
    case EDOM:      return "Math argument out of domain of func";
    case ERANGE:    return "Math result not representable";
    }

    return "Unknown error code";
}

void _elibc_trace_errno(const char* text)
{
    /* trace error */
    _elibc_trace("%s: (errno: %d - \"%s\")", text, errno, _elibc_errno_to_text(errno));
}

#endif // _ELIBC_ENABLE_TRACES

