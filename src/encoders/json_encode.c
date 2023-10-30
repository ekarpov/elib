/*
    JSON encoder
*/

#include "../elib_config.h"

#include "../http/http_param.h"
#include "json_encode.h"

/*----------------------------------------------------------------------*/

/* state flags */
#define JSON_ENCODE_STATE_CONTENT_REPORTED      0x0001      /* some content has been reported */

/*----------------------------------------------------------------------*/

/* encoder state */
typedef enum
{
    json_encode_state_init,
    json_encode_state_object,
    json_encode_state_array

} json_encode_state_t;

/* encoder stack item */
typedef struct
{
    json_encode_state_t     state;
    unsigned short          flags;

} json_encode_stack_item_t;

/*----------------------------------------------------------------------*/
/* helpers */

#define JSON_ENCODE_APPEND_CHAR(ch) \
    err = ebuffer_append_char((json_encoder->encode_buffer), (ch)); \
    if(err != ELIBC_SUCCESS) return err;

#define JSON_ENCODE_APPEND_TEXT(text) \
    err = ebuffer_append((json_encoder->encode_buffer), (text), estrlen((text))); \
    if(err != ELIBC_SUCCESS) return err;

/*----------------------------------------------------------------------*/
/* worker methods */
ELIBC_FORCE_INLINE int _json_encode_push_state(json_encoder_t* json_encoder, json_encode_state_t state)
{
    json_encode_stack_item_t stack_item;

    /* init item */
    stack_item.state = state;
    stack_item.flags = 0;

    /* push state */
    return estack_push(&json_encoder->state_stack, stack_item);
}

ELIBC_FORCE_INLINE int _json_encode_pop_state(json_encoder_t* json_encoder)
{
    EASSERT(estack_size(&json_encoder->state_stack) > 0);
    if(estack_size(&json_encoder->state_stack) == 0) return ELIBC_ERROR_ARGUMENT;

    /* pop state */
    return estack_pop(&json_encoder->state_stack);
}

ELIBC_FORCE_INLINE json_encode_stack_item_t* _json_encode_state_item(json_encoder_t* json_encoder)
{
    EASSERT(estack_size(&json_encoder->state_stack) > 0);
    if(estack_size(&json_encoder->state_stack) > 0) 
    {
        /* return state */
        return (json_encode_stack_item_t*)estack_top(&json_encoder->state_stack);
    }

    return 0;
}

/*----------------------------------------------------------------------*/
/* encoding */
int _json_validate_state(json_encoder_t* json_encoder)
{
    /* check input */
    EASSERT(json_encoder);
    if(json_encoder == 0) return ELIBC_ERROR_ARGUMENT;

    /* check state */
    EASSERT(json_encoder->encode_buffer);
    EASSERT(estack_size(&json_encoder->state_stack));
    if(json_encoder->encode_buffer == 0 || estack_size(&json_encoder->state_stack) == 0) return ELIBC_ERROR_ARGUMENT;

    return ELIBC_SUCCESS;
}

