/*
 * ELIBC debug helpers
 */

#ifndef _ETRACE_H_
#define _ETRACE_H_

/* tracing */
#ifdef _ELIBC_ENABLE_TRACES

    #define ETRACE(msg)                     _elibc_trace(msg)
    #define ETRACE1(msg, arg)               _elibc_trace(msg, arg)
    #define ETRACE2(msg, arg1, arg2)        _elibc_trace(msg, arg1, arg2)
    #define ETRACE3(msg, arg1, arg2, arg3)  _elibc_trace(msg, arg1, arg2, arg3)

    #define ETRACE_SBUFF(msg, str, slen)    _elibc_trace_sbuff(msg, str, slen)

    void _elibc_trace(const char* format_str, ...);
    void _elibc_trace_sbuff(const char* msg, const char* str, size_t slen);

    /* trace callback */
    typedef void (*_elibc_trace_callback_t)(const char* msg, void* callback_param);

    /* trace options */
    void    elibc_init_trace_callback(_elibc_trace_callback_t callback_ptr, void* callback_param);

#ifdef WIN32

    /* trace windows errors */
    #define ETRACE_WERR(msg, err)           _elibc_trace_werr(msg, err)
    #define ETRACE_WERR_LAST(msg)           _elibc_trace_werr_last(msg)

    void _elibc_trace_werr(const char* text, unsigned long err);
    void _elibc_trace_werr_last(const char* text);

    /* trace HRESULT */
    #define ETRACE_HRES(msg, res)           _elibc_trace_hres(msg, res)

    void _elibc_trace_hres(const char* text, unsigned long res);

#else

    /* trace errno */
    #define ETRACE_ERRNO(msg)               _elibc_trace_errno(msg)
    void _elibc_trace_errno(const char* text);

#endif /* WIN32 */

#else

    #define ETRACE(msg)
    #define ETRACE1(msg, arg)
    #define ETRACE2(msg, arg1, arg2)
    #define ETRACE3(msg, arg1, arg2, arg3)
    #define ETRACE_SBUFF(msg, str, slen)
    #define ETRACE_WERR(msg, err)
    #define ETRACE_WERR_LAST(msg)
    #define ETRACE_HRES(msg, res)
    #define ETRACE_ERRNO(msg)
    #define elibc_init_trace_callback(callback_ptr, callback_param)

#endif /* _ELIBC_ENABLE_TRACES */

#endif /* _ETRACE_H_ */

    
