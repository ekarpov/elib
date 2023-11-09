/*
 *  ELib configuration
 */

#ifndef _ELIB_CONFIG_H_
#define _ELIB_CONFIG_H_

/*----------------------------------------------------------------------*/
/* elibc */
#include "elibc/elibc.h"

/*----------------------------------------------------------------------*/
/* UTF types */
typedef euint8_t        utf8_t;
typedef euint16_t       utf16_t;
typedef euint32_t       utf32_t;

/*----------------------------------------------------------------------*/
/* parse constants */
#define PARSER_DEFAULT_STACK_SIZE                   16

/*----------------------------------------------------------------------*/
/* http constants */
#define HTTP_DEFAULT_RESOURCE_LENGTH                256
#define HTTP_CONTENT_BOUNDARY_LENGTH                36

/*----------------------------------------------------------------------*/
/* version */

#ifndef ELIB_VERSION
#define ELIB_VERSION "unversioned (__DATE__ __TIME__)"
#endif /* ELIB_VERSION */ 

/*----------------------------------------------------------------------*/

#endif /* _ELIB_CONFIG_H_ */