int _json_encode_begin_item(json_encoder_t* json_encoder, const char* name)
{
    int err;

    /* check state */
    err = _json_validate_state(json_encoder);
    if(err != ELIBC_SUCCESS) return err;

    /* report comma if needed */
    if(_json_encode_state_item(json_encoder)->flags & JSON_ENCODE_STATE_CONTENT_REPORTED)
    {
        JSON_ENCODE_APPEND_CHAR(',');
    }

    /* mark flag */
    _json_encode_state_item(json_encoder)->flags |= JSON_ENCODE_STATE_CONTENT_REPORTED;

    /* check if name is set */
    if(name)
    {
        /* encode name */
        JSON_ENCODE_APPEND_CHAR('\"');
        JSON_ENCODE_APPEND_TEXT(name);
        JSON_ENCODE_APPEND_CHAR('\"');
        JSON_ENCODE_APPEND_CHAR(':');
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/

/* encoder handle */
void json_encode_init(json_encoder_t* json_encoder)
{
    EASSERT(json_encoder);

    /* reset all fields */
    ememset(json_encoder, 0, sizeof(json_encoder_t));

    /* init state stack */
    estack_init(&json_encoder->state_stack, sizeof(json_encode_stack_item_t));
}

void json_encode_reset(json_encoder_t* json_encoder)
{
    /* reset encoder */
    if(json_encoder)
    {
        /* reset state */
        json_encoder->encode_buffer = 0;

        /* reset stack */
        estack_reset(&json_encoder->state_stack);
    }
}

void  json_encode_close(json_encoder_t* json_encoder)
{
    /* free buffers */
    if(json_encoder)
    {
        /* release stack */
        estack_free(&json_encoder->state_stack);

#ifndef _ELIBC_DEBUG
        /* reset all fields */
        ememset(json_encoder, 0, sizeof(json_encoder_t));
#endif /* _ELIBC_DEBUG */
    }
}

/* encode json content */
int json_encode_begin(json_encoder_t* json_encoder, ebuffer_t* encode_buffer)
{
    /* check input */
    EASSERT(json_encoder);
    EASSERT(encode_buffer);
    if(json_encoder == 0 || encode_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset encoder */
    json_encode_reset(json_encoder);

    /* copy buffer */
    json_encoder->encode_buffer = encode_buffer;

    /* add initial state */
    return _json_encode_push_state(json_encoder, json_encode_state_init);
}

int json_encode_object_begin(json_encoder_t* json_encoder, const char* name)
{
    int err;

    /* encode value start */
    err = _json_encode_begin_item(json_encoder, name);
    if(err != ELIBC_SUCCESS) return err;

    /* object start */
    JSON_ENCODE_APPEND_CHAR('{');

    /* push object state */
    return _json_encode_push_state(json_encoder, json_encode_state_object);
}

int json_encode_object_end(json_encoder_t* json_encoder)
{
    int err;

    /* check state */
    err = _json_validate_state(json_encoder);
    if(err != ELIBC_SUCCESS) return err;

    /* check stack */
    if(_json_encode_state_item(json_encoder)->state != json_encode_state_object)
    {
        ETRACE("json_encoder: invalid state while reporting object end");
        return ELIBC_ERROR_INVALID_STATE;
    }

    /* object end */
    JSON_ENCODE_APPEND_CHAR('}');

    /* pop state */
    return _json_encode_pop_state(json_encoder);
}

int json_encode_array_begin(json_encoder_t* json_encoder, const char* name)
{
    int err;

    /* encode value start */
    err = _json_encode_begin_item(json_encoder, name);
    if(err != ELIBC_SUCCESS) return err;

    /* array start */
    JSON_ENCODE_APPEND_CHAR('[');

    /* push array state */
    return _json_encode_push_state(json_encoder, json_encode_state_array);
}

int json_encode_array_end(json_encoder_t* json_encoder)
{
    int err;

    /* check state */
    err = _json_validate_state(json_encoder);
    if(err != ELIBC_SUCCESS) return err;

    /* check stack */
    if(_json_encode_state_item(json_encoder)->state != json_encode_state_array)
    {
        ETRACE("json_encoder: invalid state while reporting array end");
        return ELIBC_ERROR_INVALID_STATE;
    }

    /* array end */
    JSON_ENCODE_APPEND_CHAR(']');

    /* pop state */
    return _json_encode_pop_state(json_encoder);
}

int json_encode_value_text(json_encoder_t* json_encoder, const http_param_t* http_param, http_encoding_t encoding)
{
    int err;

    /* check input */
    EASSERT(http_param);
    if(http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* encode value start */
    err = _json_encode_begin_item(json_encoder, http_param->name);
    if(err != ELIBC_SUCCESS) return err;

    JSON_ENCODE_APPEND_CHAR('\"');

    /* encode value */
    err = http_encode_value_buffer(json_encoder->encode_buffer, http_param, encoding);
    if(err != ELIBC_SUCCESS) return err;

    JSON_ENCODE_APPEND_CHAR('\"');

    return ELIBC_SUCCESS;
}

int json_encode_value_number(json_encoder_t* json_encoder, const http_param_t* http_param)
{
    size_t pos, idx;
    int err;

    /* check input */
    EASSERT(http_param);
    if(http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* encode value start */
    err = _json_encode_begin_item(json_encoder, http_param->name);
    if(err != ELIBC_SUCCESS) return err;

    /* save buffer position */
    pos = ebuffer_pos(json_encoder->encode_buffer);

    /* encode value */
    err = http_encode_value_buffer(json_encoder->encode_buffer, http_param, HTTP_VALUE_ENCODING_NONE);
    if(err != ELIBC_SUCCESS) return err;

    /* validate number */
    if(pos < ebuffer_pos(json_encoder->encode_buffer))
    {
        for(idx = pos; idx < ebuffer_pos(json_encoder->encode_buffer); ++idx)
        {
            if(!eisdigit(ebuffer_data(json_encoder->encode_buffer)[pos + idx]))
            {
                ETRACE("json_encoder: invalid number value");
                return ELIBC_ERROR_ARGUMENT;
            }
        }

    } else
    {
        ETRACE("json_encoder: invalid number value (empty)");
        return ELIBC_ERROR_ARGUMENT;
    }

    return ELIBC_SUCCESS;
}

int json_encode_end(json_encoder_t* json_encoder)
{
    int err;

    /* check state */
    err = _json_validate_state(json_encoder);
    if(err != ELIBC_SUCCESS) return err;

    /* encode stack if something left */
    while(estack_size(&json_encoder->state_stack) > 0)
    {
        /* process state */
        if(_json_encode_state_item(json_encoder)->state == json_encode_state_object)
        {
            /* end object */
            err = json_encode_object_end(json_encoder);

        } else if(_json_encode_state_item(json_encoder)->state == json_encode_state_array)
        {
            /* end array */
            err = json_encode_array_end(json_encoder);

        } else 
        {
            /* must be the last */
            EASSERT(estack_size(&json_encoder->state_stack) == 1);
            EASSERT(_json_encode_state_item(json_encoder)->state == json_encode_state_init);

            /* stop */
            break;
        }
    }

    /* reset encoder */
    json_encode_reset(json_encoder);

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
