/*
    HTTP header helpers
*/

#include "../elib_config.h"

#include "../text/text_format.h"

#include "http_param.h"
#include "http_mime_types.h"
#include "http_header.h"

/*----------------------------------------------------------------------*/

/*
    NOTE: arrays is alphabetically sorted for binary search
*/

/* mime types */
static const char* http_header_names [] = {
	"",
	"Accept",
	"Accept-Charset",
	"Accept-Encoding",
	"Accept-Language",
	"Accept-Ranges",
	"Age",
	"Age",
	"Authorization",
	"Cache-Control",
	"Connection",
	"Content-Encoding",
	"Content-Language",
	"Content-Length",
	"Content-Location",
	"Content-MD5",
	"Content-Range",
	"Content-Type",
	"Date",
	"ETag",
	"Expect",
	"Expires",
	"From",
	"Host",
	"If-Match",
	"If-Modified-Since",
	"If-None-Match",
	"If-Range",
	"If-Unmodified-Since",
	"Last-Modified",
	"Location",
	"Max-Forwards",
	"Pragma",
	"Proxy-Authenticate",
	"Proxy-Authorization",
	"Range",
	"Referer",
	"Retry-After",
	"Server",
	"TE",
	"Trailer",
	"Transfer-Encoding",
	"Upgrade",
	"User-Agent",
	"Vary",
	"Via",
	"Warning",
	"WWW-Authenticate"
};

/* Cache-Control directives */
static const char* http_cachecontrol_directives [] = {
	"",
	"max-age",
	"must-revalidate",
	"no-cache",
	"no-store",
	"no-transform",
	"private",
	"proxy-revalidate",
	"public",
	"s-maxage"
};

/* array size */
#define HTTP_HEADER_NAMES_ARRAY_SIZE                (sizeof(http_header_names) / sizeof(http_header_names[0])) 
#define HTTP_CACHE_CONTROL_DIRECTIVES_ARRAY_SIZE    (sizeof(http_cachecontrol_directives) / sizeof(http_cachecontrol_directives[0])) 

/*----------------------------------------------------------------------*/

/* get header name */
const char* http_header_name(http_header_t http_header)
{
    if(http_header >= 0 && http_header < HTTP_HEADER_NAMES_ARRAY_SIZE)
    {
        return http_header_names[http_header];
    }

    return http_header_names[HTTP_HEADER_UNKNOWN];
}

/* find header from name */
http_header_t http_header_from_name(const char* header_name, size_t name_length)
{
    int index;

    /* compute name length if not provided */
    if(header_name != 0 && name_length == 0)
    {
        name_length = estrlen(header_name);
    }

    /* search for header name */
    index = ebinsearch_strings(http_header_names, HTTP_HEADER_NAMES_ARRAY_SIZE, 
        header_name, name_length, ELIBC_TRUE);

    if(index != ELIBC_NOT_FOUND && index > HTTP_HEADER_UNKNOWN && index < HTTP_HEADER_NAMES_ARRAY_SIZE)
    {
        return (http_header_t)index;
    }

    return HTTP_HEADER_UNKNOWN;
}

/* split header to name and value */
void http_split_header(const char* header, const char** name, size_t* name_size, 
                       const char** value, size_t* value_size)
{
    /* check input */
    EASSERT(header);
    EASSERT(name);
    EASSERT(name_size);
    EASSERT(value);
    EASSERT(value_size);
    if(header == 0 || name == 0 || name_size == 0 || value == 0 || value_size == 0) return;

    /* reset output */
    *name = 0;
    *name_size = 0;
    *value = 0;
    *value_size = 0;

    /* skip spaces */
    while(*header == ' ') header++;

    /* init name */
    *name = header;

    /* find split */
    while(header[*name_size] != 0 && header[*name_size] != ':') (*name_size) += 1;

    /* stop if not found */
    if(header[*name_size] != ':') return;

    /* init value */
    *value = header + *name_size + 1;
    
    /* skip spaces */
    while((*value)[0] == ' ') (*value)++;

    /* find value size */
    while((*value)[*value_size] != 0) (*value_size) += 1;

    /* trim name ending spaces */
    while((*name)[*name_size] == ' ' && *name_size > 0) (*name_size) -= 1;

    /* trim value ending spaces */
    while((*value)[*value_size] == ' ' && *value_size > 0) (*value_size) -= 1;
}

/*----------------------------------------------------------------------*/

