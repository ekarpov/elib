/*
    XML callback parser
*/

#include "../elib_config.h"

#include "escape_parse.h"
#include "entity_parse.h"
#include "xml_parse.h"

/*----------------------------------------------------------------------*/

/* known parser tokens */
typedef enum {

    xml_token_tag_open,
    xml_token_tag_close,
    xml_token_slash,
    xml_token_backslash,
    xml_token_space,
    xml_token_ampersand,
    xml_token_double_quote,
    xml_token_single_quote,
    xml_token_equals,
    xml_token_exclamation_mark,
    xml_token_question_mark,
    xml_token_char,
    xml_token_format_char,

    xml_token_count       /* must be the last */

} xml_token_t;

/*----------------------------------------------------------------------*/

/* parser alphabet table */
static const xml_token_t xml_alphabet[128] = {

    xml_token_format_char, /* 0x00 */ xml_token_char, /* 0x01 */
    xml_token_char, /* 0x02 */ xml_token_char, /* 0x03 */
    xml_token_char, /* 0x04 */ xml_token_char, /* 0x05 */
    xml_token_char, /* 0x06 */ xml_token_char, /* 0x07 */
    xml_token_char, /* 0x08 */ xml_token_space, /* 0x09 */
    xml_token_format_char, /* 0x0A */ xml_token_char, /* 0x0B */
    xml_token_char, /* 0x0C */ xml_token_format_char, /* 0x0D */
    xml_token_char, /* 0x0E */ xml_token_char, /* 0x0F */
    xml_token_char, /* 0x10 */ xml_token_char, /* 0x11 */
    xml_token_char, /* 0x12 */ xml_token_char, /* 0x13 */
    xml_token_char, /* 0x14 */ xml_token_char, /* 0x15 */
    xml_token_char, /* 0x16 */ xml_token_char, /* 0x17 */
    xml_token_char, /* 0x18 */ xml_token_char, /* 0x19 */
    xml_token_char, /* 0x1A */ xml_token_char, /* 0x1B */
    xml_token_char, /* 0x1C */ xml_token_char, /* 0x1D */
    xml_token_char, /* 0x1E */ xml_token_char, /* 0x1F */
    xml_token_space, /* 0x20 */ xml_token_exclamation_mark, /* 0x21 */
    xml_token_double_quote, /* 0x22 */ xml_token_char, /* 0x23 */
    xml_token_char, /* 0x24 */ xml_token_char, /* 0x25 */
    xml_token_ampersand, /* 0x26 */ xml_token_single_quote, /* 0x27 */
    xml_token_char, /* 0x28 */ xml_token_char, /* 0x29 */
    xml_token_char, /* 0x2A */ xml_token_char, /* 0x2B */
    xml_token_char, /* 0x2C */ xml_token_char, /* 0x2D */
    xml_token_char, /* 0x2E */ xml_token_slash, /* 0x2F */
    xml_token_char, /* 0x30 */ xml_token_char, /* 0x31 */
    xml_token_char, /* 0x32 */ xml_token_char, /* 0x33 */
    xml_token_char, /* 0x34 */ xml_token_char, /* 0x35 */
    xml_token_char, /* 0x36 */ xml_token_char, /* 0x37 */
    xml_token_char, /* 0x38 */ xml_token_char, /* 0x39 */
    xml_token_char, /* 0x3A */ xml_token_char, /* 0x3B */
    xml_token_tag_open, /* 0x3C */ xml_token_equals, /* 0x3D */
    xml_token_tag_close, /* 0x3E */ xml_token_question_mark, /* 0x3F */
    xml_token_char, /* 0x40 */ xml_token_char, /* 0x41 */
    xml_token_char, /* 0x42 */ xml_token_char, /* 0x43 */
    xml_token_char, /* 0x44 */ xml_token_char, /* 0x45 */
    xml_token_char, /* 0x46 */ xml_token_char, /* 0x47 */
    xml_token_char, /* 0x48 */ xml_token_char, /* 0x49 */
    xml_token_char, /* 0x4A */ xml_token_char, /* 0x4B */
    xml_token_char, /* 0x4C */ xml_token_char, /* 0x4D */
    xml_token_char, /* 0x4E */ xml_token_char, /* 0x4F */
    xml_token_char, /* 0x50 */ xml_token_char, /* 0x51 */
    xml_token_char, /* 0x52 */ xml_token_char, /* 0x53 */
    xml_token_char, /* 0x54 */ xml_token_char, /* 0x55 */
    xml_token_char, /* 0x56 */ xml_token_char, /* 0x57 */
    xml_token_char, /* 0x58 */ xml_token_char, /* 0x59 */
    xml_token_char, /* 0x5A */ xml_token_char, /* 0x5B */
    xml_token_backslash, /* 0x5C */ xml_token_char, /* 0x5D */
    xml_token_char, /* 0x5E */ xml_token_char, /* 0x5F */
    xml_token_char, /* 0x60 */ xml_token_char, /* 0x61 */
    xml_token_char, /* 0x62 */ xml_token_char, /* 0x63 */
    xml_token_char, /* 0x64 */ xml_token_char, /* 0x65 */
    xml_token_char, /* 0x66 */ xml_token_char, /* 0x67 */
    xml_token_char, /* 0x68 */ xml_token_char, /* 0x69 */
    xml_token_char, /* 0x6A */ xml_token_char, /* 0x6B */
    xml_token_char, /* 0x6C */ xml_token_char, /* 0x6D */
    xml_token_char, /* 0x6E */ xml_token_char, /* 0x6F */
    xml_token_char, /* 0x70 */ xml_token_char, /* 0x71 */
    xml_token_char, /* 0x72 */ xml_token_char, /* 0x73 */
    xml_token_char, /* 0x74 */ xml_token_char, /* 0x75 */
    xml_token_char, /* 0x76 */ xml_token_char, /* 0x77 */
    xml_token_char, /* 0x78 */ xml_token_char, /* 0x79 */
    xml_token_char, /* 0x7A */ xml_token_char, /* 0x7B */
    xml_token_char, /* 0x7C */ xml_token_char, /* 0x7D */
    xml_token_char, /* 0x7E */ xml_token_char /* 0x7F */
};

