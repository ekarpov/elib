/*
    HTTP status codes
*/

#ifndef _HTTP_STATUS_H_
#define _HTTP_STATUS_H_

/*----------------------------------------------------------------------*/

/*
    NOTE: HTTP status codes as defined in:
          https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html

          HTTP Status Codes:
          http://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
*/

/*----------------------------------------------------------------------*/
/* status codes */

/* status codes */
typedef enum 
{
    HTTP_STATUS_UNKNOWN                         = 0,

    /* Informational */
    HTTP_STATUS_CONTINUE                        = 100,
    HTTP_STATUS_SWITCHING_PROTOCOLS             = 101,
    HTTP_STATUS_PROCESSING                      = 102,

    /* Success */
    HTTP_STATUS_OK                              = 200,
    HTTP_STATUS_CREATED                         = 201,
    HTTP_STATUS_ACCEPTED                        = 202,
    HTTP_STATUS_NON_AUTHORATIVE_INFORMATION     = 203,
    HTTP_STATUS_NO_CONTENT                      = 204,
    HTTP_STATUS_RESET_CONTENT                   = 205,
    HTTP_STATUS_PARTIAL_CONTENT                 = 206,
    HTTP_STATUS_MULTI_STATUS                    = 207,
    HTTP_STATUS_ALREADY_REPORTED                = 208,
    HTTP_STATUS_IM_USED                         = 226,

    /* Redirection */
    HTTP_STATUS_MILTIPLE_CHOICES                = 300,
    HTTP_STATUS_MOVED_PERMANENTLY               = 301,
    HTTP_STATUS_FOUND                           = 302,
    HTTP_STATUS_SEE_OTHER                       = 303,
    HTTP_STATUS_NOT_MODIFIED                    = 304,
    HTTP_STATUS_USE_PROXY                       = 305,
    HTTP_STATUS_TEMPORARY_REDIRECT              = 307,
    HTTP_STATUS_PERMANENT_REDIRECT              = 308,

    /* Client Error */
    HTTP_STATUS_BAD_REQUEST                     = 400,
    HTTP_STATUS_UNAUTHORIZED                    = 401,
    HTTP_STATUS_PAYMENT_REQUIRED                = 402,
    HTTP_STATUS_FORBIDDEN                       = 403,
    HTTP_STATUS_NOT_FOUND                       = 404,
    HTTP_STATUS_METHOD_NOT_ALLOWED              = 405,
    HTTP_STATUS_NOT_ACCEPTABLE                  = 406,
    HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED   = 407,
    HTTP_STATUS_REQUEST_TIMEOUT                 = 408,
    HTTP_STATUS_CONFLICT                        = 409,
    HTTP_STATUS_GONE                            = 410,
    HTTP_STATUS_LENGTH_REQUIRED                 = 411,
    HTTP_STATUS_PRECONDITION_FAILED             = 412,
    HTTP_STATUS_PAYLOAD_TOO_LRAGE               = 413,
    HTTP_STATUS_URI_TOO_LONG                    = 414,
    HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE          = 415,
    HTTP_STATUS_RANGE_NOT_SATISFIABLE           = 416,
    HTTP_STATUS_EXPECTATION_FAILED              = 417,
    HTTP_STATUS_MISDIRECTED_REQUEST             = 421,
    HTTP_STATUS_UNPROCESSABLE_ENTITY            = 422,
    HTTP_STATUS_LOCKED                          = 423,
    HTTP_STATUS_FAILED_DEPENDENCY               = 424,
    HTTP_STATUS_UPGRADE_REQUIRED                = 426,
    HTTP_STATUS_PRECONDITION_REQUIRED           = 428,
    HTTP_STATUS_TOO_MANY_REQUESTS               = 429,
    HTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    HTTP_STATUS_UNAVAILABLE_FOR_LEGAL_REASONS   = 451,

    /* Server Error */
    HTTP_STATUS_INTERNAL_SERVER_ERROR           = 500,
    HTTP_STATUS_NOT_IMPLEMENTED                 = 501,
    HTTP_STATUS_BAD_GATEWAY                     = 502,
    HTTP_STATUS_SERVICE_UNAVAILABLE             = 503,
    HTTP_STATUS_GATEWAY_TIMEOUT                 = 504,
    HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED      = 505,
    HTTP_STATUS_VARIANT_ALSO_NEGOTIATES         = 506,
    HTTP_STATUS_INSUFFICIENT_STORAGE            = 507,
    HTTP_STATUS_LOOP_DETECTED                   = 508,
    HTTP_STATUS_NOT_EXTENDED                    = 510,
    HTTP_STATUS_NETWORK_AUTHENTICATION_REQUIRED = 511

} http_status_t;

/*----------------------------------------------------------------------*/
/* status code groups */

typedef enum 
{
    HTTP_STATUS_GROUP_UNKNOWN                   = 0,        
    HTTP_STATUS_GROUP_INFORMATIONAL,            /* 1xx: Informational - Request received, continuing process */
    HTTP_STATUS_GROUP_SUCCESS,                  /* 2xx: Success - The action was successfully received, understood, and accepted */
    HTTP_STATUS_GROUP_REDIRECTION,              /* 3xx: Redirection - Further action must be taken in order to complete the request */
    HTTP_STATUS_GROUP_CLIENT_ERROR,             /* 4xx: Client Error - The request contains bad syntax or cannot be fulfilled */
    HTTP_STATUS_GROUP_SERVER_ERROR              /* 5xx: Server Error - The server failed to fulfill an apparently valid request */

} http_status_group_t;

/*----------------------------------------------------------------------*/

/* status code group */
http_status_group_t http_status_code_group(unsigned short code);

/* reason phrase from code */
const char* http_status_reason_phrase(unsigned short code);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_STATUS_H_ */

