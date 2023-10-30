/*
    json parser
*/

#include "../elib_config.h"

#include "escape_parse.h"
#include "entity_parse.h"
#include "json_parse.h"

/*----------------------------------------------------------------------*/

/* known parser tokens */
typedef enum {

    json_token_object_begin,
    json_token_object_end,
    json_token_array_begin,
    json_token_array_end,
    json_token_space,
    json_token_string_quote,
    json_token_separator,
    json_token_value_separator,
    json_token_char,
    json_token_format_char,

    json_token_count       /* must be the last */

} json_token_t;

/*----------------------------------------------------------------------*/

/* parser alphabet table */
static const json_token_t json_alphabet[128] = {

    json_token_format_char, /* 0x00 */    json_token_char, /* 0x01 */
    json_token_char, /* 0x02 */    json_token_char, /* 0x03 */
    json_token_char, /* 0x04 */    json_token_char, /* 0x05 */
    json_token_char, /* 0x06 */    json_token_char, /* 0x07 */
    json_token_char, /* 0x08 */    json_token_space, /* 0x09 */
    json_token_format_char, /* 0x0A */   json_token_space, /* 0x0B */
    json_token_space, /* 0x0C */   json_token_format_char, /* 0x0D */
    json_token_char, /* 0x0E */    json_token_char, /* 0x0F */
    json_token_char, /* 0x10 */    json_token_char, /* 0x11 */
    json_token_char, /* 0x12 */    json_token_char, /* 0x13 */
    json_token_char, /* 0x14 */    json_token_char, /* 0x15 */
    json_token_char, /* 0x16 */    json_token_char, /* 0x17 */
    json_token_char, /* 0x18 */    json_token_char, /* 0x19 */
    json_token_char, /* 0x1A */    json_token_char, /* 0x1B */
    json_token_char, /* 0x1C */    json_token_char, /* 0x1D */
    json_token_char, /* 0x1E */    json_token_char, /* 0x1F */
    json_token_space, /* 0x20 */   json_token_char, /* 0x21 */
    json_token_string_quote, /* 0x22 */    json_token_char, /* 0x23 */
    json_token_char, /* 0x24 */    json_token_char, /* 0x25 */
    json_token_char, /* 0x26 */    json_token_char, /* 0x27 */
    json_token_char, /* 0x28 */    json_token_char, /* 0x29 */
    json_token_char, /* 0x2A */    json_token_char, /* 0x2B */
    json_token_separator, /* 0x2C */       json_token_char, /* 0x2D */
    json_token_char, /* 0x2E */    json_token_char, /* 0x2F */
    json_token_char, /* 0x30 */    json_token_char, /* 0x31 */
    json_token_char, /* 0x32 */    json_token_char, /* 0x33 */
    json_token_char, /* 0x34 */    json_token_char, /* 0x35 */
    json_token_char, /* 0x36 */    json_token_char, /* 0x37 */
    json_token_char, /* 0x38 */    json_token_char, /* 0x39 */
    json_token_value_separator, /* 0x3A */ json_token_char, /* 0x3B */
    json_token_char, /* 0x3C */    json_token_char, /* 0x3D */
    json_token_char, /* 0x3E */    json_token_char, /* 0x3F */
    json_token_char, /* 0x40 */    json_token_char, /* 0x41 */
    json_token_char, /* 0x42 */    json_token_char, /* 0x43 */
    json_token_char, /* 0x44 */    json_token_char, /* 0x45 */
    json_token_char, /* 0x46 */    json_token_char, /* 0x47 */
    json_token_char, /* 0x48 */    json_token_char, /* 0x49 */
    json_token_char, /* 0x4A */    json_token_char, /* 0x4B */
    json_token_char, /* 0x4C */    json_token_char, /* 0x4D */
    json_token_char, /* 0x4E */    json_token_char, /* 0x4F */
    json_token_char, /* 0x50 */    json_token_char, /* 0x51 */
    json_token_char, /* 0x52 */    json_token_char, /* 0x53 */
    json_token_char, /* 0x54 */    json_token_char, /* 0x55 */
    json_token_char, /* 0x56 */    json_token_char, /* 0x57 */
    json_token_char, /* 0x58 */    json_token_char, /* 0x59 */
    json_token_char, /* 0x5A */    json_token_array_begin, /* 0x5B */
    json_token_char, /* 0x5C */    json_token_array_end, /* 0x5D */
    json_token_char, /* 0x5E */    json_token_char, /* 0x5F */
    json_token_char, /* 0x60 */    json_token_char, /* 0x61 */
    json_token_char, /* 0x62 */    json_token_char, /* 0x63 */
    json_token_char, /* 0x64 */    json_token_char, /* 0x65 */
    json_token_char, /* 0x66 */    json_token_char, /* 0x67 */
    json_token_char, /* 0x68 */    json_token_char, /* 0x69 */
    json_token_char, /* 0x6A */    json_token_char, /* 0x6B */
    json_token_char, /* 0x6C */    json_token_char, /* 0x6D */
    json_token_char, /* 0x6E */    json_token_char, /* 0x6F */
    json_token_char, /* 0x70 */    json_token_char, /* 0x71 */
    json_token_char, /* 0x72 */    json_token_char, /* 0x73 */
    json_token_char, /* 0x74 */    json_token_char, /* 0x75 */
    json_token_char, /* 0x76 */    json_token_char, /* 0x77 */
    json_token_char, /* 0x78 */    json_token_char, /* 0x79 */
    json_token_char, /* 0x7A */    json_token_object_begin, /* 0x7B */
    json_token_char, /* 0x7C */    json_token_object_end, /* 0x7D */
    json_token_char, /* 0x7E */    json_token_char /* 0x7F */

};

