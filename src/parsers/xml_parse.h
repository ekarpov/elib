/*
    XML callback parser
*/

#ifndef _XML_PARSE_H_
#define _XML_PARSE_H_

/*----------------------------------------------------------------------*/

/*
    NOTE: XML specification: https://www.w3.org/TR/REC-xml/
*/

/*----------------------------------------------------------------------*/

/* xml parser events */
typedef enum {

    /* special content */
    xml_declaration,
    xml_pi_target,
    xml_pi_content,
    xml_comment,
    xml_dtd,

    /* content */
    xml_tag_begin,
    xml_tag_end,
    xml_tag_content,
    xml_attribute_name,
    xml_attribute_value,

    /* state events */
    xml_parse_error

} xml_event_t;

/*----------------------------------------------------------------------*/

/*
    Callback parameters:
     - user data associated with parse (if set by user)
     - xml parser event
     - string parameter (depends on event type)
     - integer parameter (depends on event type)
     Return: ELIBC_CONTINUE to continue or ELIBC_STOP to stop parser
*/

/* xml parser callbacks */
typedef int (*xml_callback_t)(void*, xml_event_t, const void*, size_t);

/*----------------------------------------------------------------------*/

/* parser internal states */
typedef enum {

    xml_state_tag_scan,
    xml_state_tag_open,
    xml_state_tag_extra,
    xml_state_tag_name,
    xml_state_tag,
    xml_state_tag_end,
    xml_state_tag_close,
    xml_state_content,
    xml_state_comment,
    xml_state_attribute_name,
    xml_state_attribute,
    xml_state_attribute_value,
    xml_state_attribute_string,
    xml_state_pi_begin,
    xml_state_pi_content,
    xml_state_declaration,
    xml_state_cdata,
    xml_state_dtd,
    xml_state_escape,
    xml_state_entity,

    xml_state_count       /* must be the last */

} xml_state_t;

/*----------------------------------------------------------------------*/

/* state parser */
typedef int (*xml_state_parse_t)(void*, const char*, size_t, size_t*);

/*----------------------------------------------------------------------*/

/* xml parser data */
typedef struct {

    /* state parsers */
    xml_state_parse_t       parsers[xml_state_count];

    /* state stack */
    estack_t                tag_stack;
    ebuffer_t               name_buffer;
    xml_state_t             xml_state;

    /* data buffer */
    ebuffer_t*              parse_buffer;

    /* callback pointers */
    xml_callback_t          callback;
    void*                   callback_data;

    /* callback return value */
    int                     callback_return;

    /* parser options and state */
    unsigned short          flags;
    size_t                  buffer_offset;
    int                     dtd_depth;

    /* escape sequence parsers */
    escape_parser_t         escape_parser;
    entity_parser_t         entity_parser;

} xml_parser_t;

/*----------------------------------------------------------------------*/

/* parser handle */
void    xml_init(xml_parser_t* xml_parser, xml_callback_t parser_callback, void* user_data);
void    xml_close(xml_parser_t* xml_parser);

/* options */
int     xml_decode_escapes(xml_parser_t* xml_parser, int enable_decode);

/* parse text */
int     xml_begin(xml_parser_t* xml_parser, ebuffer_t* parse_buffer);
int     xml_parse(xml_parser_t* xml_parser, const char* text, size_t text_size);
int     xml_end(xml_parser_t* xml_parser);

/* parser state */
int     xml_get_state_tag(xml_parser_t* xml_parser, const char** name_out, size_t* length_out);

/*----------------------------------------------------------------------*/

#endif /* _XML_PARSE_H_ */

