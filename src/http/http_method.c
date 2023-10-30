/*
    HTTP methods
*/

#include "../elib_config.h"

#include "http_method.h"

/*----------------------------------------------------------------------*/

/* parse method */
http_method_t http_parse_method(const char* method)
{
    EASSERT(method);
    if(method == 0) return HTTP_METHOD_UNKNOWN;

    /* process known methods */
    if(estricmp(method, "GET") == 0)        return HTTP_METHOD_GET;
    if(estricmp(method, "HEAD") == 0)       return HTTP_METHOD_HEAD;
    if(estricmp(method, "POST") == 0)       return HTTP_METHOD_POST;
    if(estricmp(method, "PUT") == 0)        return HTTP_METHOD_PUT;
    if(estricmp(method, "DELETE") == 0)     return HTTP_METHOD_DELETE;
    if(estricmp(method, "CONNECT") == 0)    return HTTP_METHOD_CONNECT;
    if(estricmp(method, "OPTIONS") == 0)    return HTTP_METHOD_OPTIONS;
    if(estricmp(method, "TRACE") == 0)      return HTTP_METHOD_TRACE;
    if(estricmp(method, "PATCH") == 0)      return HTTP_METHOD_PATCH;

    EASSERT1(0, "Unknown HTTP method");
    return HTTP_METHOD_UNKNOWN;
}

/* get method */
const char* http_method(http_method_t http_method)
{
    /* check method */
    switch(http_method)
    {
    case HTTP_METHOD_GET:                   return "GET";
    case HTTP_METHOD_HEAD:                  return "HEAD";
    case HTTP_METHOD_POST:                  return "POST";
    case HTTP_METHOD_PUT:                   return "PUT";
    case HTTP_METHOD_DELETE:                return "DELETE";
    case HTTP_METHOD_CONNECT:               return "CONNECT";
    case HTTP_METHOD_OPTIONS:               return "OPTIONS";
    case HTTP_METHOD_TRACE:                 return "TRACE";
    case HTTP_METHOD_PATCH:                 return "PATCH";
    }

    EASSERT1(0, "Unknown HTTP method");
    return "";
}

/*----------------------------------------------------------------------*/