/*----------------------------------------------------------------------*/

/* convert character to token */
#define JSON_CHAR2TOKEN(ch)         (((unsigned char)(ch)) < 0x7F ? json_alphabet[((unsigned char)(ch))] : json_token_char)

/*----------------------------------------------------------------------*/

/* flag masks */
#define JSON_FLAG_MASK_RESET            0xFF00

/* working flags */
#define JSON_FLAG_ERROR                 0x0001

/* option flags */
#define JSON_FLAG_DECODE_ESCAPE         0x0100
#define JSON_FLAG_IGNORE_WARNING        0x0200

/*----------------------------------------------------------------------*/
/* state parsers */
int _json_parser_begin(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos);
int _json_parser_object(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos);
int _json_parser_array(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos);
int _json_parser_string(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos);
int _json_parser_key(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos);
int _json_parser_value(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos);
int _json_parser_value_string(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos);
int _json_parser_value_data(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos);
int _json_parser_array_value(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos);
int _json_parser_escape(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos);
int _json_parser_entity(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos);

/*----------------------------------------------------------------------*/
/* worker methods */
/*----------------------------------------------------------------------*/
ELIBC_FORCE_INLINE int _json_push_state(json_parser_t* json_parser, json_state_t json_state)
{
    /* push old state */
    int err = estack_push(&json_parser->state_stack, json_parser->json_state);
    if(err == ELIBC_SUCCESS) 
    {
        /* replace state */
        json_parser->json_state = json_state;
    }

    return err;
}

ELIBC_FORCE_INLINE int _json_pop_state(json_parser_t* json_parser)
{
    EASSERT(estack_size(&json_parser->state_stack) > 0);
    if(estack_size(&json_parser->state_stack) == 0) return ELIBC_ERROR_ARGUMENT;

    /* replace state */
    json_parser->json_state = *(json_state_t*)estack_top(&json_parser->state_stack);

    /* pop old state */
    return estack_pop(&json_parser->state_stack);
}

ELIBC_FORCE_INLINE int _json_append_char(json_parser_t* json_parser, char json_char)
{
    if(json_parser->parse_buffer->pos + 1 < ebuffer_size(json_parser->parse_buffer))
    {
        /* just copy to avoid extra function call */
        (ebuffer_data(json_parser->parse_buffer))[json_parser->parse_buffer->pos] = json_char;
        json_parser->parse_buffer->pos++;

        return ELIBC_SUCCESS;

    } else
    {
        return ebuffer_append_char(json_parser->parse_buffer, json_char);
    }
}

