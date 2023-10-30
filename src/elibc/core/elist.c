/*
    Linked list data storage
*/

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "eassert.h"
#include "etrace.h"
#include "eerror.h"
#include "elist.h"

/*----------------------------------------------------------------------*/

/*
    NOTE: items buffer starts with internal list data of size ELIST_DATA_OFFSET.
          By doing so we can reserve zero offset as NULL iterator. 
*/

/*----------------------------------------------------------------------*/
/* constants */

/* default node count */
#define ELIST_DEFAULT_NODE_COUNT    8

/* data starts after next and previous offsets */
#define ELIST_NODE_DATA_OFFSET      2

/* node offsets */
#define ELIST_PREV_OFFSET_INDEX     0
#define ELIST_NEXT_OFFSET_INDEX     1

/* list internal offsets */
#define ELIST_HEAD_OFFSET_INDEX     0
#define ELIST_TAIL_OFFSET_INDEX     1
#define ELIST_DATA_OFFSET           2

/*----------------------------------------------------------------------*/
/* list offsets */

ELIBC_FORCE_INLINE size_t _elist_head_offset(elist_t* elist)
{
    return elist->buffer ? elist->buffer[ELIST_HEAD_OFFSET_INDEX] : ELIST_NULL_ITERATOR;
}

ELIBC_FORCE_INLINE size_t _elist_tail_offset(elist_t* elist)
{
    return elist->buffer ? elist->buffer[ELIST_TAIL_OFFSET_INDEX] : ELIST_NULL_ITERATOR;
}

ELIBC_FORCE_INLINE size_t* _elist_head_node(elist_t* elist)
{
    return elist->buffer + _elist_head_offset(elist);
}

ELIBC_FORCE_INLINE size_t* _elist_tail_node(elist_t* elist)
{
    return elist->buffer + _elist_head_offset(elist);
}

/*----------------------------------------------------------------------*/
/* helpers */

/* validate list */
ELIBC_FORCE_INLINE int _elist_validate_list(elist_t* elist)
{
    /* validate list pointer */
    if(elist == 0) 
    {
        EASSERT1(0, "elist: list pointer is not valid");
        return ELIBC_FALSE;
    }

    /* validate internal state */
    if(elist->items_count > 0)
    {
        /* items buffer must be allocated */
        if(elist->buffer == 0 || elist->node_size == 0 ||
           elist->items_count * elist->node_size + ELIST_DATA_OFFSET > elist->buffer_size) 
        {
            EASSERT1(0, "elist: internal state is not valid");
            return ELIBC_FALSE;
        }
    }

    return ELIBC_TRUE;
}

/* validate iterator */
ELIBC_FORCE_INLINE size_t _elist_validate_iter(elist_t* elist, const eliter_t eliter)
{
    /* there must be items in list */
    if(elist->items_count == 0)
    {
        EASSERT1(0, "elist: trying to use iterator on empty list");
        return ELIBC_FALSE;
    }

    /* iterator must be inside items buffer */
    if(eliter < ELIST_DATA_OFFSET || 
       eliter > (elist->items_count - 1) * elist->node_size + ELIST_DATA_OFFSET)
    {
        EASSERT1(0, "elist: iterator is out of range");
        return ELIBC_FALSE;
    }

    /* iterator must be aligned to a node size */
    if((eliter - ELIST_DATA_OFFSET) % elist->node_size != 0)
    {
        EASSERT1(0, "elist: iterator is not aligned");
        return ELIBC_FALSE;
    }

    return ELIBC_TRUE;
}

