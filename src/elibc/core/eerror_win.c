/*
 * ELIBC error codes
 */

#include <windows.h>

#include "../elibc_config.h"
#include "eerror.h"

/*
Windows System Error Codes:
http://msdn.microsoft.com/en-us/library/windows/desktop/ms681382(v=vs.85).aspx

WinInet error codes:
https://msdn.microsoft.com/en-us/library/windows/desktop/aa385465(v=vs.85).aspx
*/

/*----------------------------------------------------------------------*/

/* convert error to system error */
unsigned long elibc_error_to_win32(int elibc_error_code)
{
    switch(elibc_error_code)
    {
    case ELIBC_SUCCESS:                             return ERROR_SUCCESS;

    /* errors */
    case ELIBC_ERROR_NOT_ENOUGH_MEMORY:             return ERROR_NOT_ENOUGH_MEMORY;
    case ELIBC_ERROR_INVALID_NAME:                  return ERROR_INVALID_NAME;
    case ELIBC_ERROR_ARGUMENT:                      return ERROR_BAD_ARGUMENTS;
    case ELIBC_ERROR_INTERNAL:                      return ERROR_INTERNAL_ERROR;
    case ELIBC_ERROR_ENDOFFILE:                     return ERROR_HANDLE_EOF;
    case ELIBC_ERROR_FILE_NOT_FOUND:                return ERROR_FILE_NOT_FOUND;
    case ELIBC_ERROR_NOT_IMPLEMENTED:               return ERROR_CALL_NOT_IMPLEMENTED;
    case ELIBC_ERROR_AUTHORIZATION:                 return ERROR_ACCESS_DENIED;
    case ELIBC_ERROR_PERMISSION:                    return ERROR_ACCESS_DENIED;
    case ELIBC_ERROR_CANCELED:                      return ERROR_CANCELLED;
    case ELIBC_ERROR_TIMEOUT:                       return ERROR_TIMEOUT;
    case ELIBC_ERROR_NOT_FOUND:                     return ERROR_NOT_FOUND;
    case ELIBC_ERROR_INVALID_DATA:                  return ERROR_INVALID_DATA;
    case ELIBC_ERROR_INVALID_STATE:                 return ERROR_INVALID_STATE;
    case ELIBC_ERROR_INVALID_PASSWORD:              return ERROR_INVALID_PASSWORD;
    case ELIBC_ERROR_NOT_SUPPORTED:                 return ERROR_NOT_SUPPORTED;

    /* network errors */
    case ELIBC_ERROR_NETWORK_CONNECTION_LOST:       return 12030;   // ERROR_INTERNET_CONNECTION_ABORTED
    case ELIBC_ERROR_NETWORK_TIMEOUT:               return 12002;   // ERROR_INTERNET_TIMEOUT
    case ELIBC_ERROR_NETWORK_HOST_UNREACHABLE:      return ERROR_HOST_UNREACHABLE;
    case ELIBC_ERROR_NETWORK_CONNECTION_REFUSED:    return ERROR_CONNECTION_REFUSED; 
    case ELIBC_ERROR_NETWORK_PROXY:                 return 12165; // ERROR_INTERNET_PROXY_SERVER_UNREACHABLE
    case ELIBC_ERROR_NETWORK_CERTIFICATE:           return 12055; // ERROR_INTERNET_SEC_CERT_ERRORS

    /* parser errors are not defined in Windows */
    /*case ELIBC_ERROR_PARSER_UNKNOWN_ATTRIBUTE:    return ;*/
    /*case ELIBC_ERROR_PARSER_INVALID_INPUT:        return ;*/
    /*case ELIBC_ERROR_PARSER_INVALID_SCHEMA:       return ;*/
    /*case ELIBC_ERROR_PARSER_ATTRIBUTE_TYPE:       return ;*/
    }

    /* no mapping */
    return ERROR_INTERNAL_ERROR;
}

int  win32_to_elibc_error(unsigned long win32_error_code)
{
    switch(win32_error_code)
    {
    case ERROR_SUCCESS:                             return ELIBC_SUCCESS;
    
    /* errors */
    case ERROR_FILE_NOT_FOUND:                      return ELIBC_ERROR_FILE_NOT_FOUND;
    case ERROR_PATH_NOT_FOUND:                      return ELIBC_ERROR_FILE_NOT_FOUND;
    case ERROR_NOT_ENOUGH_MEMORY:                   return ELIBC_ERROR_NOT_ENOUGH_MEMORY;
    case ERROR_INVALID_NAME:                        return ELIBC_ERROR_INVALID_NAME;
    case ERROR_BAD_ARGUMENTS:                       return ELIBC_ERROR_ARGUMENT;
    case ERROR_INTERNAL_ERROR:                      return ELIBC_ERROR_INTERNAL;
    case ERROR_HANDLE_EOF:                          return ELIBC_ERROR_ENDOFFILE;
    case ERROR_CALL_NOT_IMPLEMENTED:                return ELIBC_ERROR_NOT_IMPLEMENTED;
    case ERROR_ACCESS_DENIED:                       return ELIBC_ERROR_PERMISSION;
    case ERROR_CANCELLED:                           return ELIBC_ERROR_CANCELED;
    case ERROR_TIMEOUT:                             return ELIBC_ERROR_TIMEOUT;
    case ERROR_NOT_FOUND:                           return ELIBC_ERROR_NOT_FOUND;
    case ERROR_INVALID_DATA:                        return ELIBC_ERROR_INVALID_DATA;
    case ERROR_INVALID_STATE:                       return ELIBC_ERROR_INVALID_STATE;
    case ERROR_INVALID_PASSWORD:                    return ELIBC_ERROR_INVALID_PASSWORD;
    case ERROR_NOT_SUPPORTED:                       return ELIBC_ERROR_NOT_SUPPORTED;

    /* network errors */
    case 12030:                                     return ELIBC_ERROR_NETWORK_CONNECTION_LOST;
    case 12002:                                     return ELIBC_ERROR_NETWORK_TIMEOUT;
    case ERROR_HOST_UNREACHABLE:                    return ELIBC_ERROR_NETWORK_HOST_UNREACHABLE;
    case ERROR_CONNECTION_REFUSED:                  return ELIBC_ERROR_NETWORK_CONNECTION_REFUSED;
    case 12165:                                     return ELIBC_ERROR_NETWORK_PROXY;
    case 12055:                                     return ELIBC_ERROR_NETWORK_CERTIFICATE;
    }

    /* no mapping */
    return ELIBC_ERROR_INTERNAL;
}

long elibc_error_to_hresult(int elibc_error_code)
{
    return HRESULT_FROM_WIN32(elibc_error_to_win32(elibc_error_code));
}

/*----------------------------------------------------------------------*/