/*----------------------------------------------------------------------*/
/* parser init */
/*----------------------------------------------------------------------*/
void json_init(json_parser_t* json_parser, json_callback_t parser_callback, void* user_data)
{
    EASSERT(json_parser);
    EASSERT(parser_callback);

    /* reset all fields */
    ememset(json_parser, 0, sizeof(json_parser_t));

    /* init state stack */
    estack_init(&json_parser->state_stack, sizeof(json_state_t));

    /* reserve default space (ignore error, if memory allocation fails we will notice this later) */
    estack_reserve(&json_parser->state_stack, PARSER_DEFAULT_STACK_SIZE);

    /* copy callback */
    json_parser->callback = parser_callback;
    json_parser->callback_data = user_data;
    json_parser->callback_return = ELIBC_CONTINUE;

    /* init state parsers */
    json_parser->parsers[json_state_begin] = (json_state_parse_t)_json_parser_begin;
    json_parser->parsers[json_state_object] = (json_state_parse_t)_json_parser_object;
    json_parser->parsers[json_state_array] = (json_state_parse_t)_json_parser_array;
    json_parser->parsers[json_state_string] = (json_state_parse_t)_json_parser_string;
    json_parser->parsers[json_state_key] = (json_state_parse_t)_json_parser_key;
    json_parser->parsers[json_state_value] = (json_state_parse_t)_json_parser_value;
    json_parser->parsers[json_state_value_string] = (json_state_parse_t)_json_parser_value_string;
    json_parser->parsers[json_state_value_data] = (json_state_parse_t)_json_parser_value_data;
    json_parser->parsers[json_state_array_value] = (json_state_parse_t)_json_parser_array_value;    
    json_parser->parsers[json_state_escape] = (json_state_parse_t)_json_parser_escape;    
    json_parser->parsers[json_state_entity] = (json_state_parse_t)_json_parser_entity;    

#ifdef _ELIBC_DEBUG
    /* check that all parsers were set */    
    {
        int i;
        for(i = 0; i < json_state_count; ++i)
        {
            EASSERT1(json_parser->parsers[i], "json_parser: state parser was not set");
        }
    }
#endif /* _ELIBC_DEBUG */
}

void json_close(json_parser_t* json_parser)
{
    /* free buffers */
    if(json_parser)
    {
        estack_free(&json_parser->state_stack);
    }
}

