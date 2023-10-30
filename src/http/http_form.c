/*
    HTTP form parameters and helpers
*/

#include "../elib_config.h"

#include "../text/text_format.h"
#include "../text/text_base64.h"

#include "http_param.h"
#include "http_mime_types.h"
#include "http_header.h"
#include "http_misc.h"
#include "http_form.h"

/*----------------------------------------------------------------------*/
/* constants */
#define HTTP_FORM_ENCODE_MINIMUM_BUFFER_SIZE        5

/*----------------------------------------------------------------------*/

#define HTTP_FORM_CONTENT_TYPE_URLENCODED_HEADER    "application/x-www-form-urlencoded"
#define HTTP_FORM_CONTENT_TYPE_MULTIPART_HEADER     "multipart/form-data; boundary="

#define HTTP_FORM_CONTENT_DISPOSITION               "Content-Disposition: form-data; name=\""
#define HTTP_FORM_CONTENT_DISPOSITION_FILENAME      "; filename=\""
#define HTTP_FORM_CONTENT_TYPE                      "Content-Type: "
#define HTTP_FORM_CONTENT_TYPE_TEXT_UTF8            "text/plain; charset=utf-8"
#define HTTP_FORM_CONTENT_TYPE_TEXT_UTF16           "text/plain; charset=utf-16"
#define HTTP_FORM_CONTENT_TRANSFER_ENCODING_BASE64  "Content-Transfer-Encoding: base64"
#define HTTP_FORM_CONTENT_TRANSFER_ENCODING_BINARY  "Content-Transfer-Encoding: binary"

/* encoding flags */
#define HTTP_FORM_ENCODING_PARAM_VALUE              0x0001
#define HTTP_FORM_ENCODING_PARAM_VALUE_RESET        0xFFFE

/*----------------------------------------------------------------------*/

/* init */
void http_form_init(http_form_t* http_form)
{
    /* check input */
    EASSERT(http_form);
    if(http_form == 0) return;

    /* reset all fields */
    ememset(http_form, 0, sizeof(http_form_t));
}

void _http_form_reset_encoder(http_form_t* http_form)
{
    /* close file if any */
    efile_close(http_form->encode_file);
    http_form->encode_file = 0;

    /* reset buffer reference */
    http_form->encode_buffer = 0;

    /* reset all fields */
    http_form->content_type = HTTP_CONTENT_TYPE_URL_ENCODED;
    http_form->content_size = 0;
    http_form->encode_param = 0;
    http_form->encode_offset = 0;
    http_form->encode_flags = 0;

    /* reset boundary */
    ememset(http_form->form_boundary, 0, sizeof(http_form->form_boundary));
    ememset(http_form->form_content, 0, sizeof(http_form->form_content));
}

void http_form_reset(http_form_t* http_form)
{
    if(http_form)
    {
        /* reset encoder */
        _http_form_reset_encoder(http_form);

        /* reset parameters */
        http_form->parameters = 0;
        http_form->parameter_count = 0;
    }
}

