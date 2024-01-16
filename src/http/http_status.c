/*
    HTTP status codes
*/

#include "../elib_config.h"

#include "http_status.h"

/*----------------------------------------------------------------------*/

/* status code group */
http_status_group_t http_status_code_group(unsigned short code)
{
    if(code >= 100 && code < 200) return HTTP_STATUS_GROUP_INFORMATIONAL;
    if(code >= 200 && code < 300) return HTTP_STATUS_GROUP_SUCCESS;
    if(code >= 300 && code < 400) return HTTP_STATUS_GROUP_REDIRECTION;
    if(code >= 400 && code < 500) return HTTP_STATUS_GROUP_CLIENT_ERROR;
    if(code >= 500 && code < 600) return HTTP_STATUS_GROUP_SERVER_ERROR;

    ETRACE("Unsupported HTTP status code %d", code);
    return HTTP_STATUS_GROUP_UNKNOWN;
}

/* reason phrase from code */
const char* http_status_reason_phrase(unsigned short code)
{
    switch(code)
    {
    /* Informational */
    case HTTP_STATUS_CONTINUE:                          return "Continue";
    case HTTP_STATUS_SWITCHING_PROTOCOLS:               return "Switching Protocols";
    case HTTP_STATUS_PROCESSING:                        return "Processing";

    /* Success */
    case HTTP_STATUS_OK:                                return "OK";
    case HTTP_STATUS_CREATED:                           return "Created";
    case HTTP_STATUS_ACCEPTED:                          return "Accepted";
    case HTTP_STATUS_NON_AUTHORATIVE_INFORMATION:       return "Non-Authoritative Information";
    case HTTP_STATUS_NO_CONTENT:                        return "No Content";
    case HTTP_STATUS_RESET_CONTENT:                     return "Reset Content";                   
    case HTTP_STATUS_PARTIAL_CONTENT:                   return "Partial Content";
    case HTTP_STATUS_MULTI_STATUS:                      return "Multi-Status";
    case HTTP_STATUS_ALREADY_REPORTED:                  return "Already Reported";
    case HTTP_STATUS_IM_USED:                           return "IM Used";

    /* Redirection */
    case HTTP_STATUS_MILTIPLE_CHOICES:                  return "Multiple Choices";
    case HTTP_STATUS_MOVED_PERMANENTLY:                 return "Moved Permanently";
    case HTTP_STATUS_FOUND:                             return "Found";                           
    case HTTP_STATUS_SEE_OTHER:                         return "See Other";
    case HTTP_STATUS_NOT_MODIFIED:                      return "Not Modified";
    case HTTP_STATUS_USE_PROXY:                         return "Use Proxy";
    case HTTP_STATUS_TEMPORARY_REDIRECT:                return "Temporary Redirect";
    case HTTP_STATUS_PERMANENT_REDIRECT:                return "Permanent Redirect";              

    /* Client Error */
    case HTTP_STATUS_BAD_REQUEST:                       return "Bad Request";
    case HTTP_STATUS_UNAUTHORIZED:                      return "Unauthorized";
    case HTTP_STATUS_PAYMENT_REQUIRED:                  return "Payment Required";
    case HTTP_STATUS_FORBIDDEN:                         return "Forbidden";
    case HTTP_STATUS_NOT_FOUND:                         return "Not Found";
    case HTTP_STATUS_METHOD_NOT_ALLOWED:                return "Method Not Allowed";
    case HTTP_STATUS_NOT_ACCEPTABLE:                    return "Not Acceptable";
    case HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED:     return "Proxy Authentication Required";
    case HTTP_STATUS_REQUEST_TIMEOUT:                   return "Request Timeout";
    case HTTP_STATUS_CONFLICT:                          return "Conflict";
    case HTTP_STATUS_GONE:                              return "Gone";
    case HTTP_STATUS_LENGTH_REQUIRED:                   return "Length Required";
    case HTTP_STATUS_PRECONDITION_FAILED:               return "Precondition Failed";
    case HTTP_STATUS_PAYLOAD_TOO_LRAGE:                 return "Payload Too Large";
    case HTTP_STATUS_URI_TOO_LONG:                      return "URI Too Long";
    case HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE:            return "Unsupported Media Type";
    case HTTP_STATUS_RANGE_NOT_SATISFIABLE:             return "Range Not Satisfiable";
    case HTTP_STATUS_EXPECTATION_FAILED:                return "Expectation Failed";
    case HTTP_STATUS_MISDIRECTED_REQUEST:               return "Misdirected Request";
    case HTTP_STATUS_UNPROCESSABLE_ENTITY:              return "Unprocessable Entity";
    case HTTP_STATUS_LOCKED:                            return "Locked";
    case HTTP_STATUS_FAILED_DEPENDENCY:                 return "Failed Dependency";
    case HTTP_STATUS_UPGRADE_REQUIRED:                  return "Upgrade Required";
    case HTTP_STATUS_PRECONDITION_REQUIRED:             return "Precondition Required";
    case HTTP_STATUS_TOO_MANY_REQUESTS:                 return "Too Many Requests";
    case HTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE:   return "Request Header Fields Too Large";
    case HTTP_STATUS_UNAVAILABLE_FOR_LEGAL_REASONS:     return "Unavailable For Legal Reasons";

    /* Server Error */
    case HTTP_STATUS_INTERNAL_SERVER_ERROR:             return "Internal Server Error";
    case HTTP_STATUS_NOT_IMPLEMENTED:                   return "Not Implemented";
    case HTTP_STATUS_BAD_GATEWAY:                       return "Bad Gateway";
    case HTTP_STATUS_SERVICE_UNAVAILABLE:               return "Service Unavailable";
    case HTTP_STATUS_GATEWAY_TIMEOUT:                   return "Gateway Timeout";
    case HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED:        return "HTTP Version Not Supported";
    case HTTP_STATUS_VARIANT_ALSO_NEGOTIATES:           return "Variant Also Negotiates";
    case HTTP_STATUS_INSUFFICIENT_STORAGE:              return "Insufficient Storage";
    case HTTP_STATUS_LOOP_DETECTED:                     return "Loop Detected";
    case HTTP_STATUS_NOT_EXTENDED:                      return "Not Extended";
    case HTTP_STATUS_NETWORK_AUTHENTICATION_REQUIRED:   return "Network Authentication Required";
    }

    ETRACE("Unsupported HTTP status code %d", code);
    return 0;
}

/*----------------------------------------------------------------------*/
