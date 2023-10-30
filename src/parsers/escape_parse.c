/*
    Escape sequence parser (utf8 encoded output)
*/

#include "../elib_config.h"

#include "../text/text_format.h"
#include "escape_parse.h"

/*----------------------------------------------------------------------*/

/*
    JavaScript character escape sequences: https://mathiasbynens.be/notes/javascript-escapes
*/

/*----------------------------------------------------------------------*/

/* constants */
#define ESCAPE_MAX_OCTAL_INPUT                  4
#define ESCAPE_MAX_HEX_INPUT                    6
#define ESCAPE_MAX_UNICODE_CHAR_INPUT           6
#define ESCAPE_MAX_UNICODE_INPUT                8
#define ESCAPE_MAX_SURROGATE_INPUT              12
#define ESCAPE_MAX_ECMA_INPUT                   12

/* unicode surrogates */
#define ESCAPE_UNICODE_SURROGATE_HIGH_START     0xD800
#define ESCAPE_UNICODE_SURROGATE_HIGH_END       0xDBFF
#define ESCAPE_UNICODE_SURROGATE_LOW_START      0xDC00
#define ESCAPE_UNICODE_SURROGATE_LOW_END        0xDFFF

/*----------------------------------------------------------------------*/
/* state parsers */

int _escape_parse_octal(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out);
int _escape_parse_hex(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out);
int _escape_parse_unicode(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out);
int _escape_parse_unicode_char(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out);
int _escape_parse_unicode_surrogate(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out);
int _escape_parse_unicode_ecma6(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out);

/*----------------------------------------------------------------------*/

