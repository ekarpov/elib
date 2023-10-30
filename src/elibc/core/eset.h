/*
    Key value set
*/

#ifndef _ESET_H_
#define _ESET_H_

/*----------------------------------------------------------------------*/

/*
    NOTE: eset is using single buffer to hold all values (values are always 
          copied) so value replacement is not efficient as memory for previous 
          value will still be in use (internally it holds offset in large 
          buffer where value is copied)
*/

/*----------------------------------------------------------------------*/

/* key */
typedef unsigned long   eset_key_t;

/* key value */
typedef struct
{
    const char*     value;
    size_t          value_size;

} eset_value_t;

/* key value set */
typedef struct
{
    earray_t        keys;
    ebuffer_t       values;

} eset_t;

/*----------------------------------------------------------------------*/

/* init and close */
void    eset_init(eset_t* eset);
void    eset_free(eset_t* eset);
void    eset_reset(eset_t* eset);

/* set properties */
size_t  eset_size(eset_t* eset);

/* check if key is present */
ebool_t eset_has_key(eset_t* eset, eset_key_t key);

/* values */
int     eset_get_value(eset_t* eset, eset_key_t key, eset_value_t* value);
int     eset_set_value(eset_t* eset, eset_key_t key, const char* value, size_t value_size);

/*----------------------------------------------------------------------*/

#endif /* _ESET_H_ */

