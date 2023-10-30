/*
    Memory buffer
*/

#ifndef _EBUFFER_H_
#define _EBUFFER_H_

/*----------------------------------------------------------------------*/

/* data buffer type */
typedef struct 
{
    char*       data;           /* allocated memory buffer */
    size_t      size;           /* buffer size in bytes */
    size_t      pos;            /* position offset */

} ebuffer_t;

/*----------------------------------------------------------------------*/

/* init and close */
void    ebuffer_init(ebuffer_t* ebuff);
void    ebuffer_reset(ebuffer_t* ebuff);
void    ebuffer_free(ebuffer_t* ebuff);

/* buffer properties */
#define ebuffer_data(ebuff)     (ebuff)->data
#define ebuffer_size(ebuff)     (ebuff)->size
#define ebuffer_pos(ebuff)      (ebuff)->pos

/* buffer manipulations */
int     ebuffer_setpos(ebuffer_t* ebuff, size_t pos);
int     ebuffer_set(ebuffer_t* ebuff, const void* value, size_t value_size);
int     ebuffer_append(ebuffer_t* ebuff, const void* value, size_t value_size);
int     ebuffer_append_char(ebuffer_t* ebuff, char value);
int     ebuffer_append_wchar(ebuffer_t* ebuff, ewchar_t value);
char*   ebuffer_append_ptr(ebuffer_t* ebuff, size_t value_size);
int     ebuffer_reserve(ebuffer_t* ebuff, size_t size);
int     ebuffer_copy(ebuffer_t* ebuff_to, ebuffer_t* ebuff_from);

/*----------------------------------------------------------------------*/

#endif /* _EBUFFER_H_ */

