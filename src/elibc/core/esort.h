/*
    Sorting implementation
*/

#ifndef _ESORT_H_
#define _ESORT_H_

/*----------------------------------------------------------------------*/

/* generic array sort */
int esort(char* items, size_t item_size, size_t item_count, eless_func_t eless_func, char* swap_buffer);

/* sort list */
int esort_list(elist_t* elist, eless_func_t eless_func);

/*----------------------------------------------------------------------*/

#endif /* _ESORT_H_ */

