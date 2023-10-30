/*
    HTTP form parameters and helpers
*/

#ifndef _HTTP_FORM_H_
#define _HTTP_FORM_H_

/*----------------------------------------------------------------------*/
/* constants */

#define HTTP_FORM_CONTENT_BOUNDARY_LENGTH       HTTP_CONTENT_BOUNDARY_LENGTH
#define HTTP_FORM_CONTENT_TYPE_LENGTH           HTTP_FORM_CONTENT_BOUNDARY_LENGTH + 36

/*----------------------------------------------------------------------*/

/* parameter encoding format */
typedef enum {

    HTTP_CONTENT_TYPE_URL_ENCODED,  /* application/x-www-form-urlencoded */
    HTTP_CONTENT_TYPE_MULTIPART     /* multipart/form-data */

} http_form_content_t;

/* http form */
typedef struct
{
    /* form parameters */
    http_param_t*           parameters;
    size_t                  parameter_count;

    /* content properties (set in http_form_encode_init) */
    http_form_content_t     content_type;
    euint64_t               content_size;

    /* encoding helpers */
    size_t                  encode_param;
    euint64_t               encode_offset;
    EFILE                   encode_file;
    unsigned short          encode_flags;
    
    /* temp buffers for encdoing */
    ebuffer_t*              encode_buffer;
    char                    form_boundary[HTTP_FORM_CONTENT_BOUNDARY_LENGTH];
    char                    form_content[HTTP_FORM_CONTENT_TYPE_LENGTH];

} http_form_t;

/*----------------------------------------------------------------------*/

/* init */
void http_form_init(http_form_t* http_form);
void http_form_reset(http_form_t* http_form);
void http_form_close(http_form_t* http_form);

/* encoding */
int http_form_encode_init(http_form_t* http_form, ebuffer_t* encode_buffer,
                          http_param_t* parameters, size_t parameter_count);
int http_form_encode(http_form_t* http_form, char* buffer, size_t buffer_size, size_t* buffer_used);

/* content */
const char* http_form_content_type(const http_form_t* http_form);
euint64_t http_form_content_size(http_form_t* http_form);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_FORM_H_ */

