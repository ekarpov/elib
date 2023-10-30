/*
    Binary search
*/

#ifndef _EBINSEARCH_H_
#define _EBINSEARCH_H_

/*----------------------------------------------------------------------*/

/*
    NOTE: search returns index in strings array or ELIBC_NOT_FOUND if string not found
*/

/* search strings */
int ebinsearch_strings(const char** strings, int string_count, 
                       const char* str, size_t str_len, ebool_t ignore_case);

/*----------------------------------------------------------------------*/

#endif /* _EBINSEARCH_H_ */

