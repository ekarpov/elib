/*
    Array helpers
*/

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "eassert.h"
#include "etrace.h"
#include "eerror.h"
#include "earray.h"

/*----------------------------------------------------------------------*/

/* init and close */
void    earray_init(earray_t* earr, size_t item_size)
{
    EASSERT(earr);
    EASSERT(item_size > 0);
    if(earr)
    {
        /* reset all fields */
        ememset(earr, 0, sizeof(earray_t));

        /* set item size */
        earr->item_size = item_size;
    }
}

void    earray_reset(earray_t* earr)
{
    EASSERT(earr);
    if(earr)
    {
        /* reset item count */
        earr->item_count = 0;
    }
}

void    earray_free(earray_t* earr)
{
    if(earr)
    {
        /* free memory */
        efree(earr->items);

        /* reset all fields */
        ememset(earr, 0, sizeof(earray_t));
    }
}

/* array manipulations */
int     earray_reserve(earray_t* earr, size_t item_count)
{
    void* tmp = 0;

    EASSERT(earr);
    if(earr == 0) return ELIBC_ERROR_ARGUMENT;

    /* do nothing if size is less than already allocated */
    if(earr->item_size * item_count <= earr->alloc_size) return ELIBC_SUCCESS;

    /* allocate memory */
    tmp = erealloc(earr->items, earr->item_size * item_count);
    if(tmp == 0) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;

    /* update buffer */
    earr->items = (char*)tmp;
    earr->alloc_size = earr->item_size * item_count;

    return ELIBC_SUCCESS;
}

int     earray_resize(earray_t* earr, size_t item_count)
{
    int err;

    EASSERT(earr);
    EASSERT(earr > 0);
    if(earr == 0 || item_count < 0) return ELIBC_ERROR_ARGUMENT;

    /* reserve enough space */
    err = earray_reserve(earr, item_count);
    if(err != ELIBC_SUCCESS) return err;

    /* update count */
    earr->item_count = item_count;

    return ELIBC_SUCCESS;
}

int     earray_append(earray_t* earr, const void* item)
{
    EASSERT(earr);
    if(earr == 0) return ELIBC_ERROR_ARGUMENT;

    /* check if we have enough memory for one more item */
    if(earr->item_size * (earr->item_count + 1) > earr->alloc_size)
    {
        /* double buffer size for efficiency */
        size_t reserve_size = (earr->item_count > 0) ? earr->item_count * 2 : 1;
        int err = earray_reserve(earr, reserve_size);
        if(err != ELIBC_SUCCESS) return err;
    }

    /* copy item */
    ememcpy(earr->items + (earr->item_size * earr->item_count), item, earr->item_size);

    /* update counter */
    earr->item_count++;

    return ELIBC_SUCCESS;
}

char*   earray_append_ptr(earray_t* earr)
{
    char* ptr;
    int err;

    EASSERT(earr);
    if(earr == 0) return 0;

    /* reserve space for extra item if needed */
    err = earray_reserve(earr, earr->item_count + 1);
    if(err != ELIBC_SUCCESS) return 0;

    /* story return pointer */
    ptr = earr->items + earr->item_count * earr->item_size;

    /* update item count */
    earr->item_count++;

    /* return pointer for value */
    return ptr;
}

/* array items */
int earray_remove(earray_t* earr, size_t index)
{
    /* check input */
    EASSERT(earr);
    if(earr == 0) return ELIBC_ERROR_ARGUMENT;

    /* check index */
    EASSERT(index < earr->item_count);
    if(index >= earr->item_count) return ELIBC_ERROR_ARGUMENT;

    /* move items if needed */
    if(index + 1 < earr->item_count)
    {
        ememcpy(earr->items + earr->item_size * index, 
                earr->items + earr->item_size * (index + 1), 
                earr->item_size * (earr->item_count - index - 1));
    }

    /* update counter */
    earr->item_count--;

    return ELIBC_SUCCESS;
}

/* swap arrays */
void earray_swap(earray_t* earr_left, earray_t* earr_right)
{
    earray_t tmp;

    EASSERT(earr_left);
    EASSERT(earr_right);
    if(earr_left == 0 || earr_right == 0) return;

    /* arrays must be of the same item size */
    EASSERT1(earr_left->item_size == earr_right->item_size, "earray: trying to swap arrays of different item size");

    /* swap */
    tmp = *earr_left;
    *earr_left = *earr_right;
    *earr_right = tmp;
}

/*----------------------------------------------------------------------*/

