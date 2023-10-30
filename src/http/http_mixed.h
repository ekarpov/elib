/*
    HTTP mixed request body encoder
*/

#ifndef _HTTP_MIXED_H_
#define _HTTP_MIXED_H_

/*----------------------------------------------------------------------*/

/*
    NOTE: HTTP multipart/mixed content explained: 
          https://www.w3.org/Protocols/rfc1341/7_2_Multipart.html
*/

/*----------------------------------------------------------------------*/
/* constants */

#define HTTP_MIXED_CONTENT_TYPE_LENGTH          HTTP_CONTENT_BOUNDARY_LENGTH + 36

/*----------------------------------------------------------------------*/

/* http mixed encoder */
typedef struct
{
    /* parameters */
    http_param_t*           parameters;
    size_t                  parameter_count;

    /* content properties */
    euint64_t               content_size;

    /* encoding helpers */
    http_encode_t           http_encode;
    size_t                  encode_param;
    euint64_t               encode_offset;
    ebool_t                 encode_value;

    /* encoding buffers */
    ebuffer_t*              encode_buffer;
    char                    content_boundary[HTTP_CONTENT_BOUNDARY_LENGTH];
    char                    content_type[HTTP_MIXED_CONTENT_TYPE_LENGTH];

} http_mixed_t;

/*----------------------------------------------------------------------*/

/* init */
void http_mixed_init(http_mixed_t* http_mixed);
void http_mixed_reset(http_mixed_t* http_mixed);
void http_mixed_close(http_mixed_t* http_mixed);

/* encoding */
int http_mixed_encode_init(http_mixed_t* http_mixed, ebuffer_t* encode_buffer,
                          http_param_t* parameters, size_t parameter_count);
int http_mixed_encode(http_mixed_t* http_mixed, char* buffer, size_t buffer_size, size_t* buffer_used);

/* content */
const char* http_mixed_content_type(const http_mixed_t* http_mixed);
euint64_t http_mixed_content_size(http_mixed_t* http_mixed);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_MIXED_H_ */

