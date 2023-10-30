/*
    HTTP parameter encoding helpers
*/

#include "../elib_config.h"

#include "../text/text_format.h"
#include "../text/text_base64.h"

#include "http_param.h"
#include "http_mime_types.h"
#include "http_header.h"
#include "http_encode.h"

/*----------------------------------------------------------------------*/

/* init */
void http_encode_init(http_encode_t* http_encode)
{
    /* check input */
    EASSERT(http_encode);
    if(http_encode == 0) return;

    /* reset all fields */
    ememset(http_encode, 0, sizeof(http_encode_t));
}

void http_encode_reset(http_encode_t* http_encode)
{
    if(http_encode)
    {
        /* close file if any */
        efile_close(http_encode->input_file);

        /* reset all fields */
        ememset(http_encode, 0, sizeof(http_encode_t));
    }
}

void http_encode_close(http_encode_t* http_encode)
{
    /* same as reset */
    http_encode_reset(http_encode);
}

/* read content */
int _http_encode_begin_file(http_encode_t* http_encode, ebuffer_t* encode_buffer, http_param_t* input_param)
{
    int err;

    /* check name encoding */
    if(http_parameter_is_utf16(input_param))
    {
        /* open */
        err = efile_openw(&http_encode->input_file, (const ewchar_t*)input_param->value, EFILE_OPEN_READ | EFILE_OPEN_EXISTING);
        if(err != ELIBC_SUCCESS)
        {
            ETRACE1("http_encode_begin: failed to open file for reading \"%S\"", (const ewchar_t*)input_param->value);
            return err;
        }

    } else
    {
        /* open */
        err = efile_open(&http_encode->input_file, input_param->value, EFILE_OPEN_READ | EFILE_OPEN_EXISTING);
        if(err != ELIBC_SUCCESS)
        {
            ETRACE1("http_encode_begin: failed to open file for reading \"%s\"", input_param->value);
            return err;
        }
    }

    /* get content type if needed */
    if(http_encode->content_type == 0)
    {
        /* guess content type */
        if(http_parameter_is_utf16(input_param))
            http_encode->content_type = http_get_file_content_typew((const ewchar_t*)input_param->value, 0, encode_buffer);
        else
            http_encode->content_type = http_get_file_content_type(input_param->value, 0);
    }

    /* content size */
    err = efile_size(http_encode->input_file, &http_encode->content_size);
    if(err != ELIBC_SUCCESS) return err;

    return ELIBC_SUCCESS;
}

