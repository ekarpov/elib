/*
    HTTP header helpers
*/

#ifndef _HTTP_HEADER_H_
#define _HTTP_HEADER_H_

/*----------------------------------------------------------------------*/

/*
    NOTE: MIME types for HTTP Content-Type header: http://www.w3.org/Protocols/rfc1341/4_Content-Type.html
          List of MIME types: http://www.freeformatter.com/mime-types-list.html
          Media Types: http://www.iana.org/assignments/media-types/media-types.xhtml
*/

/*----------------------------------------------------------------------*/

/*
    NOTE: list of HTTP headers taken from: https://www.cs.tut.fi/~jkorpela/http.html
*/

/*** HTTP 1.1 headers ***/
typedef enum {

	HTTP_HEADER_UNKNOWN = 0,
	HTTP_HEADER_ACCEPT,
	HTTP_HEADER_ACCEPT_CHARSET,
	HTTP_HEADER_ACCEPT_ENCODING,
	HTTP_HEADER_ACCEPT_LANGUAGE,
	HTTP_HEADER_ACCEPT_RANGES,
	HTTP_HEADER_AGE,
	HTTP_HEADER_ALLOW,
	HTTP_HEADER_AUTHORIZATION,
	HTTP_HEADER_CACHE_CONTROL,
	HTTP_HEADER_CONNECTION,
	HTTP_HEADER_CONTENT_ENCODING,
	HTTP_HEADER_CONTENT_LANGUAGE,
	HTTP_HEADER_CONTENT_LENGTH,
	HTTP_HEADER_CONTENT_LOCATION,
	HTTP_HEADER_CONTENT_MD5,
	HTTP_HEADER_CONTENT_RANGE,
	HTTP_HEADER_CONTENT_TYPE,
	HTTP_HEADER_DATE,
	HTTP_HEADER_ETAG,
	HTTP_HEADER_EXPECT,
	HTTP_HEADER_EXPIRES,
	HTTP_HEADER_FROM,
	HTTP_HEADER_HOST,
	HTTP_HEADER_IF_MATCH,
	HTTP_HEADER_IF_MODIFIED_SINCE,
	HTTP_HEADER_IF_NONE_MATCH,
	HTTP_HEADER_IF_RANGE,
	HTTP_HEADER_IF_UNMODIFIED_SINCE,
	HTTP_HEADER_LAST_MODIFIED,
	HTTP_HEADER_LOCATION,
	HTTP_HEADER_MAX_FORWARDS,
	HTTP_HEADER_PRAGMA,
	HTTP_HEADER_PROXY_AUTHENTICATE,
	HTTP_HEADER_PROXY_AUTHENTICATION,
	HTTP_HEADER_RANGE,
	HTTP_HEADER_REFERER,
	HTTP_HEADER_RETRY_AFTER,
	HTTP_HEADER_SERVER,
	HTTP_HEADER_TE,
	HTTP_HEADER_TRAILER,
	HTTP_HEADER_TRANSFER_ENCODING,
	HTTP_HEADER_UPGRADE,
	HTTP_HEADER_USER_AGENT,
	HTTP_HEADER_VARY,
	HTTP_HEADER_VIA,
	HTTP_HEADER_WARNING,
	HTTP_HEADER_WWW_AUTHENTICATE,
	
    HTTP_HEADER_COUNT           /* must be the last */

}  http_header_t;

/*----------------------------------------------------------------------*/
/* known header values */

/*
    NOTE: HTTP Cache-Control directives https://tools.ietf.org/html/rfc7234#section-5.2.2
*/
typedef enum {

	HTTP_CACHE_CONTROL_UNKNOWN = 0,
	HTTP_CACHE_CONTROL_MAX_AGE,
	HTTP_CACHE_CONTROL_MUST_REVALIDATE,
	HTTP_CACHE_CONTROL_NO_CACHE,
	HTTP_CACHE_CONTROL_NO_STORE,
	HTTP_CACHE_CONTROL_NO_TRANSFORM,
	HTTP_CACHE_CONTROL_PRIVATE,
	HTTP_CACHE_CONTROL_PROXY_REVALIDATE,
	HTTP_CACHE_CONTROL_PUBLIC,
	HTTP_CACHE_CONTROL_S_MAXAGE

}  http_cache_control_t;

/*----------------------------------------------------------------------*/
/*
    Header value parser callback:
     - user data associated with parser (if set by user)
     - directive (depends on header)
     - argument value (might be null if directive doesn't support values)
     - argument value length
    Return: ELIBC_SUCCESS to continue or error code to stop
*/

/* parser callbacks */
typedef int (*http_header_callback_t)(void*, int, const char*, size_t);

/*----------------------------------------------------------------------*/

/* get header name */
const char* http_header_name(http_header_t http_header);

/* find header from name */
http_header_t http_header_from_name(const char* header_name, size_t name_length);

/* split header to name and value */
void http_split_header(const char* header, const char** name, size_t* name_size, 
                       const char** value, size_t* value_size);

/*----------------------------------------------------------------------*/

/* get content type for MIME type */
const char* http_get_content_type(http_content_type_t http_content_type);

/* get content type from file name */
const char* http_get_file_content_type(const char* file_name, size_t name_length);
const char* http_get_file_content_typew(const ewchar_t* file_name, size_t name_length, ebuffer_t* temp_buffer);
const char* http_get_file_content_type_param(const http_param_t* http_param, ebuffer_t* temp_buffer);

/* get file extension for content type */
const char* http_get_file_content_extension(http_content_type_t http_content_type);

/*----------------------------------------------------------------------*/

/* parse content type header value */
int http_parse_content_type(const char* type_value, size_t type_length, 
            http_mime_type_t* mime_type, http_content_type_t* content_type);

/* parse content type parameters */
int http_parse_content_type_param(const char* type_value, size_t type_length,
            const char* param_name, const char** value, size_t* value_size);

/*----------------------------------------------------------------------*/

/* parse MIME type */
int http_parse_mime_type(const char* type_value, size_t type_length, http_mime_type_t* mime_type);

/*----------------------------------------------------------------------*/

/* parse Cache-Control header */
int http_parse_cache_control(const char* value, size_t value_length,
                   void* callback_data, http_header_callback_t callback_func);

/*----------------------------------------------------------------------*/

/* manage headers */
int http_headers_append(http_paramset_t* headers, const http_param_t* http_param, ebool_t copy_value);
int http_headers_set_name(http_paramset_t* headers, const http_param_t* http_param, ebool_t copy_value);
int http_headers_set_id(http_paramset_t* headers, unsigned short user_id, 
                        const http_param_t* http_param, ebool_t copy_value);

/* known headers */
int http_headers_set(http_paramset_t* headers, http_header_t http_header, 
                     const char* value, size_t value_length, ebool_t copy_value);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_HEADER_H_ */

