/*
    HTTP mixed request body encoder
*/

#include "../elib_config.h"

#include "http_param.h"
#include "http_misc.h"
#include "http_mime_types.h"
#include "http_header.h"
#include "http_encode.h"
#include "http_mixed.h"

/*----------------------------------------------------------------------*/

#define HTTP_MIXED_CONTENT_TYPE                     "multipart/mixed; boundary="
#define HTTP_MIXED_TEXT_CONTENT_TYPE                "text/plain; charset=utf-8"
#define HTTP_MIXED_BINARY_CONTENT_TYPE              "application/octet-stream"

#define HTTP_MIXED_ENCODE_MINIMUM_BUFFER_SIZE       5

/*----------------------------------------------------------------------*/

/* init */
void http_mixed_init(http_mixed_t* http_mixed)
{
    /* check input */
    EASSERT(http_mixed);
    if(http_mixed == 0) return;

    /* reset all fields */
    ememset(http_mixed, 0, sizeof(http_mixed_t));

    /* init encoder */
    http_encode_init(&http_mixed->http_encode);
}

void _http_mixed_reset_encoder(http_mixed_t* http_mixed)
{
    /* reset encoder */
    http_encode_reset(&http_mixed->http_encode);

    /* reset content properties */
    http_mixed->content_type[0] = 0;
    http_mixed->content_size = 0;

    /* reset all fields */
    http_mixed->encode_param = 0;
    http_mixed->encode_offset = 0;
    http_mixed->encode_value = ELIBC_FALSE;
    http_mixed->encode_buffer = 0;

    /* reset boundary */
    ememset(http_mixed->content_boundary, 0, sizeof(http_mixed->content_boundary));
    ememset(http_mixed->content_type, 0, sizeof(http_mixed->content_type));
}

void http_mixed_reset(http_mixed_t* http_mixed)
{
    if(http_mixed)
    {
        /* reset encoder */
        _http_mixed_reset_encoder(http_mixed);

        /* reset parameters */
        http_mixed->parameters = 0;
        http_mixed->parameter_count = 0;
    }
}

void http_mixed_close(http_mixed_t* http_mixed)
{
    if(http_mixed)
    {
        /* close encoder if any */
        http_encode_close(&http_mixed->http_encode);

        /* reset all fields just in case */
        ememset(http_mixed, 0, sizeof(http_mixed_t));
    }
}

/*----------------------------------------------------------------------*/
/* encoding */
/*----------------------------------------------------------------------*/

#define _HTTP_ENCODE_BUFFER_APPEND(_str) \
    if(ebuffer_append(encode_buffer, _str, sizeof(_str) - 1) != ELIBC_SUCCESS) return ELIBC_ERROR_NOT_ENOUGH_MEMORY
#define _HTTP_ENCODE_BUFFER_APPEND_STR(_str) \
    if(ebuffer_append(encode_buffer, _str, estrlen(_str)) != ELIBC_SUCCESS) return ELIBC_ERROR_NOT_ENOUGH_MEMORY
#define _HTTP_ENCODE_CONTENT_BOUNDARY(_str) \
    if(ebuffer_append(encode_buffer, _str, HTTP_CONTENT_BOUNDARY_LENGTH) != ELIBC_SUCCESS) return ELIBC_ERROR_NOT_ENOUGH_MEMORY

/*----------------------------------------------------------------------*/

const char* _http_mixed_param_content_type(ebuffer_t* encode_buffer, http_param_t* http_param)
{
    if(http_param->content_type)
    {
        /* use provided type */
        return http_param->content_type;

    } else if(http_parameter_is_text(http_param))
    {
        /* default text content type */
        http_param->content_type = HTTP_MIXED_TEXT_CONTENT_TYPE;

    } else if(http_parameter_is_filename(http_param))
    {
        /* guess from file name */
        http_param->content_type = http_get_file_content_type_param(http_param, encode_buffer);
    }

    /* check if set */
    if(http_param->content_type == 0)
    {
        /* default binary content type */
        http_param->content_type = HTTP_MIXED_BINARY_CONTENT_TYPE;
    }

    return http_param->content_type;
}

