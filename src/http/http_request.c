/*
    HTTP request parameters and helpers
*/

#include "../elib_config.h"

#include "http_param.h"
#include "http_mime_types.h"
#include "http_header.h"
#include "http_request.h"

/*----------------------------------------------------------------------*/

/* init and free request parameters */
void http_request_init(http_request_t* http_request)
{
    /* check input */
    EASSERT(http_request);
    if(http_request == 0) return;

    /* reset memory */
    ememset(http_request, 0, sizeof(http_request_t));

    /* init headers */
    http_paramset_init(&http_request->headers);
}

void http_request_reset(http_request_t* http_request)
{
    EASSERT(http_request);
    if(http_request)
    {
        /* reset properties */
        http_request->url = 0;
        http_request->method = 0;

        /* reset headers */
        http_paramset_reset(&http_request->headers);

        /* reset content */
        http_content_reset(&http_request->http_content);
    }
}

void http_request_close(http_request_t* http_request)
{
    if(http_request)
    {
        /* free headers */
        http_paramset_close(&http_request->headers);
    }
}

/* headers */
int http_request_headers_append(http_request_t* http_request, http_param_t* http_param, ebool_t copy_value)
{
    /* check input */
    EASSERT(http_request);
    if(http_request == 0) return ELIBC_ERROR_ARGUMENT;

    /* pass to headers */
    return http_headers_append(&http_request->headers, http_param, copy_value);
}

int http_request_headers_set(http_request_t* http_request, unsigned short user_id, const http_param_t* http_param, ebool_t copy_value)
{
    /* check input */
    EASSERT(http_request);
    if(http_request == 0) return ELIBC_ERROR_ARGUMENT;

    /* pass to headers */
    return http_headers_set_id(&http_request->headers, user_id, http_param, copy_value);
}

http_param_t* http_request_headers_find(http_request_t* http_request, unsigned short user_id)
{
    /* check input */
    EASSERT(http_request);
    if(http_request == 0) return 0;

    /* find parameter */
    return http_params_find_id(http_paramset_params(&http_request->headers), 
        http_paramset_size(&http_request->headers), user_id);
}

/* set request content */
int http_request_set_content(http_request_t* http_request, http_content_t* http_content)
{
    /* check input */
    EASSERT(http_request);
    if(http_request == 0) return ELIBC_ERROR_ARGUMENT;

    /* pass to content */
    return http_content_set(&http_request->http_content, http_content);
}

int http_request_set_content_iface(http_request_t* http_request, http_content_iface_t* http_content_iface)
{
    /* check input */
    EASSERT(http_request);
    if(http_request == 0) return ELIBC_ERROR_ARGUMENT;

    /* pass to content */
    return http_content_set_iface(&http_request->http_content, http_content_iface);
}

int htpp_request_set_content_data(http_request_t* http_request, const char* content_type, const char* content_data, size_t content_size)
{
    /* check input */
    EASSERT(http_request);
    if(http_request == 0) return ELIBC_ERROR_ARGUMENT;

    /* pass to content */
    return http_content_set_data(&http_request->http_content, content_type, content_data, content_size);
}

/* request content headers */
const char* http_request_content_type(const http_request_t* http_request)
{
    /* check input */
    EASSERT(http_request);
    if(http_request == 0) return 0;

    /* pass to content */
    return http_content_type(&http_request->http_content);
}

euint64_t http_request_content_size(const http_request_t* http_request)
{
    /* check input */
    EASSERT(http_request);
    if(http_request == 0) return 0;

    /* pass to content */
    return http_content_size(&http_request->http_content);
}

/* read request body */
int http_request_body_read(http_request_t* http_request, char* buffer, size_t buffer_size, size_t* buffer_used)
{
    /* check input */
    EASSERT(http_request);
    if(http_request == 0) return ELIBC_ERROR_ARGUMENT;

    /* pass to content */
    return http_content_read(&http_request->http_content, buffer, buffer_size, buffer_used);
}

/* format request */
int http_request_begin(http_request_t* http_request)
{
    EASSERT1(0, "http_request: request format not implemented");
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

int http_request_read(http_request_t* http_request, char* buffer, size_t buffer_size, size_t* buffer_used)
{
    EASSERT1(0, "http_request: request format not implemented");
    return ELIBC_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------*/

