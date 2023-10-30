/*
    HTTP response parameters and helpers
*/

#include "../elib_config.h"

#include "http_param.h"
#include "http_mime_types.h"
#include "http_header.h"
#include "http_status.h"
#include "http_response.h"

/*----------------------------------------------------------------------*/

#define HTTP_RESPONSE_HTTP_VERSION      "HTTP/1.1"

/* format helpers */
#define _HTTP_RESPONSE_ENCODE_BUFFER_APPEND(_str) \
    if(ebuffer_append(encode_buffer, _str, sizeof(_str) - 1) != ELIBC_SUCCESS) return ELIBC_ERROR_NOT_ENOUGH_MEMORY
#define _HTTP_RESPONSE_ENCODE_BUFFER_APPEND_STR(_str) \
    if(ebuffer_append(encode_buffer, _str, estrlen(_str)) != ELIBC_SUCCESS) return ELIBC_ERROR_NOT_ENOUGH_MEMORY

/*----------------------------------------------------------------------*/

/* init */
void http_response_init(http_response_t* http_response)
{
    /* check input */
    EASSERT(http_response);
    if(http_response == 0) return;

    /* reset memory */
    ememset(http_response, 0, sizeof(http_response_t));

    /* init headers */
    http_paramset_init(&http_response->headers);
}

void http_response_reset(http_response_t* http_response)
{
    EASSERT(http_response);
    if(http_response)
    {
        /* reset headers */
        http_paramset_reset(&http_response->headers);

        /* reset status */
        http_response->status_code = 0;
        http_response->reason_phrase = 0;

        /* reset content */
        http_response->content_type = 0;
        http_response->content_length = 0;
    }
}

void http_response_close(http_response_t* http_response)
{
    if(http_response)
    {
        /* free headers */
        http_paramset_close(&http_response->headers);
    }
}

/* headers */
int http_response_headers_append(http_response_t* http_response, http_param_t* http_param, ebool_t copy_value)
{
    /* check input */
    EASSERT(http_response);
    if(http_response == 0) return ELIBC_ERROR_ARGUMENT;

    /* pass to headers */
    return http_headers_append(&http_response->headers, http_param, copy_value);
}

int http_response_headers_set_name(http_response_t* http_response, const http_param_t* http_param, ebool_t copy_value)
{
    /* check input */
    EASSERT(http_response);
    if(http_response == 0) return ELIBC_ERROR_ARGUMENT;

    /* pass to headers */
    return http_headers_set_name(&http_response->headers, http_param, copy_value);
}

int http_response_headers_set_id(http_response_t* http_response, unsigned short user_id, const http_param_t* http_param, ebool_t copy_value)
{
    /* check input */
    EASSERT(http_response);
    if(http_response == 0) return ELIBC_ERROR_ARGUMENT;

    /* pass to headers */
    return http_headers_set_id(&http_response->headers, user_id, http_param, copy_value);
}

const http_param_t* http_response_headers_find(http_response_t* http_response, unsigned short user_id)
{
    /* check input */
    EASSERT(http_response);
    if(http_response == 0) return 0;

    /* find parameter */
    return http_params_find_id(http_paramset_params(&http_response->headers), 
        http_paramset_size(&http_response->headers), user_id);
}

/* status code */
int http_response_set_status(http_response_t* http_response, unsigned short status_code, const char* reason_phrase)
{
    /* check input */
    EASSERT(http_response);
    if(http_response == 0) return ELIBC_ERROR_ARGUMENT;

    /* copy reference */
    http_response->status_code = status_code;
    http_response->reason_phrase = reason_phrase;

    return ELIBC_SUCCESS;
}

/* content */
int http_response_set_content(http_response_t* http_response, const char* content_type, euint64_t content_length)
{
    /* check input */
    EASSERT(http_response);
    if(http_response == 0) return ELIBC_ERROR_ARGUMENT;

    /* copy reference */
    http_response->content_type = content_type;
    http_response->content_length = content_length;

    return ELIBC_SUCCESS;
}

euint64_t http_response_content_length(http_response_t* http_response)
{
    /* check input */
    EASSERT(http_response);
    if(http_response == 0) return 0;

    /* content length */
    return http_response->content_length;
}

/* format response */
int http_response_format_header(http_response_t* http_response, ebuffer_t* encode_buffer)
{
    const char* status_reason;
    const http_param_t* http_param;
    char buffer[32];  
    size_t idx;
    int err;

    /* check input */
    EASSERT(http_response);
    EASSERT(encode_buffer);
    if(http_response == 0 || encode_buffer == 0) return ELIBC_ERROR_ARGUMENT;
    
    /* check if status code is set */
    if(http_response->status_code == 0)
    {
        ETRACE("http_response: failed to format reponse message header, status code not set");
        return ELIBC_ERROR_ARGUMENT;
    }

    /* status code reason */
    if(http_response->reason_phrase)
    {
        /* use provided phrase */
        status_reason = http_response->reason_phrase;

    } else
    {
        /* use standard phrase */
        status_reason = http_status_reason_phrase(http_response->status_code);
        if(status_reason == 0) return ELIBC_ERROR_ARGUMENT;
    }

    /* format string to buffer (enough to fit 64 bit integer as string) */
    esnprintf(buffer, sizeof(buffer), "%" EPRIu64, http_response->content_length);

    /* set content length header */
    err = http_headers_set(&http_response->headers, HTTP_HEADER_CONTENT_LENGTH, buffer, 0, ELIBC_TRUE);
    if(err != ELIBC_SUCCESS) return err;

    /* content type */
    if(http_response->content_type != 0)
    {
        /* set content type */
        err = http_headers_set(&http_response->headers, HTTP_HEADER_CONTENT_TYPE, http_response->content_type, 0, ELIBC_TRUE);
        if(err != ELIBC_SUCCESS) return err;
    }

    /* reset output buffer */
    ebuffer_reset(encode_buffer);

    /* format status code */
    esnprintf(buffer, sizeof(buffer), "%d", http_response->status_code);

    /* format status line */
    _HTTP_RESPONSE_ENCODE_BUFFER_APPEND(HTTP_RESPONSE_HTTP_VERSION);
    _HTTP_RESPONSE_ENCODE_BUFFER_APPEND(" ");
    _HTTP_RESPONSE_ENCODE_BUFFER_APPEND_STR(buffer);
    _HTTP_RESPONSE_ENCODE_BUFFER_APPEND(" ");
    _HTTP_RESPONSE_ENCODE_BUFFER_APPEND_STR(status_reason);
    _HTTP_RESPONSE_ENCODE_BUFFER_APPEND("\r\n");

    /* format response headers */
    for(idx = 0; idx < http_paramset_size(&http_response->headers); ++idx)
    {
        /* get header */
        http_param = http_paramset_params(&http_response->headers) + idx;

        /* name must be set */
        EASSERT1(http_param->name, "http_response_format: header name not set");
        if(http_param->name == 0) continue;

        /* header name */
        _HTTP_RESPONSE_ENCODE_BUFFER_APPEND_STR(http_param->name);
        _HTTP_RESPONSE_ENCODE_BUFFER_APPEND(": ");

        /* urlencode value */
        err = http_encode_value_buffer(encode_buffer, http_param, HTTP_VALUE_ENCODING_NONE);
        if(err != ELIBC_SUCCESS) return err;

        /* header ending */
        _HTTP_RESPONSE_ENCODE_BUFFER_APPEND("\r\n");
    }
    
    /* ending CRLF */
    _HTTP_RESPONSE_ENCODE_BUFFER_APPEND("\r\n");

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
