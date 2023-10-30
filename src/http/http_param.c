/*
    HTTP request parameters
*/

#include "../elib_config.h"

/* text helpers */
#include "../text/text_base64.h"
#include "../text/text_format.h"

#include "http_param.h"

/*----------------------------------------------------------------------*/
/* parameters */
void http_param_init(http_param_t* http_param)
{
    EASSERT(http_param);
    if(http_param)
    {
        /* reset all fields */
        ememset(http_param, 0, sizeof(http_param_t));

        /* default value data properties */
        http_param->value_format = HTTP_FORMAT_TEXT_UTF8;
        http_param->value_encoding = HTTP_VALUE_ENCODING_NONE;
        http_param->value_is_copy = 0;
        http_param->transfer_encoding = HTTP_TRANSFER_ENCODING_NONE;
    }
}

void http_param_reset(http_param_t* http_param)
{
    EASSERT(http_param);
    if(http_param)
    {
        /* reset all fields */
        ememset(http_param, 0, sizeof(http_param_t));
    }
}

/* parameter properties */
ebool_t http_parameter_is_text(const http_param_t* http_param)
{
    EASSERT(http_param);
    if(http_param == 0) return ELIBC_FALSE;

    /* check format */
    return (http_param->value_format == HTTP_FORMAT_TEXT_UTF8 ||
            http_param->value_format == HTTP_FORMAT_TEXT_UTF16) ? ELIBC_TRUE : ELIBC_FALSE; 
}

ebool_t http_parameter_is_utf16(const http_param_t* http_param)
{
    EASSERT(http_param);
    if(http_param == 0) return ELIBC_FALSE;

    /* check format */
    return (http_param->value_format == HTTP_FORMAT_TEXT_UTF16 ||
            http_param->value_format == HTTP_FORMAT_FILENAME_UTF16) ? ELIBC_TRUE : ELIBC_FALSE; 
}

ebool_t http_parameter_is_filename(const http_param_t* http_param)
{
    EASSERT(http_param);
    if(http_param == 0) return ELIBC_FALSE;

    /* check format */
    return (http_param->value_format == HTTP_FORMAT_FILENAME_UTF8 ||
            http_param->value_format == HTTP_FORMAT_FILENAME_UTF16) ? ELIBC_TRUE : ELIBC_FALSE; 
}

ebool_t http_parameter_is_binary(const http_param_t* http_param)
{
    EASSERT(http_param);
    if(http_param == 0) return ELIBC_FALSE;

    /* check format */
    return (http_param->value_format == HTTP_FORMAT_BINARY_DATA ||
            http_param->value_format == HTTP_FORMAT_STREAM ||
            http_parameter_is_filename(http_param)) ? ELIBC_TRUE : ELIBC_FALSE; 
}

ebool_t http_parameter_is_stream(const http_param_t* http_param)
{
    EASSERT(http_param);
    if(http_param == 0) return ELIBC_FALSE;

    /* check format */
    return (http_param->value_format == HTTP_FORMAT_STREAM) ? ELIBC_TRUE : ELIBC_FALSE; 
}

