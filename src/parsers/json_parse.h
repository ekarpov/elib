/*
    JSON callback parser
*/

#ifndef _JSON_PARSE_H_
#define _JSON_PARSE_H_

/*----------------------------------------------------------------------*/

/*
    JSON documentation: 
    http://www.json.org/

    RFC4627 (The application/json Media Type for JavaScript Object Notation (JSON)):
    http://www.ietf.org/rfc/rfc4627.txt
*/

/*----------------------------------------------------------------------*/

/* json parser events */
typedef enum {

    json_object_begin,
    json_object_end,
    json_array_begin,
    json_array_end,
    json_keyname,
    json_value_string,
    json_value_data,
    json_parse_error

} json_event_t;

/*----------------------------------------------------------------------*/

/*
    Callback parameters:
     - user data associated with parse (if set by user)
     - json parser event
     - string parameter (depends on event type)
     - integer parameter (depends on event type)
     Return: ELIBC_CONTINUE to continue or ELIBC_STOP to stop parser
*/

/* json parser callbacks */
typedef int (*json_callback_t)(void*, json_event_t, const void*, size_t);

/*----------------------------------------------------------------------*/

/* parser internal states */
typedef enum {
    
    json_state_begin,
    json_state_object,
    json_state_array,
    json_state_string,
    json_state_key,
    json_state_value,
    json_state_value_string,
    json_state_value_data,
    json_state_array_value,
    json_state_escape,
    json_state_entity,

    json_state_count       /* must be the last */

} json_state_t;

/*----------------------------------------------------------------------*/

/* state parser */
typedef int (*json_state_parse_t)(void*, const char*, size_t, size_t*);

/*----------------------------------------------------------------------*/

/* json parser data */
typedef struct {

    /* state parsers */
    json_state_parse_t      parsers[json_state_count];

    /* state stack */
    estack_t                state_stack;
    json_state_t            json_state;

    /* data buffer */
    ebuffer_t*              parse_buffer;

    /* callback pointers */
    json_callback_t         callback;
    void*                   callback_data;

    /* callback return value */
    int                     callback_return;
    
    /* parser state */
    unsigned short          flags;

    /* escape sequence parsers */
    escape_parser_t         escape_parser;
    entity_parser_t         entity_parser;

} json_parser_t;

/*----------------------------------------------------------------------*/

/* parser handle */
void    json_init(json_parser_t* json_parser, json_callback_t parser_callback, void* user_data);
void    json_close(json_parser_t* json_parser);

/* options */
int     json_decode_escapes(json_parser_t* json_parser, int enable_decode);

/* parse text */
int     json_begin(json_parser_t* json_parser, ebuffer_t* parse_buffer);
int     json_parse(json_parser_t* json_parser, const char* text, size_t text_size);
int     json_end(json_parser_t* json_parser);

/*----------------------------------------------------------------------*/

#endif /* _JSON_PARSE_H_ */

