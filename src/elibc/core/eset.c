/*
    Key value set
*/

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "eassert.h"
#include "etrace.h"
#include "eerror.h"
#include "ebuffer.h"
#include "earray.h"
#include "eset.h"

/*----------------------------------------------------------------------*/

/* key index */
typedef struct
{
    int             key;
    size_t          value_offset;
    size_t          value_size;

} eset_keyidx_t;

/*----------------------------------------------------------------------*/

/* helper functions */
eset_keyidx_t*  _eset_find_key_index(eset_t* eset, eset_key_t key);

/*----------------------------------------------------------------------*/

/* init and close */
void eset_init(eset_t* eset)
{
    EASSERT(eset);
    if(eset)
    {
        /* reset all fields */
        ememset(eset, 0, sizeof(eset_t));

        /* init buffers */
        earray_init(&eset->keys, sizeof(eset_keyidx_t));
        ebuffer_init(&eset->values);
    }
}

void eset_reset(eset_t* eset)
{
    EASSERT(eset);
    if(eset)
    {
        /* reset buffers */
        earray_reset(&eset->keys);
        ebuffer_reset(&eset->values);
    }
}

void eset_free(eset_t* eset)
{
    if(eset)
    {
        /* free buffers */
        earray_free(&eset->keys);
        ebuffer_free(&eset->values);
    }
}

/* set properties */
size_t eset_size(eset_t* eset)
{
    EASSERT(eset);
    if(eset == 0) return 0;

    /* return index size */
    return earray_size(&eset->keys);
}

/* check if key is present */
ebool_t eset_has_key(eset_t* eset, eset_key_t key)
{
    EASSERT(eset);
    if(eset == 0) return ELIBC_FALSE;

    /* try to find key index */
    return (_eset_find_key_index(eset, key) != 0) ? ELIBC_TRUE : ELIBC_FALSE;
}

/* values */
int eset_get_value(eset_t* eset, eset_key_t key, eset_value_t* value)
{
    eset_keyidx_t* key_index;

    EASSERT(eset);
    EASSERT(value);
    if(eset == 0 || value == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset return value */
    value->value = 0;
    value->value_size = 0;

    /* try to find index */
    key_index = _eset_find_key_index(eset, key);
    if(key_index != 0)
    {
        /* init value */
        value->value = ebuffer_data(&eset->values) + key_index->value_offset;
        value->value_size = key_index->value_size;

        /* value found */
        return ELIBC_SUCCESS;
    }

    return ELIBC_NOT_FOUND;
}

int eset_set_value(eset_t* eset, eset_key_t key, const char* value, size_t value_size)
{
    eset_keyidx_t* key_index;

    EASSERT(eset);
    if(eset == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset value size if value not set */
    if(value == 0) value_size = 0;

    /* check if key already exists */
    key_index = _eset_find_key_index(eset, key);
    if(key_index == 0)
    {
        /* reserve new index */
        key_index = (eset_keyidx_t*)earray_append_ptr(&eset->keys);
        if(key_index == 0) return ELIBC_ERROR_NOT_ENOUGH_MEMORY; 
        
        /* init key */
        key_index->key = key;
    }

    /* init index */
    key_index->value_offset = ebuffer_pos(&eset->values);
    key_index->value_size = value_size;

    /* copy value if set */
    if(value != 0 && value_size != 0)
    {
        /* append value to buffer */
        int err = ebuffer_append(&eset->values, value, value_size);
        if(err != ELIBC_SUCCESS)
        {
            /* remove index */
            earray_resize(&eset->keys, earray_size(&eset->keys) - 1);

            /* return error */
            return err;
        }
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/

/* helper functions */
eset_keyidx_t*  _eset_find_key_index(eset_t* eset, eset_key_t key)
{
    eset_keyidx_t* indexes;
    size_t index_count, idx;

    indexes = (eset_keyidx_t*)earray_items(&eset->keys);
    index_count = earray_size(&eset->keys);

    /* loop over all items */
    for(idx = 0; idx < index_count; ++idx)
    {
        if(indexes[idx].key == key) return indexes + idx;
    }

    /* not found */
    return 0;
}

/*----------------------------------------------------------------------*/
