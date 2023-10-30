/*
    Memory buffer
*/

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "eassert.h"
#include "etrace.h"
#include "eerror.h"
#include "ebuffer.h"

/*----------------------------------------------------------------------*/

ELIBC_FORCE_INLINE int _ebuffer_append_reserve(ebuffer_t* ebuff, size_t value_size)
{
    size_t reserve_size;

    /* do nothing if size is less than already allocated */
    if(ebuff->pos + value_size <= ebuff->size) return ELIBC_SUCCESS;

    /* find optimal size */
    reserve_size = ebuff->size;
    if(reserve_size == 0) reserve_size = value_size;
    while(reserve_size < ebuff->pos + value_size)
    {
        reserve_size *= 2; /* double buffer size for efficiency */
    }

    /* reserve memory */
    return ebuffer_reserve(ebuff, reserve_size);
}

/* init and close */
void ebuffer_init(ebuffer_t* ebuff)
{
    EASSERT(ebuff);

    /* reset all fields */
    ememset(ebuff, 0, sizeof(ebuffer_t));
}

void ebuffer_reset(ebuffer_t* ebuff)
{
    EASSERT(ebuff);
    if(ebuff)
    {
        /* reset position */
        ebuff->pos = 0;
    }
}

void ebuffer_free(ebuffer_t* ebuff)
{
    /* free buffer */
    if(ebuff)
    {
        efree(ebuff->data);
        ebuff->data = 0;
        ebuff->size = 0;
    }
}

/* buffer manipulations */
int ebuffer_setpos(ebuffer_t* ebuff, size_t pos)
{
    /* check input */
    EASSERT(ebuff);
    if(ebuff == 0) return ELIBC_ERROR_ARGUMENT;

    /* check if we have enough space */
    if(pos > ebuff->size) return ELIBC_ERROR_ARGUMENT;

    /* set new position */
    ebuff->pos = pos;

    return ELIBC_SUCCESS;
}

int ebuffer_set(ebuffer_t* ebuff, const void* value, size_t value_size)
{
    EASSERT(ebuff);
    if(ebuff == 0) return ELIBC_ERROR_ARGUMENT;
    if(value_size != 0 && value == 0)  return ELIBC_ERROR_ARGUMENT;

    /* reset position */
    ebuff->pos = 0;

    /* append value */
    if(value_size)
        return ebuffer_append(ebuff, value, value_size);

    return ELIBC_SUCCESS;
}

int ebuffer_append(ebuffer_t* ebuff, const void* value, size_t value_size)
{
    int err;

    EASSERT(ebuff);
    EASSERT(value);
    if(ebuff == 0 || value == 0) return ELIBC_ERROR_ARGUMENT;

    /* reserve space if needed */
    err = _ebuffer_append_reserve(ebuff, value_size);
    if(err != ELIBC_SUCCESS) return err;

    /* append value */
    ememcpy(ebuff->data + ebuff->pos, value, value_size);

    /* update position */
    ebuff->pos += value_size;

    return ELIBC_SUCCESS;
}

int ebuffer_append_char(ebuffer_t* ebuff, char value)
{
    int err;

    EASSERT(ebuff);
    if(ebuff == 0) return ELIBC_ERROR_ARGUMENT;

    /* reserve space if needed */
    err = _ebuffer_append_reserve(ebuff, 1);
    if(err != ELIBC_SUCCESS) return err;

    /* append value */
    ebuff->data[ebuff->pos] = value;
    ebuff->pos++;

    return ELIBC_SUCCESS;
}

int ebuffer_append_wchar(ebuffer_t* ebuff, ewchar_t value)
{
    return ebuffer_append(ebuff, &value, sizeof(ewchar_t ));
}

char* ebuffer_append_ptr(ebuffer_t* ebuff, size_t value_size)
{
    char* ptr;
    int err;

    EASSERT(ebuff);
    if(ebuff == 0) return 0;

    /* reserve space if needed */
    err = _ebuffer_append_reserve(ebuff, value_size);
    if(err != ELIBC_SUCCESS) return 0;

    /* story return pointer */
    ptr = ebuff->data + ebuff->pos;

    /* update position */
    ebuff->pos += value_size;

    /* return pointer for value */
    return ptr;
}

int ebuffer_reserve(ebuffer_t* ebuff, size_t size)
{
    char* tmp=0;

    EASSERT(ebuff);
    EASSERT(size > 0);
    if(ebuff == 0 || size <= 0) return ELIBC_ERROR_ARGUMENT;

    /* do nothing if size is less than already allocated */
    if(size <= ebuff->size) return ELIBC_SUCCESS;

    /* check if we have memory already */
    if(ebuff->data)
    {
        /* keep old pointer just in case */
        tmp = ebuff->data;

        /* allocate */
        ebuff->data = (char*)erealloc(ebuff->data, size);

    } else 
    {
        /* allocate new buffer */
        ebuff->data = (char*)emalloc(size);
    }

    /* check memory */
    if(ebuff->data == 0)
    {
        /* restore buffer */
        ebuff->data = tmp;

        /* memory allocation failed */
        ETRACE("ebuffer_reserve failed");
        return ELIBC_ERROR_NOT_ENOUGH_MEMORY;
    }

    /* use new size */
    ebuff->size = size;

    return ELIBC_SUCCESS;
}

int ebuffer_copy(ebuffer_t* ebuff_to, ebuffer_t* ebuff_from)
{
    EASSERT(ebuff_to);
    EASSERT(ebuff_from);
    if(ebuff_to == 0 || ebuff_from == 0) return ELIBC_ERROR_ARGUMENT;

    /* copy data */
    return ebuffer_set(ebuff_to, ebuff_from->data, ebuff_from->pos);
}

/*----------------------------------------------------------------------*/
