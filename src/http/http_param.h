/*
    HTTP request parameters
*/

#ifndef _HTTP_PARAM_H_
#define _HTTP_PARAM_H_

/*----------------------------------------------------------------------*/

/* parameter value format */
typedef enum {

    HTTP_FORMAT_TEXT_UTF8 = 0,           
    HTTP_FORMAT_TEXT_UTF16,
    HTTP_FORMAT_FILENAME_UTF8,
    HTTP_FORMAT_FILENAME_UTF16,
    HTTP_FORMAT_BINARY_DATA,
    HTTP_FORMAT_STREAM

} http_format_t;

/* parameter value encoding */
typedef enum {

    HTTP_VALUE_ENCODING_NONE = 0,           
    HTTP_VALUE_ENCODING_URLENCODE,
    HTTP_VALUE_ENCODING_UTF8_ESCAPED,
    HTTP_VALUE_ENCODING_BASE64

} http_encoding_t;

/* binary data transfer encoding */
typedef enum {

    HTTP_TRANSFER_ENCODING_NONE = 0,           
    HTTP_TRANSFER_ENCODING_BASE64

} http_transfer_encoding_t;

/*
    NOTE: for url encoding all UTF16 parameters are internally converted to UTF8 first (unless value is already encoded)
*/

/*----------------------------------------------------------------------*/

/* streaming */

/*
    Parameter streaming callback:
     - user data associated with stream (stream_data in http_param_t)
     - parameter id (user_id in http_param_t)
     - buffer pointer
     - buffer size in bytes
     - buffer size used (out parameter)
    Returns: ELIBC_SUCCEESS if successful, ELIBC_ERROR_ENDOFFILE if end of content,
             or error code otherwise
*/
typedef int (*http_stream_read_t)(void*, unsigned short, void*, size_t, size_t*);

/*----------------------------------------------------------------------*/

/*
    NOTE: Stream data (file and stream) can be used only with base64 or no encoding because 
          urlencoding will require that data will be read twice to compute content length.
          
    NOTE: value_size must be set for HTTP_FORMAT_STREAM as well, content encoder will read
          from stream as much as specified in this parameter. value_size for file name
          must be set to name length in bytes (e.g. utf16 requires two bytes per character)
*/

/*
    NOTE: if parameter is file name it must be zero terminated and value_size must include 
          ending zero. If not value will be forcibly copied and end of line added.
*/

/* http parameter */
typedef struct
{
    const char*             name;
    const char*             value;
    size_t                  value_size;         /* always in bytes */

    /* value data properties */
    http_format_t           value_format;       /* data format */
    http_encoding_t         value_encoding;     /* set if value is already encoded */
    unsigned short          value_is_copy;      /* value storage flags */

    /* optional parameters */
    unsigned short          user_id;            /* can be used to identify parameters */
    const char*             content_type;       /* if set will be sent as Content-Type: 
                                                   in multipart form encoding */

    /* stream callback (mandatory for HTTP_FORMAT_STREAM format) */
    void*                   stream_data;
    http_stream_read_t      stream_read_func;

    /* data transfer encoding */
    http_transfer_encoding_t    transfer_encoding;    

} http_param_t;

/*
    NOTE: http parameter set doesn't copy parameter names, so pointers must be valid
          until request is sent
*/

/* http parameter set */
typedef struct
{
    earray_t                parameters;
    ebuffer_t               buffer;         /* buffer to store values if copy is needed */

} http_paramset_t;

/*----------------------------------------------------------------------*/

/* parameters */
void http_param_init(http_param_t* http_param);
void http_param_reset(http_param_t* http_param);

/* parameter values */
#define http_param_str(http_param)          ((const char*)(http_param).value)
#define http_param_wstr(http_param)         ((const ewchar_t*)(http_param).value)
#define http_param_len(http_param)          ((http_param).value_size)
#define http_param_wlen(http_param)         (((http_param).value_size)/sizeof(ewchar_t))

/* parameter value properties */
ebool_t http_parameter_is_text(const http_param_t* http_param);
ebool_t http_parameter_is_utf16(const http_param_t* http_param);
ebool_t http_parameter_is_filename(const http_param_t* http_param);
ebool_t http_parameter_is_binary(const http_param_t* http_param);
ebool_t http_parameter_is_stream(const http_param_t* http_param);

/* encode name */
int http_encode_name(char* buffer, size_t* output_size, const http_param_t* http_param, http_encoding_t encoding);
int http_encode_name_buffer(ebuffer_t* ebuffer, const http_param_t* http_param, http_encoding_t encoding);

/* encode value (NOTE: file or stream contents are not encoded, only file name) */
int http_encode_value(char* buffer, size_t* output_size, const http_param_t* http_param, http_encoding_t encoding);
int http_encode_value_buffer(ebuffer_t* ebuffer, const http_param_t* http_param, http_encoding_t encoding);

/* compute encoded value size (NOTE: set "use_file" if actual file needs to be used instead of file name) */
euint64_t http_encoded_value_size(const http_param_t* http_param, http_encoding_t encoding, ebool_t use_file);

/*----------------------------------------------------------------------*/

/* init and free parameter set */
void http_paramset_init(http_paramset_t* http_paramset);
void http_paramset_reset(http_paramset_t* http_paramset);
void http_paramset_close(http_paramset_t* http_paramset);

/* find parameters */
http_param_t* http_params_find_id(http_param_t* http_params, size_t parameter_count,
                                        unsigned short user_id);
http_param_t* http_params_find_name(http_param_t* http_params, size_t parameter_count,
                                          const char* param_name, size_t name_length);

/* find parameters from set */
http_param_t* http_paramset_find_id(http_paramset_t* http_paramset, unsigned short user_id);
http_param_t* http_paramset_find_name(http_paramset_t* http_paramset, const char* param_name, size_t name_length);

/* manage parameter set */
http_param_t* http_paramset_reserve(http_paramset_t* http_paramset);
int http_paramset_append(http_paramset_t* http_paramset, const http_param_t* http_param, ebool_t copy_value);

/* set parameters by id or name (add new or replace existing) */
int http_paramset_set_name(http_paramset_t* http_paramset, const http_param_t* http_param, ebool_t copy_value);
int http_paramset_set_id(http_paramset_t* http_paramset, unsigned short user_id,
                      const http_param_t* http_param, ebool_t copy_value);

/* modify parameters */
int http_paramset_change_name(http_paramset_t* http_paramset, unsigned short user_id, const char* name);

/* remove parameters */
int http_paramset_remove_id(http_paramset_t* http_paramset, unsigned short user_id);

/* get parameters */
http_param_t* http_paramset_params(const http_paramset_t* http_paramset); 
size_t        http_paramset_size(const http_paramset_t* http_paramset);   

/*----------------------------------------------------------------------*/
/* encoding helpers */

/* replace %param_name% with parameter value (returns error if parameter not found) */
int http_encode_template(ebuffer_t* ebuffer, const char* tmpl, http_param_t* http_params, 
                         size_t parameter_count, ebool_t encode);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_PARAM_H_ */