int _http_mixed_encode_value_header(http_mixed_t* http_mixed, ebuffer_t* encode_buffer, http_param_t* http_param)
{
    const char* content_type = 0;

    /* get content type for parameter */
    content_type = _http_mixed_param_content_type(encode_buffer, http_param);

    /* reset buffer */
    ebuffer_reset(encode_buffer);

    /* append boundary (CRLF + "--" + boundary + CRLF) */
    _HTTP_ENCODE_BUFFER_APPEND("\r\n--");
    _HTTP_ENCODE_CONTENT_BOUNDARY(http_mixed->content_boundary);
    _HTTP_ENCODE_BUFFER_APPEND("\r\n");

    /* content type */
    _HTTP_ENCODE_BUFFER_APPEND("Content-Type: ");
    _HTTP_ENCODE_BUFFER_APPEND_STR(content_type);
    _HTTP_ENCODE_BUFFER_APPEND("\r\n");

    /* extra CRLF before content */
    _HTTP_ENCODE_BUFFER_APPEND("\r\n");

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/

int http_mixed_encode_init(http_mixed_t* http_mixed, ebuffer_t* encode_buffer,
                          http_param_t* parameters, size_t parameter_count)
{
    size_t para_idx;
    int err;

    EASSERT(http_mixed);
    EASSERT(encode_buffer);
    if(http_mixed == 0 || encode_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset encoder */
    _http_mixed_reset_encoder(http_mixed);

    /* encoding buffer reference */
    http_mixed->encode_buffer = encode_buffer;
    ebuffer_reset(http_mixed->encode_buffer);

    /* parameters reference */
    http_mixed->parameters = parameters;
    http_mixed->parameter_count = parameter_count;

    /* stop if no parameters set */
    if(http_mixed->parameters == 0 || http_mixed->parameter_count == 0) return ELIBC_SUCCESS;

    /* generate random boundary */
    http_format_content_boundary(http_mixed->content_boundary, sizeof(http_mixed->content_boundary));

    /* just make sure content type will fit */
    EASSERT(sizeof(http_mixed->content_type) >= sizeof(HTTP_MIXED_CONTENT_TYPE) + sizeof(http_mixed->content_boundary));

    /* format content type */
    ememcpy(http_mixed->content_type, HTTP_MIXED_CONTENT_TYPE, sizeof(HTTP_MIXED_CONTENT_TYPE) - 1);
    ememcpy(http_mixed->content_type + (sizeof(HTTP_MIXED_CONTENT_TYPE) - 1), 
            http_mixed->content_boundary, sizeof(http_mixed->content_boundary));
    http_mixed->content_type[(sizeof(HTTP_MIXED_CONTENT_TYPE) - 1) + sizeof(http_mixed->content_boundary)] = 0;

    /* compute content size */
    http_mixed->content_size = 0;

    /* loop over all parameters */
    for(para_idx = 0; para_idx < parameter_count; ++para_idx)
    {
        /* check if parameter value is set correctly */
        if(parameters[para_idx].value == 0 || parameters[para_idx].value_size == 0)
        {
            ETRACE("http_mixed_encode_init: parameter value is not set or empty");
            return ELIBC_ERROR_ARGUMENT;
        }

        /* compute parameter value size */
        http_mixed->content_size += http_encoded_value_size(parameters + para_idx, HTTP_VALUE_ENCODING_NONE, ELIBC_TRUE);

        /* format header */
        err = _http_mixed_encode_value_header(http_mixed, http_mixed->encode_buffer, parameters + para_idx);
        if(err != ELIBC_SUCCESS) return err;

        /* add header size */
        http_mixed->content_size += ebuffer_pos(http_mixed->encode_buffer);
    }

    /* end of content (CRLF--boundary--) */
    http_mixed->content_size += sizeof(http_mixed->content_boundary) + 6;

    return ELIBC_SUCCESS;
}

int http_mixed_encode(http_mixed_t* http_mixed, char* buffer, size_t buffer_size, size_t* buffer_used)
{
    size_t buffer_left, data_left, copy_size;
    http_param_t* http_param;
    int err;

    /* validate input */
    EASSERT(http_mixed);
    EASSERT(buffer);
    EASSERT(buffer_used);
    EASSERT(buffer_size > 0);
    if(http_mixed == 0 || buffer == 0 || buffer_used == 0 ||
       buffer_size == 0 || http_mixed->encode_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* check if there is something to encode still */
    if(http_mixed->encode_param > http_mixed->parameter_count || http_mixed->parameter_count == 0) return ELIBC_ERROR_ENDOFFILE;

    /* buffer size must not be too small */
    if(buffer_size <= HTTP_MIXED_ENCODE_MINIMUM_BUFFER_SIZE)
    {
        ETRACE("http_mixed_encode: input buffer is too small");
        return ELIBC_ERROR_ARGUMENT;
    }

    /* reset counters */
    *buffer_used = 0;

    /* encode data */
    while(http_mixed->encode_param < http_mixed->parameter_count && *buffer_used < buffer_size)
    {
        buffer_left = buffer_size - *buffer_used;

        /* stop if too small buffer left */
        if(buffer_left <= HTTP_MIXED_ENCODE_MINIMUM_BUFFER_SIZE) break;

        /* current parameter */
        http_param = http_mixed->parameters + http_mixed->encode_param;

        /* check what we are encoding */
        if(!http_mixed->encode_value)
        {
            /* check if we have header in buffer */
            if(http_mixed->encode_offset == 0)
            {
                /* format header */
                err = _http_mixed_encode_value_header(http_mixed, http_mixed->encode_buffer, http_param);
                if(err != ELIBC_SUCCESS) return err;
            }

            /* data left */
            data_left = ebuffer_pos(http_mixed->encode_buffer) - (size_t)http_mixed->encode_offset;
            copy_size = (buffer_left > data_left) ? data_left : buffer_left;

            /* copy from buffer */
            ememcpy(buffer + *buffer_used, ebuffer_data(http_mixed->encode_buffer) + http_mixed->encode_offset, copy_size);
            *buffer_used += copy_size;
            http_mixed->encode_offset += copy_size;

            /* check if we copied whole buffer */
            EASSERT(http_mixed->encode_offset <= ebuffer_pos(http_mixed->encode_buffer));
            if(http_mixed->encode_offset >= ebuffer_pos(http_mixed->encode_buffer))
            {
                /* switch to value copy */
                http_mixed->encode_value = ELIBC_TRUE;
                http_mixed->encode_offset = 0;
            }

        } else
        {
            /* check if value encoding has started */
            if(http_mixed->encode_offset == 0)
            {
                /* init encoder */
                err = http_encode_begin(&http_mixed->http_encode, http_mixed->encode_buffer, http_param);
                if(err != ELIBC_SUCCESS) return err;
            }

            copy_size = 0;

            /* encode value */
            err = http_encode_read(&http_mixed->http_encode, buffer + *buffer_used, buffer_left, &copy_size);
            if(err != ELIBC_SUCCESS && err != ELIBC_ERROR_ENDOFFILE) return err;

            /* update offsets */
            *buffer_used += copy_size;
            http_mixed->encode_offset += copy_size;

            /* check if end of content */
            if(err == ELIBC_ERROR_ENDOFFILE)
            {
                /* switch to next parameter */
                http_mixed->encode_param++;
                http_mixed->encode_value = ELIBC_FALSE;
                http_mixed->encode_offset = 0;
            }
        }
    }

    /* encode end boundary */
    if(http_mixed->encode_param == http_mixed->parameter_count && *buffer_used < buffer_size)
    {
        /* format buffer first */
        if(http_mixed->encode_offset == 0)
        {
            ebuffer_t* encode_buffer = http_mixed->encode_buffer;

            /* reset buffer */
            ebuffer_reset(encode_buffer);

            /* format */
            _HTTP_ENCODE_BUFFER_APPEND("\r\n--");
            _HTTP_ENCODE_CONTENT_BOUNDARY(http_mixed->content_boundary);
            _HTTP_ENCODE_BUFFER_APPEND("--");
        }

        /* buffer size */
        buffer_left = buffer_size - *buffer_used;
        copy_size = ebuffer_pos(http_mixed->encode_buffer) - (size_t)http_mixed->encode_offset;
        if(buffer_left < copy_size) copy_size = buffer_left;

        /* copy */
        ememcpy(buffer + *buffer_used, ebuffer_data(http_mixed->encode_buffer) + http_mixed->encode_offset, copy_size);
        
        /* update counters */
        *buffer_used += copy_size;
        http_mixed->encode_offset += copy_size;

        /* check if we copied whole buffer */
        if(http_mixed->encode_offset >= ebuffer_pos(http_mixed->encode_buffer))
        {
            http_mixed->encode_param++;
        }

        return ELIBC_SUCCESS;
    }

    return ELIBC_SUCCESS;
}

/* content */
const char* http_mixed_content_type(const http_mixed_t* http_mixed)
{
    EASSERT(http_mixed);
    if(http_mixed == 0) return 0;

    return http_mixed->content_type;
}

euint64_t http_mixed_content_size(http_mixed_t* http_mixed)
{
    EASSERT(http_mixed);
    if(http_mixed == 0) return 0;

    return http_mixed->content_size;
}

/*----------------------------------------------------------------------*/
