/*
 * ELIBC error codes
 */

#include <errno.h>

#include "../elibc_config.h"
#include "eerror.h"

/*----------------------------------------------------------------------*/

/* convert error to system error */
unsigned long elibc_error_to_win32(int elibc_error_code)
{
    switch(elibc_error_code)
    {
    case ELIBC_SUCCESS:                             return 0;

    /* errors */
    case ELIBC_ERROR_NOT_ENOUGH_MEMORY:             return ENOMEM;
    case ELIBC_ERROR_INVALID_NAME:                  return EINVAL;
    case ELIBC_ERROR_ARGUMENT:                      return EINVAL;
    case ELIBC_ERROR_FILE_NOT_FOUND:                return ENOENT;
    case ELIBC_ERROR_NOT_IMPLEMENTED:               return ENOSYS;
    /*case ELIBC_ERROR_AUTHORIZATION:                 return EAUTH;*/
    case ELIBC_ERROR_PERMISSION:                    return EACCES;
    case ELIBC_ERROR_CANCELED:                      return ECANCELED;
    case ELIBC_ERROR_TIMEOUT:                       return ETIMEDOUT;
    case ELIBC_ERROR_NOT_FOUND:                     return ENOENT;
    case ELIBC_ERROR_INVALID_DATA:                  return EINVAL;
    case ELIBC_ERROR_INVALID_STATE:                 return EINVAL;
    case ELIBC_ERROR_INVALID_PASSWORD:              return EINVAL;
    case ELIBC_ERROR_NOT_SUPPORTED:                 return ENOTSUP;

    /* no mapping */
    /*case ELIBC_ERROR_INTERNAL:                      return ;*/
    /*case ELIBC_ERROR_ENDOFFILE:                     return ;*/

    /* network errors */
    case ELIBC_ERROR_NETWORK_CONNECTION_LOST:       return ECONNABORTED;   
    case ELIBC_ERROR_NETWORK_TIMEOUT:               return ETIMEDOUT;   
    case ELIBC_ERROR_NETWORK_HOST_UNREACHABLE:      return EHOSTUNREACH;
    case ELIBC_ERROR_NETWORK_CONNECTION_REFUSED:    return ECONNREFUSED; 
    /*case ELIBC_ERROR_NETWORK_PROXY:                 return ;*/ 
    /*case ELIBC_ERROR_NETWORK_CERTIFICATE:           return ;*/ 

    /* parser errors are not defined */
    /*case ELIBC_ERROR_PARSER_UNKNOWN_ATTRIBUTE:    return ;*/
    /*case ELIBC_ERROR_PARSER_INVALID_INPUT:        return ;*/
    /*case ELIBC_ERROR_PARSER_INVALID_SCHEMA:       return ;*/
    /*case ELIBC_ERROR_PARSER_ATTRIBUTE_TYPE:       return ;*/
    }

    /* no mapping */
    return ENOTSUP;
}

int  win32_to_elibc_error(unsigned long win32_error_code)
{
    switch(win32_error_code)
    {
    case 0:                                         return ELIBC_SUCCESS;
    
    /* errors */
    case ENOENT:                                    return ELIBC_ERROR_FILE_NOT_FOUND;
    case ENOMEM:                                    return ELIBC_ERROR_NOT_ENOUGH_MEMORY;
    case EINVAL:                                    return ELIBC_ERROR_ARGUMENT;
    case ENOSYS:                                    return ELIBC_ERROR_NOT_IMPLEMENTED;
    case EACCES:                                    return ELIBC_ERROR_PERMISSION;
    case ECANCELED:                                 return ELIBC_ERROR_CANCELED;
    /*case EAUTH:                                     return ELIBC_ERROR_AUTHORIZATION;*/
    case ETIMEDOUT:                                 return ELIBC_ERROR_TIMEOUT;
    case ENOTSUP:                                   return ELIBC_ERROR_NOT_SUPPORTED;

    /* no mapping */
    /*case :                                          return ELIBC_ERROR_INTERNAL;*/
    /*case :                                          return ELIBC_ERROR_ENDOFFILE;*/
    /*case :                                          return ELIBC_ERROR_NOT_FOUND;*/
    /*case :                                          return ELIBC_ERROR_INVALID_DATA;*/
    /*case :                                          return ELIBC_ERROR_INVALID_STATE;*/
    /*case :                                          return ELIBC_ERROR_INVALID_PASSWORD;*/

    /* network errors */
    case ECONNABORTED:                              return ELIBC_ERROR_NETWORK_CONNECTION_LOST;
    /*case ETIMEDOUT:                                 return ELIBC_ERROR_NETWORK_TIMEOUT;*/
    case EHOSTUNREACH:                              return ELIBC_ERROR_NETWORK_HOST_UNREACHABLE;
    case ECONNREFUSED:                              return ELIBC_ERROR_NETWORK_CONNECTION_REFUSED;
    /*case :                                          return ELIBC_ERROR_NETWORK_PROXY;*/
    /*case :                                          return ELIBC_ERROR_NETWORK_CERTIFICATE;*/
    }

    /* no mapping */
    return ELIBC_ERROR_INTERNAL;
}

/*----------------------------------------------------------------------*/