/*----------------------------------------------------------------------*/
/* options */
/*----------------------------------------------------------------------*/
int json_decode_escapes(json_parser_t* json_parser, int enable_decode)
{
    EASSERT(json_parser);

    /* set flag */
    if(enable_decode)
        json_parser->flags |= JSON_FLAG_DECODE_ESCAPE;
    else
        json_parser->flags &= ~((unsigned short)JSON_FLAG_DECODE_ESCAPE);

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* parse text */
/*----------------------------------------------------------------------*/
int json_begin(json_parser_t* json_parser, ebuffer_t* parse_buffer)
{
    EASSERT(json_parser);
    EASSERT(parse_buffer);
    if(json_parser == 0 || parse_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* copy buffer reference */
    json_parser->parse_buffer = parse_buffer;

    /* reset parser state */
    estack_reset(&json_parser->state_stack);
    ebuffer_reset(json_parser->parse_buffer);
    json_parser->flags &= JSON_FLAG_MASK_RESET;

    /* beginning state */
    json_parser->json_state = json_state_begin;
    
    return ELIBC_SUCCESS;
}

int json_parse(json_parser_t* json_parser, const char* text, size_t text_size)
{
    json_token_t json_token;
    size_t char_pos;
    int err;

    EASSERT(json_parser);
    EASSERT(json_parser->parse_buffer);
    if(json_parser == 0 || json_parser->parse_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset error */
    err = ELIBC_SUCCESS;

    /* process all characters */
    for(char_pos = 0; char_pos < text_size && err == ELIBC_SUCCESS && json_parser->callback_return == ELIBC_CONTINUE; ++char_pos)
    {
        /* get token from char */
        json_token = JSON_CHAR2TOKEN(text[char_pos]);

        /* ignore special characters */
        if(json_token == json_token_format_char) continue;

        /* ignore spaces */
        if(json_token == json_token_space &&
           json_parser->json_state != json_state_string && 
           json_parser->json_state != json_state_value)
        {
            /* just ignore spaces */
            continue;
        }

        /* process text */
        EASSERT((int)json_parser->json_state < json_state_count);
        err = json_parser->parsers[json_parser->json_state](json_parser, text, text_size, &char_pos);

        /* stop if error */
        if(json_parser->flags & JSON_FLAG_ERROR)
        {
#ifdef _ELIBC_DEBUG
            /* dump debug error report */
            {
                char __tmp[128];
                size_t __text_left = text_size - char_pos;
                size_t __text_copy = __text_left > sizeof(__tmp) - 1 ? sizeof(__tmp) - 1 : __text_left;

                estrncpy(__tmp, text + char_pos, __text_copy);
                __tmp[__text_copy] = 0;

                ETRACE1("json_parser: syntax error at  -> %s", __tmp);
            }
#endif /* _ELIBC_DEBUG */

            /* report syntax error */
            if(json_parser->callback(json_parser->callback_data, json_parse_error, 0, char_pos) != ELIBC_CONTINUE)
            {
                /* stop */
                return ELIBC_ERROR_PARSER_INVALID_INPUT;
            }

            /* reset error flag and continue */
            json_parser->flags &= ~((unsigned short)JSON_FLAG_ERROR);            
        }
    }

    return err;
}

int json_end(json_parser_t* json_parser)
{
    EASSERT(json_parser);

    /* validate parser state */
    if(estack_size(&json_parser->state_stack) != 0 || json_parser->json_state != json_state_begin)
    {
        ETRACE("json_parser: parser finished in incomplete state, not enough input was given");
        return ELIBC_ERROR_PARSER_INVALID_INPUT;
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* state parsers */
/*----------------------------------------------------------------------*/
int _json_parser_begin(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos)
{
    EUNUSED(text_size);

    /* process character */
    switch(JSON_CHAR2TOKEN(text[*pos]))
    {
    case json_token_object_begin:
        /* report object started */
        json_parser->callback_return = json_parser->callback(json_parser->callback_data, json_object_begin, 0, 0);

        /* reset data buffer */
        ebuffer_reset(json_parser->parse_buffer);

        /* update state */
        return _json_push_state(json_parser, json_state_object);
        break;

    case json_token_array_begin:
        /* report array started */
        json_parser->callback_return = json_parser->callback(json_parser->callback_data, json_array_begin, 0, 0);

        /* reset data buffer */
        ebuffer_reset(json_parser->parse_buffer);

        /* update state */
        return _json_push_state(json_parser, json_state_array);
        break;

    default:
        /* mark error */
        json_parser->flags |= JSON_FLAG_ERROR;
        break;
    }

    return ELIBC_SUCCESS;
}

int _json_parser_object(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos)
{
    EUNUSED(text_size);

    /* process character */
    switch(JSON_CHAR2TOKEN(text[*pos]))
    {
    case json_token_string_quote:
        /* return character back */
        --(*pos);

        /* jump to key name state */
        return _json_push_state(json_parser, json_state_key);
        break;

    case json_token_separator:
        /* jump to key name state */
        return _json_push_state(json_parser, json_state_key);
        break;

    case json_token_object_end:        
        /* report object ended */
        json_parser->callback_return = json_parser->callback(json_parser->callback_data, json_object_end, 0, 0);

        /* pop up object */
        _json_pop_state(json_parser);        
        break;

    default:
        /* mark error */
        json_parser->flags |= JSON_FLAG_ERROR;
        break;
    }

    return ELIBC_SUCCESS;
}

int _json_parser_array(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos)
{
    EUNUSED(text_size);

    /* process character */
    switch(JSON_CHAR2TOKEN(text[*pos]))
    {
    case json_token_array_end:
        /* report array ended */
        json_parser->callback_return = json_parser->callback(json_parser->callback_data, json_array_end, 0, 0);

        /* pop up object */
        _json_pop_state(json_parser);
        break;

    case json_token_separator:
        /* jump to value state */
        return _json_push_state(json_parser, json_state_array_value);
        break;

    case json_token_array_begin:
        /* report array started (NOTE: array of array with empty array name) */
        json_parser->callback_return = json_parser->callback(json_parser->callback_data, json_array_begin, 
            ebuffer_data(json_parser->parse_buffer), ebuffer_pos(json_parser->parse_buffer));

        /* reset data buffer */
        ebuffer_reset(json_parser->parse_buffer);

        /* jump to array state */
        return _json_push_state(json_parser, json_state_array);
        break;

    default:
        /* return character back */
        --(*pos);

        /* jump to value state */
        return _json_push_state(json_parser, json_state_array_value);
        break;
    }

    return ELIBC_SUCCESS;
}

int _json_parser_string(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos)
{
    json_token_t json_token;
    int err = ELIBC_SUCCESS;

    /* process as many characters as possible */
    while(*pos < text_size && err == ELIBC_SUCCESS)
    {
        /* strings are allowed to have characters that are not in JSON alphabet */
        if(text[*pos] & 0x80)
        {
            /* copy as is */
            json_token = json_token_char;

        } else
        {
            /* get token from char */
            json_token = JSON_CHAR2TOKEN(text[*pos]);
        }

        /* ignore special characters */
        if(json_token == json_token_format_char) 
        {
            /* next char */
            ++(*pos);
            continue;
        }

        /* process character */
        if(json_token == json_token_string_quote)
        {
            /* string ended, pop stack */
            _json_pop_state(json_parser);

            /* stop */
            return ELIBC_SUCCESS;

        } else if(text[*pos] == '\\')
        {
            /* init escape parser */
            escape_begin(&json_parser->escape_parser);

            /* jump to escape state */
            return _json_push_state(json_parser, json_state_escape);

        } else if(text[*pos] == '&')
        {
            /* init entity parser */
            entity_begin(&json_parser->entity_parser);

            /* jump to entity state */
            return _json_push_state(json_parser, json_state_entity);

        } else
        {
            /* add character */
            err = _json_append_char(json_parser, text[*pos]);
        }

        /* next char */
        ++(*pos);
    }

    return err;
}

int _json_parser_key(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos)
{
    EUNUSED(text_size);

    /* process character */
    switch(JSON_CHAR2TOKEN(text[*pos]))
    {
    case json_token_string_quote:
        /* jump to string state */
        return _json_push_state(json_parser, json_state_string);
        break;

    case json_token_value_separator:
        /* jump to value state */
        return _json_push_state(json_parser, json_state_value);
        break;

    default:
        /* return character back */
        --(*pos);

        /* pop up state */
        _json_pop_state(json_parser);
        break;
    }

    return ELIBC_SUCCESS;
}

int _json_parser_value(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos)
{
    json_token_t json_token;
    int err = ELIBC_SUCCESS;

    /* get token from char */
    json_token = JSON_CHAR2TOKEN(text[*pos]);

    /* ignore spaces in front */
    while(json_token == json_token_space || json_token == json_token_format_char)
    {
        /* check if there is text still */
        if(*pos + 1 >= text_size) return ELIBC_SUCCESS;

        /* next char */
        ++(*pos);

        /* next token */
        json_token = JSON_CHAR2TOKEN(text[*pos]);
    }

    /* process character */
    switch(json_token)
    {
    case json_token_char:
    case json_token_string_quote:
        /* report keyname */
        json_parser->callback_return = json_parser->callback(json_parser->callback_data, json_keyname, 
            ebuffer_data(json_parser->parse_buffer), ebuffer_pos(json_parser->parse_buffer));

        /* reset data buffer */
        ebuffer_reset(json_parser->parse_buffer);

        if(json_token == json_token_string_quote)
        {
            /* replace state */
            json_parser->json_state = json_state_value_string;

            /* jump to string state */
            err = _json_push_state(json_parser, json_state_string);

        } else
        {
            /* replace state */
            json_parser->json_state = json_state_value_data;

            /* add character */
            err = _json_append_char(json_parser, text[*pos]);
        }

        break;

    case json_token_object_begin:
        /* report object started */
        json_parser->callback_return = json_parser->callback(json_parser->callback_data, json_object_begin, 
            ebuffer_data(json_parser->parse_buffer), ebuffer_pos(json_parser->parse_buffer));

        /* reset data buffer */
        ebuffer_reset(json_parser->parse_buffer);

        /* pop from value and jump to object state */
        json_parser->json_state = json_state_object;
        break;

    case json_token_array_begin:
        /* report array started */
        json_parser->callback_return = json_parser->callback(json_parser->callback_data, json_array_begin, 
            ebuffer_data(json_parser->parse_buffer), ebuffer_pos(json_parser->parse_buffer));

        /* reset data buffer */
        ebuffer_reset(json_parser->parse_buffer);

        /* pop from value and jump to array state */
        json_parser->json_state = json_state_array;
        break;

    default:
        /* mark error */
        json_parser->flags |= JSON_FLAG_ERROR;
        break;
    }

    return err;
}

int _json_parser_value_string(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos)
{
    EUNUSED(text);
    EUNUSED(text_size);

    /* the purpose of this state is just to report proper value */
    json_parser->callback_return = json_parser->callback(json_parser->callback_data, json_value_string, 
        ebuffer_data(json_parser->parse_buffer), ebuffer_pos(json_parser->parse_buffer));

    /* reset data buffer */
    ebuffer_reset(json_parser->parse_buffer);

    /* pop up object */
    _json_pop_state(json_parser);

    /* return character back */
    --(*pos);

    return ELIBC_SUCCESS;
}

int _json_parser_value_data(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos)
{
    json_token_t json_token;
    int err = ELIBC_SUCCESS;

    /* process text */
    while(*pos < text_size && err == ELIBC_SUCCESS)
    {
        /* get token from char */
        json_token = JSON_CHAR2TOKEN(text[*pos]);

        if(json_token == json_token_char)
        {
            /* add character */
            err = _json_append_char(json_parser, text[*pos]);

        } else
        {
            /* report data */
            json_parser->callback_return = json_parser->callback(json_parser->callback_data, json_value_data, 
                ebuffer_data(json_parser->parse_buffer), ebuffer_pos(json_parser->parse_buffer));

            /* reset data buffer */
            ebuffer_reset(json_parser->parse_buffer);

            /* pop up object */
            _json_pop_state(json_parser);

            /* return character back */
            --(*pos);

            return ELIBC_SUCCESS;
        }

        /* next char */
        ++(*pos);
    }

    return ELIBC_SUCCESS;
}

int _json_parser_array_value(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos)
{
    json_token_t json_token;
    int err = ELIBC_SUCCESS;

    /* get token from char */
    json_token = JSON_CHAR2TOKEN(text[*pos]);

    /* ignore spaces in front */
    while(json_token == json_token_space || json_token == json_token_format_char)
    {
        /* check if there is text still */
        if(*pos + 1 >= text_size) return ELIBC_SUCCESS;

        /* next char */
        ++(*pos);

        /* next token */
        json_token = JSON_CHAR2TOKEN(text[*pos]);
    }

    /* process character */
    switch(json_token)
    {
    case json_token_char:
    case json_token_string_quote:
        if(json_token == json_token_string_quote)
        {
            /* replace state */
            json_parser->json_state = json_state_value_string;

            /* jump to string state */
            err = _json_push_state(json_parser, json_state_string);

        } else
        {
            /* replace state */
            json_parser->json_state = json_state_value_data;

            /* add character */
            err = _json_append_char(json_parser, text[*pos]);
        }
        break;

    default:
        /* replace state */
        json_parser->json_state = json_state_value;

        /* return character back */
        --(*pos);
        break;
    }

    return err;
}

int _json_parser_escape(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos)
{
    escape_result_t escape_result;
    size_t text_used = 0;
    int err;

    /* process text */
    err = escape_parse(&json_parser->escape_parser, text + *pos, (text_size - *pos), &escape_result, &text_used);
    
    /* update used text */
    *pos += (text_used - 1);

    /* check if enough input */
    if(err == ELIBC_SUCCESS && escape_result == escape_result_continue) return ELIBC_SUCCESS;

    /* check if we need to output parser result */
    if(err == ELIBC_SUCCESS && escape_result == escape_result_ready && (json_parser->flags & JSON_FLAG_DECODE_ESCAPE))
    {
        /* append processed escape */
        err = ebuffer_append(json_parser->parse_buffer, 
                                escape_output(&json_parser->escape_parser), 
                                escape_output_len(&json_parser->escape_parser));
    } else
    {
        /* append original text */
        err = ebuffer_append(json_parser->parse_buffer, 
                       escape_input(&json_parser->escape_parser), 
                       escape_input_len(&json_parser->escape_parser));
    }

    /* pop up stack */
    _json_pop_state(json_parser);

    return err;
}

int _json_parser_entity(json_parser_t* json_parser, const char* text, size_t text_size, size_t* pos)
{
    entity_result_t entity_result;
    size_t text_used = 0;
    int err;

    /* process text */
    err = entity_parse(&json_parser->entity_parser, text + *pos, (text_size - *pos), &entity_result, &text_used);
    
    /* update used text */
    *pos += (text_used - 1);

    /* check if enough input */
    if(err == ELIBC_SUCCESS && entity_result == entity_result_continue) return ELIBC_SUCCESS;

    /* check if we need to output parser result */
    if(err == ELIBC_SUCCESS && entity_result == entity_result_ready && (json_parser->flags & JSON_FLAG_DECODE_ESCAPE))
    {
        /* append processed entity */
        err = ebuffer_append(json_parser->parse_buffer, 
                                entity_output(&json_parser->entity_parser), 
                                entity_output_len(&json_parser->entity_parser));
    } else
    {
        /* append original text */
        err = ebuffer_append(json_parser->parse_buffer, 
                       entity_input(&json_parser->entity_parser), 
                       entity_input_len(&json_parser->entity_parser));
    }

    /* pop up stack */
    _json_pop_state(json_parser);

    return err;
}

/*----------------------------------------------------------------------*/


