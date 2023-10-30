/*
    Array helpers
*/

#ifndef _EARRAY_H_
#define _EARRAY_H_

/*----------------------------------------------------------------------*/

/* array type */
typedef struct
{
    char*       items;              /* data buffer */
    size_t      alloc_size;         /* allocated buffer size in bytes */
    size_t      item_size;          /* single item size */
    size_t      item_count;         /* number of items */

} earray_t;

/*----------------------------------------------------------------------*/

/* init and close */
void    earray_init(earray_t* earr, size_t item_size);
void    earray_reset(earray_t* earr);
void    earray_free(earray_t* earr);

/* array manipulations */
int     earray_reserve(earray_t* earr, size_t item_count);
int     earray_resize(earray_t* earr, size_t item_count);
int     earray_append(earray_t* earr, const void* item);
char*   earray_append_ptr(earray_t* earr);

/* array items */
int     earray_remove(earray_t* earr, size_t index);

/* swap arrays */
void    earray_swap(earray_t* earr_left, earray_t* earr_right);

/* array properties */
#define earray_size(earr)           ((earr)->item_count)
#define earray_item_size(earr)      ((earr)->item_size)
#define earray_items(earr)          ((earr)->items)
#define earray_at(earr, index)      ((earr)->items + (index)*(earr)->item_size)

/* common types */
#define earray_int_at(earr, index)      (*(int*)((earr)->items))
#define earray_ulong_at(earr, index)    (*(unsigned long*)((earr)->items))

/*----------------------------------------------------------------------*/

#endif /* _EARRAY_H_ */