/* get content type for MIME type */
const char* http_get_content_type(http_content_type_t http_content_type)
{
    /* check if size matches */
    EASSERT(http_content_type < http_content_types_count());
    if(http_content_type >= http_content_types_count()) return 0;

    /* get content type */
    return http_content_types()[http_content_type];
}

/* get content type from file name */
const char* http_get_file_content_type(const char* file_name, size_t name_length)
{
    const http_file_content_type_t* mappings;
    int mapping_count;

    size_t pos;
    int left, right, mid, result;

    /* check input */
    EASSERT(file_name);
    if(file_name == 0) return 0;

    /* compute name length if not provided */
    if(name_length == 0)
    {
        name_length = estrlen(file_name);
    }
    if(name_length == 0) return 0;

    /* get mappings */
    mappings = http_file_ext_mappings();
    mapping_count = http_file_ext_mappings_count();

    /* check just in case */
    EASSERT(mappings);
    EASSERT(mapping_count);
    if(mappings == 0 || mapping_count == 0) return 0;

    /* find extension first */
    pos = name_length - 1;
    while(pos > 0 && file_name[pos - 1] != '.') pos--;

    /* NOTE: if '.' not found we assume that file_name is extension only */

    left = 0;
    right = (int) mapping_count;

    /* binary search from extension mapping */
    while(left <= right && left < mapping_count)
    {
        /* middle point */ 
        mid = left + (right - left) / 2;

        /* compare strings */
        result = estricmp(file_name + pos, mappings[mid].file_extension);

        /* check result */
        if(result == 0)
        {
            /* return type */
            return http_get_content_type(mappings[mid].content_type);

        } else if(result < 0)
        {
            right = mid - 1;

        } else
        {
            left = mid + 1;
        }
    }

    /* not found, use default */
    return http_get_content_type(HTTP_CONTENT_TYPE_APPLICATION_OCTET_STREAM);
}

/* get content type from file name */
const char* http_get_file_content_typew(const ewchar_t* file_name, size_t name_length, ebuffer_t* temp_buffer)
{
    size_t len_utf8;
    char* filename_utf8;
    const char* content_type;
    int err;
        
    /* check input */
    EASSERT(file_name);
    EASSERT(temp_buffer);
    if(file_name == 0 || temp_buffer == 0) return 0;

    /* reset buffer */
    ebuffer_reset(temp_buffer);

    if(name_length == 0)
    {
        name_length = ewcslen(file_name);
    }
    if(name_length == 0) return 0;

    /* required size */
    len_utf8 = utf16_in_utf8((const utf16_t*)file_name, name_length);

    /* reserve memory from temp buffer */
    err = ebuffer_reserve(temp_buffer, len_utf8 + 1);
    if(err != ELIBC_SUCCESS) return 0;

    filename_utf8 = ebuffer_data(temp_buffer);

    /* convert string */
    utf16_to_utf8((const utf16_t*)file_name, name_length, (utf8_t*)filename_utf8, &len_utf8);
    filename_utf8[len_utf8] = 0;

    /* try to guess from file name */
    content_type = http_get_file_content_type(filename_utf8, len_utf8);

    /* reset buffer after use */
    ebuffer_reset(temp_buffer);

    return content_type;
}

const char* http_get_file_content_type_param(const http_param_t* http_param, ebuffer_t* temp_buffer)
{
    /* check input */
    EASSERT(http_param);
    if(http_param == 0 || http_param->value == 0 || http_param->value_size == 0) return 0;

    /* check encoding */
    if(http_parameter_is_utf16(http_param))
        return http_get_file_content_typew((const ewchar_t*)http_param->value, (size_t)http_param->value_size / sizeof(ewchar_t), temp_buffer);
    else
        return http_get_file_content_type(http_param->value, (size_t)http_param->value_size);
}


/* get file extension for content type */
const char* http_get_file_content_extension(http_content_type_t http_content_type)
{
    const char** mappings;
    int mapping_count;

    /* get mappings */
    mappings = http_content_types_file_ext();
    mapping_count = http_content_types_file_ext_count();

    /* check just in case */
    EASSERT(mappings);
    EASSERT(mapping_count);
    if(mappings == 0 || mapping_count == 0) return 0;

    /* check that content type fits */
    EASSERT(http_content_type >= 0 && http_content_type < mapping_count);
    if(http_content_type < 0 || http_content_type >= mapping_count) return 0;

    /* return file extension */
    return mappings[http_content_type];
}

