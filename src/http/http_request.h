/*
    HTTP request parameters and helpers
*/

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

/*----------------------------------------------------------------------*/

#include "http_param.h"
#include "http_urlformat.h"
#include "http_url.h"
#include "http_form.h"
#include "http_encode.h"
#include "http_content.h"

/*----------------------------------------------------------------------*/

/* http request flags */
#define HTTP_REQUEST_FOLLOW_REDIRECT            0x0001      /* automatically redirect */
#define HTTP_REQUEST_IGNORE_UNTRUSTED_CERT      0x0002      /* allow untrusted certificates */

/*----------------------------------------------------------------------*/

/* http request parameters */
typedef struct
{
    /* properties */
    const char*             url;
    const char*             method;
    unsigned short          flags;

    /* headers */
    http_paramset_t         headers;

    /* content */
    http_content_t          http_content;

} http_request_t;

/*----------------------------------------------------------------------*/

/* init and free request parameters */
void http_request_init(http_request_t* http_request);
void http_request_reset(http_request_t* http_request);
void http_request_close(http_request_t* http_request);

/* headers */
int http_request_headers_append(http_request_t* http_request, http_param_t* http_param, ebool_t copy_value);
int http_request_headers_set(http_request_t* http_request, unsigned short user_id, const http_param_t* http_param, ebool_t copy_value);
http_param_t* http_request_headers_find(http_request_t* http_request, unsigned short user_id);

/* set request content */
int http_request_set_content(http_request_t* http_request, http_content_t* http_content);
int http_request_set_content_iface(http_request_t* http_request, http_content_iface_t* http_content_iface);
int htpp_request_set_content_data(http_request_t* http_request, const char* content_type, const char* content_data, size_t content_size);

/* content headers */
const char* http_request_content_type(const http_request_t* http_request);
euint64_t   http_request_content_size(const http_request_t* http_request);

/* read request body */
int http_request_body_read(http_request_t* http_request, char* buffer, size_t buffer_size, size_t* buffer_used);

/* format request */
int http_request_begin(http_request_t* http_request);
int http_request_read(http_request_t* http_request, char* buffer, size_t buffer_size, size_t* buffer_used);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_REQUEST_H_ */

