/*
    HTTP url formatting
*/

#include "../elib_config.h"

#include "http_param.h"
#include "http_url.h"
#include "http_urlformat.h"

/*----------------------------------------------------------------------*/

/* init */
void http_urlformat_init(http_urlformat_t* http_urlformat)
{
    EASSERT(http_urlformat);
    if(http_urlformat)
    {
        /* reset all fields */
        ememset(http_urlformat, 0, sizeof(http_urlformat_t));

        /* init parameters */
        http_paramset_init(&http_urlformat->format_parameters);
        http_paramset_init(&http_urlformat->query_parameters);

        /* init buffer */
        ebuffer_init(&http_urlformat->url);
    }
}

void http_urlformat_reset(http_urlformat_t* http_urlformat)
{
    EASSERT(http_urlformat);
    if(http_urlformat)
    {
        /* reset parameters */
        http_paramset_reset(&http_urlformat->format_parameters);
        http_paramset_reset(&http_urlformat->query_parameters);

        /* reset url buffer */
        ebuffer_reset(&http_urlformat->url);
    }
}

void http_urlformat_close(http_urlformat_t* http_urlformat)
{
    EASSERT(http_urlformat);
    if(http_urlformat)
    {
        /* free parameters */
        http_paramset_close(&http_urlformat->format_parameters);
        http_paramset_close(&http_urlformat->query_parameters);

        /* free url buffer */
        ebuffer_free(&http_urlformat->url);
    }
}

