/*
 *  Fixed size string comparison
*/


#include "../elibc_config.h"
#include "../core/eassert.h"

#include "estdlib.h"

/*
    NOTE: standard strncmp function doesn't work correctly with substrings
*/

int estrncmp2(const char* left, size_t left_length, const char* right, size_t right_length)
{
    size_t idx;

    /* check input */
    EASSERT(left);
    EASSERT(right);

    /* if length not set assume zero terminated */
    if(left_length == 0) left_length = (size_t)-1;
    if(right_length == 0) right_length = (size_t)-1;

    /* match strings */
    for(idx = 0; idx < left_length && idx < right_length; ++idx)
    {
        /* match characters */
        if(left[idx] != right[idx] || left[idx] == 0 || right[idx] == 0) 
        {
            /* result */
            return (unsigned char)left[idx] - (unsigned char)right[idx];
        }
    }

    /* make sure that we do not match substring */
    if(idx == left_length && idx < right_length)
    {
        /* left string ended before right */
        if(right[idx] != 0) return -1;

    } else if(idx == right_length && idx < left_length)
    {
        /* right string ended before left */
        if(left[idx] != 0) return 1;
    }

    /* strings are equal */
    return 0;
}

int estrnicmp2(const char* left, size_t left_length, const char* right, size_t right_length)
{
    size_t idx;

    /* check input */
    EASSERT(left);
    EASSERT(right);

    /* if length not set assume zero terminated */
    if(left_length == 0) left_length = (size_t)-1;
    if(right_length == 0) right_length = (size_t)-1;

    /* match strings */
    for(idx = 0; idx < left_length && idx < right_length; ++idx)
    {
        /* match characters (ignore case) */
        if(etolower((unsigned char)left[idx]) != etolower((unsigned char)right[idx]) || left[idx] == 0 || right[idx] == 0) 
        {
            /* result (ignore case) */
            return etolower((unsigned char)left[idx]) - etolower((unsigned char)right[idx]);
        }
    }

    /* make sure that we do not match substring */
    if(idx == left_length && idx < right_length)
    {
        /* left string ended before right */
        if(right[idx] != 0) return -1;

    } else if(idx == right_length && idx < left_length)
    {
        /* right string ended before left */
        if(left[idx] != 0) return 1;
    }

    /* strings are equal */
    return 0;
}

int ewcsncmp2(const ewchar_t* left, size_t left_length, const ewchar_t* right, size_t right_length)
{
    size_t idx;

    /* check input */
    EASSERT(left);
    EASSERT(right);

    /* if length not set assume zero terminated */
    if(left_length == 0) left_length = (size_t)-1;
    if(right_length == 0) right_length = (size_t)-1;

    /* match strings */
    for(idx = 0; idx < left_length && idx < right_length; ++idx)
    {
        /* match characters */
        if(left[idx] != right[idx] || left[idx] == 0 || right[idx] == 0) 
        {
            /* result */
            return (unsigned int)left[idx] - (unsigned int)right[idx];
        }
    }

    /* make sure that we do not match substring */
    if(idx == left_length && idx < right_length)
    {
        /* left string ended before right */
        if(right[idx] != 0) return -1;

    } else if(idx == right_length && idx < left_length)
    {
        /* right string ended before left */
        if(left[idx] != 0) return 1;
    }

    /* strings are equal */
    return 0;
}
