/*
 * ELIBC error codes
 */

#include "../elibc_config.h"
#include "eassert.h"
#include "eerror.h"

/*----------------------------------------------------------------------*/

/* get error text message */
const char* elibc_error_text(int elibc_error_code)
{
    switch(elibc_error_code)
    {
    case ELIBC_SUCCESS:                             return "No error";

    /* errors */
    case ELIBC_ERROR_NOT_ENOUGH_MEMORY:             return "Not enough memory";
    case ELIBC_ERROR_INVALID_NAME:                  return "Invalid name";
    case ELIBC_ERROR_ARGUMENT:                      return "Invalid argument(s)";
    case ELIBC_ERROR_INTERNAL:                      return "Internal error";
    case ELIBC_ERROR_ENDOFFILE:                     return "End of file";
    case ELIBC_ERROR_FILE_NOT_FOUND:                return "File not found";
    case ELIBC_ERROR_NOT_IMPLEMENTED:               return "Not implemented";
    case ELIBC_ERROR_AUTHORIZATION:                 return "Authorization failed";
    case ELIBC_ERROR_PERMISSION:                    return "Permission denied";
    case ELIBC_ERROR_CANCELED:                      return "Canceled";
    case ELIBC_ERROR_TIMEOUT:                       return "Timeout";
    case ELIBC_ERROR_NOT_FOUND:                     return "Not found";
    case ELIBC_ERROR_INVALID_DATA:                  return "Invalid data";
    case ELIBC_ERROR_INVALID_STATE:                 return "Invalid state";
    case ELIBC_ERROR_INVALID_PASSWORD:              return "Invalid password";
    case ELIBC_ERROR_NOT_SUPPORTED:                 return "Not supported";

    /* network errors */
    case ELIBC_ERROR_NETWORK_CONNECTION_LOST:       return "Network connection lost";
    case ELIBC_ERROR_NETWORK_TIMEOUT:               return "Network connection timed out";
    case ELIBC_ERROR_NETWORK_HOST_UNREACHABLE:      return "Network host unreachable";
    case ELIBC_ERROR_NETWORK_CONNECTION_REFUSED:    return "Network connection refused";
    case ELIBC_ERROR_NETWORK_PROXY:                 return "Network proxy error";
    case ELIBC_ERROR_NETWORK_CERTIFICATE:           return "Network certificate error";

    /* parser errors are not defined in Windows */
    case ELIBC_ERROR_PARSER_UNKNOWN_ATTRIBUTE:      return "Parser failed with unknown attribute";
    case ELIBC_ERROR_PARSER_INVALID_INPUT:          return "Parser input is not valid";
    case ELIBC_ERROR_PARSER_INVALID_SCHEMA:         return "Parser schema is not valid";
    case ELIBC_ERROR_PARSER_ATTRIBUTE_TYPE:         return "Parser failed with unknown attribute type";
    }

    /* unknown error */
    EASSERT1(0, "Unsupported error code");
    return "Unknown error";
}

/*----------------------------------------------------------------------*/
