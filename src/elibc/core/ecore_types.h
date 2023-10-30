/*
 *  Common types and helpers
 */

#ifndef _ECORE_TYPES_H_
#define _ECORE_TYPES_H_

/*----------------------------------------------------------------------*/
/* comparison functions */

/*
    Less function:
     - pointer to left item
     - pointer to right item
    Returns: ELIBC_TRUE if left item is less than right or ELIBC_FALSE otherwise
*/
typedef int (*eless_func_t)(const void*, const void*);

/*
    Compare function:
     - pointer to left item
     - pointer to right item
    Returns: negative value if left item is less than right, positive value if left 
             item is greater than right and zero if items are equal
*/
typedef int (*ecmp_func_t)(const void*, const void*);

/*----------------------------------------------------------------------*/
/* common comparison functions */

/* less */
int eless_int_func(const void* left_item, const void* right_item);
int eless_str_func(const void* left_item, const void* right_item);
int eless_strw_func(const void* left_item, const void* right_item);

/* compare */
int ecmp_int_func(const void* left_item, const void* right_item);
int ecmp_str_func(const void* left_item, const void* right_item);
int ecmp_strw_func(const void* left_item, const void* right_item);

/*----------------------------------------------------------------------*/

#endif /* _ECORE_TYPES_H_ */