/*----------------------------------------------------------------------*/

/* parse content type header value */
int http_parse_content_type(const char* type_value, size_t type_length, 
            http_mime_type_t* mime_type, http_content_type_t* content_type)
{
    size_t pos, type_start;
    int index;

    /* check input */
    EASSERT(type_value);
    EASSERT(mime_type);
    EASSERT(content_type);
    if(type_value == 0 || mime_type == 0 || content_type == 0) return ELIBC_ERROR_ARGUMENT; 

    if(type_length == 0)
        type_length = estrlen(type_value);

    if(type_length == 0) return ELIBC_ERROR_ARGUMENT;

    /* init return values */
    *mime_type = HTTP_MIME_TYPE_UNKNOWN;
    *content_type = HTTP_CONTENT_TYPE_UNKNOWN;

    pos = 0;
    
    /* skip spaces */
    while(pos < type_length && eisspace(type_value[pos])) pos++;

    /* find type */
    type_start = pos;
    while(pos < type_length && type_value[pos] != '/') pos++;

    /* check if type found */
    if(type_start < pos && pos < type_length)
    {
        /* find type */
        index = ebinsearch_strings(http_mime_types(), (int)http_mime_types_count(), 
            type_value + type_start, pos - type_start, ELIBC_TRUE);

        /* set type */
        if(index != ELIBC_NOT_FOUND)
        {
            *mime_type = (http_mime_type_t)index;
        }
    }

    /* find subtype */
    while(pos < type_length && type_value[pos] != ';') pos++;

    /* check if subtype found */
    if(type_start < pos)
    {
        /* find full content type */
        index = ebinsearch_strings(http_content_types(), (int)http_content_types_count(), 
            type_value + type_start, pos - type_start, ELIBC_TRUE);

        /* set content type */
        if(index != ELIBC_NOT_FOUND)
        {
            *content_type = (http_content_type_t)index;
        }
    }

    return ELIBC_SUCCESS;
}

