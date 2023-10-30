/*
    Sorting implementation
*/

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "ecore_types.h"
#include "eassert.h"
#include "etrace.h"
#include "eerror.h"
#include "elist.h"
#include "esort.h"

/*----------------------------------------------------------------------*/
/* constants */
/*----------------------------------------------------------------------*/

#define ESORT_CUTOFF_THRESHOLD              8
#define ESORT_LIST_CUTOFF_THRESHOLD         8

/*----------------------------------------------------------------------*/
/* helper functions */
/*----------------------------------------------------------------------*/
ELIBC_FORCE_INLINE void _eswap_items(char* items, size_t item_size, size_t left_pos, size_t right_pos, char* swap_buffer)
{
    /* left item to temp */
    ememcpy(swap_buffer, items + left_pos * item_size, item_size);

    /* right item to left */
    ememcpy(items + left_pos * item_size, items + right_pos * item_size, item_size);

    /* temp to right item */
    ememcpy(items + right_pos * item_size, swap_buffer, item_size);
}

void _einsertsort(char* items, size_t item_size, size_t start_pos, size_t end_pos, eless_func_t eless_func, char* swap_buffer)
{
    size_t pos, cmp_pos;

    /* loop over all elements */
    for(pos = start_pos + 1; pos < end_pos; ++pos)
    {
        /* move to the beginning */
        for(cmp_pos = pos; cmp_pos > start_pos; --cmp_pos)
        {
            /* check order */
            if(eless_func(items + (cmp_pos-1) * item_size, items + cmp_pos * item_size)) break;
            
            /* sawp */
            _eswap_items(items, item_size, cmp_pos-1, cmp_pos, swap_buffer);
        }
    }
}

void _esort_iteration(char* items, size_t item_size, size_t start_pos, size_t end_pos, eless_func_t eless_func, char* swap_buffer)
{
    size_t pivot, low, high;

    /* check if ready*/
    if(end_pos - start_pos <= ESORT_CUTOFF_THRESHOLD) 
    {
        /* finish with insertion sort */
        _einsertsort(items, item_size, start_pos, end_pos, eless_func, swap_buffer);

        /* stop */
        return; 
    }

    /* use median as pivot */
    pivot = start_pos + (end_pos - start_pos) / 2;

    /* move pivot point to the end */
    _eswap_items(items, item_size, pivot, end_pos - 1, swap_buffer);

    /* adjust pivot iterator */
    pivot = end_pos - 1;

    /* borders */
    low = start_pos;
    high = pivot - 1; /* ignore pivot value */

    /* sort */
    while(low < high)
    {
        /* find low swap point */
        while(low < high && eless_func(items + low * item_size, items + pivot * item_size)) ++low;

        /* find high swap point */
        while(low < high && eless_func(items + pivot * item_size, items + high * item_size)) --high;

        /* swap if needed */
        if(low < high)
        {
            _eswap_items(items, item_size, low, high, swap_buffer);
            ++low;
            --high;
        }
    }

    /* check if we need to move intersection point */
    if(eless_func(items + low * item_size, items + pivot * item_size))
    {
        ++low;
    }
    
    /* move pivot at intersection point */
    _eswap_items(items, item_size, low, pivot, swap_buffer);

    /* sort parts */
    _esort_iteration(items, item_size, start_pos, low, eless_func, swap_buffer);
    _esort_iteration(items, item_size, high + 1, end_pos, eless_func, swap_buffer);
}