/* reserve space for extra item */
int _elist_reserve_space(elist_t* elist)
{
    /* check if we have items at all */
    if(elist->buffer)
    {
        /* check if we can fit one more node */
        if((elist->items_count + 1) * elist->node_size + ELIST_DATA_OFFSET >= elist->buffer_size)
        {
            /* reserve extra memory */
            void* tmp = erealloc(elist->buffer, 2 * elist->buffer_size * sizeof(size_t));
            if(tmp == 0) return ELIBC_FALSE;

            /* update buffer */
            elist->buffer = (size_t*)tmp;
            elist->buffer_size *= 2;
        }

    } else
    {
        EASSERT(elist->items_count == 0);

        /* reserve default space */
        elist->buffer_size = ELIST_DATA_OFFSET + ELIST_DEFAULT_NODE_COUNT * elist->node_size;

        /* alloc memory */
        elist->buffer = (size_t*)emalloc(elist->buffer_size * sizeof(size_t));
        if(elist->buffer == 0)
        {
            elist->buffer_size = 0;
            return ELIBC_FALSE;
        }

        /* init internal data */
        elist->buffer[ELIST_HEAD_OFFSET_INDEX] = ELIST_NULL_ITERATOR;
        elist->buffer[ELIST_TAIL_OFFSET_INDEX] = ELIST_NULL_ITERATOR;
    }

    return ELIBC_TRUE;
}

/*----------------------------------------------------------------------*/
/* init and close */
/*----------------------------------------------------------------------*/
void elist_init(elist_t* elist, size_t item_size)
{
    EASSERT(elist);
    EASSERT(item_size > 0);
    if(elist == 0 || item_size <= 0) return;

    /* reset all fields */
    ememset(elist, 0, sizeof(elist_t));

    /* set item size */
    elist->item_size = item_size;
    
    /* node size in multiple of size_t */
    if(item_size % sizeof(size_t) == 0)
    {
        elist->node_size = item_size / sizeof(size_t);

    } else
    {
        /* round item size to next multiple of size_t */
        elist->node_size = item_size / sizeof(size_t) + 1;
    }

    /* space for node offsets */
    elist->node_size += ELIST_NODE_DATA_OFFSET;
}

void elist_free(elist_t* elist)
{
    if(elist)
    {
        /* release memory */
        efree(elist->buffer);

        /* reset just in case */
        ememset(elist, 0, sizeof(elist_t));
    }
}

void elist_reset(elist_t* elist)
{
    EASSERT(elist);
    if(elist == 0) return;

    /* reset item count */
    elist->items_count = 0;

    /* reset head and tail */
    if(elist->buffer)
    {
        elist->buffer[ELIST_HEAD_OFFSET_INDEX] = ELIST_NULL_ITERATOR;
        elist->buffer[ELIST_TAIL_OFFSET_INDEX] = ELIST_NULL_ITERATOR;
    }
}

/*----------------------------------------------------------------------*/
/* list size (items count) */
/*----------------------------------------------------------------------*/
size_t elist_size(elist_t* elist)
{
    /* check input */
    EASSERT(elist);
    if(elist == 0) return 0;

    return elist->items_count;
}

size_t elist_item_size(elist_t* elist)
{
    /* check input */
    EASSERT(elist);
    if(elist == 0) return 0;

    return elist->item_size;
}

/*----------------------------------------------------------------------*/
/* iterator access */
/*----------------------------------------------------------------------*/
int elist_head(elist_t* elist, eliter_t* eliter_out)
{
    /* check input */
    if(!_elist_validate_list(elist)) return ELIBC_ERROR_ARGUMENT;

    /* check output */
    EASSERT(eliter_out);
    if(eliter_out == 0) return ELIBC_ERROR_ARGUMENT;

    /* head node offset */
    *eliter_out = _elist_head_offset(elist);

    return ELIBC_SUCCESS;
}

int elist_tail(elist_t* elist, eliter_t* eliter_out)
{
    /* check input */
    if(!_elist_validate_list(elist)) return ELIBC_ERROR_ARGUMENT;

    /* check output */
    EASSERT(eliter_out);
    if(eliter_out == 0) return ELIBC_ERROR_ARGUMENT;

    /* tail node offset */
    *eliter_out = _elist_tail_offset(elist);

    return ELIBC_SUCCESS;
}

int elist_next(elist_t* elist, const eliter_t eliter, eliter_t* eliter_out)
{
    /* check input */
    if(!_elist_validate_list(elist)) return ELIBC_ERROR_ARGUMENT;

    /* check output */
    EASSERT(eliter_out);
    if(eliter_out == 0) return ELIBC_ERROR_ARGUMENT;

    /* validate iterator */
    if(!_elist_validate_iter(elist, eliter)) return ELIBC_ERROR_ARGUMENT;

    /* read next offset */
    *eliter_out = (elist->buffer + eliter)[ELIST_NEXT_OFFSET_INDEX];

    return ELIBC_SUCCESS;
}

