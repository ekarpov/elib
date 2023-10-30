/*
    HTTP response parameters and helpers
*/

#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

/*----------------------------------------------------------------------*/

/*
    NOTE: HTTP response format as defined in:
          https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html
*/

/*----------------------------------------------------------------------*/

#include "http_param.h"

/*----------------------------------------------------------------------*/

/* http response parameters */
typedef struct
{
    /* headers */
    http_paramset_t         headers;

    /* status code */
    unsigned short          status_code;
    const char*             reason_phrase;

    /* content */
    const char*             content_type;
    euint64_t               content_length;

} http_response_t;

/*----------------------------------------------------------------------*/

/* init */
void http_response_init(http_response_t* http_response);
void http_response_reset(http_response_t* http_response);
void http_response_close(http_response_t* http_response);

/*
    NOTE: do not add Content-Type and Content-Length headers, they will be added based on content
*/

/* headers */
int http_response_headers_append(http_response_t* http_response, http_param_t* http_param, ebool_t copy_value);
int http_response_headers_set_name(http_response_t* http_response, const http_param_t* http_param, ebool_t copy_value);
int http_response_headers_set_id(http_response_t* http_response, unsigned short user_id, const http_param_t* http_param, ebool_t copy_value);
const http_param_t* http_response_headers_find(http_response_t* http_response, unsigned short user_id);

/* status code */
int http_response_set_status(http_response_t* http_response, unsigned short status_code, const char* reason_phrase);

/* content */
int http_response_set_content(http_response_t* http_response, const char* content_type, euint64_t content_length);
euint64_t http_response_content_length(http_response_t* http_response);

/* format http response message header */
int http_response_format_header(http_response_t* http_response, ebuffer_t* encode_buffer);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_RESPONSE_H_ */

