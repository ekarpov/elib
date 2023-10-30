/*
    Stack implementation
*/

#ifndef _ESTACK_H_
#define _ESTACK_H_

/*----------------------------------------------------------------------*/

/* NOTE: stack is just convenience wrapper on array */
typedef earray_t   estack_t;

/*----------------------------------------------------------------------*/

/* init and close */
#define estack_init                     earray_init
#define estack_reset                    earray_reset
#define estack_free                     earray_free
#define estack_reserve                  earray_reserve

/* stack properties */
#define estack_top(estack)              earray_at((estack), earray_size((estack)) - 1)
//#define estack_pos(estack)              (ebuffer_pos((estack)) / value_size)
#define estack_at                       earray_at
#define estack_size                     earray_size
#define estack_item_size                earray_item_size

/* stack manipulations */
#define estack_push(estack, value)      earray_append((estack), &value)
#define estack_pop(estack)              earray_resize((estack), earray_size((estack)) - 1)

/*----------------------------------------------------------------------*/

#endif /* _ESTACK_H_ */ 

