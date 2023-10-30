/*
 * ELIBC error codes
 */

#ifndef _EERROR_H_
#define _EERROR_H_

/*----------------------------------------------------------------------*/
/* status codes */
#define ELIBC_SUCCESS                               1

/* errors */
#define ELIBC_ERROR_NOT_ENOUGH_MEMORY               -1
#define ELIBC_ERROR_INVALID_NAME                    -2
#define ELIBC_ERROR_ARGUMENT                        -3
#define ELIBC_ERROR_INTERNAL                        -4
#define ELIBC_ERROR_ENDOFFILE                       -5
#define ELIBC_ERROR_FILE_NOT_FOUND                  -6
#define ELIBC_ERROR_NOT_SUPPORTED                   -7
#define ELIBC_ERROR_NOT_IMPLEMENTED                 -8
#define ELIBC_ERROR_AUTHORIZATION                   -9
#define ELIBC_ERROR_PERMISSION                      -10
#define ELIBC_ERROR_CANCELED                        -11
#define ELIBC_ERROR_TIMEOUT                         -12
#define ELIBC_ERROR_NOT_FOUND                       -13
#define ELIBC_ERROR_INVALID_DATA                    -14
#define ELIBC_ERROR_INVALID_STATE                   -15
#define ELIBC_ERROR_INVALID_PASSWORD                -16

/* network errors */
#define ELIBC_ERROR_NETWORK_CONNECTION_LOST         -101
#define ELIBC_ERROR_NETWORK_TIMEOUT                 -102
#define ELIBC_ERROR_NETWORK_HOST_UNREACHABLE        -103
#define ELIBC_ERROR_NETWORK_CONNECTION_REFUSED      -104
#define ELIBC_ERROR_NETWORK_PROXY                   -105
#define ELIBC_ERROR_NETWORK_CERTIFICATE             -106

/* parser errors */
#define ELIBC_ERROR_PARSER_UNKNOWN_ATTRIBUTE        -201
#define ELIBC_ERROR_PARSER_INVALID_INPUT            -202
#define ELIBC_ERROR_PARSER_INVALID_SCHEMA           -203
#define ELIBC_ERROR_PARSER_ATTRIBUTE_TYPE           -204

/*----------------------------------------------------------------------*/
/* continue or stop after error */
#define ELIBC_CONTINUE                              1
#define ELIBC_STOP                                  0

/*----------------------------------------------------------------------*/

#ifdef WIN32

/* convert error to system error */
unsigned long elibc_error_to_win32(int elibc_error_code);
int  win32_to_elibc_error(unsigned long win32_error_code);
long elibc_error_to_hresult(int elibc_error_code);

#else

/* convert error to errno */
// TODO:
unsigned long elibc_error_to_errno(int elibc_error_code);

#endif /* WIN32 */

/*----------------------------------------------------------------------*/

/* get error text message */
const char* elibc_error_text(int elibc_error_code);

/*----------------------------------------------------------------------*/

#endif /* _EERROR_H_ */

