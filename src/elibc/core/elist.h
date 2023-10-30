/*
    Linked list data storage
*/

#ifndef _ELIST_H_
#define _ELIST_H_

/*----------------------------------------------------------------------*/

/*
    NOTE: elist is using single buffer to hold all values and links to next
          and previous elements inside list (values are always copied). When
          item is removed it will be replaced with the last element and its
          offsets will be updated.

          Items are of fixed size. When item is set it will be copied from 
          provided pointer (item_size bytes). Items are stored in memory 
          aligned to the nearest multiple of size_t for efficiency reason and
          to avoid bus errors (unaligned memory access) in certains systems.
*/

/*----------------------------------------------------------------------*/
/* constants */

#define ELIST_NULL_ITERATOR         0

/*----------------------------------------------------------------------*/

/* list structure */
typedef struct
{
    size_t*         buffer;
    size_t          buffer_size;

    size_t          items_count;
    size_t          item_size;
    size_t          node_size;

} elist_t;

/* list iterator */
typedef size_t      eliter_t;

/*----------------------------------------------------------------------*/

/* init and close */
void elist_init(elist_t* elist, size_t item_size);
void elist_free(elist_t* elist);
void elist_reset(elist_t* elist);

/* list size (items count) */
size_t  elist_size(elist_t* elist);
size_t  elist_item_size(elist_t* elist);

/*
    NOTE: iterator access functions will return iterator in eliter_out or
          ELIST_NULL_ITERATOR if no more items can be accessed.
*/

/* iterator access */
int elist_head(elist_t* elist, eliter_t* eliter_out);
int elist_tail(elist_t* elist, eliter_t* eliter_out);
int elist_next(elist_t* elist, const eliter_t eliter, eliter_t* eliter_out);
int elist_previous(elist_t* elist, const eliter_t eliter, eliter_t* eliter_out);

/* manage items */
int elist_remove(elist_t* elist, const eliter_t eliter, eliter_t* next_out);
int elist_swap(elist_t* elist, const eliter_t eliter_left, const eliter_t eliter_right);

/*
    NOTE: if item is null list will reserve memory for item, use elist_item to fill actual data
*/

/* add items (use ELIST_NULL_ITERATOR to append at the end or insert in front of list, eliter_out may be zero) */
int elist_append(elist_t* elist, const eliter_t eliter, const void* item, eliter_t* eliter_out);
int elist_insert(elist_t* elist, const eliter_t eliter, const void* item, eliter_t* eliter_out);

/* modify items */
int elist_replace(elist_t* elist, const eliter_t eliter, const void* item);

/* items data */
void* elist_item(elist_t* elist, const eliter_t eliter);

/*----------------------------------------------------------------------*/

#endif /* _ELIST_H_ */