/* parse text (do not pass first backslash to parser) */
int escape_begin(escape_parser_t* escape_parser)
{
    EASSERT(escape_parser);
    if(escape_parser == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset all fields */
    ememset(escape_parser, 0, sizeof(escape_parser_t));

    /* reset state */
    escape_parser->state = escape_parse_init;

    return ELIBC_SUCCESS;
}

int escape_parse(escape_parser_t* escape_parser, const char* text, size_t text_size, 
                     escape_result_t* result_out, size_t* size_out)
{
    size_t pos = 0;

    EASSERT(escape_parser);
    EASSERT(text);
    EASSERT(text_size > 0);
    EASSERT(result_out);
    if(escape_parser == 0 || text == 0 || text_size <= 0 || result_out == 0) return ELIBC_ERROR_ARGUMENT;

    /* set local pointer if not set */
    if(size_out == 0) size_out = &pos;

    /* init output */
    *result_out = escape_result_continue;
    *size_out = 0;

    /* check state */
    if(escape_parser->state == escape_parse_init)
    {
        /* init input */
        escape_parser->input[0] = '\\';
        escape_parser->input[1] = text[0];
        escape_parser->input_pos = 2;

        /* check format instruction */
        switch(text[0])
        {
        case 'b':
            escape_parser->output[0] = '\b';
            break;
        case 'f':
            escape_parser->output[0] = '\f';
            break;
        case 'n':
            escape_parser->output[0] = '\n';
            break;
        case 'r':
            escape_parser->output[0] = '\r';
            break;
        case 't':
            escape_parser->output[0] = '\t';
            break;
        case 'v':
            escape_parser->output[0] = '\v';
            break;
        case '\'':
            escape_parser->output[0] = '\'';
            break;
        case '"':
            escape_parser->output[0] = '"';
            break;
        case '\\':
            escape_parser->output[0] = '\\';
            break;
        case '/':
            escape_parser->output[0] = '/';
            break;
        case 'x':
        case 'X':
            escape_parser->state = escape_parse_hex;
            break;
        case 'U':
            escape_parser->state = escape_parse_unicode;
            break;
        case 'u':
            escape_parser->state = escape_parse_unicode_char;
            break;

        default:
            /* octal mode if character is digit */
            if(eisdigit(text[0]))
            {
                escape_parser->state = escape_parse_octal;

            } else
            {
                /* copy character as is */
                escape_parser->output[0] = text[0];
            }
            break;
        }

        /* next char */
        (*size_out)++;

        /* check if this was single char escape */
        if(escape_parser->state == escape_parse_init)
        {
            /* result */
            *result_out = escape_result_ready;

            /* output is single character */
            escape_parser->output_pos = 1;

            /* stop */
            return ELIBC_SUCCESS;
        }

        /* stop if there is no more data */
        if((*size_out) >= text_size) return ELIBC_SUCCESS;
    }

    /* process characters */
    EASSERT(escape_parser->state != escape_parse_init);
    switch(escape_parser->state)
    {
    case escape_parse_octal:
        return _escape_parse_octal(escape_parser, text, text_size, result_out, size_out);
        break;

    case escape_parse_hex:
        return _escape_parse_hex(escape_parser, text, text_size, result_out, size_out);
        break;

    case escape_parse_unicode:
        return _escape_parse_unicode(escape_parser, text, text_size, result_out, size_out);
        break;

    case escape_parse_unicode_char:
        return _escape_parse_unicode_char(escape_parser, text, text_size, result_out, size_out);
        break;

    case escape_parse_unicode_ecma6:
        return _escape_parse_unicode_ecma6(escape_parser, text, text_size, result_out, size_out);
        break;

    case escape_parse_unicode_surrogate:
        return _escape_parse_unicode_surrogate(escape_parser, text, text_size, result_out, size_out);
        break;
    }

    EASSERT1(0, "escape_parser: invalid internal state");
    return ELIBC_ERROR_INTERNAL;
}

/* input sequence (input is stored in case e.g. conversion failed, backslash is included) */
const char* escape_input(escape_parser_t* escape_parser)
{
    EASSERT(escape_parser);
    if(escape_parser == 0) return 0;

    return escape_parser->input;
}

size_t escape_input_len(escape_parser_t* escape_parser)
{
    EASSERT(escape_parser);
    if(escape_parser == 0) return 0;

    return escape_parser->input_pos;
}

/* output sequence (utf8 encoded) */
const char* escape_output(escape_parser_t* escape_parser)
{
    EASSERT(escape_parser);
    if(escape_parser == 0) return 0;

    return escape_parser->output;
}

size_t escape_output_len(escape_parser_t* escape_parser)
{
    EASSERT(escape_parser);
    if(escape_parser == 0) return 0;

    return escape_parser->output_pos;
}

/*----------------------------------------------------------------------*/
/* worker methods */
int _escape_convert_utf32(escape_parser_t* escape_parser, euint32_t hex_num)
{
    /* maximum output size */
    escape_parser->utf16_pos = sizeof(escape_parser->utf16_buffer) / sizeof(escape_parser->utf16_buffer[0]);

    /* convert utf32 into utf16 first */
    if(utf32_to_utf16(&hex_num, 1, escape_parser->utf16_buffer, &escape_parser->utf16_pos) == 0)
    {
        ETRACE("escape_parser: failed to convert Unicode character to utf16");
        return ELIBC_ERROR_INVALID_DATA;
    }

    /* convert to utf8 */
    if(utf16_to_utf8(escape_parser->utf16_buffer, escape_parser->utf16_pos, 
                    (utf8_t*)escape_parser->output, &(escape_parser->output_pos)) != escape_parser->utf16_pos)
    {
        ETRACE("escape_parser: failed to convert Unicode character to utf8");
        return ELIBC_ERROR_INVALID_DATA;
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* state parsers */

int _escape_parse_octal(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out)
{
    size_t idx;

    /* copy characters */
    for(; (*size_out) < text_size && eisdigit(text[*size_out]) && escape_parser->input_pos < ESCAPE_MAX_OCTAL_INPUT; ++(*size_out))
    {
        /* copy input */
        escape_parser->input[escape_parser->input_pos] = text[*size_out];
        escape_parser->input_pos++;
    }

    /* validate state */
    EASSERT(escape_parser->input_pos <= ESCAPE_MAX_OCTAL_INPUT);

    /* stop if not a digit or maximum size (\ooo)*/
    if(((*size_out) < text_size && !eisdigit(text[*size_out])) || escape_parser->input_pos >= ESCAPE_MAX_OCTAL_INPUT)
    {
        /* check if there is something */
        if(escape_parser->input_pos == 1)
        {
            ETRACE("escape_parser: invalid character while parsing octal escape");
            return ELIBC_ERROR_INVALID_DATA;
        }

        /* convert output */
        for(idx = 1; idx < escape_parser->input_pos; ++idx) 
        {
            /* convert \ooo to single char */
            escape_parser->output[0] = escape_parser->output[0] * 10 + (escape_parser->input[idx] - '0');
        }

        /* output is single char */
        escape_parser->output_pos = 1;

        /* mark as ready */
        *result_out = escape_result_ready;
    }

    return ELIBC_SUCCESS;
}

int _escape_parse_hex(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out)
{
    /* copy characters */
    for(; (*size_out) < text_size && is_valid_hexchar(text[*size_out]) && escape_parser->input_pos < ESCAPE_MAX_HEX_INPUT; ++(*size_out))
    {
        /* copy input */
        escape_parser->input[escape_parser->input_pos] = text[*size_out];
        escape_parser->input_pos++;
    }

    /* validate state */
    EASSERT(escape_parser->input_pos <= ESCAPE_MAX_HEX_INPUT);

    /* stop if not a hex or maximum size */
    if(((*size_out) < text_size && !is_valid_hexchar(text[*size_out])) || escape_parser->input_pos >= ESCAPE_MAX_HEX_INPUT)
    {
        /* check if there is something */
        if(escape_parser->input_pos == 2)
        {
            ETRACE("escape_parser: invalid character while parsing hex escape");
            return ELIBC_ERROR_INVALID_DATA;
        }

        /* convert output (ignore \x) */
        hexstr_to_uint16(escape_parser->input + 2, escape_parser->input_pos - 2, escape_parser->utf16_buffer);

        /* check if character is Unicode */
        if(escape_parser->utf16_buffer[0] < 0x80)
        {
            /* output single char */
            escape_parser->output[0] = (char)escape_parser->utf16_buffer[0];
            escape_parser->output_pos = 1;

        } else
        {
            /* convert to utf8 */
            if(utf16_to_utf8(escape_parser->utf16_buffer, 1, (utf8_t*)escape_parser->output, &(escape_parser->output_pos)) != 1)
            {
                ETRACE("escape_parser: failed to convert Unicode character in hexadecimal notation to utf8");
                return ELIBC_ERROR_INVALID_DATA;
            }
        }

        /* mark as ready */
        *result_out = escape_result_ready;
    }

    return ELIBC_SUCCESS;
}

int _escape_parse_unicode(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out)
{
    euint32_t hex_num;
    int err;

    /* copy characters */
    for(; (*size_out) < text_size && is_valid_hexchar(text[*size_out]) && escape_parser->input_pos < ESCAPE_MAX_UNICODE_INPUT; ++(*size_out))
    {
        /* copy input */
        escape_parser->input[escape_parser->input_pos] = text[*size_out];
        escape_parser->input_pos++;
    }

    /* validate state */
    EASSERT(escape_parser->input_pos <= ESCAPE_MAX_UNICODE_INPUT);

    /* stop if not a hex or maximum size */
    if(((*size_out) < text_size && !is_valid_hexchar(text[*size_out])) || escape_parser->input_pos >= ESCAPE_MAX_UNICODE_INPUT)
    {
        /* check if there is something */
        if(escape_parser->input_pos == 2)
        {
            ETRACE("escape_parser: invalid character while parsing Unicode escape");
            return ELIBC_ERROR_INVALID_DATA;
        }

        /* convert output (ignore \U) */
        hexstr_to_uint32(escape_parser->input + 2, escape_parser->input_pos - 2, &hex_num);

        /* check if character is Unicode */
        if(hex_num < 0x80)
        {
            /* output single char */
            escape_parser->output[0] = (char)hex_num;
            escape_parser->output_pos = 1;

        } else
        {
            /* convert to utf8 */
            err = _escape_convert_utf32(escape_parser, hex_num);
            if(err != ELIBC_SUCCESS) return err;
        }

        /* mark as ready */
        *result_out = escape_result_ready;
    }

    return ELIBC_SUCCESS;
}

int _escape_parse_unicode_char(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out)
{
    euint32_t hex_num;
    int err;

    /* check for ECMAScript 6 Unicode escapes */
    if(escape_parser->input_pos == 2 && (*size_out) < text_size && text[*size_out] == '{')
    {
        /* copy input */
        escape_parser->input[escape_parser->input_pos] = text[*size_out];
        escape_parser->input_pos++;
        (*size_out)++;

        /* jump to ECMA state parser */
        escape_parser->state = escape_parse_unicode_ecma6;

        /* switch to state parser */
        if((*size_out) < text_size)
            return _escape_parse_unicode_ecma6(escape_parser, text, text_size, result_out, size_out);
        else
            return ELIBC_SUCCESS;

    }

    /* copy characters */
    for(; (*size_out) < text_size && is_valid_hexchar(text[*size_out]) && escape_parser->input_pos < ESCAPE_MAX_UNICODE_CHAR_INPUT; ++(*size_out))
    {
        /* copy input */
        escape_parser->input[escape_parser->input_pos] = text[*size_out];
        escape_parser->input_pos++;
    }

    /* validate state */
    EASSERT(escape_parser->input_pos <= ESCAPE_MAX_UNICODE_CHAR_INPUT);

    /* stop if not a hex or maximum size */
    if(((*size_out) < text_size && !is_valid_hexchar(text[*size_out])) || escape_parser->input_pos >= ESCAPE_MAX_UNICODE_CHAR_INPUT)
    {
        /* check if there is something */
        if(escape_parser->input_pos == 2)
        {
            ETRACE("escape_parser: invalid character while parsing Unicode character escape");
            return ELIBC_ERROR_INVALID_DATA;
        }

        /* convert output (ignore \u) */
        hexstr_to_uint32(escape_parser->input + 2, escape_parser->input_pos - 2, &hex_num);

        /* check if character is Unicode */
        if(hex_num < 0x80)
        {
            /* output single char */
            escape_parser->output[0] = (char)hex_num;
            escape_parser->output_pos = 1;

        } else if(hex_num >= ESCAPE_UNICODE_SURROGATE_HIGH_START && 
                  hex_num <= ESCAPE_UNICODE_SURROGATE_HIGH_END)
        {
            /* save to buffer */
            escape_parser->utf16_buffer[0] = (euint16_t)hex_num;

            /* unicode character is surrogate, jump to surrogate state */
            escape_parser->state = escape_parse_unicode_surrogate;
            escape_parser->low_offset = escape_parser->input_pos;

            /* switch to state parser */
            if((*size_out) < text_size)
                return _escape_parse_unicode_surrogate(escape_parser, text, text_size, result_out, size_out);
            else
                return ELIBC_SUCCESS;

        } else
        {
            /* convert to utf8 */
            err = _escape_convert_utf32(escape_parser, hex_num);
            if(err != ELIBC_SUCCESS) return err;
        }

        /* mark as ready */
        *result_out = escape_result_ready;
    }

    return ELIBC_SUCCESS;
}

int _escape_parse_unicode_surrogate(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out)
{
    euint32_t hex_num;

    /* check if there is input */
    if((*size_out) >= text_size) return ELIBC_SUCCESS;

    /* we assume that surrogates follow each other without any spaces */
    if(escape_parser->input_pos == escape_parser->low_offset)
    {
        if(text[*size_out] != '\\') 
        {
            ETRACE("escape_parser: expecting low surrogate to follow (expecting escape character)");
            return ELIBC_ERROR_INVALID_DATA;
        }

        /* copy input */
        escape_parser->input[escape_parser->input_pos] = text[*size_out];
        escape_parser->input_pos++;
        (*size_out)++;
    } 
    
    if(escape_parser->input_pos == escape_parser->low_offset + 1)
    {
        if(text[*size_out] != 'u' && text[*size_out] != 'U') 
        {
            ETRACE("escape_parser: expecting low surrogate to follow (expecting 'u')");
            return ELIBC_ERROR_INVALID_DATA;
        }

        /* copy input */
        escape_parser->input[escape_parser->input_pos] = text[*size_out];
        escape_parser->input_pos++;
        (*size_out)++;
    }

    /* copy characters */
    for(; (*size_out) < text_size && is_valid_hexchar(text[*size_out]) && escape_parser->input_pos < ESCAPE_MAX_SURROGATE_INPUT; ++(*size_out))
    {
        /* copy input */
        escape_parser->input[escape_parser->input_pos] = text[*size_out];
        escape_parser->input_pos++;
    }

    /* validate state */
    EASSERT(escape_parser->input_pos <= ESCAPE_MAX_SURROGATE_INPUT);

    /* stop if not a hex or maximum size */
    if(((*size_out) < text_size && !is_valid_hexchar(text[*size_out])) || escape_parser->input_pos >= ESCAPE_MAX_SURROGATE_INPUT)
    {
        /* check if there is something */
        if(escape_parser->input_pos == escape_parser->low_offset + 2)
        {
            ETRACE("escape_parser: invalid character while parsing Unicode surrogate escape");
            return ELIBC_ERROR_INVALID_DATA;
        }

        /* convert output (ignore \u) */
        hexstr_to_uint32(escape_parser->input + escape_parser->low_offset + 2, 
                      escape_parser->input_pos - escape_parser->low_offset - 2, 
                      &hex_num);

        /* expecting low surrogate */
        if(hex_num >= ESCAPE_UNICODE_SURROGATE_LOW_START && 
           hex_num <= ESCAPE_UNICODE_SURROGATE_LOW_END)
        {
            /* copy next */
            escape_parser->utf16_buffer[1] = (euint16_t)hex_num;

            /* convert to utf8 */
            if(utf16_to_utf8(escape_parser->utf16_buffer, 2, (utf8_t*)escape_parser->output, &(escape_parser->output_pos)) != 2)
            {
                ETRACE("escape_parser: failed to convert Unicode surrogate pair to utf8");
                return ELIBC_ERROR_INVALID_DATA;
            }

            /* mark as ready */
            *result_out = escape_result_ready;

        } else
        {
            ETRACE("escape_parser: expecting low surrogate to follow");
            return ELIBC_ERROR_INVALID_DATA;
        }
    }

    return ELIBC_SUCCESS;
}

int _escape_parse_unicode_ecma6(escape_parser_t* escape_parser, const char* text, size_t text_size, escape_result_t* result_out, size_t* size_out)
{
    euint32_t hex_num;
    int err;

    /* copy characters */
    for(; (*size_out) < text_size && text[*size_out] != '}' && escape_parser->input_pos < ESCAPE_MAX_ECMA_INPUT; ++(*size_out))
    {
        /* copy input */
        escape_parser->input[escape_parser->input_pos] = text[*size_out];
        escape_parser->input_pos++;

        /* validate characters */
        if(!is_valid_hexchar(text[*size_out]))
        {
            ETRACE("escape_parser: invalid hex character while parsing ECMA escape sequence");
            return ELIBC_ERROR_INVALID_DATA;
        }
    }

    /* validate state */
    EASSERT(escape_parser->input_pos <= ESCAPE_MAX_ECMA_INPUT);

    /* check if ready */
    if((*size_out) < text_size && text[*size_out] == '}')
    {
        /* copy last char */
        escape_parser->input[escape_parser->input_pos] = text[*size_out];
        escape_parser->input_pos++;
        (*size_out)++;

        /* validate state */
        EASSERT(escape_parser->input_pos >= 4);

        /* check if there is something */
        if(escape_parser->input_pos == 4)
        {
            ETRACE("escape_parser: empty ECMA sequence");
            return ELIBC_ERROR_INVALID_DATA;
        }

        /* convert output (ignore \u{ and } ) */
        hexstr_to_uint32(escape_parser->input + 3, escape_parser->input_pos - 4, &hex_num);

        /* convert to utf8 */
        err = _escape_convert_utf32(escape_parser, hex_num);
        if(err != ELIBC_SUCCESS) return err;

        /* mark as ready */
        *result_out = escape_result_ready;

    } else
    {
        /* check if maximum input */
        if(escape_parser->input_pos == ESCAPE_MAX_ECMA_INPUT)
        {
            ETRACE("escape_parser: maximum ECMA escape sequence number reached");
            return ELIBC_ERROR_INVALID_DATA;
        }
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