/*----------------------------------------------------------------------*/
/* generic array sort */
/*----------------------------------------------------------------------*/
int esort(char* items, size_t item_size, size_t item_count, eless_func_t eless_func, char* swap_buffer)
{
    char* temp_buffer=0;

    /* check input */
    EASSERT(items);
    EASSERT(item_size);
    EASSERT(eless_func);
    if(items == 0 || item_size == 0 || eless_func == 0) return ELIBC_ERROR_ARGUMENT;

    /* ignore if there is one or zero items */
    if(item_count <= 1) return ELIBC_SUCCESS;

    /* alloc swap buffer if not provided */
    if(swap_buffer == 0)
    {
        temp_buffer = (char*)emalloc(item_size);
        if(temp_buffer == 0) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;

        /* set reference */
        swap_buffer = temp_buffer;
    }

    /* sort */
    _esort_iteration(items, item_size, 0, item_count, eless_func, swap_buffer);

    /* release swap buffer if any */
    if(temp_buffer)
    {
        efree(temp_buffer);
        temp_buffer = 0;
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* sort list */
/*----------------------------------------------------------------------*/
int _einsertsort_list(elist_t* elist, const eliter_t start_pos, const eliter_t end_pos, eless_func_t eless_func)
{
    size_t cmp_pos;
    int err = ELIBC_SUCCESS;

    EASSERT(start_pos);
    EASSERT(end_pos);

    /* check if something to sort */
    if(start_pos == end_pos) return ELIBC_SUCCESS;

    /* loop over all elements */
    cmp_pos = start_pos;
    while(cmp_pos != end_pos)
    {
        size_t cmp_left = cmp_pos;
        size_t cmp_right;

        /* next item */
        err = elist_next(elist, cmp_pos, &cmp_pos);
        if(err != ELIBC_SUCCESS) break;

        /* move to the beginning */
        cmp_right = cmp_pos;
        while(cmp_right != start_pos)
        {
            /* check order */
            if(eless_func(elist_item(elist, cmp_left), elist_item(elist, cmp_right))) break;
            
            /* swap */
            err = elist_swap(elist, cmp_left, cmp_right);
            if(err != ELIBC_SUCCESS) break;

            cmp_right = cmp_left; 

            /* previous */
            err = elist_previous(elist, cmp_left, &cmp_left);
            if(err != ELIBC_SUCCESS) break;
        }
    }

    return err;
}

size_t _esort_list_item_count(elist_t* elist, eliter_t start_pos, const eliter_t end_pos)
{
    size_t item_count = 1;
    int err = ELIBC_SUCCESS;

    EASSERT(start_pos);
    EASSERT(end_pos);

    /* count items */
    while(start_pos != end_pos)
    {
        item_count++;

        /* next */
        err = elist_next(elist, start_pos, &start_pos);
        if(err != ELIBC_SUCCESS) break;
    }

    EASSERT(err == ELIBC_SUCCESS);
    return item_count;
}

eliter_t _esort_list_advance(elist_t* elist, eliter_t start_pos, size_t count)
{
    size_t item_count = 0;
    int err;

    EASSERT(start_pos);

    /* advance iterator */
    while(item_count < count)
    {
        item_count++;

        /* next */
        err = elist_next(elist, start_pos, &start_pos);
        if(err != ELIBC_SUCCESS) break;
    }

    EASSERT(err == ELIBC_SUCCESS);
    return start_pos;
}

int _esort_list_iteration(elist_t* elist, const eliter_t start_pos, const eliter_t end_pos, eless_func_t eless_func)
{
    eliter_t pivot, low, high;
    size_t item_count;
    int err;

    /* find size */
    item_count = _esort_list_item_count(elist, start_pos, end_pos);

    /* check if ready*/
    if(item_count <= ESORT_LIST_CUTOFF_THRESHOLD) 
    {
        /* finish with insertion sort */
        return _einsertsort_list(elist, start_pos, end_pos, eless_func);
    }

    /* use median as pivot */
    pivot = _esort_list_advance(elist, start_pos, item_count / 2);

    /* move pivot point to the end */
    err = elist_swap(elist, pivot, end_pos);
    if(err != ELIBC_SUCCESS) return err;

    /* adjust pivot iterator */
    pivot = end_pos;

    /* beginning */
    low = start_pos;

    /* end (ignore pivot value) */
    err = elist_previous(elist, pivot, &high);
    if(err != ELIBC_SUCCESS) return err;

    /* sort */
    while(low != high)
    {
        /* find low swap point */
        while(low != high && eless_func(elist_item(elist, low), elist_item(elist, pivot))) 
        {
            err = elist_next(elist, low, &low);
            if(err != ELIBC_SUCCESS) return err;
        }

        /* find high swap point */
        while(low != high && eless_func(elist_item(elist, pivot), elist_item(elist, high)))
        {
            err = elist_previous(elist, high, &high);
            if(err != ELIBC_SUCCESS) return err;
        }

        /* swap if needed */
        if(low != high)
        {
            err = elist_swap(elist, low, high);
            if(err != ELIBC_SUCCESS) return err;

            err = elist_next(elist, low, &low);
            if(err != ELIBC_SUCCESS) return err;

            if(low != high)
            {
                err = elist_previous(elist, high, &high);
                if(err != ELIBC_SUCCESS) return err;
            }
        }
    }

    /* check if we need to move intersection point */
    if(eless_func(elist_item(elist, low), elist_item(elist, pivot)))
    {
        err = elist_next(elist, low, &low);
        if(err != ELIBC_SUCCESS) return err;
    }
    
    /* move pivot at intersection point */
    err = elist_swap(elist, low, pivot);
    if(err != ELIBC_SUCCESS) return err;

    /* move high to next */
    err = elist_next(elist, high, &high);
    if(err != ELIBC_SUCCESS) return err;

    /* sort left part */
    err = _esort_list_iteration(elist, start_pos, low, eless_func);
    if(err != ELIBC_SUCCESS) return err;

    /* sort right part */
    err = _esort_list_iteration(elist, high, end_pos, eless_func);
    if(err != ELIBC_SUCCESS) return err;

    return err;
}

int esort_list(elist_t* elist, eless_func_t eless_func)
{
    eliter_t list_head, list_tail;
    int err;

    /* check input */
    EASSERT(elist);
    EASSERT(eless_func);
    if(elist == 0 || eless_func == 0) return ELIBC_ERROR_ARGUMENT;

    /* ignore if there is one or zero items */
    if(elist_size(elist) <= 1) return ELIBC_SUCCESS;

    /* list head */
    err = elist_head(elist, &list_head);
    if(err != ELIBC_SUCCESS) return err;

    /* list tail */
    err = elist_tail(elist, &list_tail);
    if(err != ELIBC_SUCCESS) return err;

    /* sort */
    err = _esort_list_iteration(elist, list_head, list_tail, eless_func);

    EASSERT(err == ELIBC_SUCCESS);
    return err;
}

/*----------------------------------------------------------------------*/

