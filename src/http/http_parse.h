/*
    HTTP parser
*/

#ifndef _HTTP_PARSE_H_
#define _HTTP_PARSE_H_

/*----------------------------------------------------------------------*/

/*
    RFC2616 (Hypertext Transfer Protocol -- HTTP/1.1):
    https://www.ietf.org/rfc/rfc2616.txt

    RFC7230 (Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing):
    https://www.ietf.org/rfc/rfc2616.txt

    HTTP request:
    https://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html

    HTTP response:
    https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html
*/

/*----------------------------------------------------------------------*/

/* http parser events */
typedef enum {

    http_event_version,
    http_event_method,
    http_event_uri,
    http_event_status_code,             /* value will point to unsigned short data */
    http_event_reason_phrase,
    http_event_content_length,          /* value will point to euint64_t data */
    http_event_content_type,
    http_event_header,                  /* value will point to http_header_t data */
    http_event_header_name,             /* reported for all headers */
    http_event_header_value,            /* reported for all headers */
    http_event_headers_ready,           /* return ELIBC_STOP from callback to process content separately */
    http_event_content,
    http_event_done,                    /* reported when content length has been reached */
    http_event_syntax_error


} http_event_t;

/*----------------------------------------------------------------------*/

/*
    Callback parameters:
     - user data associated with parse (if set by user)
     - http parser event
     - value (depends on event type)
     - value size (depends on event type)
     Return: ELIBC_CONTINUE to continue or ELIBC_STOP to stop parser
*/

/* http parser callbacks */
typedef int (*http_parse_callback_t)(void*, http_event_t, const void*, size_t);

/*----------------------------------------------------------------------*/

/* http parser state */
typedef enum {

    http_state_begin,
    http_state_version,
    http_state_method,
    http_state_uri,
    http_state_status_code,
    http_state_reason_phrase,
    http_state_header_line,
    http_state_header_name,
    http_state_header_value,
    http_state_wait_colon,
    http_state_wait_value,
    http_state_linefeed,
    http_state_content,
    http_state_done,

    http_state_count,             /* must be the last */

} http_state_t;

/* http parser type */
typedef enum {

    http_parse_request,
    http_parse_response

} http_parse_type_t;

/*----------------------------------------------------------------------*/

/* state parser */
typedef int (*http_state_parse_t)(void*, const char*, size_t, size_t*);

/*----------------------------------------------------------------------*/

/* http parser data */
typedef struct {

    /* state parsers */
    http_state_parse_t      parsers[http_state_count];

    /* parser state */
    http_parse_type_t       parse_type;
    http_state_t            http_state;
    http_header_t           active_header;
    unsigned short          flags;
    unsigned short          lf_pos;

    /* content */
    euint64_t               content_length;
    euint64_t               content_read;

    /* data buffer */
    ebuffer_t*              parse_buffer;

    /* callback pointers */
    http_parse_callback_t   callback;
    void*                   callback_data;

    /* callback return value */
    int                     callback_return;
    
} http_parser_t;

/*----------------------------------------------------------------------*/

/* parser handle */
void    http_parse_init(http_parser_t* http_parser, http_parse_callback_t parser_callback, void* user_data);
void    http_parse_close(http_parser_t* http_parser);

/* parse  */
int     http_parse_begin(http_parser_t* http_parser, http_parse_type_t type, ebuffer_t* parse_buffer);
int     http_parse(http_parser_t* http_parser, const char* data, size_t data_size, size_t* data_used);
int     http_parse_ready(http_parser_t* http_parser);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_PARSE_H_ */

