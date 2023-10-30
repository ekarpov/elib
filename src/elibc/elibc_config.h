/*
 *  ELIBC configuration
 */

#ifndef _ELIBC_CONFIG_H_
#define _ELIBC_CONFIG_H_

/*----------------------------------------------------------------------*/
/* integer types */
#if defined(_MSC_VER)

typedef     __int8              eint8_t;
typedef     unsigned __int8     euint8_t;
typedef     __int16             eint16_t;
typedef     unsigned __int16    euint16_t;
typedef     __int32             eint32_t;
typedef     unsigned __int32    euint32_t;
typedef     __int64             eint64_t;
typedef     unsigned __int64    euint64_t;

/* compiler specific eint64_t and euint64_t printf format modifier */
#define     _EPRI64_FORMAT_MODIFIER_        "I64"

#else

#include <stdint.h>

typedef     int8_t              eint8_t;
typedef     uint8_t             euint8_t;
typedef     int16_t             eint16_t;
typedef     uint16_t            euint16_t;
typedef     int32_t             eint32_t;
typedef     uint32_t            euint32_t;
typedef     long long           eint64_t;
typedef     unsigned long long  euint64_t;

/* compiler specific eint64_t and euint64_t printf format modifier */
#define     _EPRI64_FORMAT_MODIFIER_        "ll"

#endif

/*----------------------------------------------------------------------*/
/* format modifiers to print 64 bits integers */

/* NOTE: use space before EPRId64 in format string */
#define EPRId64         _EPRI64_FORMAT_MODIFIER_ "d"
#define EPRIi64         _EPRI64_FORMAT_MODIFIER_ "i"
#define EPRIo64         _EPRI64_FORMAT_MODIFIER_ "o"
#define EPRIu64         _EPRI64_FORMAT_MODIFIER_ "u"
#define EPRIx64         _EPRI64_FORMAT_MODIFIER_ "x"
#define EPRIX64         _EPRI64_FORMAT_MODIFIER_ "X"

/*----------------------------------------------------------------------*/
/* boolean*/
typedef enum
{
    ELIBC_FALSE     = 0,
    ELIBC_TRUE      = 1

} ebool_t;

/*----------------------------------------------------------------------*/
/* platform options */
#if defined(WIN32) || defined(_WIN64)

/* OS type */
#ifndef _ELIBC_OS_WINDOWS
#define _ELIBC_OS_WINDOWS
#endif /* _ELIBC_OS_WINDOWS */ 

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif /* _CRT_SECURE_NO_WARNINGS */

#ifdef _WIN64
	#define _ELIBC_64_BITS_SUPPORT
#endif /* _WIN64 */

/* file handle */
typedef void*                   EFILE;

#else

/* file handle */
typedef int                     EFILE;

#endif /* platform */

/*----------------------------------------------------------------------*/
/* Standard library */
/* define this when standard library is not available */
/* #define _ELIBC_NO_STDLIB */

/*----------------------------------------------------------------------*/
/* endianness */

#if !defined(ELIBC_LITTLE_ENDIAN) && !defined(ELIBC_BIG_ENDIAN)

/* little endian by default */
#define ELIBC_LITTLE_ENDIAN
/*#define ELIBC_BIG_ENDIAN */

#endif 

/*----------------------------------------------------------------------*/
/* file size */
typedef     euint64_t           efilesize_t;

/*----------------------------------------------------------------------*/
/* compiler options */
#if defined(WIN32) && defined(_MSC_VER)
#define ELIBC_FORCE_INLINE  __forceinline
#elif defined(__GNUC__)
#define ELIBC_FORCE_INLINE  inline __attribute__((always_inline))
#else
#define ELIBC_FORCE_INLINE
#endif

/*----------------------------------------------------------------------*/
/* language options */

/*----------------------------------------------------------------------*/
/* debug defines */
#if defined(_DEBUG) && !defined(NDEBUG)

    /* debug mode */
    #define _ELIBC_DEBUG
    #define _ELIBC_ENABLE_TRACES

#ifdef WIN32
    /* performance tools only supported in Windows */
    #define _ELIBC_ENABLE_PERFORMANCE_TOOLS
#endif

#endif /* defined(_DEBUG) && !defined(NDEBUG) */

/*----------------------------------------------------------------------*/
/* buffer sizes */
#define _EMAX_DBG_MSG               512

/*----------------------------------------------------------------------*/
/* helper macro */
#define EUNUSED(variable)           (void)variable

/*----------------------------------------------------------------------*/
/* convenience macro */
#define ELIBC_NOT_FOUND             -1

/*----------------------------------------------------------------------*/

#endif /* _ELIBC_CONFIG_H_ */

