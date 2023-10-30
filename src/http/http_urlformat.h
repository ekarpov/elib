/*
    HTTP url formatting
*/

#ifndef _HTTP_URLFORMAT_H_
#define _HTTP_URLFORMAT_H_

/*----------------------------------------------------------------------*/

/* http url format properties */
typedef struct
{
    http_paramset_t         format_parameters;
    http_paramset_t         query_parameters;

    ebuffer_t               url;

} http_urlformat_t;

/*----------------------------------------------------------------------*/

/* init */
void http_urlformat_init(http_urlformat_t* http_urlformat);
void http_urlformat_reset(http_urlformat_t* http_urlformat);
void http_urlformat_close(http_urlformat_t* http_urlformat);

/* format parameters */
int http_urlformat_append(http_urlformat_t* http_urlformat, http_param_t* http_param, ebool_t copy_value);
int http_urlformat_set(http_urlformat_t* http_urlformat, http_param_t* http_param, unsigned short user_id, ebool_t copy_value);
http_param_t* http_urlformat_find(http_urlformat_t* http_urlformat, unsigned short user_id);

/* query parameters */
int http_urlformat_query_append(http_urlformat_t* http_urlformat, http_param_t* http_param, ebool_t copy_value);
int http_urlformat_query_set(http_urlformat_t* http_urlformat, http_param_t* http_param, unsigned short user_id, ebool_t copy_value);
http_param_t* http_urlformat_query_find(http_urlformat_t* http_urlformat, unsigned short user_id);
http_param_t* http_urlformat_query_find_name(http_urlformat_t* http_urlformat, const char* param_name, size_t name_length);

/* parameter sets */
http_paramset_t* http_format_parameters(http_urlformat_t* http_urlformat);
http_paramset_t* http_format_query_parameters(http_urlformat_t* http_urlformat);

/* format http url */
const char* http_format_url(http_urlformat_t* http_urlformat, const char* url_base, const char* url_format);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_URLFORMAT_H_ */