int http_encode_begin(http_encode_t* http_encode, ebuffer_t* encode_buffer, http_param_t* input_param)
{
    int err;

    /* check input */
    EASSERT(http_encode);
    EASSERT(input_param);
    if(http_encode == 0 || input_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* value must be set */
    EASSERT(input_param->value);
    EASSERT(input_param->value_size);
    if(input_param->value == 0 || input_param->value_size == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset encoder */
    http_encode_reset(http_encode);

    /* copy input */
    http_encode->input_param = input_param;

    /* check parameter type */
    if(http_parameter_is_filename(input_param))
    {
        err = _http_encode_begin_file(http_encode, encode_buffer, input_param);
        if(err != ELIBC_SUCCESS) return err;

    } else
    {
        /* content type must be set */
        EASSERT1(input_param->content_type, "http_encode_begin: content type must be set");
        if(input_param->content_type == 0) return ELIBC_ERROR_ARGUMENT;

        /* validate value */
        if(input_param->value_size != 0 && input_param->value == 0)
        {
            ETRACE("http_encode_begin: parameter value not set");
            return ELIBC_ERROR_ARGUMENT;
        }

        /* content type */
        http_encode->content_type = input_param->content_type;

        /* check if content needs to be encoded */
        if(http_parameter_is_utf16(input_param) && input_param->value_size)
        {
            /* encoded size */
            http_encode->content_size = utf16_in_utf8((const utf16_t*)input_param->value, (size_t)input_param->value_size / sizeof(utf16_t));

        } else
        {
            /* raw size */
            http_encode->content_size = input_param->value_size;
        }
    }

    /* content left to read */
    http_encode->content_left = http_encode->content_size;

    return ELIBC_SUCCESS;
}

int http_encode_begin_chunk(http_encode_t* http_encode, ebuffer_t* encode_buffer, http_param_t* input_param, 
                      euint64_t chunk_offset, euint64_t chunk_length)
{
    int err;

    /* start encoding */
    err = http_encode_begin(http_encode, encode_buffer, input_param);
    if(err != ELIBC_SUCCESS) return err;

    /* ignore if chunk is not set */
    if(chunk_offset == 0 && chunk_length == 0)
    {
        /* chunked ecoding not needed */
        return ELIBC_SUCCESS;
    }

    /* chunked encoding not supported for streams */
    if(http_parameter_is_stream(input_param))
    {
        ETRACE("http_encode_begin_chunk: chunked encoding not supported for streams");
        return ELIBC_ERROR_NOT_SUPPORTED;
    }

    /* validate content size */
    if(chunk_offset >= http_encode->content_size || 
       chunk_offset + chunk_length > http_encode->content_size)
    {
        EASSERT1(0, "http_encode_begin: chunk offsets must be less than content size");
        return ELIBC_ERROR_ARGUMENT;
    }

    /* default values */
    if(chunk_length == 0) 
    {
        chunk_length = http_encode->content_size - chunk_offset;
    }

    /* update offsets if needed */
    if(chunk_offset > 0 && http_encode->input_file)
    {
        /* update file position */
        err = efile_seek(http_encode->input_file, chunk_offset);
        if(err != ELIBC_SUCCESS) return err;
    }

    /* copy offsets */
    http_encode->input_offset = chunk_offset;

    /* update content sizes */
    http_encode->content_size = chunk_length;
    http_encode->content_left = http_encode->content_size;

    return ELIBC_SUCCESS;
}

int http_encode_read(http_encode_t* http_encode, char* buffer, size_t buffer_size, size_t* buffer_used)
{
    size_t input_used;
    int err;

    /* check input */
    EASSERT(http_encode);
    EASSERT(buffer);
    EASSERT(buffer_used);
    EASSERT(buffer_size > 0);
    if(http_encode == 0 || buffer == 0 || buffer_used == 0 || buffer_size == 0) return ELIBC_ERROR_ARGUMENT;

    /* check if there is some content still */
    if(http_encode->content_left == 0) return ELIBC_ERROR_ENDOFFILE;

    /* limit size to content left */
    if(buffer_size > http_encode->content_left)
        buffer_size = (size_t)http_encode->content_left;

    /* read content */
    if(http_encode->input_file)
    {
        /* read content from file */
        err = efile_read(http_encode->input_file, buffer, buffer_size, buffer_used);
                        
        if(err != ELIBC_SUCCESS && err != ELIBC_ERROR_ENDOFFILE) 
        {
            ETRACE("http_encode: failed to read data from file");
            efile_close(http_encode->input_file);
            http_encode->input_file = 0;
            return err;
        }

    } else if(http_parameter_is_stream(http_encode->input_param))
    {
        EASSERT(http_encode->input_param);
        if(http_encode->input_param == 0) return ELIBC_ERROR_INVALID_STATE;

        /* read from stream */
        err = http_encode->input_param->stream_read_func(http_encode->input_param->stream_data, 
                                                         http_encode->input_param->user_id, buffer, buffer_size, buffer_used);
        if(err != ELIBC_SUCCESS) return err;

    } else
    {
        EASSERT(http_encode->input_param);
        if(http_encode->input_param == 0) return ELIBC_ERROR_INVALID_STATE;

        EASSERT(http_encode->input_param->value);
        if(http_encode->input_param->value == 0) return ELIBC_ERROR_INVALID_STATE;

        /* init max size */
        *buffer_used = buffer_size;

        /* check if content needs to be encoded */
        if(http_parameter_is_utf16(http_encode->input_param))
        {
            /* convert */
            input_used = utf16_to_utf8((const utf16_t*)(http_encode->input_param->value + http_encode->input_offset),
                                       (size_t)(http_encode->input_param->value_size - http_encode->input_offset) / sizeof(utf16_t),
                                       (utf8_t*)buffer,
                                       buffer_used);

            /* update offsets */
            http_encode->input_offset += input_used;

        } else
        {
            /* copy */
            ememcpy(buffer, http_encode->input_param->value + http_encode->input_offset, *buffer_used);

            /* update offsets */
            http_encode->input_offset += *buffer_used;
        }
    }

    /* error if nothing has been read */
    if(*buffer_used == 0) return ELIBC_ERROR_ENDOFFILE;

    /* update content left */
    EASSERT(*buffer_used <= http_encode->content_left);
    http_encode->content_left -= *buffer_used;

    return ELIBC_SUCCESS;
}

/* content */
const char* http_encode_content_type(const http_encode_t* http_encode)
{
    EASSERT(http_encode);
    if(http_encode == 0) return 0;

    return http_encode->content_type;
}

euint64_t http_encode_content_size(http_encode_t* http_encode)
{
    EASSERT(http_encode);
    if(http_encode == 0) return 0;

    return http_encode->content_size;
}

/*----------------------------------------------------------------------*/
