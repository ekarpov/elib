/*
 *  Standard library functions
*/

#ifndef _ESTDLIB_H_
#define _ESTDLIB_H_

/*----------------------------------------------------------------------*/

/* check if we are using standard library */
#ifndef _ELIBC_NO_STDLIB

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <wchar.h>

/*----------------------------------------------------------------------*/
/* standard library types */
/*----------------------------------------------------------------------*/

/* wide char */
typedef wchar_t     ewchar_t;

/* time */
typedef time_t      etime_t;
typedef struct tm   etm_t;

/*----------------------------------------------------------------------*/
/* standard library functions */
/*----------------------------------------------------------------------*/

/* memory functions */
#define emalloc     malloc
#define erealloc    realloc
#define ecalloc     calloc
#define efree       free
#define ememcpy     memcpy
#define ememmove    memmove
#define ememset     memset
#define ememcmp     memcmp

/* string functions */
#define estrcpy     strcpy
#define estrncpy    strncpy
#define ewcscpy     wcscpy
#define ewcsncpy    wcsncpy
#define estrcat     strcat
#define ewcscat     wcscat
#define estrlen     strlen
#define ewcslen     wcslen
#define estrcmp     strcmp
#define ewcscmp     wcscmp
#define estrncmp    strncmp
#define ewcsncmp    wcsncmp
#define estrstr     strstr
#define ewcsstr     wcsstr

/* string conversions */
#define eatoi       atoi
#define estrtol     strtol
#define ewcstol     wcstol

/* string format */
#define esprintf    sprintf
#define evsprintf   vsprintf
#define eswprintf   swprintf
#define evswprintf  vswprintf

/* character types */
#define etolower    tolower
#define etoupper    toupper
#define eisalpha    isalpha
#define eisalnum    isalnum
#define eiswalnum   iswalnum
#define eisdigit    isdigit
#define eiswdigit   iswdigit
#define eisspace    isspace
#define eiswspace   iswspace
#define eisascii    isascii
#define eiswascii   iswascii
#define eisprint    isprint
#define eiswprint   iswprint

/* sorting */
#define eqsort      qsort

/* time */
#define etime       time
#define emktime     mktime
#define egmtime     gmtime

/* random numbers */
#define esrand      srand
#define erand       rand

/* helpers */
#define emax        max
#define emin        min

/*----------------------------------------------------------------------*/
/* OS specific functions */
/*----------------------------------------------------------------------*/
#ifdef _ELIBC_OS_WINDOWS

/* case not sensitive string comparison */
#define estricmp        _stricmp
#define ewcsicmp        _wcsicmp
#define estrnicmp       _strnicmp
#define ewcsnicmp       _wcsnicmp

/* string functions */
#define estrdup         _strdup
#define ewcsdup         _wcsdup

/* UTC time conversion */
#define etimegm         _mkgmtime

/* string to large integer conversion */
#define estrtoull       _strtoui64 

#else

/* case not sensitive string comparison */
#define estricmp        strcasecmp
#define ewcsicmp        /* TODO: */
#define estrnicmp       strncasecmp
#define ewcsnicmp       /* TODO: */

/* string functions */
#define estrdup         strdup
#define ewcsdup         wcsdup

/* UTC time conversion */
#define etimegm         timegm

/* string to large integer conversion */
#define estrtoull       strtoull 

#endif /* _ELIBC_OS_WINDOWS */
/*----------------------------------------------------------------------*/

#else

/*----------------------------------------------------------------------*/
/* standard library functions */
/*----------------------------------------------------------------------*/

/*
    TODO:
*/

#endif /* _ELIBC_NO_STDLIB */

/*----------------------------------------------------------------------*/
/* Visual Studio prior to 2015 doesn't support snprintf family functions */
/*----------------------------------------------------------------------*/
#if defined(_MSC_VER) && _MSC_VER < 1900

#define esnprintf       _esnprintf
#define evsnprintf      _evsnprintf

int _evsnprintf(char* str_out, size_t size, const char* format, va_list ap);
int _esnprintf(char* str_out, size_t size, const char* format, ...);

#else

#define esnprintf       snprintf
#define evsnprintf      vsnprintf

#endif

/*----------------------------------------------------------------------*/
/* extra functions */

/* string comparison */
int estrncmp2(const char* left, size_t left_length, const char* right, size_t right_length);
int estrnicmp2(const char* left, size_t left_length, const char* right, size_t right_length);
int ewcsncmp2(const ewchar_t* left, size_t left_length, const ewchar_t* right, size_t right_length);

/* string conversion */
int eatoi2(const char* str_in, size_t str_length);
int ewatoi2(const ewchar_t* str_in, size_t str_length);

/*----------------------------------------------------------------------*/

#endif /* _ESTDLIB_H_ */



