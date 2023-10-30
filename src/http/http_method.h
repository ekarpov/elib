/*
    HTTP methods
*/

#ifndef _HTTP_METHOD_H_
#define _HTTP_METHOD_H_

/*----------------------------------------------------------------------*/

/*
    NOTE: HTTP methods as defined in:
          https://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html

          RFC7231 (Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content):
          https://tools.ietf.org/html/rfc7231

          RFC5789 (PATCH Method for HTTP):
          https://tools.ietf.org/html/rfc5789
*/

/*----------------------------------------------------------------------*/

/* methods */
typedef enum 
{
    HTTP_METHOD_UNKNOWN     = 0,

    HTTP_METHOD_GET,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_PATCH

} http_method_t;

/*----------------------------------------------------------------------*/

/* parse method */
http_method_t   http_parse_method(const char* method);

/* get method */
const char*     http_method(http_method_t http_method);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_METHOD_H_ */

