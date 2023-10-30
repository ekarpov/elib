/*
    Binary search
*/

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "eassert.h"
#include "ebinsearch.h"

/*----------------------------------------------------------------------*/

/* search strings */
int ebinsearch_strings(const char** strings, int string_count, 
                       const char* str, size_t str_len, ebool_t ignore_case)
{
    int left, right, mid, result;

    /* validate input */
    EASSERT(strings);
    EASSERT(string_count);
    if(strings == 0 || string_count == 0) return ELIBC_NOT_FOUND;

    /* return not found if string is empty */
    if(str == 0 || str_len == 0) return ELIBC_NOT_FOUND;

    left = 0;
    right = string_count;

    /* binary search */   
    while(left <= right && left < string_count)
    {
        /* middle point */ 
        mid = left + (right - left) / 2;

        /* compare strings */
        if(ignore_case)
            result = estrnicmp2(str, str_len, strings[mid] , 0);
        else
            result = estrncmp2(str, str_len, strings[mid], 0);

        /* check result */
        if(result == 0)
            return mid;
        else if(result < 0)
            right = mid - 1;
        else
            left = mid + 1;
    }

    return ELIBC_NOT_FOUND;
}

/*----------------------------------------------------------------------*/