void http_form_close(http_form_t* http_form)
{
    if(http_form)
    {
        /* close file if any */
        efile_close(http_form->encode_file);

        /* reset all fields */
        ememset(http_form, 0, sizeof(http_form_t));
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
    if(ebuffer_append(encode_buffer, _str, HTTP_FORM_CONTENT_BOUNDARY_LENGTH) != ELIBC_SUCCESS) return ELIBC_ERROR_NOT_ENOUGH_MEMORY

int _http_form_encode_param_header(http_form_t* http_form, ebuffer_t* encode_buffer, const http_param_t* http_param)
{
    int err;

    /* boundary */
    _HTTP_ENCODE_BUFFER_APPEND("--");
    _HTTP_ENCODE_CONTENT_BOUNDARY(http_form->form_boundary);
    _HTTP_ENCODE_BUFFER_APPEND("\r\n");

    /* content disposition */
    _HTTP_ENCODE_BUFFER_APPEND(HTTP_FORM_CONTENT_DISPOSITION);
    _HTTP_ENCODE_BUFFER_APPEND_STR(http_param->name);
    _HTTP_ENCODE_BUFFER_APPEND("\"");

    /* check if parameter is file name */
    if(http_parameter_is_filename(http_param))
    {
        /* append ';filename=' to content disposition, filename will be url encoded */
        _HTTP_ENCODE_BUFFER_APPEND(HTTP_FORM_CONTENT_DISPOSITION_FILENAME);

        /*
            NOTE: we encode file name as utf8 url-encoded
        */

        /* encode file name */
        err = http_encode_value_buffer(encode_buffer, http_param, HTTP_VALUE_ENCODING_URLENCODE);
        if(err != ELIBC_SUCCESS) return err;

        _HTTP_ENCODE_BUFFER_APPEND("\"");
    }

    _HTTP_ENCODE_BUFFER_APPEND("\r\n");

    /* content encoding */
    if(http_parameter_is_text(http_param))
    {
        /* always send text as utf8 */
        _HTTP_ENCODE_BUFFER_APPEND(HTTP_FORM_CONTENT_TYPE);
        _HTTP_ENCODE_BUFFER_APPEND(HTTP_FORM_CONTENT_TYPE_TEXT_UTF8);
        _HTTP_ENCODE_BUFFER_APPEND("\r\n");

    } else if(http_param->content_type)
    {            
        _HTTP_ENCODE_BUFFER_APPEND(HTTP_FORM_CONTENT_TYPE);
        _HTTP_ENCODE_BUFFER_APPEND_STR(http_param->content_type);
        _HTTP_ENCODE_BUFFER_APPEND("\r\n");
    }

    /* check if we need to add transfer encoding parameters */
    if(http_parameter_is_binary(http_param))
    {
        /* check if we need to encode binary data */
        if(http_param->transfer_encoding == HTTP_TRANSFER_ENCODING_BASE64)
        {
            _HTTP_ENCODE_BUFFER_APPEND(HTTP_FORM_CONTENT_TRANSFER_ENCODING_BASE64);
            _HTTP_ENCODE_BUFFER_APPEND("\r\n");

        } else
        {
            _HTTP_ENCODE_BUFFER_APPEND(HTTP_FORM_CONTENT_TRANSFER_ENCODING_BINARY);
            _HTTP_ENCODE_BUFFER_APPEND("\r\n");
        }
    }

    _HTTP_ENCODE_BUFFER_APPEND("\r\n");

    return ELIBC_SUCCESS;
}

euint64_t _http_form_content_size(http_form_t* http_form, const http_param_t* parameters, size_t parameter_count)
{
    size_t para_idx, name_len;
    euint64_t param_size;
    euint64_t content_size = 0;

    /* loop over all parameters */
    for(para_idx = 0; para_idx < parameter_count; ++para_idx)
    {
        /* check content type */
        if(http_form->content_type == HTTP_CONTENT_TYPE_URL_ENCODED)
        {
            /* parameter value size */
            param_size = http_encoded_value_size(parameters + para_idx, HTTP_VALUE_ENCODING_URLENCODE, ELIBC_FALSE);

            /* name length */
            name_len = estrlen(parameters[para_idx].name);

            /* update size */
            content_size += name_len + param_size + 2; /* &name=value */

        } else
        {
            /* parameter value size */
            if(http_parameter_is_binary(parameters + para_idx))
            {
                /* check if base64 encoding is needed */
                if(parameters[para_idx].transfer_encoding == HTTP_TRANSFER_ENCODING_BASE64)
                    param_size = http_encoded_value_size(parameters + para_idx, HTTP_VALUE_ENCODING_BASE64, ELIBC_TRUE);
                else
                    param_size = http_encoded_value_size(parameters + para_idx, HTTP_VALUE_ENCODING_NONE, ELIBC_TRUE);

            } else
            {
                /* NOTE: this method will handle if utf16 to utf8 conversion is needed, otherwise send text as is */
                param_size = http_encoded_value_size(parameters + para_idx, HTTP_VALUE_ENCODING_NONE, ELIBC_FALSE);
            }

            /* reset buffer */
            ebuffer_reset(http_form->encode_buffer);

            /* encode header to buffer */
            if(_http_form_encode_param_header(http_form, http_form->encode_buffer, parameters + para_idx) != ELIBC_SUCCESS)
            {
                ETRACE("_http_form_content_size: failed to encode param header");
                return 0;
            }

            /* update size (extra 2 is for end of line) */
            content_size += ebuffer_pos(http_form->encode_buffer) + param_size + 2;
        }
    }

    /* boundary at the end */
    if(http_form->content_type == HTTP_CONTENT_TYPE_URL_ENCODED)
    {
        content_size -= 1; /* & is not needed for first parameter */ 

    } else
    {
        content_size += HTTP_FORM_CONTENT_BOUNDARY_LENGTH + 6; /* --boundary--\r\n */
    }

    return content_size;
}

/* encoding */
int http_form_encode_init(http_form_t* http_form, ebuffer_t* encode_buffer,
                          http_param_t* parameters, size_t parameter_count)
{
    size_t para_idx;

    EASSERT(http_form);
    EASSERT(encode_buffer);
    if(http_form == 0 || encode_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset encoder */
    _http_form_reset_encoder(http_form);

    /* encoding buffer reference */
    http_form->encode_buffer = encode_buffer;
    ebuffer_reset(http_form->encode_buffer);

    /* parameters reference */
    http_form->parameters = parameters;
    http_form->parameter_count = parameter_count;

    /* default content */
    http_form->content_type = HTTP_CONTENT_TYPE_URL_ENCODED;
    http_form->content_size = 0;

    /* stop if no parameters set */
    if(http_form->parameters == 0 || http_form->parameter_count == 0) return ELIBC_SUCCESS;

    /* validate and compute content size and type */
    for(para_idx = 0; para_idx < parameter_count; ++para_idx)
    {
        /* check if parameter name is set correctly */
        if(parameters[para_idx].name == 0)
        {
            ETRACE1("http_form_init: form parameter name is not set, user id = %d", parameters[para_idx].user_id);
            return ELIBC_ERROR_ARGUMENT;
        }

        /* check if parameter value is set correctly */
        if(parameters[para_idx].value == 0 || parameters[para_idx].value_size == 0)
        {
            ETRACE1("http_form_init: form parameter value is not set for \"%s\"", parameters[para_idx].name);
            return ELIBC_ERROR_ARGUMENT;
        }

        /* use multipart encoding if any parameter has non-text data */
        if(!http_parameter_is_text(parameters + para_idx))
        {
            /* change content type */
            http_form->content_type = HTTP_CONTENT_TYPE_MULTIPART;
        }

        /* validate stream parameters */
        if(http_parameter_is_stream(parameters + para_idx) && parameters[para_idx].stream_read_func == 0)
        {
            ETRACE1("http_form_init: callback is missing for stream parameter \"%s\"", parameters[para_idx].name);
            return ELIBC_ERROR_ARGUMENT;
        }

        /* check if content type is set for file parameters */
        if(http_parameter_is_filename(parameters + para_idx))
        {
            if(parameters[para_idx].content_type == 0)
            {
                if(parameters[para_idx].value_format == HTTP_FORMAT_FILENAME_UTF8)
                {
                    parameters[para_idx].content_type = http_get_file_content_type(parameters[para_idx].value, 
                                                                                   (size_t)parameters[para_idx].value_size);
                } else if(parameters[para_idx].value_format == HTTP_FORMAT_FILENAME_UTF16)
                {
                    parameters[para_idx].content_type = http_get_file_content_typew((const ewchar_t*)parameters[para_idx].value, 
                                                                                    (size_t)parameters[para_idx].value_size / sizeof(ewchar_t),
                                                                                    encode_buffer);
                }

                if(parameters[para_idx].content_type == 0)
                {
                    ETRACE1("http_form_init: failed ot guess content type from file name for parameter \"%s\"", parameters[para_idx].name);
                    return ELIBC_ERROR_ARGUMENT;
                }
            }
        }

        /* validate that content type is set for binary parameters */
        if(http_parameter_is_binary(parameters + para_idx) && parameters[para_idx].content_type == 0)
        {
            ETRACE1("http_form_init: content type is not set for binary parameter \"%s\"", parameters[para_idx].name);
            return ELIBC_ERROR_ARGUMENT;
        }
    }

    /* init boundary if needed */
    if(http_form->content_type == HTTP_CONTENT_TYPE_MULTIPART)
    {
        size_t header_length = sizeof(HTTP_FORM_CONTENT_TYPE_MULTIPART_HEADER) - 1;

        /* generate random boundary */
        http_format_content_boundary(http_form->form_boundary, HTTP_FORM_CONTENT_BOUNDARY_LENGTH);

        /* must fit */
        EASSERT(header_length + HTTP_FORM_CONTENT_BOUNDARY_LENGTH < HTTP_FORM_CONTENT_TYPE_LENGTH);

        /* format content header */
        ememcpy(http_form->form_content, HTTP_FORM_CONTENT_TYPE_MULTIPART_HEADER, header_length);
        ememcpy(http_form->form_content + header_length, http_form->form_boundary, HTTP_FORM_CONTENT_BOUNDARY_LENGTH);
        http_form->form_content[header_length + HTTP_FORM_CONTENT_BOUNDARY_LENGTH] = 0;
    }

    /* compute content size */
    http_form->content_size = _http_form_content_size(http_form, parameters, parameter_count);

    return ELIBC_SUCCESS;
}

/* url encoding helpers */
int _http_form_encode_urlencoded(http_form_t* http_form, char* buffer, size_t buffer_size, size_t* buffer_used)
{
    size_t buffer_left;
    size_t encoded_size, output_size;
    const http_param_t* http_param;

    /* check if there is something to encode still */
    if(http_form->encode_param >= http_form->parameter_count) return ELIBC_ERROR_ENDOFFILE;

    /* fill data */
    *buffer_used = 0;
    while(*buffer_used < buffer_size)
    {
        buffer_left = buffer_size - *buffer_used;

        /* stop if too small buffer left */
        if(buffer_left < HTTP_FORM_ENCODE_MINIMUM_BUFFER_SIZE) break;

        /* current parameter */
        http_param = http_form->parameters + http_form->encode_param;

        /* check what part we are encoding */
        if(!(http_form->encode_flags & HTTP_FORM_ENCODING_PARAM_VALUE))
        {
            /* append '&' sign if needed */
            if(http_form->encode_offset == 0 && http_form->encode_param > 0)
            {
                buffer[*buffer_used] = '&';
                *buffer_used += 1;
            }

            /* copy name */
            while(*buffer_used < buffer_size && http_param->name[http_form->encode_offset] != 0)
            {
                buffer[*buffer_used] = http_param->name[http_form->encode_offset];
                *buffer_used += 1;
                http_form->encode_offset++;
            }

            /* check if we copied whole name */
            if(http_param->name[http_form->encode_offset] == 0)
            {
                /* switch to value copy */
                http_form->encode_flags |= HTTP_FORM_ENCODING_PARAM_VALUE;
                http_form->encode_offset = 0;
            }

        } else
        {
            /* append '=' sign if needed */
            if(http_form->encode_offset == 0)
            {
                buffer[*buffer_used] = '=';
                *buffer_used += 1;
                buffer_left--;
            }

            /* buffer left */
            output_size = buffer_left;

            /* check if value needs to be converted first from utf16 to utf8 */
            if(http_parameter_is_utf16(http_param))
            {
                /* utf16 to utf8 and url encode */
                encoded_size = utf16_to_utf8url((const utf16_t*)(http_param->value + http_form->encode_offset), 
                    (size_t)(http_param->value_size - http_form->encode_offset) / sizeof(utf16_t), buffer + *buffer_used, &output_size);

                encoded_size *= sizeof(utf16_t);

            } else
            {
                /* url encode */
                encoded_size = url_encode(http_param->value + http_form->encode_offset, 
                    (size_t)(http_param->value_size - http_form->encode_offset), buffer + *buffer_used, &output_size);
            }

            /* update counters */
            *buffer_used += output_size;
            http_form->encode_offset += encoded_size;

            /* check if we copied whole value */
            EASSERT(http_form->encode_offset <= http_param->value_size);
            if(http_form->encode_offset >= http_param->value_size)
            {
                /* switch to name copy */
                http_form->encode_flags &= HTTP_FORM_ENCODING_PARAM_VALUE_RESET;
                http_form->encode_offset = 0;

                /* next parameter */
                http_form->encode_param++;
                if(http_form->encode_param >= http_form->parameter_count) break;
            }

        }
    }

    return ELIBC_SUCCESS;
}

/* form encoding helpers */
int _http_form_encode_prepare_parameter(http_form_t* http_form, const http_param_t* http_param)
{
    int err;

    /* open file if needed */
    if(http_parameter_is_filename(http_param) && http_form->encode_file == 0)
    {
        if(http_parameter_is_utf16(http_param))
            err = efile_openw(&http_form->encode_file, (const ewchar_t*)http_param->value, EFILE_OPEN_READ | EFILE_OPEN_EXISTING);
        else
            err = efile_open(&http_form->encode_file, http_param->value, EFILE_OPEN_READ | EFILE_OPEN_EXISTING);

        if(err != ELIBC_SUCCESS)
        {
            ETRACE("_http_form_encode_multipart: failed to open file for reading");
            return err;
        }
    }

    /* validate that stream callback is set */
    if(http_parameter_is_stream(http_param) && http_param->stream_read_func == 0)
    {
        ETRACE("_http_form_encode_multipart: stream callback not set");
        return ELIBC_ERROR_ARGUMENT;
    }

    return ELIBC_SUCCESS;
}

int _http_form_encode_read_file(http_form_t* http_form, void* buffer, size_t read_size, size_t* data_size)
{
    int err;

    /* read content from file */
    err = efile_read(http_form->encode_file, buffer, read_size, data_size);
                        
    if(err != ELIBC_SUCCESS && err != ELIBC_ERROR_ENDOFFILE) 
    {
        ETRACE("_http_form_encode_multipart: failed to read data from file");
        efile_close(http_form->encode_file);
        http_form->encode_file = 0;
    }

    return err;
}

int _http_form_encode_read_sream(const http_param_t* http_param, void* buffer, size_t read_size, size_t* data_size)
{
    int err;

    /* read stream content */
    err = http_param->stream_read_func(http_param->stream_data, http_param->user_id, buffer, read_size, data_size);

    if(err != ELIBC_SUCCESS && err != ELIBC_ERROR_ENDOFFILE) 
    {
        ETRACE("_http_form_encode_multipart: failed to read data from stream");
    }

    return err;
}

int _http_form_encode_multipart(http_form_t* http_form, char* buffer, size_t buffer_size, size_t* buffer_used)
{
    ebuffer_t* encode_buffer = http_form->encode_buffer;
    size_t buffer_left, header_left, copy_size;
    size_t encoded_size, output_size;
    const http_param_t* http_param;
    int err = 0;

    /* reset counters */
    *buffer_used = 0;

    /* check if there is something to encode still */
    if(http_form->encode_param > http_form->parameter_count) return ELIBC_ERROR_ENDOFFILE;

    /* fill data */
    while(http_form->encode_param < http_form->parameter_count && *buffer_used < buffer_size)
    {
        buffer_left = buffer_size - *buffer_used;

        /* stop if too small buffer left */
        if(buffer_left < HTTP_FORM_ENCODE_MINIMUM_BUFFER_SIZE) break;

        /* current parameter */
        http_param = http_form->parameters + http_form->encode_param;

        /* check what part we are encoding */
        if(!(http_form->encode_flags & HTTP_FORM_ENCODING_PARAM_VALUE))
        {
            /* format parameter header first */
            if(http_form->encode_offset == 0)
            {
                /* reset buffer */
                ebuffer_reset(encode_buffer);

                /* append end of line if needed */
                if(http_form->encode_param > 0)
                {
                    _HTTP_ENCODE_BUFFER_APPEND("\r\n");
                }

                /* encode header to buffer */
                err = _http_form_encode_param_header(http_form, encode_buffer, http_param);
                if(err != ELIBC_SUCCESS) return err;
            }

            /* copy header */
            header_left = ebuffer_pos(encode_buffer) - (size_t)http_form->encode_offset;
            copy_size = (buffer_left > header_left) ? header_left : buffer_left;

            /* copy */
            ememcpy(buffer + *buffer_used, ebuffer_data(encode_buffer) + http_form->encode_offset, copy_size);
            *buffer_used += copy_size;
            http_form->encode_offset += copy_size;

            /* check if we copied whole buffer */
            EASSERT(http_form->encode_offset <= ebuffer_pos(encode_buffer));
            if(http_form->encode_offset >= ebuffer_pos(encode_buffer))
            {
                /* switch to value copy */
                http_form->encode_flags |= HTTP_FORM_ENCODING_PARAM_VALUE;
                http_form->encode_offset = 0;
            }

        } else
        {
            const char* data_ptr = 0;
            size_t data_size = 0;
            int end_of_file = 0;

            /* prepare parameter if needed */
            err = _http_form_encode_prepare_parameter(http_form, http_param);
            if(err != ELIBC_SUCCESS) return err;

            /* init data reference with value itself by default */
            data_ptr = http_param->value;
            data_size = (size_t)http_param->value_size;

            /* if binary data needs to be encoded we need to read content to buffer first */
            if((http_parameter_is_filename(http_param) || http_parameter_is_stream(http_param)) &&
               http_param->transfer_encoding == HTTP_TRANSFER_ENCODING_BASE64)
            {
                /* if previous block has been encoded already we need to read new one */
                if(http_form->encode_offset == 0)
                {
                    size_t read_size = ebuffer_size(encode_buffer);
                        
                    /* for base64 encoding read multiple of 3 */
                    read_size = read_size - (read_size % 3);
                    EASSERT(read_size > 0);

                    /* reset buffer */
                    ebuffer_reset(encode_buffer);

                    if(http_parameter_is_filename(http_param))
                    {
                        /* read content from file */
                        err = _http_form_encode_read_file(http_form, ebuffer_data(encode_buffer), read_size, &data_size);

                    } else
                    {
                        /* read content from stream */
                        err = _http_form_encode_read_sream(http_param, ebuffer_data(encode_buffer), read_size, &data_size);
                    }

                    /* check errors */
                    if(err != ELIBC_SUCCESS && err != ELIBC_ERROR_ENDOFFILE) return err;

                    /* check for end of content */
                    end_of_file = (data_size == 0 || err == ELIBC_ERROR_ENDOFFILE);
                    if(end_of_file) data_size = 0;

                    /* store read size as buffer position */
                    ebuffer_setpos(encode_buffer, data_size);
                }

                /* init data pointer with temporary buffer */
                data_ptr = ebuffer_data(encode_buffer);
                data_size =  ebuffer_pos(encode_buffer);
            }

            /* process data */
            if(http_parameter_is_binary(http_param) && http_param->transfer_encoding == HTTP_TRANSFER_ENCODING_BASE64)
            {
                /* encode data if there is something to encode */
                if(http_form->encode_offset < data_size)
                {
                    /* base64 encode */
                    output_size = buffer_left;
                    encoded_size = base64_encode((const euint8_t*)(data_ptr + http_form->encode_offset),
                        data_size - (size_t)http_form->encode_offset, (euint8_t*)(buffer + *buffer_used), &output_size);
                } else
                {
                    /* no content to encode */
                    output_size = 0;
                    encoded_size = 0;
                }

            } else if(http_parameter_is_filename(http_param) || http_parameter_is_stream(http_param))
            {
                /* read content directly */
                if(http_parameter_is_filename(http_param))
                {
                    /* read file */
                    err = _http_form_encode_read_file(http_form, buffer + *buffer_used, buffer_left, &data_size);

                } else
                {
                    /* read content from stream */
                    err = _http_form_encode_read_sream(http_param, buffer + *buffer_used, buffer_left, &data_size);
                }

                if(err != ELIBC_SUCCESS && err != ELIBC_ERROR_ENDOFFILE) return err;

                /* check for end of content */
                end_of_file = (data_size == 0 || err == ELIBC_ERROR_ENDOFFILE);
                if(end_of_file) data_size = 0;

                output_size = data_size;
                encoded_size = data_size;

            } else if(http_parameter_is_utf16(http_param))
            {
                /* encode utf16 to utf8 */
                encoded_size = utf16_to_utf8((const utf16_t*)(http_param->value + http_form->encode_offset), 
                    (size_t)(http_param->value_size - http_form->encode_offset) / sizeof(utf16_t), (utf8_t*)(buffer + *buffer_used), &output_size);

                encoded_size *= sizeof(utf16_t);

            } else
            {
                /* copy data as is */
                encoded_size = (size_t)(http_param->value_size - http_form->encode_offset);
                if(buffer_left < encoded_size) encoded_size = buffer_left;

                /* copy */
                ememcpy(buffer + *buffer_used, http_param->value + http_form->encode_offset, encoded_size);
                output_size = encoded_size;
            }

            /* update data processed */
            http_form->encode_offset += encoded_size;
            *buffer_used += output_size;

            /* check if we have processed all data */
            EASSERT(http_form->encode_offset <= data_size);
            if(http_form->encode_offset >= data_size)
            {
                /* check if we can read next block still */
                if((http_parameter_is_filename(http_param) || http_parameter_is_stream(http_param)) && !end_of_file)
                {
                    /* reset offset to read next block */
                    http_form->encode_offset = 0;

                } else
                {
                    /* append end of parameter */
                    _HTTP_ENCODE_BUFFER_APPEND("\r\n");

                    /* close file if any */
                    if(http_form->encode_file)
                    {
                        efile_close(http_form->encode_file);
                        http_form->encode_file = 0;
                    }

                    /* switch to name copy */
                    http_form->encode_flags &= HTTP_FORM_ENCODING_PARAM_VALUE_RESET;
                    http_form->encode_offset = 0;

                    /* next parameter */
                    http_form->encode_param++;
                    if(http_form->encode_param >= http_form->parameter_count) break;
                }
            }
        }
    }

    /* encode end of form */
    if(http_form->encode_param == http_form->parameter_count && *buffer_used < buffer_size)
    {
        /* format buffer first */
        if(http_form->encode_offset == 0)
        {
            /* reset buffer */
            ebuffer_reset(encode_buffer);

            /* format */
            _HTTP_ENCODE_BUFFER_APPEND("\r\n--");
            _HTTP_ENCODE_CONTENT_BOUNDARY(http_form->form_boundary);
            _HTTP_ENCODE_BUFFER_APPEND("--\r\n");
        }

        /* buffer size */
        buffer_left = buffer_size - *buffer_used;
        copy_size = ebuffer_pos(encode_buffer) - (size_t)http_form->encode_offset;
        if(buffer_left < copy_size) copy_size = buffer_left;

        /* copy */
        ememcpy(buffer + *buffer_used, ebuffer_data(encode_buffer) + http_form->encode_offset, copy_size);
        
        /* update counters */
        *buffer_used += copy_size;
        http_form->encode_offset += copy_size;

        /* check if we copied whole buffer */
        if(http_form->encode_offset >= ebuffer_pos(encode_buffer))
        {
            http_form->encode_param++;
        }

        return ELIBC_SUCCESS;
    }

    return ELIBC_SUCCESS;
}

int http_form_encode(http_form_t* http_form, char* buffer, size_t buffer_size, size_t* buffer_used)
{
    EASSERT(http_form);
    EASSERT(buffer);
    EASSERT(buffer_used);
    EASSERT(buffer_size > 0);
    if(http_form == 0 || buffer == 0 || buffer_used == 0 ||
       buffer_size == 0 || http_form->encode_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* check minimum buffer size */
    if(buffer_size <= HTTP_FORM_ENCODE_MINIMUM_BUFFER_SIZE)
    {
        ETRACE("http_form_encode: input buffer is too small");
        return ELIBC_ERROR_ARGUMENT;
    }

    /* stop if no parameters set */
    if(http_form->parameters == 0 || http_form->parameter_count == 0)
    {
        *buffer_used = 0;
        return ELIBC_ERROR_ENDOFFILE;
    }

    /* check content type */
    if(http_form->content_type == HTTP_CONTENT_TYPE_URL_ENCODED)
    {
        return _http_form_encode_urlencoded(http_form, buffer, buffer_size, buffer_used);
    } else
    {
        return _http_form_encode_multipart(http_form, buffer, buffer_size, buffer_used);
    }
}

/* content */
const char* http_form_content_type(const http_form_t* http_form)
{
    EASSERT(http_form);
    if(http_form == 0) return 0;

    switch(http_form->content_type)
    {
    case HTTP_CONTENT_TYPE_URL_ENCODED: return HTTP_FORM_CONTENT_TYPE_URLENCODED_HEADER;
    case HTTP_CONTENT_TYPE_MULTIPART: return http_form->form_content;
    }

    EASSERT(0);
    return 0;
}

euint64_t http_form_content_size(http_form_t* http_form)
{
    EASSERT(http_form);
    if(http_form == 0) return 0;

    return http_form->content_size;
}

/*----------------------------------------------------------------------*/
