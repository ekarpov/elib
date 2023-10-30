/*
 * ELIBC debug helpers
 */

#ifndef _EASSERT_H_
#define _EASSERT_H_

#ifdef _DEBUG

    void _elibc_assert(const char* file, int line, const char* msg);

#if defined(WIN32) && defined(_MSC_VER)
    /* use debug break intrinsic under visual C compiler */
    #define EASSERT(expr)        { if (!(expr)) {_elibc_assert(__FILE__, __LINE__, 0); __debugbreak() ; } }
    #define EASSERT1(expr,msg)   { if (!(expr)) {_elibc_assert(__FILE__, __LINE__, (msg)); __debugbreak() ; } }
#else
    #define EASSERT(expr)        { if (!(expr)) _elibc_assert(__FILE__, __LINE__, 0); }
    #define EASSERT1(expr,msg)   { if (!(expr)) _elibc_assert(__FILE__, __LINE__, (msg)); }
#endif

#else

    #define EASSERT(expr)
    #define EASSERT1(expr,msg)

#endif /* _DEBUG */

#endif /* _EASSERT_H_ */



