/*
    HTTP parser
*/

#include "../elib_config.h"

#include "http_param.h"
#include "http_mime_types.h"
#include "http_header.h"
#include "http_parse.h"

/*----------------------------------------------------------------------*/

/* flag masks */
#define HTTP_FLAG_MASK_RESET            0xFF00

/* working flags */
#define HTTP_FLAG_ERROR                 0x0001
#define HTTP_FLAG_HEADER_VALUE          0x0002

/*----------------------------------------------------------------------*/
/* state parsers */
int _http_parser_begin(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_version(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_method(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_uri(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_status_code(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_reason_phrase(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_header_line(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_header_name(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_header_value(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_wait_colon(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_wait_value(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_linefeed(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_content(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);
int _http_parser_done(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos);

/*----------------------------------------------------------------------*/
/* helpers */

#define _http_is_space(ch)      ((ch) == ' ' || (ch) == '\t')
#define _http_is_value(ch)      (((unsigned char)(ch)) > 0x20 && ((unsigned char)(ch)) < 0x7f)
#define _http_is_digit(ch)      (((unsigned char)(ch)) >= '0' && ((unsigned char)(ch)) <= '9')

/*----------------------------------------------------------------------*/
/* worker methods */
ELIBC_FORCE_INLINE int _http_append_char(http_parser_t* http_parser, char http_char)
{
    if(http_parser->parse_buffer->pos + 1 < ebuffer_size(http_parser->parse_buffer))
    {
        /* just copy to avoid extra function call */
        (ebuffer_data(http_parser->parse_buffer))[http_parser->parse_buffer->pos] = http_char;
        http_parser->parse_buffer->pos++;

        return ELIBC_SUCCESS;

    } else
    {
        return ebuffer_append_char(http_parser->parse_buffer, http_char);
    }
}

ELIBC_FORCE_INLINE int _http_skip_spaces(const char* data, size_t data_size, size_t* pos)
{
    /* ignore spaces */
    while(_http_is_space(data[*pos]))
    {
        /* check if there is data still */
        if(*pos + 1 >= data_size) return ELIBC_FALSE;

        /* next char */
        ++(*pos);
    }

    return ELIBC_TRUE;
}

ELIBC_FORCE_INLINE int _http_copy_value(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos, int* err)
{
    /* reset error */
    *err = ELIBC_SUCCESS;

    /* copy ascii characters */
    while((*pos) < data_size && _http_is_value(data[*pos]))
    {
        /* copy char */
        *err = _http_append_char(http_parser, data[*pos]);
        if(*err != ELIBC_SUCCESS) return ELIBC_FALSE;

        /* next char */
        ++(*pos);
    }

    /* stop if data ended */
    if((*pos) == data_size) return ELIBC_FALSE;

    /* validate character */
    if(data[*pos] != '\r' && !_http_is_space(data[*pos]))
    {
        ETRACE("http_parser: unsupported character");

        /* mark error */
        http_parser->flags |= HTTP_FLAG_ERROR;
        return ELIBC_FALSE;
    }

    /* value must be set (or invalid character reported) */
    EASSERT(ebuffer_pos(http_parser->parse_buffer));
    if(ebuffer_pos(http_parser->parse_buffer) == 0) 
    {
        /* mark error */
        http_parser->flags |= HTTP_FLAG_ERROR;
        return ELIBC_FALSE;
    }

    return ELIBC_TRUE;
}

ELIBC_FORCE_INLINE void _http_report_event(http_parser_t* http_parser, http_event_t http_event)
{
    /* report event */
    http_parser->callback_return = http_parser->callback(http_parser->callback_data, 
                                                         http_event, 
                                                         ebuffer_data(http_parser->parse_buffer), 
                                                         ebuffer_pos(http_parser->parse_buffer));

    /* reset buffer */
    ebuffer_reset(http_parser->parse_buffer);
}

ELIBC_FORCE_INLINE int _http_report_header_value(http_parser_t* http_parser)
{
    int err;

    /* parse known values */
    if(http_parser->active_header == HTTP_HEADER_CONTENT_LENGTH)
    {
        /* append end of line */
        err = ebuffer_append_char(http_parser->parse_buffer, 0);
        if(err != ELIBC_SUCCESS) return err;

        /* parse number */
        http_parser->content_length = estrtoull(ebuffer_data(http_parser->parse_buffer), 0, 10);

        /* report content legth */
        http_parser->callback_return = http_parser->callback(http_parser->callback_data, 
                                                             http_event_content_length, 
                                                             &http_parser->content_length, 
                                                             sizeof(http_parser->content_length));

    } else if(http_parser->active_header == HTTP_HEADER_CONTENT_TYPE)
    {
        /* report content type */
        http_parser->callback_return = http_parser->callback(http_parser->callback_data, 
                                                             http_event_content_type, 
                                                             ebuffer_data(http_parser->parse_buffer), 
                                                             ebuffer_pos(http_parser->parse_buffer));
    }

    /* check if need to stop */
    if(http_parser->callback_return == ELIBC_CONTINUE) 
    {
        /* report name */
        _http_report_event(http_parser, http_event_header_value);
    }

    return ELIBC_SUCCESS;
}

ELIBC_FORCE_INLINE int _http_validate_char(http_parser_t* http_parser, char http_char, char expected_char)
{
    /* match characters */
    if(http_char != expected_char)
    {
        /* mark error */
        http_parser->flags |= HTTP_FLAG_ERROR;
        return ELIBC_FALSE;
    }

    return ELIBC_TRUE;
}

ELIBC_FORCE_INLINE int _http_validate_space(http_parser_t* http_parser, char http_char)
{
    /* check if character is space */
    if(!_http_is_space(http_char))
    {
        /* mark error */
        http_parser->flags |= HTTP_FLAG_ERROR;
        return ELIBC_FALSE;
    }

    return ELIBC_TRUE;
}

ELIBC_FORCE_INLINE int _http_validate_value(http_parser_t* http_parser, char http_char)
{
    /* check if character is value */
    if(!_http_is_value(http_char))
    {
        /* mark error */
        http_parser->flags |= HTTP_FLAG_ERROR;
        return ELIBC_FALSE;
    }

    return ELIBC_TRUE;
}

/*----------------------------------------------------------------------*/

/* parser handle */
void http_parse_init(http_parser_t* http_parser, http_parse_callback_t parser_callback, void* user_data)
{
    EASSERT(http_parser);
    EASSERT(parser_callback);

    /* reset all fields */
    ememset(http_parser, 0, sizeof(http_parser_t));
    
    /* copy callback */
    http_parser->callback = parser_callback;
    http_parser->callback_data = user_data;
    http_parser->callback_return = ELIBC_CONTINUE;

    /* init state parsers */
    http_parser->parsers[http_state_begin] = (http_state_parse_t)_http_parser_begin;
    http_parser->parsers[http_state_version] = (http_state_parse_t)_http_parser_version;
    http_parser->parsers[http_state_method] = (http_state_parse_t)_http_parser_method;
    http_parser->parsers[http_state_uri] = (http_state_parse_t)_http_parser_uri;
    http_parser->parsers[http_state_status_code] = (http_state_parse_t)_http_parser_status_code;
    http_parser->parsers[http_state_reason_phrase] = (http_state_parse_t)_http_parser_reason_phrase;
    http_parser->parsers[http_state_header_line] = (http_state_parse_t)_http_parser_header_line;
    http_parser->parsers[http_state_header_name] = (http_state_parse_t)_http_parser_header_name;
    http_parser->parsers[http_state_header_value] = (http_state_parse_t)_http_parser_header_value;
    http_parser->parsers[http_state_wait_colon] = (http_state_parse_t)_http_parser_wait_colon;
    http_parser->parsers[http_state_wait_value] = (http_state_parse_t)_http_parser_wait_value;
    http_parser->parsers[http_state_linefeed] = (http_state_parse_t)_http_parser_linefeed;
    http_parser->parsers[http_state_content] = (http_state_parse_t)_http_parser_content;
    http_parser->parsers[http_state_done] = (http_state_parse_t)_http_parser_done;

#ifdef _ELIBC_DEBUG
    /* check that all parsers were set */    
    {
        int i;
        for(i = 0; i < http_state_count; ++i)
        {
            EASSERT1(http_parser->parsers[i], "http_parser: state parser was not set");
        }
    }
#endif /* _ELIBC_DEBUG */
}

void http_parse_close(http_parser_t* http_parser)
{
    if(http_parser)
    {
        /* reset all fields just in case */
        ememset(http_parser, 0, sizeof(http_parser_t));
    }
}

/*----------------------------------------------------------------------*/

/* parse  */
int http_parse_begin(http_parser_t* http_parser, http_parse_type_t type, ebuffer_t* parse_buffer)
{
    EASSERT(http_parser);
    EASSERT(parse_buffer);
    if(http_parser == 0 || parse_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* copy buffer reference */
    http_parser->parse_buffer = parse_buffer;

    /* reset working buffer */
    ebuffer_reset(http_parser->parse_buffer);

    /* reset state */
    http_parser->content_length = 0;
    http_parser->content_read = 0;
    http_parser->callback_return = ELIBC_CONTINUE;
    http_parser->lf_pos = 0;

    /* reset flags */
    http_parser->flags &= HTTP_FLAG_MASK_RESET;

    /* init state */
    http_parser->parse_type = type;
    http_parser->http_state = http_state_begin;
    
    return ELIBC_SUCCESS;
}

int http_parse(http_parser_t* http_parser, const char* data, size_t data_size, size_t* data_used)
{
    size_t char_pos;
    int err;

    EASSERT(http_parser);
    EASSERT(http_parser->parse_buffer);
    EASSERT(http_parser->callback);
    if(http_parser == 0 || http_parser->parse_buffer == 0 || http_parser->callback == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset error */
    err = ELIBC_SUCCESS;

    /* process all characters */
    for(char_pos = 0; char_pos < data_size && err == ELIBC_SUCCESS && http_parser->callback_return == ELIBC_CONTINUE; ++char_pos)
    {
        /* stop if all content parsed */
        if(http_parser->http_state == http_state_done) break;

        /* process text */
        EASSERT((int)http_parser->http_state < http_state_count);
        err = http_parser->parsers[http_parser->http_state](http_parser, data, data_size, &char_pos);

        /* stop if error */
        if(http_parser->flags & HTTP_FLAG_ERROR)
        {
#ifdef _ELIBC_DEBUG
            /* dump debug error report */
            {
                char __tmp[128];
                size_t __text_left = data_size - char_pos;
                size_t __text_copy = __text_left > sizeof(__tmp) - 1 ? sizeof(__tmp) - 1 : __text_left;

                estrncpy(__tmp, data + char_pos, __text_copy);
                __tmp[__text_copy] = 0;

                ETRACE1("http_parser: syntax error at  -> %s", __tmp);
            }
#endif /* _ELIBC_DEBUG */

            /* report syntax error */
            if(http_parser->callback(http_parser->callback_data, http_event_syntax_error, 0, char_pos) != ELIBC_CONTINUE)
            {
                /* stop */
                return ELIBC_ERROR_PARSER_INVALID_INPUT;
            }

            /* reset error flag and continue */
            http_parser->flags &= ~((unsigned short)HTTP_FLAG_ERROR);            
        }
    }

    /* data used */
    if(data_used) *data_used = char_pos;

    return err;
}

int http_parse_ready(http_parser_t* http_parser)
{
    EASSERT(http_parser);
    if(http_parser == 0) return ELIBC_FALSE;

    /* check state */
    return (http_parser->http_state == http_state_done) ? ELIBC_TRUE : ELIBC_FALSE;
}

/*----------------------------------------------------------------------*/
/* state parsers */
int _http_parser_begin(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    /* ignore spaces in the beginning of HTTP request */
    if(!_http_skip_spaces(data, data_size, pos)) return ELIBC_SUCCESS;

    /* switch to next state */
    if(http_parser->parse_type == http_parse_request)
        http_parser->http_state = http_state_method;
    else
        http_parser->http_state = http_state_version;

    /* return character back */
    --(*pos);

    return ELIBC_SUCCESS;
}

int _http_parser_version(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    int err;

    /* copy characters */
    if(!_http_copy_value(http_parser, data, data_size, pos, &err)) return err;

    /* report version */
    _http_report_event(http_parser, http_event_version);

    /* switch to next state */
    if(http_parser->parse_type == http_parse_request)
    {
        /* character must be CR */
        if(!_http_validate_char(http_parser, data[*pos], '\r'))
        {
            ETRACE("http_parser: expecting CR after version");
            return ELIBC_SUCCESS;
        }

        /* switch to end of line */
        http_parser->http_state = http_state_linefeed;
        http_parser->lf_pos = 1;

    } else
    {
        /* character must be space */
        if(!_http_validate_space(http_parser, data[*pos]))
        {
            ETRACE("http_parser: expecting single space after version");
            return ELIBC_SUCCESS;
        }

        /* switch to status code */
        http_parser->http_state = http_state_status_code;
    }

    return ELIBC_SUCCESS;
}

int _http_parser_method(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    int err;

    /* copy characters */
    if(!_http_copy_value(http_parser, data, data_size, pos, &err)) return err;

    /* report method */
    _http_report_event(http_parser, http_event_method);

    /* character must be space */
    if(!_http_validate_space(http_parser, data[*pos]))
    {
        ETRACE("http_parser: expecting single space after method");
        return ELIBC_SUCCESS;
    }

    /* switch to next state */
    http_parser->http_state = http_state_uri;

    return ELIBC_SUCCESS;
}

int _http_parser_uri(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    int err;

    /* copy characters */
    if(!_http_copy_value(http_parser, data, data_size, pos, &err)) return err;

    /* report URI */
    _http_report_event(http_parser, http_event_uri);

    /* character must be space */
    if(!_http_validate_space(http_parser, data[*pos]))
    {
        ETRACE("http_parser: expecting single space after URI");
        return ELIBC_SUCCESS;
    }

    /* switch to next state */
    http_parser->http_state = http_state_version;

    return ELIBC_SUCCESS;
}

int _http_parser_status_code(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    unsigned short status_code;
    int err;

    /* copy digits */
    while((*pos) < data_size && _http_is_digit(data[*pos]))
    {
        /* copy char */
        err = _http_append_char(http_parser, data[*pos]);
        if(err != ELIBC_SUCCESS) return err;

        /* next char */
        ++(*pos);
    }

    /* stop if data ended */
    if((*pos) == data_size) return ELIBC_SUCCESS;

    /* expecting 3 digits status code */
    if(ebuffer_pos(http_parser->parse_buffer) != 3) 
    {
        ETRACE("http_parser: expecting 3 digits status code");

        /* mark error */
        http_parser->flags |= HTTP_FLAG_ERROR;
        return ELIBC_SUCCESS;
    }

    /* character must be space */
    if(!_http_validate_space(http_parser, data[*pos]))
    {
        ETRACE("http_parser: expecting single space status code");
        return ELIBC_SUCCESS;
    }

    /* parse code */
    status_code = (unsigned short) eatoi2(ebuffer_data(http_parser->parse_buffer), ebuffer_pos(http_parser->parse_buffer));

    /* report code */
    http_parser->callback_return = http_parser->callback(http_parser->callback_data, 
                                                         http_event_status_code, 
                                                         &status_code, 
                                                         sizeof(status_code));

    /* reset buffer */
    ebuffer_reset(http_parser->parse_buffer);

    /* switch to next state */
    http_parser->http_state = http_state_reason_phrase;

    return ELIBC_SUCCESS;
}

int _http_parser_reason_phrase(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    int err;

    /* copy value until LF */
    while((*pos) < data_size && data[*pos] != '\r')
    {
        /* validate char */
        if(!_http_is_space(data[*pos]) && !_http_is_value(data[*pos]))
        {
            ETRACE("http_parser: expecting CR after reason phrase");

            /* mark error */
            http_parser->flags |= HTTP_FLAG_ERROR;
            return ELIBC_SUCCESS;
        }

        /* copy char */
        err = _http_append_char(http_parser, data[*pos]);
        if(err != ELIBC_SUCCESS) return err;

        /* next char */
        ++(*pos);
    }

    /* stop if data ended */
    if((*pos) == data_size) return ELIBC_SUCCESS;

    /* character must be CR */
    EASSERT(data[*pos] == '\r');

    /* report phrase */
    _http_report_event(http_parser, http_event_reason_phrase);

    /* switch to end of line */
    http_parser->http_state = http_state_linefeed;
    http_parser->lf_pos = 1;

    return ELIBC_SUCCESS;
}

int _http_parser_header_line(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    /* ignore spaces in front */
    if(!_http_skip_spaces(data, data_size, pos)) return ELIBC_SUCCESS;

    /* character must be value */
    if(!_http_validate_value(http_parser, data[*pos]))
    {
        ETRACE("http_parser: expecting header name");
        return ELIBC_SUCCESS;
    }

    /* header name must not start with colon */
    if(data[*pos] == ':')
    {
        ETRACE("http_parser: header name must not start with colon");

        /* mark error */
        http_parser->flags |= HTTP_FLAG_ERROR;
        return ELIBC_SUCCESS;
    }

    /* switch to header name */
    http_parser->http_state = http_state_header_name;

    /* return character back */
    --(*pos);

    return ELIBC_SUCCESS;
}

int _http_parser_header_name(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    const char* header_name;
    size_t name_length;
    int err;

    /* copy name characters */
    while((*pos) < data_size && _http_is_value(data[*pos]) && data[*pos] != ':')
    {
        /* copy char */
        err = _http_append_char(http_parser, data[*pos]);
        if(err != ELIBC_SUCCESS) return err;

        /* next char */
        ++(*pos);
    }

    /* stop if data ended */
    if((*pos) == data_size) return ELIBC_SUCCESS;

    /* character must be colon or space */
    if(data[*pos] != ':' && !_http_is_space(data[*pos]))
    {
        ETRACE("http_parser: expecting header name to end with colon or space");

        /* mark error */
        http_parser->flags |= HTTP_FLAG_ERROR;
        return ELIBC_SUCCESS;
    }

    /* value must be set (or invalid character reported) */
    EASSERT(ebuffer_pos(http_parser->parse_buffer));
    if(ebuffer_pos(http_parser->parse_buffer) == 0) 
    {
        ETRACE("http_parser: empty header name");

        /* mark error */
        http_parser->flags |= HTTP_FLAG_ERROR;
        return ELIBC_SUCCESS;
    }

    /* header */
    header_name = ebuffer_data(http_parser->parse_buffer);
    name_length = ebuffer_pos(http_parser->parse_buffer);

    /* parse header */
    http_parser->active_header = http_header_from_name(header_name, name_length);
    if(http_parser->active_header != HTTP_HEADER_UNKNOWN)
    {
        /* report header id */
        http_parser->callback_return = http_parser->callback(http_parser->callback_data, 
                                                             http_event_header, 
                                                             &http_parser->active_header, 
                                                             sizeof(http_parser->active_header));

        if(http_parser->callback_return != ELIBC_CONTINUE) return ELIBC_SUCCESS;
    }

    /* report header name */
    _http_report_event(http_parser, http_event_header_name);

    /* next state */
    if(_http_is_space(data[*pos]))
        http_parser->http_state = http_state_wait_colon;
    else
        http_parser->http_state = http_state_wait_value;

    return ELIBC_SUCCESS;
}

int _http_parser_header_value(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    int err;

    /* copy value until LF */
    while((*pos) < data_size && data[*pos] != '\r')
    {
        /* validate char */
        if(!_http_is_space(data[*pos]) && !_http_is_value(data[*pos]))
        {
            ETRACE("http_parser: invalid value character");

            /* mark error */
            http_parser->flags |= HTTP_FLAG_ERROR;
            return ELIBC_SUCCESS;
        }

        /* copy char */
        err = _http_append_char(http_parser, data[*pos]);
        if(err != ELIBC_SUCCESS) return err;

        /* next char */
        ++(*pos);
    }

    /* stop if data ended */
    if((*pos) == data_size) return ELIBC_SUCCESS;

    /* character must be LF */
    EASSERT(data[*pos] == '\r');

    /* mark value flag */
    http_parser->flags |= HTTP_FLAG_HEADER_VALUE;

    /* switch to end of line */
    http_parser->http_state = http_state_linefeed;
    http_parser->lf_pos = 1;

    return ELIBC_SUCCESS;
}

int _http_parser_wait_colon(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    /* skip spaces until colon */
    while((*pos) < data_size && _http_is_space(data[*pos]) && data[*pos] != ':')
    {
        /* next char */
        ++(*pos);
    }

    /* stop if data ended */
    if((*pos) == data_size) return ELIBC_SUCCESS;

    /* character must be a colon */
    if(!_http_validate_char(http_parser, data[*pos], ':'))
    {
        ETRACE("http_parser: expecting header name to end with colon");
        return ELIBC_SUCCESS;
    }

    /* wait for header value */
    http_parser->http_state = http_state_wait_value;

    return ELIBC_SUCCESS;
}

int _http_parser_wait_value(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    /* ignore spaces in front */
    if(!_http_skip_spaces(data, data_size, pos)) return ELIBC_SUCCESS;

    /* switch to header value */
    http_parser->http_state = http_state_header_value;

    /* return character back */
    --(*pos);

    return ELIBC_SUCCESS;
}

int _http_parser_linefeed(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    int err;

    /* validate input */
    if(http_parser->lf_pos == 1 || http_parser->lf_pos == 3)
    {
        /* expecting LF */
        if(!_http_validate_char(http_parser, data[*pos], '\n'))
        {
            ETRACE("http_parser: expecting LF after CR");
            return ELIBC_SUCCESS;
        }
    }

    /* check position */
    if(http_parser->lf_pos == 1)
    {
        /* next */
        http_parser->lf_pos++;

    } else if(http_parser->lf_pos == 2)
    {
        /* if CRLF is followed by space in header value it should be ignored */
        if(_http_is_space(data[*pos]) && (http_parser->flags & HTTP_FLAG_HEADER_VALUE))
        {
            /* jump back to header value */
            http_parser->http_state = http_state_header_value;

            /* return character back */
            --(*pos);

        } else if(data[*pos] == '\r')
        {
            /* next */
            http_parser->lf_pos++;

        } else
        {
            /* report header value */
            err = _http_report_header_value(http_parser);
            if(err != ELIBC_SUCCESS) return err;

            /* character must be value */
            if(!_http_validate_value(http_parser, data[*pos]))
            {
                ETRACE("http_parser: expecting header name");
                return ELIBC_SUCCESS;
            }

            /* jump to header */
            http_parser->http_state = http_state_header_line;

            /* return character back */
            --(*pos);
        }

    } else if(http_parser->lf_pos == 3)
    {
        /* inform that headers are ready */
        _http_report_event(http_parser, http_event_headers_ready);        

        /* check if we expect content */
        if(http_parser->content_length != 0)
        {
            /* jump to content state */
            http_parser->http_state = http_state_content;

        } else
        {
            /* we are done */
            _http_report_event(http_parser, http_event_done);

            /* end state */
            http_parser->http_state = http_state_done;
        }
    }

    return ELIBC_SUCCESS;
}

int _http_parser_content(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    /* count content left */
    euint64_t content_left = http_parser->content_length - http_parser->content_read;

    /* data left */
    size_t data_left = data_size - (*pos);

    /* read size */
    size_t report_size = (data_left < content_left) ? data_left : (size_t)content_left;

    /* report content */
    http_parser->callback_return = http_parser->callback(http_parser->callback_data, 
                                                         http_event_content, 
                                                         data + (*pos), 
                                                         report_size);

    /* check if we continue */
    if(http_parser->callback_return != ELIBC_CONTINUE) return ELIBC_SUCCESS;

    /* update counters */
    http_parser->content_read += report_size;
    (*pos) += report_size;

    /* check if all content reported */
    EASSERT(http_parser->content_read <= http_parser->content_length);
    if(http_parser->content_read == http_parser->content_length)
    {
        /* we are done */
        _http_report_event(http_parser, http_event_done);

        /* end state */
        http_parser->http_state = http_state_done;
    }

    return ELIBC_SUCCESS;
}

int _http_parser_done(http_parser_t* http_parser, const char* data, size_t data_size, size_t* pos)
{
    /* skip */
    (*pos) = data_size;

    EASSERT1(0, "http_parser: nothing to parse, all content reported");
    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
