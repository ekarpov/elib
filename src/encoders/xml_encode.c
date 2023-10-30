/*
    XML encoder
*/

#include "../elib_config.h"

#include "../http/http_param.h"
#include "xml_encode.h"

/*----------------------------------------------------------------------*/

/* encoder stack item */
typedef struct
{
    // TODO:
    unsigned short          flags;

} xml_encode_stack_item_t;

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/

/* encoder handle */
void xml_encode_init(xml_encoder_t* xml_encoder)
{
    EASSERT(xml_encoder);

    /* reset all fields */
    ememset(xml_encoder, 0, sizeof(xml_encoder_t));

    /* init state stack */
    estack_init(&xml_encoder->state_stack, sizeof(xml_encode_stack_item_t));
}

void xml_encode_reset(xml_encoder_t* xml_encoder)
{
    /* reset encoder */
    if(xml_encoder)
    {
        /* reset state */
        xml_encoder->encode_buffer = 0;

        /* reset stack */
        estack_reset(&xml_encoder->state_stack);
    }
}

void  xml_encode_close(xml_encoder_t* xml_encoder)
{
    /* free buffers */
    if(xml_encoder)
    {
        /* release stack */
        estack_free(&xml_encoder->state_stack);

#ifndef _ELIBC_DEBUG
        /* reset all fields */
        ememset(xml_encoder, 0, sizeof(xml_encoder_t));
#endif /* _ELIBC_DEBUG */
    }
}

/* encode xml content */
int xml_encode_begin(xml_encoder_t* xml_encoder, ebuffer_t* encode_buffer)
{
    /* check input */
    EASSERT(xml_encoder);
    EASSERT(encode_buffer);
    if(xml_encoder == 0 || encode_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset encoder */
    xml_encode_reset(xml_encoder);

    /* copy buffer */
    xml_encoder->encode_buffer = encode_buffer;

    /* TODO: */
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int xml_encode_object_begin(xml_encoder_t* xml_encoder, const char* name)
{
    /* TODO: */
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int xml_encode_object_end(xml_encoder_t* xml_encoder)
{
    /* TODO: */
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int xml_encode_array_begin(xml_encoder_t* xml_encoder, const char* name)
{
    /* TODO: */
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int xml_encode_array_end(xml_encoder_t* xml_encoder)
{
    /* TODO: */
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int xml_encode_value_tag(xml_encoder_t* xml_encoder, const http_param_t* http_param, http_encoding_t encoding)
{
    /* TODO: */
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int xml_encode_value_attribute(xml_encoder_t* xml_encoder, const http_param_t* http_param)
{
    /* TODO: */
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int xml_encode_end(xml_encoder_t* xml_encoder)
{
    /* TODO: */
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------*/
