/*
 * ELIBC debug helpers
 */

#ifndef _ETRACE_H_
#define _ETRACE_H_

/* tracing */
#ifdef _ELIBC_ENABLE_TRACES

    void _elibc_trace(const char* _file, int _line, const char* fmt, ...);

    /* trace callback */
    typedef void (*_elibc_trace_callback_t)(const char* msg, void* callback_param);

    /* trace options */
    void    elibc_init_trace_callback(_elibc_trace_callback_t callback_ptr, void* callback_param);

#ifdef _ELIBC_OS_WINDOWS

    #define ETRACE(fmt, ...)                do { _elibc_trace(__FILE__, __LINE__, fmt, __VA_ARGS__); } while (0)

    /* trace windows errors */
    #define ETRACE_WERR(msg, err)           _elibc_trace_werr(__FILE__, __LINE__, msg, err)
    #define ETRACE_WERR_LAST(msg)           _elibc_trace_werr_last(__FILE__, __LINE__, msg)

    void _elibc_trace_werr(const char* _file, int _line, const char* text, unsigned long err);
    void _elibc_trace_werr_last(const char* _file, int _line, const char* text);

    /* trace HRESULT */
    #define ETRACE_HRES(msg, res)           _elibc_trace_hres(__FILE__, __LINE__, msg, res)

    void _elibc_trace_hres(const char* _file, int _line, const char* text, unsigned long res);

#else

    #define ETRACE(fmt, ...)                do { _elibc_trace(__FILE__, __LINE__, fmt, ##__VA_ARGS__); } while (0)

    /* trace errno */
    #define ETRACE_ERRNO(msg)               _elibc_trace_errno(__FILE__, __LINE__, msg)
    void _elibc_trace_errno(const char* _file, int _line, const char* text);

#endif /* WIN32 */

#else

    #define ETRACE(fmt, ...)            do { } while (0)
    #define ETRACE_WERR(msg, err)
    #define ETRACE_WERR_LAST(msg)
    #define ETRACE_HRES(msg, res)
    #define ETRACE_ERRNO(msg)
    #define elibc_init_trace_callback(callback_ptr, callback_param)

#endif /* _ELIBC_ENABLE_TRACES */

#endif /* _ETRACE_H_ */

    
