/*
    HTTP url parameters and helpers
*/

#include "../elib_config.h"

#include "http_url.h"

/*----------------------------------------------------------------------*/
/* parser helpers */
const char* _http_url_parse_schema(const char* url, http_urlinfo_t *http_urlinfo)
{
    /* url may not have schema at all */
    const char* url_begin = url;

    /* reset schema */
    http_urlinfo->secure = 0;

    /* parse schema */
    if(*url != 'h') return url_begin; 
    ++url;

    if(*url != 't') return url_begin; 
    ++url;

    if(*url != 't') return url_begin; 
    ++url;

    if(*url != 'p') return url_begin; 
    ++url;

    if(*url == 's')
    {
        http_urlinfo->secure = 1;
        ++url;
    }
    if(*url != ':') return url_begin; 
    ++url;

    /* no slashes after : means syntax error */
    if(*url != '/') return 0; 
    ++url;

    if(*url != '/') return 0; 
    ++url;

    return url;
}

const char* _http_url_parse_host(const char* url, http_urlinfo_t *http_urlinfo)
{
    /* init host */
    http_urlinfo->host = url;
    http_urlinfo->host_length = 0;

    /* parse host */
    while(*url != 0 && *url != '/' && *url != ':')
    {
        /* check if valid host char */
        if((*url >= 'a' && *url <= 'z') ||
           (*url >= 'A' && *url <= 'Z') ||
           (*url >= '0' && *url <= '9') ||
            *url == '_' || *url == '-' || *url == '.')
        {
            http_urlinfo->host_length++;
            url++;

        } else
        {
            ETRACE1("http_url_parse: syntax error while parsing host name at %s", url);
            return 0;
        }
    }

    return url;
}

const char* _http_url_parse_port(const char* url, http_urlinfo_t *http_urlinfo)
{
    /* default port */
    euint32_t port = (http_urlinfo->secure) ? 443 : 80;

    /* ignore if no port */
    if(*url != ':') 
    {
        http_urlinfo->port = port;
        return url;
    }

    /* skip colon */
    url++;

    /* parse port */
    http_urlinfo->port = 0;
    while(*url != 0 && *url != '/')
    {
        /* check if valid port char */
        if(*url >= '0' && *url <= '9')
        {
            /* parse port */
            http_urlinfo->port = 10 * http_urlinfo->port + (*url - '0');
            url++;

        } else
        {
            ETRACE1("http_url_parse: syntax error while parsing port at %s", url);

            /* set default */
            http_urlinfo->port = port;
            return 0;
        }
    }

    return url;
}

const char* _http_url_parse_path(const char* url, http_urlinfo_t *http_urlinfo)
{
    /* skip slash if any */
    while(*url == '/') url++;

    /* set path */
    http_urlinfo->path = url;

    return url;
}

const char* _http_url_find(const char* url, char ch)
{
    /* find character */
    while(*url != 0 && *url != ch) url++;

    /* check if found */
    if(*url == ch) return url;

    return 0;
}

void _http_url_find_fragment(const char* url, http_urlinfo_t *http_urlinfo)
{
    /* find */
    http_urlinfo->fragment = _http_url_find(url, '#');

    /* skip # */
    if(http_urlinfo->fragment)
        http_urlinfo->fragment++;
}

void _http_url_find_arguments(const char* url, http_urlinfo_t *http_urlinfo)
{
    /* find */
    http_urlinfo->arguments = _http_url_find(url, '?');

    /* skip ? */
    if(http_urlinfo->arguments)
        http_urlinfo->arguments++;

    /* check if empty */
    if(http_urlinfo->arguments && http_urlinfo->arguments[0] == '#')
    {
        /* fragment starts right after ? */
        http_urlinfo->arguments = 0;
    }
}

/*----------------------------------------------------------------------*/

/* parse url (in place) */
int http_url_parse(const char* url, http_urlinfo_t* http_urlinfo,
                   void* callback_data, http_urlinfo_callback_t callback_func)
{
    EASSERT(url);
    EASSERT(http_urlinfo);
    if(url == 0 || http_urlinfo == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset result */
    ememset(http_urlinfo, 0 ,sizeof(http_urlinfo_t));

    /* parse schema */
    url = _http_url_parse_schema(url, http_urlinfo);
    if(url == 0) return ELIBC_ERROR_PARSER_INVALID_INPUT;

    /* parse host */
    url = _http_url_parse_host(url, http_urlinfo);
    if(url == 0 || http_urlinfo->host_length == 0) return ELIBC_ERROR_PARSER_INVALID_INPUT;

    /* parse port */
    url = _http_url_parse_port(url, http_urlinfo);
    if(url == 0) return ELIBC_ERROR_PARSER_INVALID_INPUT;

    /* parse path */
    url = _http_url_parse_path(url, http_urlinfo);
    if(url == 0) return ELIBC_ERROR_PARSER_INVALID_INPUT;

    /* find arguments */
    _http_url_find_arguments(url, http_urlinfo);

    /* find fragment */
    _http_url_find_fragment(url, http_urlinfo);

    /* parse query arguments (if callback is set and there are arguments) */
    if(http_urlinfo->arguments && callback_func)
    {
        return http_url_parse_arguments(http_urlinfo->arguments, callback_data, callback_func);
    }

    return ELIBC_SUCCESS;
}

int http_url_parse_arguments(const char* arguments, void* callback_data, http_urlinfo_callback_t callback_func)
{
    const char* argument=0;
    const char* value=0;
    int argument_length, value_length=0;
    int in_name;

    /* init first argument */
    in_name = 1;
    argument = arguments;
    argument_length = 0;

    /* parse arguments */
    while(*arguments != 0 && *arguments != '#')
    {
        if(*arguments == '=' && in_name == 1)
        {
            /* check if argument is not empty */
            if(argument_length == 0)
            {
                ETRACE1("http_url_parse: empty argument at %s", arguments);
                return ELIBC_ERROR_PARSER_INVALID_INPUT;
            }

            /* init value */
            value = arguments + 1;
            value_length = 0;
            in_name = 0;

        } else if(*arguments == '&' && in_name == 0)
        {
            /* check if value is not empty */
            if(value_length)
            {
                /* report parsed argument */
                callback_func(callback_data, argument, argument_length, value, value_length);

            } else
            {
                /* report empty value */
                callback_func(callback_data, argument, argument_length, 0, 0);
            }

            /* init argument */
            argument = arguments + 1;
            argument_length = 0;
            in_name = 1;

        } else
        {
            if(in_name)
                argument_length++;
            else
                value_length++;
        }

        arguments++;
    }

    /* report last value if any */
    if(argument_length != 0 && value_length != 0)
    {
        /* report parsed argument */
        callback_func(callback_data, argument, argument_length, value, value_length);
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/