/*----------------------------------------------------------------------*/

/* keywords */
#define XML_KEYWORD_COMMENTS                "--"
#define XML_KEYWORD_DTD                     "DOCTYPE"
#define XML_KEYWORD_CDATA                   "[CDATA["
#define XML_KEYWORD_PI_END                  "?>"
#define XML_KEYWORD_CDATA_END               "]]>"
#define XML_KEYWORD_COMMENTS_BEGIN          "<!--"
#define XML_KEYWORD_COMMENTS_END            "-->"

/*----------------------------------------------------------------------*/

/* convert character to token */
#define XML_CHAR2TOKEN(ch)         (((unsigned char)(ch)) < 0x7F ? xml_alphabet[((unsigned char)(ch))] : xml_token_char)

/*----------------------------------------------------------------------*/
/* flag masks */
#define XML_FLAG_MASK_RESET                 0xFF00

/* working flags */
#define XML_FLAG_ERROR                      0x0001
#define XML_FLAG_HAS_CONTENT                0x0002
//#define XML_FLAG_SKIP_SPACES                0x0004
#define XML_FLAG_PARSE_COMMENT              0x0010
#define XML_FLAG_PARSE_CDATA                0x0020
#define XML_FLAG_PARSE_ESCAPE               0x0040
#define XML_FLAG_PARSE_ENTITY               0x0080

/* options flags */
#define XML_FLAG_DECODE_ESCAPE              0x0100

/*----------------------------------------------------------------------*/
/* state parsers */
/*----------------------------------------------------------------------*/