/* parse content type parameters */
int http_parse_content_type_param(const char* type_value, size_t type_length,
            const char* param_name, const char** value, size_t* value_size)
{
    size_t pos, param_start, param_end, value_start, value_end;
    int param_found;

    /* check input */
    EASSERT(type_value);
    EASSERT(type_length);
    EASSERT(param_name);
    EASSERT(value);
    EASSERT(value_size);
    if(type_value == 0 || type_length == 0 || 
       param_name == 0 || value == 0 || value_size == 0) return ELIBC_ERROR_ARGUMENT; 

    /* reset output */
    *value = 0;
    *value_size = 0;

    pos = 0;
    
    /* skip type first */
    while(pos < type_length && type_value[pos] != ';') pos++;

    /* skip ';' */
    pos++;

    /* find parameters */
    param_start = pos;
    param_found = 0;
    while(pos <= type_length)
    {
        /* check if param name found */
        if(type_value[pos] == '=')
        {
            param_end = pos;

            /* trim spaces from name */
            while(param_start < type_length && eisspace(type_value[param_start])) param_start++;
            while(param_end > param_start && eisspace(type_value[param_end - 1])) param_end--;

            /* match name */
            if(param_end > param_start && estrnicmp2(param_name, 0, type_value + param_start, param_end - param_start) == 0)
            {
                param_found = 1;
                value_start = pos + 1;
            }

        } else if(type_value[pos] == ';' || pos == type_length)
        {
            /* copy value if parameter found */
            if(param_found)
            {
                value_end = pos;

                /* trim spaces from value */
                while(value_start < type_length && eisspace(type_value[value_start])) value_start++;
                while(value_end >= value_start && eisspace(type_value[value_end - 1])) value_end--;

                /* remove quotation marks if any */
                if(value_start < type_length && value_end > value_start)
                {
                    if(type_value[value_start] == '\"' && type_value[value_end - 1] == '\"')
                    {
                        value_start++;
                        value_end--;
                    }
                }

                /* init value */
                *value = type_value + value_start;
                *value_size = value_end - value_start;

                /* stop search */
                return ELIBC_SUCCESS;
            }

            /* next parameter */
            param_start = pos + 1;
        }

        pos++;
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/

/* parse MIME type */
int http_parse_mime_type(const char* type_value, size_t type_length, http_mime_type_t* mime_type)
{
    size_t pos, type_start;
    int index;

    /* check input */
    EASSERT(type_value);
    EASSERT(mime_type);
    if(type_value == 0 || mime_type == 0) return ELIBC_ERROR_ARGUMENT; 

    if(type_length == 0)
        type_length = estrlen(type_value);

    if(type_length == 0) return ELIBC_ERROR_ARGUMENT;

    /* init return value */
    *mime_type = HTTP_MIME_TYPE_UNKNOWN;

    pos = 0;
    
    /* skip spaces */
    while(pos < type_length && eisspace(type_value[pos])) pos++;

    /* find type */
    type_start = pos;
    while(pos < type_length && type_value[pos] != '/') pos++;

    /* check if type found */
    if(type_start < pos && pos < type_length)
    {
        /* find type */
        index = ebinsearch_strings(http_mime_types(), (int)http_mime_types_count(), 
            type_value + type_start, pos - type_start, ELIBC_TRUE);

        /* set type */
        if(index != ELIBC_NOT_FOUND)
        {
            *mime_type = (http_mime_type_t)index;
            return ELIBC_SUCCESS;
        }
    }

    return ELIBC_NOT_FOUND;
}

/*----------------------------------------------------------------------*/
/* common http header value parser */

/*
    Parser syntax: directive, directive=value, directive="value"
*/

int _http_parse_header_value(const char* value, size_t value_length, const char** directive_names, int name_count,
                             void* callback_data, http_header_callback_t callback_func)
{
    size_t pos, end_pos;
    int directive, err;

    EASSERT(directive_names);
    EASSERT(name_count);
    EASSERT(callback_func);

    /* compute value length if not provided */
    if(value != 0 && value_length == 0)
    {
        value_length = estrlen(value);
    }

    /* ignore if header value is empty */
    if(value == 0 || value_length == 0) return ELIBC_SUCCESS;

    /* parse */
    pos = 0;
    while(pos < value_length)
    {
        /* trim spaces in front */
        while(pos < value_length && eisspace(value[pos])) pos++;

        /* stop if at the end */
        if(pos == value_length) break;

        /* find directive */
        end_pos = pos + 1;
        while(end_pos < value_length && !eisspace(value[end_pos]) && value[end_pos] != ',' && value[end_pos] != '=') end_pos++;

        /* find directive */
        directive = ebinsearch_strings(directive_names, name_count, 
            value + pos, end_pos - pos, ELIBC_TRUE);

        /* check if found (NOTE: assume unknown directive is at zero index) */
        if(directive == ELIBC_NOT_FOUND) directive = 0;

        /* trim ending spaces if any */
        pos = end_pos;
        while(pos < value_length && eisspace(value[pos])) pos++;

        /* check if directive is ready */
        if(pos == value_length || value[pos] == ',')
        {
            /* no value, report directive */
            err = callback_func(callback_data, directive, 0, 0);
            if(err != ELIBC_SUCCESS) return err;

            /* jump to next */
            pos++;
            continue;

        } else if(value[end_pos] != '=')
        {
            /* syntax error */
            ETRACE("http_parse_header_value: invalid directive separator character");
            return ELIBC_ERROR_PARSER_INVALID_INPUT;
        }

        /* skip separator */
        pos++;

        /* trim spaces in front of the value */
        while(pos < value_length && eisspace(value[pos])) pos++;

        /* find value */
        end_pos = pos;
        if(end_pos < value_length && (value[end_pos] == '\"' || value[end_pos] == '\''))
        {
            /* quotation mark in use */
            char mark = value[end_pos];

            /* skip quotaion */
            pos++;
            end_pos++;

            /* parse value */
            while(end_pos < value_length && value[end_pos] != mark) end_pos++;

            /* make sure we found quotation end */
            if(end_pos == value_length || value[end_pos] != mark)
            {
                ETRACE("http_parse_header_value: missing quotation close mark");
                return ELIBC_ERROR_PARSER_INVALID_INPUT;
            }

            /* report value */
            err = callback_func(callback_data, directive, value + pos, end_pos - pos);
            if(err != ELIBC_SUCCESS) return err;

            /* skip quotation */
            end_pos++;

        } else
        {
            /* parse value */
            while(end_pos < value_length && !eisspace(value[end_pos]) && value[end_pos] != ',') end_pos++;

            /* report value */
            if(end_pos > pos)
            {
                /* report value */
                err = callback_func(callback_data, directive, value + pos, end_pos - pos);
                if(err != ELIBC_SUCCESS) return err;

            } else
            {
                /* value is empty */
                err = callback_func(callback_data, directive, 0, 0);
                if(err != ELIBC_SUCCESS) return err;
            }
        }

        /* trim spaces after the value */
        pos = end_pos;
        while(pos < value_length && eisspace(value[pos])) pos++;

        /* check separator */
        if(pos < value_length && value[end_pos] != ',')
        {
            /* syntax error */
            ETRACE("http_parse_header_value: invalid directive separator character");
            return ELIBC_ERROR_PARSER_INVALID_INPUT;
        }

        /* skip separator */
        pos++;
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/

/* parse Cache-Control header */
int http_parse_cache_control(const char* value, size_t value_length,
                   void* callback_data, http_header_callback_t callback_func)
{
    /* callback must be set */
    EASSERT(callback_func);
    if(callback_func == 0) return ELIBC_ERROR_ARGUMENT;

    /* parse */
    return _http_parse_header_value(value, value_length, 
                                    http_cachecontrol_directives, HTTP_CACHE_CONTROL_DIRECTIVES_ARRAY_SIZE,
                                    callback_data, callback_func);
}

/*----------------------------------------------------------------------*/

ebool_t _http_headers_validate(const http_param_t* http_param)
{
    /* check input */
    EASSERT(http_param);
    if(http_param == 0) return ELIBC_FALSE;

    /* check parameter value type */
    if(http_parameter_is_filename(http_param) || http_parameter_is_stream(http_param))
    {
        ETRACE("http_headers: file and stream parameters cannot be set as headers");
        return ELIBC_FALSE;
    }

    /* name must be set */
    if(http_param->name == 0)
    {
        ETRACE("http_headers: header name must be set");
        return ELIBC_FALSE;
    }

    return ELIBC_TRUE;
}

/* manage headers */
int http_headers_append(http_paramset_t* headers, const http_param_t* http_param, ebool_t copy_value)
{
    /* check input */
    EASSERT(headers);
    if(headers == 0) return ELIBC_ERROR_ARGUMENT;

    /* validate header */
    if(!_http_headers_validate(http_param))  return ELIBC_ERROR_ARGUMENT;

    /* append */
    return http_paramset_append(headers, http_param, copy_value);
}

int http_headers_set_name(http_paramset_t* headers, const http_param_t* http_param, ebool_t copy_value)
{
    /* check input */
    EASSERT(headers);
    if(headers == 0) return ELIBC_ERROR_ARGUMENT;

    /* validate header */
    if(!_http_headers_validate(http_param))  return ELIBC_ERROR_ARGUMENT;

    /* set parameter */
    return http_paramset_set_name(headers, http_param, copy_value);
}

int http_headers_set_id(http_paramset_t* headers, unsigned short user_id, 
                        const http_param_t* http_param, ebool_t copy_value)
{
    /* check input */
    EASSERT(headers);
    if(headers == 0) return ELIBC_ERROR_ARGUMENT;

    /* validate header */
    if(!_http_headers_validate(http_param))  return ELIBC_ERROR_ARGUMENT;

    /* set parameter */
    return http_paramset_set_id(headers, user_id, http_param, copy_value);
}

/* known headers */
int http_headers_set(http_paramset_t* headers, http_header_t http_header, 
                     const char* value, size_t value_length, ebool_t copy_value)
{
    http_param_t http_param;

    /* check input */
    EASSERT(headers);
    if(headers == 0) return ELIBC_ERROR_ARGUMENT;

    /* check header */
    if(http_header <= HTTP_HEADER_UNKNOWN && http_header >= HTTP_HEADER_COUNT)
    {
        ETRACE("http_headers_set: unsupported header requested");
        return ELIBC_ERROR_ARGUMENT;
    }

    /* check value */
    if(value_length != 0 && value == 0)
    {
        ETRACE("http_headers_set: value is not valid");
        return ELIBC_ERROR_ARGUMENT;
    }

    /* if value_length not set assume zero terminated string */
    if(value_length == 0 && value != 0)
    {
        value_length = estrlen(value);
    }

    /* init parameter */
    http_param_init(&http_param);

    /* set parameter value */
    http_param.name = http_header_name(http_header);
    http_param.value = value;
    http_param.value_size = value_length;

    /* set parameter */
    return http_paramset_set_name(headers, &http_param, copy_value);
}

/*----------------------------------------------------------------------*/

