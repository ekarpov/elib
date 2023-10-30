/*
 *  Common types and helpers
 */

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "eassert.h"
#include "ecore_types.h"

/*----------------------------------------------------------------------*/
/* less */
/*----------------------------------------------------------------------*/
int eless_int_func(const void* left_item, const void* right_item)
{
    EASSERT(left_item);
    EASSERT(right_item);

    /* compare */
    return (*(int*)left_item) < (*(int*)right_item) ? ELIBC_TRUE : ELIBC_FALSE;
}

int eless_str_func(const void* left_item, const void* right_item)
{
    EASSERT(left_item);
    EASSERT(right_item);

    /* compare */
    return (estrcmp(*(char**)left_item, *(char**)right_item) < 0) ? ELIBC_TRUE : ELIBC_FALSE;
}

int eless_strw_func(const void* left_item, const void* right_item)
{
    EASSERT(left_item);
    EASSERT(right_item);

    /* compare */
    return (ewcscmp(*(ewchar_t**)left_item, *(ewchar_t**)right_item) < 0) ? ELIBC_TRUE : ELIBC_FALSE;
}

/*----------------------------------------------------------------------*/
/* compare */
/*----------------------------------------------------------------------*/
int ecmp_int_func(const void* left_item, const void* right_item)
{
    EASSERT(left_item);
    EASSERT(right_item);

    /* compare */
    return (*(int*)left_item) - (*(int*)right_item);
}

int ecmp_str_func(const void* left_item, const void* right_item)
{
    EASSERT(left_item);
    EASSERT(right_item);

    /* compare */
    return estrcmp(*(char**)left_item, *(char**)right_item);
}

int ecmp_strw_func(const void* left_item, const void* right_item)
{
    EASSERT(left_item);
    EASSERT(right_item);

    /* compare */
    return ewcscmp(*(ewchar_t**)left_item, *(ewchar_t**)right_item);
}

/*----------------------------------------------------------------------*/