int elist_previous(elist_t* elist, const eliter_t eliter, eliter_t* eliter_out)
{
    /* check input */
    if(!_elist_validate_list(elist)) return ELIBC_ERROR_ARGUMENT;

    /* check output */
    EASSERT(eliter_out);
    if(eliter_out == 0) return ELIBC_ERROR_ARGUMENT;

    /* validate iterator */
    if(!_elist_validate_iter(elist, eliter)) return ELIBC_ERROR_ARGUMENT;

    /* read previous offset */
    *eliter_out = (elist->buffer + eliter)[ELIST_PREV_OFFSET_INDEX];

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* manage items */
/*----------------------------------------------------------------------*/
int elist_remove(elist_t* elist, const eliter_t eliter, eliter_t* next_out)
{
    size_t next_node, prev_node;
    size_t last_next, last_prev;
    size_t last_node_offset;

    /* check input */
    if(!_elist_validate_list(elist)) return ELIBC_ERROR_ARGUMENT;

    /* validate iterator */
    if(!_elist_validate_iter(elist, eliter)) return ELIBC_ERROR_ARGUMENT;
    
    /* reset output */
    if(next_out) *next_out = ELIST_NULL_ITERATOR;

    /* process special cases */
    if(elist->items_count == 1)
    {
        /* special case when there is only one item left */
        elist->items_count = 0;
        elist->buffer[ELIST_HEAD_OFFSET_INDEX] = ELIST_NULL_ITERATOR;
        elist->buffer[ELIST_TAIL_OFFSET_INDEX] = ELIST_NULL_ITERATOR;

        return ELIBC_SUCCESS;
    }

    /*
        NOTE: node is removed by replacing it with last node in items buffer.
              Node data is copied and offsets are updated. Special care is taken
              if head or tail is being removed.
    */

    /* node offsets */
    prev_node = (elist->buffer + eliter)[ELIST_PREV_OFFSET_INDEX];
    next_node = (elist->buffer + eliter)[ELIST_NEXT_OFFSET_INDEX];

    /* last node offset */
    last_node_offset = ELIST_DATA_OFFSET + (elist->items_count - 1) * elist->node_size;

    /* copy next node iterator if needed */
    if(next_out) 
    {
        if(next_node != last_node_offset)
            *next_out = next_node;
        else
            *next_out = eliter;
    }

    /* replace removed node with last node (if not last node is being removed) */
    if(last_node_offset != eliter)
    {
        /* last node offsets */
        last_prev = (elist->buffer + last_node_offset)[ELIST_PREV_OFFSET_INDEX];
        last_next = (elist->buffer + last_node_offset)[ELIST_NEXT_OFFSET_INDEX];

        /* copy data */
        ememcpy(elist->buffer + eliter, elist->buffer + last_node_offset, elist->node_size * sizeof(size_t));

        /* update node that originaly pointed to last */
        if(last_prev && last_prev != eliter)
            (elist->buffer + last_prev)[ELIST_NEXT_OFFSET_INDEX] = eliter;

        /* update node that originaly pointed to last */
        if(last_next && last_next != eliter)
            (elist->buffer + last_next)[ELIST_PREV_OFFSET_INDEX] = eliter;

        /* check if removed node pointed to last node */
        if(prev_node == last_node_offset)
            (elist->buffer + eliter)[ELIST_NEXT_OFFSET_INDEX] = next_node;

        /* check if removed node pointed to last node */
        if(next_node == last_node_offset)
            (elist->buffer + eliter)[ELIST_PREV_OFFSET_INDEX] = prev_node;

        /* check if head points to last node */
        if(last_node_offset == elist->buffer[ELIST_HEAD_OFFSET_INDEX])
            elist->buffer[ELIST_HEAD_OFFSET_INDEX] = eliter;

        /* check if tail points to last node */
        if(last_node_offset == elist->buffer[ELIST_TAIL_OFFSET_INDEX])
            elist->buffer[ELIST_TAIL_OFFSET_INDEX] = eliter;
    }

    /* update next node "previous" */
    if(next_node && next_node != last_node_offset)
        (elist->buffer + next_node)[ELIST_PREV_OFFSET_INDEX] = prev_node;
            
    /* update previous node "next" */
    if(prev_node && prev_node != last_node_offset)
        (elist->buffer + prev_node)[ELIST_NEXT_OFFSET_INDEX] = next_node;

    /* check if head node is removed */
    if(eliter == elist->buffer[ELIST_HEAD_OFFSET_INDEX] && 
       next_node != last_node_offset && 
       next_node != ELIST_NULL_ITERATOR)
    {
        /* point head to next node */
        elist->buffer[ELIST_HEAD_OFFSET_INDEX] = next_node;
    } 

    /* check if tail node is removed */
    if(eliter == elist->buffer[ELIST_TAIL_OFFSET_INDEX] && 
       prev_node != last_node_offset && 
       prev_node != ELIST_NULL_ITERATOR)
    {
        /* point tail to previous node */
        elist->buffer[ELIST_TAIL_OFFSET_INDEX] = prev_node;
    }

    /* decrease item count */
    elist->items_count--;

    return ELIBC_SUCCESS;
}

int elist_swap(elist_t* elist, const eliter_t eliter_left, const eliter_t eliter_right)
{
    size_t* swap_buffer;
    size_t* left_buffer;
    size_t* right_buffer;

    /* check input */
    if(!_elist_validate_list(elist)) return ELIBC_ERROR_ARGUMENT;

    /* validate iterators */
    if(!_elist_validate_iter(elist, eliter_left) ||
       !_elist_validate_iter(elist, eliter_right)) return ELIBC_ERROR_ARGUMENT;

    /* make sure we have extra space to swap items */
    if(!_elist_reserve_space(elist)) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;

    /* extra space buffer */
    swap_buffer = elist->buffer + ELIST_DATA_OFFSET + elist->items_count * elist->node_size;

    /* items data pointers */
    left_buffer = elist->buffer + eliter_left + ELIST_NODE_DATA_OFFSET;
    right_buffer = elist->buffer + eliter_right + ELIST_NODE_DATA_OFFSET;

    /* left item to temp */
    ememcpy(swap_buffer, left_buffer, elist->item_size);

    /* right item to left */
    ememcpy(left_buffer, right_buffer, elist->item_size);

    /* temp to right item */
    ememcpy(right_buffer, swap_buffer, elist->item_size);

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* add items */
/*----------------------------------------------------------------------*/
size_t _elist_add_new_node(elist_t* elist, const void* item, size_t prev_node, size_t next_node)
{
    size_t add_node_offset;

    /* append new node at the end */
    add_node_offset = ELIST_DATA_OFFSET + elist->items_count * elist->node_size;

    /* set offsets */
    (elist->buffer + add_node_offset)[ELIST_PREV_OFFSET_INDEX] = prev_node;
    (elist->buffer + add_node_offset)[ELIST_NEXT_OFFSET_INDEX] = next_node;

    /* copy item data if needed */
    if(item)
    {
        ememcpy(elist->buffer + add_node_offset + ELIST_NODE_DATA_OFFSET, item, elist->item_size);
    }

    /* increase item count */
    elist->items_count++;

    /* new node offset */
    return add_node_offset;
}

int _elist_add_first_node(elist_t* elist, const void* item, eliter_t* eliter_out)
{
    size_t new_node_offset;

    /* add node */
    new_node_offset = _elist_add_new_node(elist, item, ELIST_NULL_ITERATOR, ELIST_NULL_ITERATOR);

    /* update head and tail as well */
    elist->buffer[ELIST_HEAD_OFFSET_INDEX] = new_node_offset;
    elist->buffer[ELIST_TAIL_OFFSET_INDEX] = new_node_offset;
    
    /* return iterator if needed */
    if(eliter_out) *eliter_out = new_node_offset;

    return ELIBC_SUCCESS;
}

int _elist_add_item_impl(elist_t* elist, eliter_t eliter, const void* item, eliter_t* eliter_out, int after_iter)
{
    size_t next_node, prev_node;
    size_t new_node_offset;

    /* check input */
    if(!_elist_validate_list(elist)) return ELIBC_ERROR_ARGUMENT;

    /* validate iterator */
    if(eliter != ELIST_NULL_ITERATOR)
    {
        if(!_elist_validate_iter(elist, eliter)) return ELIBC_ERROR_ARGUMENT;
    }

    /* reserve space if needed */
    if(!_elist_reserve_space(elist)) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;

    /* handle special case when list is empty */
    if(elist->items_count == 0)
    {
        /* add first node */
        return _elist_add_first_node(elist, item, eliter_out);
    }

    /* use tail or head node if iterator not set */
    if(eliter == ELIST_NULL_ITERATOR)
    {
        eliter = elist->buffer[after_iter ? ELIST_TAIL_OFFSET_INDEX : ELIST_HEAD_OFFSET_INDEX];
    }

    /* must be valid now */
    EASSERT(eliter != ELIST_NULL_ITERATOR);
    if(eliter == ELIST_NULL_ITERATOR) return ELIBC_ERROR_INTERNAL;

    /* node offsets */
    if(after_iter)
    {
        prev_node = eliter;
        next_node = (elist->buffer + eliter)[ELIST_NEXT_OFFSET_INDEX];

    } else
    {
        prev_node = (elist->buffer + eliter)[ELIST_PREV_OFFSET_INDEX];
        next_node = eliter;
    }

    /* add node */
    new_node_offset = _elist_add_new_node(elist, item, prev_node, next_node);

    /* set new head or tail if needed */
    if(eliter == elist->buffer[ELIST_TAIL_OFFSET_INDEX] && after_iter)
    {
        elist->buffer[ELIST_TAIL_OFFSET_INDEX] = new_node_offset;

    } else if(eliter == elist->buffer[ELIST_HEAD_OFFSET_INDEX] && !after_iter)
    {
        elist->buffer[ELIST_HEAD_OFFSET_INDEX] = new_node_offset;
    }

    /* update offsets */
    if(prev_node != ELIST_NULL_ITERATOR)
        (elist->buffer + prev_node)[ELIST_NEXT_OFFSET_INDEX] = new_node_offset;
    if(next_node != ELIST_NULL_ITERATOR)
        (elist->buffer + next_node)[ELIST_PREV_OFFSET_INDEX] = new_node_offset;

    /* return iterator if needed */
    if(eliter_out) *eliter_out = new_node_offset;

    return ELIBC_SUCCESS;
}

int elist_append(elist_t* elist, const eliter_t eliter, const void* item, eliter_t* eliter_out)
{
    return _elist_add_item_impl(elist, eliter, item, eliter_out, ELIBC_TRUE);
}

int elist_insert(elist_t* elist, const eliter_t eliter, const void* item, eliter_t* eliter_out)
{
    return _elist_add_item_impl(elist, eliter, item, eliter_out, ELIBC_FALSE);
}

/*----------------------------------------------------------------------*/
/* modify items */
/*----------------------------------------------------------------------*/
int elist_replace(elist_t* elist, const eliter_t eliter, const void* item)
{
    /* check input */
    if(!_elist_validate_list(elist)) return ELIBC_ERROR_ARGUMENT;

    /* validate iterator */
    if(!_elist_validate_iter(elist, eliter)) return ELIBC_ERROR_ARGUMENT;

    /* check item */
    EASSERT(item);
    if(item == 0) return ELIBC_ERROR_ARGUMENT;

    /* replace node data */
    ememcpy(elist->buffer + eliter + ELIST_NODE_DATA_OFFSET, item, elist->item_size);

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* items data  */
/*----------------------------------------------------------------------*/
void* elist_item(elist_t* elist, const eliter_t eliter)
{
    /* check input */
    if(!_elist_validate_list(elist)) return 0;

    /* validate iterator */
    if(!_elist_validate_iter(elist, eliter)) return 0;

    /* node data */
    return (elist->buffer + eliter + ELIST_NODE_DATA_OFFSET);
}

/*----------------------------------------------------------------------*/