/* format parameters */
int http_urlformat_append(http_urlformat_t* http_urlformat, http_param_t* http_param, ebool_t copy_value)
{
    /* check input */
    EASSERT(http_urlformat);
    EASSERT(http_param);
    if(http_urlformat == 0 || http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* append */
    return http_paramset_append(&http_urlformat->format_parameters, http_param, copy_value);
}

int http_urlformat_set(http_urlformat_t* http_urlformat, http_param_t* http_param, unsigned short user_id, ebool_t copy_value)
{
    /* check input */
    EASSERT(http_urlformat);
    EASSERT(http_param);
    if(http_urlformat == 0 || http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* set parameter */
    return http_paramset_set_id(&http_urlformat->format_parameters, user_id, http_param, copy_value);
}

http_param_t* http_urlformat_find(http_urlformat_t* http_urlformat, unsigned short user_id)
{
    /* check input */
    EASSERT(http_urlformat);
    if(http_urlformat == 0) return 0;

    /* find parameter */
    return http_params_find_id(http_paramset_params(&http_urlformat->format_parameters),
        http_paramset_size(&http_urlformat->format_parameters), user_id);
}

/* query parameters */
int http_urlformat_query_append(http_urlformat_t* http_urlformat, http_param_t* http_param, ebool_t copy_value)
{
    /* check input */
    EASSERT(http_urlformat);
    EASSERT(http_param);
    if(http_urlformat == 0 || http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* append */
    return http_paramset_append(&http_urlformat->query_parameters, http_param, copy_value);
}

int http_urlformat_query_set(http_urlformat_t* http_urlformat, http_param_t* http_param, unsigned short user_id, ebool_t copy_value)
{
    /* check input */
    EASSERT(http_urlformat);
    EASSERT(http_param);
    if(http_urlformat == 0 || http_param == 0) return ELIBC_ERROR_ARGUMENT;

    /* set parameter */
    return http_paramset_set_id(&http_urlformat->query_parameters, user_id, http_param, copy_value);
}

http_param_t* http_urlformat_query_find(http_urlformat_t* http_urlformat, unsigned short user_id)
{
    /* check input */
    EASSERT(http_urlformat);
    if(http_urlformat == 0) return 0;

    /* find parameter */
    return http_params_find_id(http_paramset_params(&http_urlformat->query_parameters),
        http_paramset_size(&http_urlformat->query_parameters), user_id);
}

http_param_t* http_urlformat_query_find_name(http_urlformat_t* http_urlformat, const char* param_name, size_t name_length)
{
    /* check input */
    EASSERT(http_urlformat);
    if(http_urlformat == 0) return 0;

    /* find parameter */
    return http_params_find_name(http_paramset_params(&http_urlformat->query_parameters),
        http_paramset_size(&http_urlformat->query_parameters), param_name, name_length);
}

/* parameter sets */
http_paramset_t* http_format_parameters(http_urlformat_t* http_urlformat)
{
    /* check input */
    EASSERT(http_urlformat);
    if(http_urlformat == 0) return 0;

    return &http_urlformat->format_parameters;
}

http_paramset_t* http_format_query_parameters(http_urlformat_t* http_urlformat)
{
    /* check input */
    EASSERT(http_urlformat);
    if(http_urlformat == 0) return 0;

    return &http_urlformat->query_parameters;
}

/* format http url */
const char* http_format_url(http_urlformat_t* http_urlformat, const char* url_base, const char* url_format)
{
    int query_started = 0;
    int path_started = 0;
    size_t param_idx;
    char url_char;
    int err;

    http_param_t*   format_parameters;
    size_t          format_count;
    http_param_t*   query_parameters;
    size_t          query_count;

    /* check input */
    EASSERT(http_urlformat);
    if(http_urlformat == 0) return 0;

    /* reset url buffer */
    ebuffer_reset(&http_urlformat->url);

    /* reserve default space */
    err = ebuffer_reserve(&http_urlformat->url, HTTP_DEFAULT_RESOURCE_LENGTH);
    if(err != ELIBC_SUCCESS) return 0;

    /* append base part if set */
    if(url_base)
    {
        for(; *url_base != 0; ++url_base)
        {
            /* check special symbols */
            if(*url_base == '?') query_started = 1;
            if(*url_base == '/') path_started = 1;

            /* append char */
            err = ebuffer_append_char(&http_urlformat->url, *url_base); 
            if(err != ELIBC_SUCCESS) return 0;
        }
    }

    /* init references */
    format_parameters = http_paramset_params(&http_urlformat->format_parameters);
    format_count = http_paramset_size(&http_urlformat->format_parameters);
    query_parameters = http_paramset_params(&http_urlformat->query_parameters);
    query_count = http_paramset_size(&http_urlformat->query_parameters);

    /* format base url first */
    if(url_format != 0)
    {
        int in_param = 0;
        const char* param_name = 0;
        size_t param_len = 0;

        /* loop over all chars */
        for(; *url_format != 0;  ++url_format)
        {
            /* current char */
            url_char = *url_format;

            if(in_param)
            {
                /* check if parameter ends */
                if(url_char == '%')
                {
                    /* find parameter by name */
                    const http_param_t* http_param = http_params_find_name(format_parameters,
                                format_count, param_name, param_len);

                    /* check if found */
                    if(http_param && http_param->value != 0 && http_param->value_size > 0)
                    {
                        /* append parameter value to url (do not url-encode) */
                        err = http_encode_value_buffer(&http_urlformat->url, http_param, HTTP_VALUE_ENCODING_NONE);

                    } else
                    {
                        ETRACE("http_format_url: url_format parameter not set");

                        /* just append parameter name to url */
                        err = ebuffer_append(&http_urlformat->url, param_name, param_len);
                    }

                    /* check errors */
                    if(err != ELIBC_SUCCESS) return 0;

                    /* reset parameter flag */
                    in_param = 0;

                } else
                {
                    ++param_len;
                }

            } else
            {
                /* check if parameter starts */
                if(url_char == '%')
                {
                    /* init parameter name */
                    param_name = url_format + 1;
                    param_len = 0;
                    in_param = 1;

                } else
                {
                    /* check for special symbols */
                    if(url_char == '?') query_started = 1;
                    if(url_char == '/') path_started = 1;

                    /* append char */
                    err = ebuffer_append_char(&http_urlformat->url, url_char); 
                    if(err != ELIBC_SUCCESS) return 0;
                }
            }           
        }
    } else
    {
        EASSERT(format_count == 0);
        if(format_count != 0)
        {
            ETRACE("http_format_url: format parameters ignored as url_format is not set");
        }
    }

    /* append query parameters */
    if(query_count != 0)
    {
        /* append separator if path not started yet */
        if(!path_started) 
        {
            err = ebuffer_append_char(&http_urlformat->url, '/');
            if(err != ELIBC_SUCCESS) return 0;

            path_started = 1;
        }

        EASSERT(query_parameters);
        if(query_parameters == 0) return 0;

        /* loop over parameters */
        for(param_idx = 0; param_idx < query_count; ++param_idx)
        {
            /* format separator */
            url_char = '&';
            if(!query_started) 
            {
                url_char = '?';
                query_started = 1;
            }

            /* validate parameter */
            if(query_parameters[param_idx].name == 0 || query_parameters[param_idx].name[0] == 0)
            {
                ETRACE("http_format_url: parameter name not set, parameter will be ignored");
                continue;
            }

            /* validate value */
            if(query_parameters[param_idx].value == 0 || query_parameters[param_idx].value[0] == 0)
            {
                ETRACE("http_format_url: parameter value not set, parameter will be ignored");
                continue;
            }

            /* append separator */
            err = ebuffer_append_char(&http_urlformat->url, url_char); 
            if(err != ELIBC_SUCCESS) return 0;

            /* append parameter name */
            err = ebuffer_append(&http_urlformat->url, query_parameters[param_idx].name,
                                                       estrlen(query_parameters[param_idx].name));
            if(err != ELIBC_SUCCESS) return 0;

            /* value separator */
            err = ebuffer_append_char(&http_urlformat->url, '=');
            if(err != ELIBC_SUCCESS) return 0;

            /* append value (url-encoded) */
            err = http_encode_value_buffer(&http_urlformat->url, query_parameters + param_idx, HTTP_VALUE_ENCODING_URLENCODE);
            if(err != ELIBC_SUCCESS) return 0;
        }
    }

    /* append end of string */
    err = ebuffer_append_char(&http_urlformat->url, 0);
    if(err != ELIBC_SUCCESS) return 0;

    /* return value */
    return (const char*)ebuffer_data(&http_urlformat->url);
}

/*----------------------------------------------------------------------*/
