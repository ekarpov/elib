/*
    HTTP parameter encoding helpers
*/

#ifndef _HTTP_UPLOAD_H_
#define _HTTP_UPLOAD_H_

/*----------------------------------------------------------------------*/

/* http encode */
typedef struct
{
    /* input */
    http_param_t*           input_param;
    EFILE                   input_file;
    euint64_t               input_offset;
    euint64_t               content_left;

    /* content properties */
    efilesize_t             content_size;
    const char*             content_type;

} http_encode_t;

/*----------------------------------------------------------------------*/

/* init */
void http_encode_init(http_encode_t* http_encode);
void http_encode_reset(http_encode_t* http_encode);
void http_encode_close(http_encode_t* http_encode);

/* read content */
int http_encode_begin(http_encode_t* http_encode, ebuffer_t* encode_buffer, http_param_t* input_param);
int http_encode_begin_chunk(http_encode_t* http_encode, ebuffer_t* encode_buffer, http_param_t* input_param, 
                      euint64_t chunk_offset, euint64_t chunk_length);
int http_encode_read(http_encode_t* http_encode, char* buffer, size_t buffer_size, size_t* buffer_used);

/* content */
const char* http_encode_content_type(const http_encode_t* http_encode);
euint64_t http_encode_content_size(http_encode_t* http_encode);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_UPLOAD_H_ */

