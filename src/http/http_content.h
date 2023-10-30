/*
    HTTP content
*/

#ifndef _HTTP_CONTENT_H_
#define _HTTP_CONTENT_H_

/*----------------------------------------------------------------------*/
/* content interface */

/*
    Content type:
     - user data associated with content (user_data from http_content_iface_t)
    Returns: content type, to be used for "Content-Type" http header
*/
typedef const char* (*http_get_content_type_t)(void*);

/*
    Content size:
     - user data associated with content (user_data from http_content_iface_t)
    Returns: content size, to be used for "Content-Size" http header
*/
typedef euint64_t (*http_get_content_size_t)(void*);

/*
    Reads content into provided buffer:
     - user data associated with content (user_data from http_content_iface_t)
     - buffer pointer
     - buffer size in bytes
     - buffer size used (out parameter)
    Returns: ELIBC_SUCCESS if successful, ELIBC_ERROR_ENDOFFILE if end of content,
             or error code otherwise
*/
typedef int (*http_read_content_t)(void*, char*, size_t, size_t*);

/* http content interface */
typedef struct
{
    void*                       user_data;
    http_get_content_type_t     http_content_type;
    http_get_content_size_t     http_content_size;
    http_read_content_t         http_content_read;

} http_content_iface_t;

/*----------------------------------------------------------------------*/

/* http content */
typedef struct
{
    /* content interface */
    http_content_iface_t        content_iface;

    /* content reference */
    const char*                 content_type;
    const char*                 content_data;
    euint64_t                   content_size;
    size_t                      content_offset;

} http_content_t;

/*----------------------------------------------------------------------*/

/* reset content */
void http_content_reset(http_content_t* http_content);

/* set content (NOTE: data is not copied so all pointers must be valid until content is no longer needed) */
int http_content_set(http_content_t* http_content, http_content_t* http_content_from);
int http_content_set_iface(http_content_t* http_content, http_content_iface_t* http_content_iface);
int http_content_set_data(http_content_t* http_content, const char* content_type, const char* content_data, euint64_t content_size);

/* content properties */
const char* http_content_type(const http_content_t* http_content);
euint64_t   http_content_size(const http_content_t* http_content);

/* read content */
int http_content_read(http_content_t* http_content, char* buffer, size_t buffer_size, size_t* buffer_used);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_CONTENT_H_ */