/* encode name */
int http_encode_name(char* buffer, size_t* output_size, const http_param_t* http_param, http_encoding_t encoding)
{
    size_t name_length;

    EASSERT(buffer);
    EASSERT(http_param);
    if(buffer == 0 || http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* name must be set */
    EASSERT(http_param->name);
    if(http_param->name == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset output size */
    if(output_size) *output_size = 0;

    /* name length */
    name_length = estrlen(http_param->name);

    /* check encoding */
    if(encoding == HTTP_VALUE_ENCODING_NONE)
    {
        /* copy size if needed */
        if(output_size) *output_size = (size_t)name_length;

        /* copy as is */
        ememcpy(buffer, http_param->name, (size_t)name_length);

    } else if(encoding == HTTP_VALUE_ENCODING_URLENCODE)
    {
        if(url_encode(http_param->name, (size_t)name_length, buffer, output_size) != name_length)
        {
            ETRACE("http_encode_name: failed to percent encode value");
            return ELIBC_ERROR_ARGUMENT;
        }

    } else if(encoding == HTTP_VALUE_ENCODING_UTF8_ESCAPED)
    {
        if(utf8_to_utf8esc((const utf8_t*)http_param->name, (size_t)name_length, buffer, output_size) != name_length)
        {
            ETRACE("http_encode_name: failed to utf8 escape value");
            return ELIBC_ERROR_ARGUMENT;
        }

    } else if(encoding == HTTP_VALUE_ENCODING_BASE64)
    {
        /* encode */
        if(base64_encode((const euint8_t*)http_param->name, (size_t)name_length, (euint8_t*)buffer, output_size) != name_length)
        {
            ETRACE("http_encode_name: failed to base64 encode value");
            return ELIBC_ERROR_ARGUMENT;
        }

    } else
    {
        EASSERT1(0, "http_encode_name: unsupported encoding requested");
        return ELIBC_ERROR_ARGUMENT;
    }

    return ELIBC_SUCCESS;
}

int http_encode_name_buffer(ebuffer_t* ebuffer, const http_param_t* http_param, http_encoding_t encoding)
{
    char* encode_ptr;
    size_t name_length;
    size_t encode_size;

    /* check input */
    EASSERT(ebuffer);
    EASSERT(http_param);
    if(ebuffer == 0 || http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* name must be set */
    EASSERT(http_param->name);
    if(http_param->name == 0) return ELIBC_ERROR_ARGUMENT;

    /* name length */
    name_length = estrlen(http_param->name);

    /* get required size */
    if(encoding == HTTP_VALUE_ENCODING_NONE)
    {
        encode_size = name_length;

    } else if(encoding == HTTP_VALUE_ENCODING_URLENCODE)
    {
        encode_size = url_encoded_size(http_param->name, (size_t)name_length); 

    } else if(encoding == HTTP_VALUE_ENCODING_UTF8_ESCAPED)
    {
        encode_size = utf8_in_utf8esc((const utf8_t*)http_param->name, (size_t)name_length); 

    } else if(encoding == HTTP_VALUE_ENCODING_BASE64)
    {
        encode_size = base64_encoded_size(name_length);

    } else
    {
        EASSERT1(0, "http_encode_name: unsupported encoding requested");
        return ELIBC_ERROR_ARGUMENT;
    }

    /* reserve enough space */
    encode_ptr = ebuffer_append_ptr(ebuffer, encode_size); 
    if(encode_ptr == 0) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;

    /* encode to memory buffer */
    return http_encode_name(encode_ptr, 0, http_param, encoding);
}

/* encode value (NOTE: file or stream contents are not encoded, only file name) */
int http_encode_value(char* buffer, size_t* output_size, const http_param_t* http_param, http_encoding_t encoding)
{
    euint64_t value_size;

    EASSERT(buffer);
    EASSERT(http_param);
    if(buffer == 0 || http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* value must be set */
    EASSERT(http_param->value);
    EASSERT(http_param->value_size);
    if(http_param->value == 0 || http_param->value_size == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset output size */
    if(output_size) *output_size = 0;

    /* ignore stream */
    if(http_parameter_is_stream(http_param)) return ELIBC_SUCCESS;

    value_size = http_param->value_size;

    /* file name must end with zero, if so we need to ignore it */
    if(http_parameter_is_filename(http_param))
    {
        if(http_param->value_format == HTTP_FORMAT_FILENAME_UTF8)
            value_size -= sizeof(char);
        else if(http_param->value_format == HTTP_FORMAT_FILENAME_UTF16)
            value_size -= sizeof(ewchar_t);
    }

    /* check if value needs to be encoded */
    if(encoding == HTTP_VALUE_ENCODING_NONE || http_param->value_encoding == encoding)
    {
        /* check if we need to convert utf16 to utf8 */
        if(http_parameter_is_utf16(http_param))
        {
            if(utf16_to_utf8((const utf16_t*)http_param->value, (size_t)value_size / sizeof(utf16_t), (utf8_t*)buffer, output_size) != value_size / sizeof(utf16_t))
            {
                ETRACE("http_encode_value: failed to convert value from utf16 to utf8");
                return ELIBC_ERROR_ARGUMENT;
            }
        } else
        {
            /* copy size if needed */
            if(output_size) *output_size = (size_t)value_size;

            /* copy as is */
            ememcpy(buffer, http_param->value, (size_t)value_size);
        }

    } else if(encoding == HTTP_VALUE_ENCODING_URLENCODE)
    {
        /* check if value needs to be converted first from utf16 to utf8 */
        if(http_parameter_is_utf16(http_param))
        {
            if(utf16_to_utf8url((const utf16_t*)http_param->value, (size_t)value_size / sizeof(utf16_t), buffer, output_size) != value_size / sizeof(utf16_t))
            {
                ETRACE("http_encode_value: failed to percent encode utf16 value");
                return ELIBC_ERROR_ARGUMENT;
            }
        } else
        {
            if(url_encode(http_param->value, (size_t)value_size, buffer, output_size) != value_size)
            {
                ETRACE("http_encode_value: failed to percent encode value");
                return ELIBC_ERROR_ARGUMENT;
            }
        }

    } else if(encoding == HTTP_VALUE_ENCODING_UTF8_ESCAPED)
    {
        /* check if value needs to be converted first from utf16 to utf8 */
        if(http_parameter_is_utf16(http_param))
        {
            if(utf16_to_utf8esc((const utf16_t*)http_param->value, (size_t)value_size / sizeof(utf16_t), buffer, output_size) != value_size / sizeof(utf16_t))
            {
                ETRACE("http_encode_value: failed to utf8 escape encode utf16 value");
                return ELIBC_ERROR_ARGUMENT;
            }
        } else
        {
            if(utf8_to_utf8esc((const utf8_t*)http_param->value, (size_t)value_size, buffer, output_size) != value_size)
            {
                ETRACE("http_encode_value: failed to utf8 escape encode value");
                return ELIBC_ERROR_ARGUMENT;
            }
        }

    } else if(encoding == HTTP_VALUE_ENCODING_BASE64)
    {
        /* encode */
        if(base64_encode((const euint8_t*)http_param->value, (size_t)value_size, (euint8_t*)buffer, output_size) != value_size)
        {
            ETRACE("http_encode_value: failed to base64 encode value");
            return ELIBC_ERROR_ARGUMENT;
        }

    } else
    {
        EASSERT1(0, "http_encode_value: unsupported parameter encoding requested");
        return ELIBC_ERROR_ARGUMENT;
    }

    return ELIBC_SUCCESS;
}

int http_encode_value_buffer(ebuffer_t* ebuffer, const http_param_t* http_param, http_encoding_t encoding)
{
    char* encode_ptr;
    size_t encode_size;

    EASSERT(ebuffer);
    EASSERT(http_param);
    if(ebuffer == 0 || http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* value must be set */
    EASSERT(http_param->value);
    EASSERT(http_param->value_size);
    if(http_param->value == 0 || http_param->value_size == 0) return ELIBC_ERROR_ARGUMENT;

    /* get required size */
    encode_size = (size_t)http_encoded_value_size(http_param, encoding, ELIBC_FALSE);

    /* reserve enough space */
    encode_ptr = ebuffer_append_ptr(ebuffer, encode_size); 
    if(encode_ptr == 0) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;

    /* encode to memory buffer */
    return http_encode_value(encode_ptr, 0, http_param, encoding);
}

/* encoded value size (NOTE: set "use_file" if actual file needs to be used instead of file name) */
euint64_t http_encoded_value_size(const http_param_t* http_param, http_encoding_t encoding, ebool_t use_file)
{
    int err;

    EASSERT(http_param);
    if(http_param == 0) return 0;

    /* value must be set */
    EASSERT(http_param->value);
    EASSERT(http_param->value_size);
    if(http_param->value == 0 || http_param->value_size == 0) return ELIBC_ERROR_ARGUMENT;

    /* check if content is file and we need to use that */
    if(use_file && http_parameter_is_filename(http_param))
    {
        /* get file size */
        euint64_t file_size = 0;
        if(http_parameter_is_utf16(http_param))
        {
            err = efile_size_namew((const ewchar_t*)http_param->value, &file_size);
        } else 
        {
            err = efile_size_name(http_param->value, &file_size);
        }

        if(err != ELIBC_SUCCESS)
        {
            ETRACE("http_value_size: failed to open file name (is value zero terminated?)");
            return 0;
        }

        /* check required encoding */
        if(encoding == HTTP_VALUE_ENCODING_NONE)
        {
            return file_size;

        } else if(encoding == HTTP_VALUE_ENCODING_BASE64)
        {
            /* base64 encoded size */
            return base64_encoded_size(file_size);

        } else
        {
            EASSERT1(0, "http_value_size: not supported binary content encoding");
            return 0;
        }

    } else if(http_parameter_is_stream(http_param))
    {
        /* check required encoding */
        if(encoding == HTTP_VALUE_ENCODING_NONE)
        {
            return http_param->value_size;

        } else if(encoding == HTTP_VALUE_ENCODING_BASE64)
        {
            /* base64 encoded size */
            return base64_encoded_size(http_param->value_size);

        } else
        {
            EASSERT1(0, "http_value_size: not supported stream encoding");
            return 0;
        }

    } else
    {
        euint64_t value_size = http_param->value_size;

        /* file name must end with zero, if so we need to ignore it */
        if(http_parameter_is_filename(http_param))
        {
            if(http_param->value_format == HTTP_FORMAT_FILENAME_UTF8)
                value_size -= sizeof(char);
            else if(http_param->value_format == HTTP_FORMAT_FILENAME_UTF16)
                value_size -= sizeof(ewchar_t);
        }

        /* check if value needs to be encoded */
        if(encoding == HTTP_VALUE_ENCODING_NONE || http_param->value_encoding == encoding)
        {
            /* check if we need to convert utf16 to utf8 */
            if(http_parameter_is_utf16(http_param))
            {
                /* utf16 in utf8 */
                return utf16_in_utf8((const utf16_t*)http_param->value, (int)value_size / sizeof(utf16_t));

            } else 
            {
                /* same size as value size */
                return value_size;
            }

        } else if(encoding == HTTP_VALUE_ENCODING_URLENCODE)
        {
            /* check if utf16 needs to be converted first */
            if(http_parameter_is_utf16(http_param))
            {
                /* utf16 in utf8url encoded */
                return utf16_in_utf8url((const utf16_t*)http_param->value, (int)value_size / sizeof(utf16_t));

            } else
            {
                /* percent encoded size */
                return url_encoded_size(http_param->value, (size_t)value_size);
            }

        } else if(encoding == HTTP_VALUE_ENCODING_UTF8_ESCAPED)
        {
            /* check if utf16 needs to be converted first */
            if(http_parameter_is_utf16(http_param))
            {
                /* utf16 in utf8 escaped */
                return utf16_in_utf8esc((const utf16_t*)http_param->value, (int)value_size / sizeof(utf16_t));

            } else
            {
                /* utf8 escaped */
                return utf8_in_utf8esc((const utf8_t*)http_param->value, (size_t)value_size);
            }

        } else if(encoding == HTTP_VALUE_ENCODING_BASE64)
        {
            /* base64 encoded size */
            return base64_encoded_size(sizeof(euint64_t));
        }
    }

    EASSERT1(0, "http_value_size: unsupported parameter encoding requested");
    return ELIBC_ERROR_ARGUMENT;
}

/* init and free parameters */
void http_paramset_init(http_paramset_t* http_paramset)
{
    /* check input */
    EASSERT(http_paramset);
    if(http_paramset == 0) return;

    /* reset request fields */
    ememset(http_paramset, 0, sizeof(http_paramset_t));

    /* init buffers */
    earray_init(&http_paramset->parameters, sizeof(http_param_t));
    ebuffer_init(&http_paramset->buffer);
}

void http_paramset_reset(http_paramset_t* http_paramset)
{
    size_t idx;

    EASSERT(http_paramset);
    if(http_paramset)
    {
        /* reset parameters first */
        for(idx = 0; idx < http_paramset_size(http_paramset); ++idx)
        {
            http_param_reset(http_paramset_params(http_paramset) + idx);
        }

        /* reset buffers */
        earray_reset(&http_paramset->parameters);
        ebuffer_reset(&http_paramset->buffer);
    }
}

void http_paramset_close(http_paramset_t* http_paramset)
{
    if(http_paramset)
    {
        /* reset parameters first */
        size_t idx;
        for(idx = 0; idx < http_paramset_size(http_paramset); ++idx)
        {
            http_param_reset(http_paramset_params(http_paramset) + idx);
        }

        /* free buffers */
        earray_free(&http_paramset->parameters);
        ebuffer_free(&http_paramset->buffer);
    }
}

/* find parameters */
http_param_t*  http_params_find_id(http_param_t*  http_params, size_t parameter_count,
                                    unsigned short user_id)
{
    size_t idx_val;

    /* check input */
    if(http_params == 0 || parameter_count == 0) return 0;

    /* check if we already have value */
    for(idx_val = 0; idx_val < parameter_count; ++idx_val)
    {
        if(http_params[idx_val].user_id == user_id)
        {
            /* found */
            return http_params + idx_val;
        }
    }

    /* not found */
    return 0;
}

http_param_t* http_params_find_name(http_param_t *http_params, size_t parameter_count,
                                      const char* param_name, size_t name_length)
{
    size_t idx_val;

    /* check input */
    if(http_params == 0 || param_name == 0 || parameter_count == 0) return 0;

    /* check if we already have value */
    for(idx_val = 0; idx_val < parameter_count; ++idx_val)
    {
        /* ignore if name is empty */
        if(http_params[idx_val].name == 0) continue;

        /* compare names */
        if(estrncmp2(http_params[idx_val].name, 0, param_name, name_length) == 0)
        {
            /* found */
            return http_params + idx_val;
        }
    }

    /* not found */
    return 0;
}

/* find parameters from set */
http_param_t*  http_paramset_find_id(http_paramset_t* http_paramset, unsigned short user_id)
{
    /* check input */
    EASSERT(http_paramset);
    if(http_paramset == 0) return 0;

    /* find */
    return http_params_find_id((http_param_t*)earray_items(&(http_paramset)->parameters),
                               earray_size(&(http_paramset)->parameters),
                               user_id);
}

http_param_t*  http_paramset_find_name(http_paramset_t* http_paramset, const char* param_name, size_t name_length)
{
    /* check input */
    EASSERT(http_paramset);
    if(http_paramset == 0) return 0;

    /* find */
    return http_params_find_name((http_param_t*)earray_items(&(http_paramset)->parameters),
                               earray_size(&(http_paramset)->parameters),
                               param_name, name_length);
}

/* manage parameters */
http_param_t* http_paramset_reserve(http_paramset_t* http_paramset)
{
    http_param_t* ret_param = 0;
    earray_t* params_buffer = 0;

    /* check input */
    EASSERT(http_paramset);
    if(http_paramset == 0) return 0;

    /* parameters buffer */
    params_buffer = &http_paramset->parameters;

    /* reserve enough space */
    ret_param = (http_param_t*)earray_append_ptr(params_buffer);
    if(ret_param)
    {
        /* init return parameter */
        http_param_init(ret_param);
    }

    return ret_param;
}

int _http_paramset_param_added(http_paramset_t* http_paramset, http_param_t* http_param, ebool_t copy_value)
{
    ebool_t append_zero = ELIBC_FALSE;
    size_t copy_size;

    /* use last parameter if not provided */
    if(http_param == 0)
    {
        /* get last parameter */
        http_param = http_paramset_params(http_paramset) +
                http_paramset_size(http_paramset) - 1;
    }

    /* check if we need to append end of line */
    if(http_parameter_is_filename(http_param) && http_param->value_size)
    {
        /*
            NOTE: in case of file parameter we need zero terminated file name for file
                  functions to work. So if file name doesn't end with zero we force to 
                  copy value and append extra zero.
        */

        /* check if value ends with zero */
        if(http_parameter_is_utf16(http_param))
        {
            if(((const ewchar_t*)http_param->value)[(http_param->value_size / sizeof(ewchar_t)) - 1] != 0)
            {
                append_zero = ELIBC_TRUE;
                http_param->value_size += sizeof(ewchar_t);
            }

        } else
        {
            if(http_param->value[http_param->value_size - 1] != 0)
            {
                append_zero = ELIBC_TRUE;
                http_param->value_size += 1;
            }
        }

        /* force copy if needed */
        if(append_zero)
            copy_value = ELIBC_TRUE;
    }

    /* copy value if needed */
    if(copy_value)
    {
        /* current buffer reference */
        char* buff_ptr = ebuffer_data(&http_paramset->buffer);

        /* reserve memory */
        char* value_copy = ebuffer_append_ptr(&http_paramset->buffer, (size_t)http_param->value_size);
        if(value_copy == 0) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;

        /*
            NOTE: we are using single buffer to store parameter values, in case buffer memory gets
                  re-allocated we need to make sure that value pointers point to correct offset in the
                  buffer. This is somewhat ugly hack but doing so we can avoid multiple memory allocations
                  and de-allocations while setting http parameters (that happens quite often).
        */

        /* check if buffer has been re-allocated */
        if(buff_ptr && buff_ptr != ebuffer_data(&http_paramset->buffer))
        {
            /* buffer offset */
            size_t buffer_offset = ebuffer_data(&http_paramset->buffer) - buff_ptr;

            /* get items */
            http_param_t* params = http_paramset_params(http_paramset);
            size_t item_count = http_paramset_size(http_paramset);

            /* update all copied values */
            size_t idx;
            for(idx = 0; idx < item_count; ++idx)
            {
                if(params[idx].value_is_copy)
                {
                    /* update value offset */
                    params[idx].value += buffer_offset;
                }
            }
        }

        /* value size */
        copy_size = (size_t)http_param->value_size;

        /* append end of line if needed */
        if(append_zero)
        {
            if(http_parameter_is_utf16(http_param))
            {
                ((ewchar_t*)value_copy)[(http_param->value_size / sizeof(ewchar_t)) - 1] = 0;
                copy_size -= sizeof(ewchar_t);
            }
            else
            {
                value_copy[http_param->value_size - 1] = 0;
                copy_size -= 1;
            }
        }

        /* copy */
        ememcpy(value_copy, http_param->value, copy_size);
        http_param->value = value_copy;
    }

    /* copy flag */
    http_param->value_is_copy = copy_value;

    return ELIBC_SUCCESS;
}

int http_paramset_append(http_paramset_t* http_paramset, const http_param_t* http_param, ebool_t copy_value)
{
    int err;

    /* check input */
    EASSERT(http_paramset);
    EASSERT(http_param);
    if(http_paramset == 0 || http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* append parameter */
    err = earray_append(&http_paramset->parameters, http_param);
    if(err != ELIBC_SUCCESS) return err;

    /* handle added parameter */
    return _http_paramset_param_added(http_paramset, 0, copy_value);
}

/* set parameters by id or name (add new or replace existing) */
int http_paramset_set_name(http_paramset_t* http_paramset, const http_param_t* http_param, ebool_t copy_value)
{
    http_param_t* user_param = 0;

    /* check input */
    EASSERT(http_paramset);
    EASSERT(http_param);
    EASSERT(http_param->name);
    if(http_paramset == 0 || http_param == 0 || http_param->name == 0) return ELIBC_ERROR_ARGUMENT;

    /* try to find parameter by name */
    user_param = (http_param_t*) http_params_find_name(http_paramset_params(http_paramset),
                                                       http_paramset_size(http_paramset), http_param->name, 0);
    if(user_param != 0)
    {
        /* just copy parameter */
        *user_param = *http_param;

        /* handle modified parameter */
        return _http_paramset_param_added(http_paramset, user_param, copy_value);

    } else
    {
        /* append parameter */
        int err = earray_append(&http_paramset->parameters, http_param);
        if(err != ELIBC_SUCCESS) return err;

        /* handle last added parameter */
        return _http_paramset_param_added(http_paramset, 0, copy_value);
    }
}

int http_paramset_set_id(http_paramset_t* http_paramset, unsigned short user_id, const http_param_t* http_param, ebool_t copy_value)
{
    http_param_t* user_param = 0;

    /* check input */
    EASSERT(http_paramset);
    EASSERT(http_param);
    if(http_paramset == 0 || http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* try to find parameter first */
    user_param = (http_param_t*) http_params_find_id(http_paramset_params(http_paramset),
                                                    http_paramset_size(http_paramset), user_id);
    if(user_param != 0)
    {
        /* just copy parameter */
        *user_param = *http_param;

        /* handle modified parameter */
        return _http_paramset_param_added(http_paramset, user_param, copy_value);

    } else
    {
        /* append parameter */
        int err = earray_append(&http_paramset->parameters, http_param);
        if(err != ELIBC_SUCCESS) return err;

        /* handle last added parameter */
        return _http_paramset_param_added(http_paramset, 0, copy_value);
    }
}

/* modify parameters */
int http_paramset_change_name(http_paramset_t* http_paramset, unsigned short user_id, const char* name)
{
    size_t idx;

    /* check input */
    EASSERT(http_paramset);
    EASSERT(name);
    if(http_paramset == 0 || name == 0) return ELIBC_ERROR_ARGUMENT;

    /* find parameter */
    for(idx = 0; idx < earray_size(&http_paramset->parameters); ++idx)
    {
        /* check id */
        if(((http_param_t*)earray_at(&http_paramset->parameters, idx))->user_id == user_id)
        {
            /* replace name */
            ((http_param_t*)earray_at(&http_paramset->parameters, idx))->name = name;

            /* stop */
            return ELIBC_SUCCESS;
        }
    }

    /* not found */
    return ELIBC_ERROR_NOT_FOUND;
}

/* remove parameters */
int http_paramset_remove_id(http_paramset_t* http_paramset, unsigned short user_id)
{
    size_t idx;

    /* check input */
    EASSERT(http_paramset);
    if(http_paramset == 0) return ELIBC_ERROR_ARGUMENT;

    /* find parameter */
    for(idx = 0; idx < earray_size(&http_paramset->parameters); ++idx)
    {
        /* check id */
        if(((http_param_t*)earray_at(&http_paramset->parameters, idx))->user_id == user_id)
        {
            /*
                NOTE: ignore value, even if copy it points to common buffer that will be released at the end
            */

            /* remove */
            return earray_remove(&http_paramset->parameters, idx);
        }
    }

    /* not found */
    return ELIBC_ERROR_NOT_FOUND;
}

/* get parameters */
http_param_t* http_paramset_params(const http_paramset_t* http_paramset)
{
    /* check input */
    EASSERT(http_paramset);
    if(http_paramset == 0) return 0;

    return (http_param_t*)earray_items(&(http_paramset)->parameters);
}

size_t http_paramset_size(const http_paramset_t* http_paramset)       
{
    /* check input */
    EASSERT(http_paramset);
    if(http_paramset == 0) return 0;

    return earray_size(&(http_paramset)->parameters);
}

/*----------------------------------------------------------------------*/
/* encoding helpers */

/* replace %param_name% with parameter value (returns error if parameter not found) */
int http_encode_template(ebuffer_t* ebuffer, const char* tmpl, http_param_t* http_params, 
                         size_t parameter_count, ebool_t encode)
{
    int in_param = 0;
    const char* param_name = 0;
    size_t param_len = 0;
    int err;

    /* check input */
    EASSERT(ebuffer);
    EASSERT(tmpl);
    if(ebuffer == 0 || tmpl == 0) return ELIBC_ERROR_ARGUMENT;

    /* loop over template */
    for(; *tmpl != 0;  ++tmpl)
    {
        if(in_param)
        {
            /* check if parameter ends */
            if(*tmpl == '%')
            {
                /* find parameter by name */
                const http_param_t* http_param = http_params_find_name(http_params,
                            parameter_count, param_name, param_len);

                /* check if found */
                if(http_param && http_param->value != 0 && http_param->value_size > 0)
                {
                    /* replace with parameter value */
                    if(encode)
                        err = http_encode_value_buffer(ebuffer, http_param, HTTP_VALUE_ENCODING_URLENCODE);
                    else
                        err = http_encode_value_buffer(ebuffer, http_param, HTTP_VALUE_ENCODING_NONE);

                } else
                {
                    ETRACE("http_encode_template: template parameter not found");
                    return ELIBC_ERROR_NOT_FOUND;
                }

                /* reset parameter flag */
                in_param = 0;

            } else
            {
                ++param_len;
            }

        } else
        {
            /* check if parameter starts */
            if(*tmpl == '%')
            {
                /* init parameter name */
                param_name = tmpl + 1;
                param_len = 0;
                in_param = 1;

            } else
            {
                /* append char */
                err = ebuffer_append_char(ebuffer, *tmpl); 
                if(err != ELIBC_SUCCESS) return 0;
            }
        }           
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