int _xml_parser_tag_scan(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_tag_open(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_tag_extra(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_tag_name(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_tag(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_tag_end(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_tag_close(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_content(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_comment(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_attribute_name(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_attribute(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_attribute_value(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_attribute_string(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_pi_begin(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_pi_content(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_cdata(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_dtd(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_escape(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);
int _xml_parser_entity(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos);

/*----------------------------------------------------------------------*/
/* internal types */
/*----------------------------------------------------------------------*/

/* tag stack item */
typedef struct {

    size_t              name_offset;
    size_t              name_length;

} xml_tag_t;

/* keyword matching result */
typedef enum {

    XML_RESULT_NOT_MATCHING = 0,
    XML_RESULT_MATCHING,
    XML_RESULT_CONTINUE

} xml_match_result_t;

/*----------------------------------------------------------------------*/
/* worker methods */
/*----------------------------------------------------------------------*/

ELIBC_FORCE_INLINE xml_match_result_t _xml_match_keyword(xml_parser_t* xml_parser, const char* keyword, size_t length, int ending)
{
    size_t match_offset = 0;
    size_t idx;

    /* check offset */
    if(ending && ebuffer_pos(xml_parser->parse_buffer) > length)
    {
        match_offset = ebuffer_pos(xml_parser->parse_buffer) - length;

    } else
    {
        match_offset = xml_parser->buffer_offset;
    }

    /* match parser buffer */
    for(idx = 0; idx < length && idx + match_offset < ebuffer_pos(xml_parser->parse_buffer); ++idx)
    {
        if(ebuffer_data(xml_parser->parse_buffer)[idx + match_offset] != keyword[idx]) return XML_RESULT_NOT_MATCHING;
    }

    /* check result */
    if(idx + match_offset == ebuffer_pos(xml_parser->parse_buffer))
    {
        return (idx == length) ? XML_RESULT_MATCHING : XML_RESULT_CONTINUE;
    }
    
    return XML_RESULT_NOT_MATCHING;
}

ELIBC_FORCE_INLINE void _xml_set_flag(xml_parser_t* xml_parser, unsigned short flag)
{
    /* set flag */
    xml_parser->flags |= flag;
}

ELIBC_FORCE_INLINE void _xml_clear_flag(xml_parser_t* xml_parser, unsigned short flag)
{
    /* clear flag */
    xml_parser->flags &= (~flag);
}

ELIBC_FORCE_INLINE int _xml_test_flag(xml_parser_t* xml_parser, unsigned short flag)
{
    /* test flag */
    return (xml_parser->flags & flag) ? ELIBC_TRUE : ELIBC_FALSE;
}

ELIBC_FORCE_INLINE size_t _xml_tag_count(xml_parser_t* xml_parser)
{
    return estack_size(&xml_parser->tag_stack);
}

ELIBC_FORCE_INLINE xml_tag_t* _xml_tag_stack_top(xml_parser_t* xml_parser)
{
    return (xml_tag_t*)estack_top(&xml_parser->tag_stack);
}

int _xml_skip_spaces(const char* text, size_t text_size, size_t* pos)
{
    /* ignore spaces */
    while(XML_CHAR2TOKEN(text[*pos]) == xml_token_space || 
          XML_CHAR2TOKEN(text[*pos]) == xml_token_format_char)
    {
        /* check if there is text still */
        if(*pos + 1 >= text_size) return ELIBC_FALSE;

        /* next char */
        ++(*pos);
    }

    return ELIBC_TRUE;
}

ELIBC_FORCE_INLINE int _xml_append_char(xml_parser_t* xml_parser, char xml_char)
{
    if(xml_parser->parse_buffer->pos + 1 < ebuffer_size(xml_parser->parse_buffer))
    {
        /* just copy to avoid extra function call */
        (ebuffer_data(xml_parser->parse_buffer))[xml_parser->parse_buffer->pos] = xml_char;
        xml_parser->parse_buffer->pos++;

        return ELIBC_SUCCESS;

    } else
    {
        return ebuffer_append_char(xml_parser->parse_buffer, xml_char);
    }
}

ELIBC_FORCE_INLINE void _xml_report_event(xml_parser_t* xml_parser, xml_event_t xml_event)
{
    /* report event */
    xml_parser->callback_return = xml_parser->callback(xml_parser->callback_data, 
                                                       xml_event, 
                                                       ebuffer_data(xml_parser->parse_buffer), 
                                                       ebuffer_pos(xml_parser->parse_buffer));

    /* reset buffer */
    ebuffer_reset(xml_parser->parse_buffer);
}

void _xml_report_tag_content(xml_parser_t* xml_parser)
{
    xml_token_t xml_token;
    size_t idx;

    int report_content = ELIBC_FALSE;

    /* ignore content that has only spaces and format characters */
    for(idx = 0; idx < ebuffer_pos(xml_parser->parse_buffer); ++idx)
    {
        xml_token = XML_CHAR2TOKEN(ebuffer_data(xml_parser->parse_buffer)[idx]);
        if(xml_token != xml_token_format_char && xml_token != xml_token_space)
        {
            report_content = ELIBC_TRUE;
            break;
        }
    }

    /* report content */
    if(report_content)
    {
        xml_parser->callback(xml_parser->callback_data, 
                             xml_tag_content, 
                             ebuffer_data(xml_parser->parse_buffer), 
                             ebuffer_pos(xml_parser->parse_buffer));
    }

    /* reset buffer */
    ebuffer_reset(xml_parser->parse_buffer);

    /* reset content flag */
    _xml_clear_flag(xml_parser, XML_FLAG_HAS_CONTENT);

    /* reset content offset */
    xml_parser->buffer_offset = 0;
}

ELIBC_FORCE_INLINE int _xml_handle_tag_open(xml_parser_t* xml_parser, xml_state_t next_state)
{
    xml_tag_t xml_tag;
    int err;

    /* tag name must be set */
    EASSERT(ebuffer_pos(xml_parser->parse_buffer));
    if(ebuffer_pos(xml_parser->parse_buffer) == 0) return ELIBC_ERROR_INTERNAL;

    /* init tag */
    xml_tag.name_offset = ebuffer_pos(&xml_parser->name_buffer);
    xml_tag.name_length = ebuffer_pos(xml_parser->parse_buffer);

    /* append value to name buffer */
    err = ebuffer_append(&xml_parser->name_buffer, 
                         ebuffer_data(xml_parser->parse_buffer), 
                         ebuffer_pos(xml_parser->parse_buffer));
    if(err != ELIBC_SUCCESS) return err;

    /* push tag name */
    err = estack_push(&xml_parser->tag_stack, xml_tag);
    if(err != ELIBC_SUCCESS) return err;

    /* report event */
    _xml_report_event(xml_parser, xml_tag_begin);

    /* jump to next state */
    xml_parser->xml_state = next_state;

    return ELIBC_SUCCESS;
}

ELIBC_FORCE_INLINE int _xml_handle_tag_close(xml_parser_t* xml_parser)
{
    xml_tag_t* xml_tag;
    const char* tag_name;
    size_t tag_length;

    /* stack must not be empty */
    EASSERT(estack_size(&xml_parser->tag_stack) > 0);
    if(estack_size(&xml_parser->tag_stack) == 0) return ELIBC_ERROR_ARGUMENT;

    /* stack tag */
    xml_tag = (xml_tag_t*)estack_top(&xml_parser->tag_stack);

    /* name offset must macth */
    EASSERT(ebuffer_pos(&xml_parser->name_buffer) - xml_tag->name_offset == xml_tag->name_length);
    if(ebuffer_pos(&xml_parser->name_buffer) - xml_tag->name_offset != xml_tag->name_length) return ELIBC_ERROR_ARGUMENT;

    /* get name from stack */
    tag_name = ebuffer_data(&xml_parser->name_buffer) + xml_tag->name_offset;
    tag_length = xml_tag->name_length;

    /* validate name if set */
    if(ebuffer_pos(xml_parser->parse_buffer) > 0)
    {
        /* validate tag name */
        if(estrncmp2(tag_name, tag_length, ebuffer_data(xml_parser->parse_buffer), ebuffer_pos(xml_parser->parse_buffer)) != 0)
        {
            ETRACE("xml_parser: closing tag name doesn't match");

            /* set error */
            _xml_set_flag(xml_parser, XML_FLAG_ERROR);
        }
    }

    /* report tag end event */
    xml_parser->callback(xml_parser->callback_data, xml_tag_end, tag_name, tag_length);

    /* pop tag name from buffer */
    ebuffer_setpos(&xml_parser->name_buffer, xml_tag->name_offset);

    /* pop tag from stack */
    estack_pop(&xml_parser->tag_stack);

    /* reset buffer */
    ebuffer_reset(xml_parser->parse_buffer);

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/

/* parser handle */
void xml_init(xml_parser_t* xml_parser, xml_callback_t parser_callback, void* user_data)
{
    EASSERT(xml_parser);
    EASSERT(parser_callback);

    if(xml_parser == 0 || parser_callback == 0) return;

    /* reset all fields */
    ememset(xml_parser, 0, sizeof(xml_parser_t));

    /* init state stack */
    estack_init(&xml_parser->tag_stack, sizeof(xml_tag_t));
    ebuffer_init(&xml_parser->name_buffer);

    /* reserve default space (ignore error, if memory allocation fails we will notice this later) */
    estack_reserve(&xml_parser->tag_stack, PARSER_DEFAULT_STACK_SIZE);

    /* copy callback */
    xml_parser->callback = parser_callback;
    xml_parser->callback_data = user_data;
    xml_parser->callback_return = ELIBC_CONTINUE;

    /* init state parsers */
    xml_parser->parsers[xml_state_tag_scan] = (xml_state_parse_t)_xml_parser_tag_scan;
    xml_parser->parsers[xml_state_tag_open] = (xml_state_parse_t)_xml_parser_tag_open;
    xml_parser->parsers[xml_state_tag_extra] = (xml_state_parse_t)_xml_parser_tag_extra;
    xml_parser->parsers[xml_state_tag_name] = (xml_state_parse_t)_xml_parser_tag_name;
    xml_parser->parsers[xml_state_tag] = (xml_state_parse_t)_xml_parser_tag;
    xml_parser->parsers[xml_state_tag_end] = (xml_state_parse_t)_xml_parser_tag_end;
    xml_parser->parsers[xml_state_tag_close] = (xml_state_parse_t)_xml_parser_tag_close;
    xml_parser->parsers[xml_state_content] = (xml_state_parse_t)_xml_parser_content;
    xml_parser->parsers[xml_state_comment] = (xml_state_parse_t)_xml_parser_comment;
    xml_parser->parsers[xml_state_attribute_name] = (xml_state_parse_t)_xml_parser_attribute_name;
    xml_parser->parsers[xml_state_attribute] = (xml_state_parse_t)_xml_parser_attribute;
    xml_parser->parsers[xml_state_attribute_value] = (xml_state_parse_t)_xml_parser_attribute_value;
    xml_parser->parsers[xml_state_attribute_string] = (xml_state_parse_t)_xml_parser_attribute_string;
    xml_parser->parsers[xml_state_pi_begin] = (xml_state_parse_t)_xml_parser_pi_begin;
    xml_parser->parsers[xml_state_pi_content] = (xml_state_parse_t)_xml_parser_pi_content;
    xml_parser->parsers[xml_state_declaration] = (xml_state_parse_t)_xml_parser_pi_content;
    xml_parser->parsers[xml_state_cdata] = (xml_state_parse_t)_xml_parser_cdata;
    xml_parser->parsers[xml_state_dtd] = (xml_state_parse_t)_xml_parser_dtd;
    xml_parser->parsers[xml_state_escape] = (xml_state_parse_t)_xml_parser_escape;
    xml_parser->parsers[xml_state_entity] = (xml_state_parse_t)_xml_parser_entity;

#ifdef _ELIBC_DEBUG
    /* check that all parsers were set */
    {
        int i;
        for(i = 0; i < xml_state_count; ++i)
        {
            EASSERT1(xml_parser->parsers[i], "xml_parser: state parser was not set");
        }
    }
#endif /* _ELIBC_DEBUG */
}

void xml_close(xml_parser_t* xml_parser)
{
    /* free buffers */
    if(xml_parser)
    {
        estack_free(&xml_parser->tag_stack);
        ebuffer_free(&xml_parser->name_buffer);
    }
}

/* options */
int xml_decode_escapes(xml_parser_t* xml_parser, int enable_decode)
{
    EASSERT(xml_parser);
    if(xml_parser == 0) return ELIBC_ERROR_ARGUMENT;

    /* set flag */
    if(enable_decode)
        _xml_set_flag(xml_parser, XML_FLAG_DECODE_ESCAPE);
    else
        _xml_clear_flag(xml_parser, XML_FLAG_DECODE_ESCAPE);

    return ELIBC_SUCCESS;
}

/* parse text */
int xml_begin(xml_parser_t* xml_parser, ebuffer_t *parse_buffer)
{
    EASSERT(xml_parser);
    EASSERT(xml_parser->callback);
    EASSERT(parse_buffer);
    if(xml_parser == 0 || xml_parser->callback == 0 || parse_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* copy data buffer reference */
    xml_parser->parse_buffer = parse_buffer;

    /* reset parser state */
    estack_reset(&xml_parser->tag_stack);
    ebuffer_reset(xml_parser->parse_buffer);
    ebuffer_reset(&xml_parser->name_buffer);

    /* reset working flags */
    xml_parser->flags &= XML_FLAG_MASK_RESET;
    xml_parser->buffer_offset = 0;
    xml_parser->dtd_depth = 0;

    /* beginning state */
    xml_parser->xml_state = xml_state_tag_scan;

    return ELIBC_SUCCESS;
}

int xml_parse(xml_parser_t* xml_parser, const char* text, size_t text_size)
{
    size_t char_pos;
    int err;

    EASSERT(xml_parser);
    EASSERT(xml_parser->callback);
    EASSERT(xml_parser->parse_buffer);
    if(xml_parser == 0 || xml_parser->callback == 0 || xml_parser->parse_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* ignore if text is empty */
    if(text == 0 || text_size == 0) return ELIBC_SUCCESS;

    /* reset error */
    err = ELIBC_SUCCESS;

    /* process all characters */
    for(char_pos = 0; char_pos < text_size && err == ELIBC_SUCCESS && xml_parser->callback_return == ELIBC_CONTINUE; ++char_pos)
    {
        /* get token from char */
        //xml_token_t xml_token = XML_CHAR2TOKEN(text[char_pos]);

        /* skip spaces if needed */
        //if(_xml_test_flag(xml_parser, XML_FLAG_SKIP_SPACES))
        //{
        //    /* skip spaces */
        //    if(xml_token == xml_token_space || xml_token == xml_token_format_char) continue;

        //    /* reset flag */
        //    _xml_clear_flag(xml_parser, XML_FLAG_SKIP_SPACES);
        //} 
        
        /* special states */
        if(_xml_test_flag(xml_parser, XML_FLAG_PARSE_COMMENT))
        {
            /* comments */
            err = xml_parser->parsers[xml_state_comment](xml_parser, text, text_size, &char_pos);

        } else if(_xml_test_flag(xml_parser, XML_FLAG_PARSE_CDATA))
        {
            /* CDATA */
            err = xml_parser->parsers[xml_state_cdata](xml_parser, text, text_size, &char_pos);

        } else if(_xml_test_flag(xml_parser, XML_FLAG_PARSE_ESCAPE))
        {
            /* escapes */
            err = xml_parser->parsers[xml_state_escape](xml_parser, text, text_size, &char_pos);

        } else if(_xml_test_flag(xml_parser, XML_FLAG_PARSE_ENTITY))
        {
            /* entities */
            err = xml_parser->parsers[xml_state_entity](xml_parser, text, text_size, &char_pos);

        } else
        {
            /* default processing */
            EASSERT((int)xml_parser->xml_state < xml_state_count);
            err = xml_parser->parsers[xml_parser->xml_state](xml_parser, text, text_size, &char_pos);
        }

        /* stop if error */
        if(_xml_test_flag(xml_parser, XML_FLAG_ERROR))
        {
#ifdef _ELIBC_DEBUG
            /* dump debug error report */
            {
                char __tmp[128];
                size_t __text_left = text_size - char_pos;
                size_t __text_copy = __text_left > sizeof(__tmp) - 1 ? sizeof(__tmp) - 1 : __text_left;

                estrncpy(__tmp, text + char_pos, __text_copy);
                __tmp[__text_copy] = 0;

                ETRACE("xml_parser: syntax error at  -> %s", __tmp);
            }
#endif /* _ELIBC_DEBUG */

            /* report syntax error */
            if(xml_parser->callback(xml_parser->callback_data, xml_parse_error, 0, char_pos) != ELIBC_CONTINUE)
            {
                /* stop */
                return ELIBC_ERROR_PARSER_INVALID_INPUT;
            }

            /* reset error flag and continue */
            _xml_clear_flag(xml_parser, XML_FLAG_ERROR);
        }
    }

    return err;
}

int xml_end(xml_parser_t* xml_parser)
{
    EASSERT(xml_parser);
    if(xml_parser == 0) return ELIBC_ERROR_ARGUMENT;

    /* validate parser state */
    if(estack_size(&xml_parser->tag_stack) != 0 ||
       xml_parser->xml_state != xml_state_tag_scan)
    {
        ETRACE("xml_parser: parser finished in incomplete state, not enough input was given");
        return ELIBC_ERROR_PARSER_INVALID_INPUT;
    }

    return ELIBC_SUCCESS;
}

/* parser state */
int xml_get_state_tag(xml_parser_t* xml_parser, const char** name_out, size_t* length_out)
{
    xml_tag_t* xml_tag;

    /* check input */
    EASSERT(xml_parser);
    EASSERT(name_out);
    EASSERT(length_out);
    if(xml_parser == 0 || name_out == 0 || length_out == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset output */
    *name_out = 0;
    *length_out = 0;

    /* stack must not be empty */
    EASSERT(estack_size(&xml_parser->tag_stack) > 0);
    if(estack_size(&xml_parser->tag_stack) == 0) return ELIBC_ERROR_ARGUMENT;

    /* stack tag */
    xml_tag = (xml_tag_t*)estack_top(&xml_parser->tag_stack);

    /* name offset must macth */
    EASSERT(ebuffer_pos(&xml_parser->name_buffer) - xml_tag->name_offset == xml_tag->name_length);
    if(ebuffer_pos(&xml_parser->name_buffer) - xml_tag->name_offset != xml_tag->name_length) return ELIBC_ERROR_ARGUMENT;

    /* get name from stack */
    *name_out = ebuffer_data(&xml_parser->name_buffer) + xml_tag->name_offset;
    *length_out = xml_tag->name_length;

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* state parsers */
int _xml_parser_tag_scan(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    /* ignore spaces in front */
    if(!_xml_skip_spaces(text, text_size, pos)) return ELIBC_SUCCESS;

    /* must start with tag open */
    if(XML_CHAR2TOKEN(text[*pos]) == xml_token_tag_open)
    {
        /* jump to tag open state */
        xml_parser->xml_state = xml_state_tag_open;

    } else
    {
        /* set error */
        _xml_set_flag(xml_parser, XML_FLAG_ERROR);
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_tag_open(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    /* process character */
    switch(XML_CHAR2TOKEN(text[*pos]))
    {
    case xml_token_question_mark:
        /* jump to processing instruction */
        xml_parser->xml_state = xml_state_pi_begin;
        break;

    case xml_token_exclamation_mark:
        /* jump to keyword matching */
        xml_parser->xml_state = xml_state_tag_extra;
        break;

    case xml_token_slash:
        /* jump to tag end state */
        xml_parser->xml_state = xml_state_tag_end;
        break;

    case xml_token_char:
        /* return character back */
        --(*pos);

        /* jump to tag name */
        xml_parser->xml_state = xml_state_tag_name;
        break;

    default:
        /* syntax error */
        _xml_set_flag(xml_parser, XML_FLAG_ERROR);
        break;
    }

    /* report content if any */
    if(_xml_test_flag(xml_parser, XML_FLAG_HAS_CONTENT) && 
       xml_parser->xml_state != xml_state_tag_extra)
    {
        /* report content */
        _xml_report_tag_content(xml_parser);
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_tag_extra(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    int err;
    int continue_match;
    xml_match_result_t result;

    /* process characters */
    for(; (*pos) < text_size; ++(*pos))
    {
        /* append characters to buffer */
        err = _xml_append_char(xml_parser, text[*pos]);
        if(err != ELIBC_SUCCESS) return err;

        continue_match = 0;

        /* comments */
        result = _xml_match_keyword(xml_parser, XML_KEYWORD_COMMENTS, sizeof(XML_KEYWORD_COMMENTS) - 1, ELIBC_FALSE);
        if(result == XML_RESULT_MATCHING)
        {
            /* remove keyword from buffer */
            ebuffer_setpos(xml_parser->parse_buffer, ebuffer_pos(xml_parser->parse_buffer) - sizeof(XML_KEYWORD_COMMENTS) + 1);

            /* set flag to parse comments */
            _xml_set_flag(xml_parser, XML_FLAG_PARSE_COMMENT);

            /* jump next to content state */
            xml_parser->xml_state = xml_state_content;
            break;
        } 
        
        if(result == XML_RESULT_CONTINUE) continue_match = ELIBC_TRUE;

        /* CDATA */
        result = _xml_match_keyword(xml_parser, XML_KEYWORD_CDATA, sizeof(XML_KEYWORD_CDATA) - 1, ELIBC_FALSE);
        if(result == XML_RESULT_MATCHING)
        {
            /* remove keyword from buffer */
            ebuffer_setpos(xml_parser->parse_buffer, ebuffer_pos(xml_parser->parse_buffer) - sizeof(XML_KEYWORD_CDATA) + 1);

            /* set flag to parse cdata */
            _xml_set_flag(xml_parser, XML_FLAG_PARSE_CDATA);

            /* jump next to content state */
            xml_parser->xml_state = xml_state_content;
            break;
        } 
        
        if(result == XML_RESULT_CONTINUE) continue_match = ELIBC_TRUE;

        /* DTD */
        result = _xml_match_keyword(xml_parser, XML_KEYWORD_DTD, sizeof(XML_KEYWORD_DTD) - 1, ELIBC_FALSE);
        if(result == XML_RESULT_MATCHING)
        {
            /* remove keyword from buffer */
            ebuffer_setpos(xml_parser->parse_buffer, ebuffer_pos(xml_parser->parse_buffer) - sizeof(XML_KEYWORD_DTD) + 1);

            /* init state */
            xml_parser->dtd_depth = 1;

            /* jump to dtd state */
            xml_parser->xml_state = xml_state_dtd;
            break;
        } 
        
        if(result == XML_RESULT_CONTINUE) continue_match = ELIBC_TRUE;

        /* check state */
        if(!continue_match)
        {
            /* syntax error */
            _xml_set_flag(xml_parser, XML_FLAG_ERROR);
            break;
        }
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_tag_name(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    int err;

    /* process characters */
    for(; (*pos) < text_size; ++(*pos))
    {
        switch(XML_CHAR2TOKEN(text[*pos]))
        {
        case xml_token_char:
            /* append characters to buffer */
            err = _xml_append_char(xml_parser, text[*pos]);
            if(err != ELIBC_SUCCESS) return err;
            break;

        case xml_token_format_char:
        case xml_token_space:
            /* report tag open and jump to tag state */
            return _xml_handle_tag_open(xml_parser, xml_state_tag);
            break;

        case xml_token_tag_close:
            /* report tag open and jump to content state */
            return _xml_handle_tag_open(xml_parser, xml_state_content);
            break;

        case xml_token_slash:
            /* report tag open and jump to tag close state */
            return _xml_handle_tag_open(xml_parser, xml_state_tag_close);
            break;

        default:
            /* syntax error */        
            _xml_set_flag(xml_parser, XML_FLAG_ERROR);
            return ELIBC_SUCCESS;
            break;
        }
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_tag(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    /* ignore spaces in front */
    if(!_xml_skip_spaces(text, text_size, pos)) return ELIBC_SUCCESS;

    /* process character */
    switch(XML_CHAR2TOKEN(text[*pos]))
    {
    case xml_token_char:
        /* return character back */
        --(*pos);

        /* jump to attribute name */
        xml_parser->xml_state = xml_state_attribute_name;
        break;

    case xml_token_tag_close:
        /* tag ready, jump to content */
        xml_parser->xml_state = xml_state_content;
        break;

    case xml_token_slash:
        /* jump to tag close state */
        xml_parser->xml_state = xml_state_tag_close;
        break;

    default:
        /* syntax error */
        _xml_set_flag(xml_parser, XML_FLAG_ERROR);
        break;
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_tag_end(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    int err;

    /* process characters */
    for(; (*pos) < text_size; ++(*pos))
    {
        switch(XML_CHAR2TOKEN(text[*pos]))
        {
        case xml_token_char:
            /* append characters to buffer */
            err = _xml_append_char(xml_parser, text[*pos]);
            if(err != ELIBC_SUCCESS) return err;
            break;

        case xml_token_format_char:
        case xml_token_space:
            /* jump to tag close state */
            xml_parser->xml_state = xml_state_tag_close;
            return ELIBC_SUCCESS;
            break;

        case xml_token_tag_close:
            /* return character back */
            --(*pos);

            /* jump to tag close state */
            xml_parser->xml_state = xml_state_tag_close;
            return ELIBC_SUCCESS;
            break;

        default:
            /* syntax error */        
            _xml_set_flag(xml_parser, XML_FLAG_ERROR);
            return ELIBC_SUCCESS;
            break;
        }
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_tag_close(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    int err;

    /* ignore spaces in front */
    if(!_xml_skip_spaces(text, text_size, pos)) return ELIBC_SUCCESS;

    /* must be tag close */
    if(XML_CHAR2TOKEN(text[*pos]) == xml_token_tag_close)
    {
        /* report tag close */
        err = _xml_handle_tag_close(xml_parser);
        if(err != ELIBC_SUCCESS) return err;

        /* check if there are tags on stack */
        if(estack_size(&xml_parser->tag_stack) != 0)
            /* jump to content state */
            xml_parser->xml_state = xml_state_content;
        else
            /* jump to tag scan state */
            xml_parser->xml_state = xml_state_tag_scan;

    } else
    {
        /* set error */
        _xml_set_flag(xml_parser, XML_FLAG_ERROR);
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_content(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    xml_token_t xml_token;
    int err;

    /* set content flag */
    _xml_set_flag(xml_parser, XML_FLAG_HAS_CONTENT);

    /* process characters */
    for(; (*pos) < text_size; ++(*pos))
    {
        /* current token */
        xml_token = XML_CHAR2TOKEN(text[*pos]);

        /* process token */
        if(xml_token == xml_token_backslash)
        {
            /* init escape parser */
            escape_begin(&xml_parser->escape_parser);

            /* start escape sequence reading */
            _xml_set_flag(xml_parser, XML_FLAG_PARSE_ESCAPE);

            /* stop */
            break;

        } else if(xml_token == xml_token_ampersand)
        {
            /* init entity parser */
            entity_begin(&xml_parser->entity_parser);

            /* start escape sequence reading */
            _xml_set_flag(xml_parser, XML_FLAG_PARSE_ENTITY);

            /* stop */
            break;

        } else if(xml_token == xml_token_tag_open)
        {
            /* save content offset */
            xml_parser->buffer_offset = ebuffer_pos(xml_parser->parse_buffer);

            /* jump to tag open state */
            xml_parser->xml_state = xml_state_tag_open;
            break;

        } else
        {
            /* append characters to buffer */
            err = _xml_append_char(xml_parser, text[*pos]);
            if(err != ELIBC_SUCCESS) return err;
        }
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_comment(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    int err;

    /* read content until comments end keyword */
    for(; (*pos) < text_size; ++(*pos))
    {
        /* append characters to buffer */
        err = _xml_append_char(xml_parser, text[*pos]);
        if(err != ELIBC_SUCCESS) return err;

        /* match keyword */
        if(_xml_match_keyword(xml_parser, XML_KEYWORD_COMMENTS_END, sizeof(XML_KEYWORD_COMMENTS_END) - 1, ELIBC_TRUE) == XML_RESULT_MATCHING)
        {
            /* remove keyword from buffer */
            ebuffer_setpos(xml_parser->parse_buffer, ebuffer_pos(xml_parser->parse_buffer) - sizeof(XML_KEYWORD_COMMENTS_END) + 1);

            /* report */
            EASSERT(xml_parser->buffer_offset < ebuffer_pos(xml_parser->parse_buffer))
            if(xml_parser->buffer_offset < ebuffer_pos(xml_parser->parse_buffer))
            {
                xml_parser->callback(xml_parser->callback_data, 
                                     xml_comment, 
                                     ebuffer_data(xml_parser->parse_buffer) + xml_parser->buffer_offset, 
                                     ebuffer_pos(xml_parser->parse_buffer) - xml_parser->buffer_offset);

                /* reset buffer */
                ebuffer_setpos(xml_parser->parse_buffer, xml_parser->buffer_offset);
            }

            /* reset flag */
            _xml_clear_flag(xml_parser, XML_FLAG_PARSE_COMMENT);

            /* stop */
            break;
        }
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_attribute_name(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    int err;

    /* process characters */
    for(; (*pos) < text_size; ++(*pos))
    {
        switch(XML_CHAR2TOKEN(text[*pos]))
        {
        case xml_token_char:
            /* append characters to buffer */
            err = _xml_append_char(xml_parser, text[*pos]);
            if(err != ELIBC_SUCCESS) return err;
            break;

        case xml_token_format_char:
        case xml_token_space:
            /* report attribute name */
            _xml_report_event(xml_parser, xml_attribute_name);

            /* jump to attribute state */
            xml_parser->xml_state = xml_state_attribute;
            return ELIBC_SUCCESS;
            break;

        case xml_token_equals:
            /* report attribute name */
            _xml_report_event(xml_parser, xml_attribute_name);

            /* jump to attribute value state */
            xml_parser->xml_state = xml_state_attribute_value;
            return ELIBC_SUCCESS;
            break;

        default:
            /* syntax error */        
            _xml_set_flag(xml_parser, XML_FLAG_ERROR);
            return ELIBC_SUCCESS;
            break;
        }
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_attribute(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    /* ignore spaces in front */
    if(!_xml_skip_spaces(text, text_size, pos)) return ELIBC_SUCCESS;

    /* must be equals sign */
    if(XML_CHAR2TOKEN(text[*pos]) == xml_token_equals)
    {
        /* jump to attribute value state */
        xml_parser->xml_state = xml_state_attribute_value;

    } else
    {
        /*
            NOTE: attribute must have value in XML
        */

        /* set error */
        _xml_set_flag(xml_parser, XML_FLAG_ERROR);
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_attribute_value(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    /* ignore spaces in front */
    if(!_xml_skip_spaces(text, text_size, pos)) return ELIBC_SUCCESS;

    /* value must start with double quote */
    if(XML_CHAR2TOKEN(text[*pos]) == xml_token_double_quote)
    {
        /* jump to string reading */
        xml_parser->xml_state = xml_state_attribute_string;

    } else
    {
        /* set error */
        _xml_set_flag(xml_parser, XML_FLAG_ERROR);
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_attribute_string(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    xml_token_t xml_token;
    int err;

    /* process characters */
    for(; (*pos) < text_size; ++(*pos))
    {
        /* process character */
        xml_token = XML_CHAR2TOKEN(text[*pos]);

        /* check token */
        if(xml_token == xml_token_backslash && (xml_parser->flags & XML_FLAG_DECODE_ESCAPE))
        {
            /* init escape parser */
            escape_begin(&xml_parser->escape_parser);

            /* start escape sequence reading */
            _xml_set_flag(xml_parser, XML_FLAG_PARSE_ESCAPE);

            /* stop */
            break;

        } else if(xml_token == xml_token_ampersand && (xml_parser->flags & XML_FLAG_DECODE_ESCAPE))
        {
            /* init entity parser */
            entity_begin(&xml_parser->entity_parser);

            /* start escape sequence reading */
            _xml_set_flag(xml_parser, XML_FLAG_PARSE_ENTITY);

            /* stop */
            break;

        } else if(xml_token == xml_token_double_quote)
        {
            /* report value */
            _xml_report_event(xml_parser, xml_attribute_value);

            /* jump to tag state */
            xml_parser->xml_state = xml_state_tag;

            /* stop */
            break;

        } else
        {
            /* append characters to buffer */
            err = _xml_append_char(xml_parser, text[*pos]);
            if(err != ELIBC_SUCCESS) return err;
        }
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_pi_begin(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    int err;

    /* process characters */
    for(; (*pos) < text_size; ++(*pos))
    {
        switch(XML_CHAR2TOKEN(text[*pos]))
        {
        case xml_token_char:
            /* append characters */
            err = _xml_append_char(xml_parser, text[*pos]);
            if(err != ELIBC_SUCCESS) return err;
            break;

        case xml_token_format_char:
        case xml_token_space:
            /* check if this is declaration */
            if(estrnicmp("xml", ebuffer_data(xml_parser->parse_buffer), 
                         ebuffer_pos(xml_parser->parse_buffer)) == 0)
            {
                /* reset buffer */
                ebuffer_reset(xml_parser->parse_buffer);

                /* declaration must be in the beginning */
                if(_xml_tag_count(xml_parser) == 0)
                {
                    /* jump to declaration state */
                    xml_parser->xml_state = xml_state_declaration;

                } else
                {
                    ETRACE("xml_parser: XML declaration must not be nested");

                    /* syntax error */
                    _xml_set_flag(xml_parser, XML_FLAG_ERROR);
                }
            } else
            {
                /* report PI target */ 
                _xml_report_event(xml_parser, xml_pi_target);

                /* jump to content state */
                xml_parser->xml_state = xml_state_pi_content;
            }

            return ELIBC_SUCCESS;
            break;

        default:
            /* syntax error */        
            _xml_set_flag(xml_parser, XML_FLAG_ERROR);
            return ELIBC_SUCCESS;
            break;
        }
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_pi_content(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    int err;

    /* read content until PI end keyword */
    for(; (*pos) < text_size; ++(*pos))
    {
        /* append characters to buffer */
        err = _xml_append_char(xml_parser, text[*pos]);
        if(err != ELIBC_SUCCESS) return err;

        /* PI end keyword */
        if(_xml_match_keyword(xml_parser, XML_KEYWORD_PI_END, sizeof(XML_KEYWORD_PI_END) - 1, ELIBC_TRUE) == XML_RESULT_MATCHING)
        {
            /* remove keyword from buffer */
            ebuffer_setpos(xml_parser->parse_buffer, ebuffer_pos(xml_parser->parse_buffer) - sizeof(XML_KEYWORD_PI_END) + 1);

            /* report */
            if(xml_parser->xml_state == xml_state_declaration)
                _xml_report_event(xml_parser, xml_declaration);
            else
                _xml_report_event(xml_parser, xml_pi_content);

            /* jump to next state */
            xml_parser->xml_state = xml_state_tag_scan;

            /* stop */
            break;
        }
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_cdata(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    int err;

    /* read content until CDATA end keyword */
    for(; (*pos) < text_size; ++(*pos))
    {
        /* append characters to buffer */
        err = _xml_append_char(xml_parser, text[*pos]);
        if(err != ELIBC_SUCCESS) return err;

        /* match keyword */
        if(_xml_match_keyword(xml_parser, XML_KEYWORD_CDATA_END, sizeof(XML_KEYWORD_CDATA_END) - 1, ELIBC_TRUE) == XML_RESULT_MATCHING)
        {
            /* remove keyword from buffer */
            ebuffer_setpos(xml_parser->parse_buffer, ebuffer_pos(xml_parser->parse_buffer) - sizeof(XML_KEYWORD_CDATA_END) + 1);

            /* reset flag */
            _xml_clear_flag(xml_parser, XML_FLAG_PARSE_CDATA);

            /* stop */
            break;
        }
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_dtd(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    xml_token_t xml_token;
    int err;

    /* read content until end of DTD */
    for(; (*pos) < text_size; ++(*pos))
    {
        /* process characters */
        xml_token = XML_CHAR2TOKEN(text[*pos]);

        if(xml_token == xml_token_tag_open)
        {
            /* increase depth count */
            xml_parser->dtd_depth++;

        } else if(xml_token == xml_token_tag_close)
        {
            /* decrease depth count */
            xml_parser->dtd_depth--;

            /* stop if end of DTD */
            if(xml_parser->dtd_depth == 0)
            {
                /* report DTD */
                _xml_report_event(xml_parser, xml_dtd);

                /* jump to scan state */
                xml_parser->xml_state = xml_state_tag_scan;

                /* stop */
                break;
            }
        }

        /* append characters to buffer */
        err = _xml_append_char(xml_parser, text[*pos]);
        if(err != ELIBC_SUCCESS) return err;

        /* match comments */
        if(_xml_match_keyword(xml_parser, XML_KEYWORD_COMMENTS_BEGIN, sizeof(XML_KEYWORD_COMMENTS_BEGIN) - 1, ELIBC_TRUE) == XML_RESULT_MATCHING)
        {
            /* remove keyword from buffer */
            ebuffer_setpos(xml_parser->parse_buffer, ebuffer_pos(xml_parser->parse_buffer) - sizeof(XML_KEYWORD_COMMENTS_BEGIN) + 1);

            /* parse comments */
            _xml_set_flag(xml_parser, XML_FLAG_PARSE_COMMENT);

            /* decrease depth count */
            xml_parser->dtd_depth--;

            /* save content offset */
            xml_parser->buffer_offset = ebuffer_pos(xml_parser->parse_buffer);

            /* stop */
            break;
        }
    }

    return ELIBC_SUCCESS;
}

int _xml_parser_escape(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    escape_result_t escape_result;
    size_t text_used = 0;
    int err;

    /* process text */
    err = escape_parse(&xml_parser->escape_parser, text + *pos, text_size - *pos, &escape_result, &text_used);
    
    /* update used text */
    *pos += (text_used - 1);

    /* check if enough input */
    if(err == ELIBC_SUCCESS && escape_result == escape_result_continue) return ELIBC_SUCCESS;

    /* check if we need to output parser result */
    if(err == ELIBC_SUCCESS && escape_result == escape_result_ready && (xml_parser->flags & XML_FLAG_DECODE_ESCAPE))
    {
        /* append processed escape */
        err = ebuffer_append(xml_parser->parse_buffer, 
                                escape_output(&xml_parser->escape_parser), 
                                escape_output_len(&xml_parser->escape_parser));
    } else
    {
        /* append original text */
        err = ebuffer_append(xml_parser->parse_buffer, 
                       escape_input(&xml_parser->escape_parser), 
                       escape_input_len(&xml_parser->escape_parser));
    }

    /* reset flag */
    _xml_clear_flag(xml_parser, XML_FLAG_PARSE_ESCAPE);

    return err;
}

int _xml_parser_entity(xml_parser_t* xml_parser, const char* text, size_t text_size, size_t* pos)
{
    entity_result_t entity_result;
    size_t text_used = 0;
    int err;

    /* process text */
    err = entity_parse(&xml_parser->entity_parser, text + *pos, text_size - *pos, &entity_result, &text_used);
    
    /* update used text */
    *pos += (text_used - 1);

    /* check if enough input */
    if(err == ELIBC_SUCCESS && entity_result == entity_result_continue) return ELIBC_SUCCESS;

    /* check if we need to output parser result */
    if(err == ELIBC_SUCCESS && entity_result == entity_result_ready && (xml_parser->flags & XML_FLAG_DECODE_ESCAPE))
    {
        /* append processed entity */
        err = ebuffer_append(xml_parser->parse_buffer, 
                                entity_output(&xml_parser->entity_parser), 
                                entity_output_len(&xml_parser->entity_parser));
    } else
    {
        /* append original text */
        err = ebuffer_append(xml_parser->parse_buffer, 
                       entity_input(&xml_parser->entity_parser), 
                       entity_input_len(&xml_parser->entity_parser));
    }

    /* reset flag */
    _xml_clear_flag(xml_parser, XML_FLAG_PARSE_ENTITY);

    return err;
}

/*----------------------------------------------------------------------*/
