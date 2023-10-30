/*
    Escape sequence parser (utf8 encoded output)
*/

#ifndef _ESCAPE_PARSE_H_
#define _ESCAPE_PARSE_H_

/*----------------------------------------------------------------------*/

/*

    NOTE: Supported escape sequences: 

    \b: backspace (U+0008 BACKSPACE)
    \f: form feed (U+000C FORM FEED)
    \n: line feed (U+000A LINE FEED)
    \r: carriage return (U+000D CARRIAGE RETURN)
    \t: horizontal tab (U+0009 CHARACTER TABULATION)
    \v: vertical tab (U+000B LINE TABULATION)
    \': single quote (U+0027 APOSTROPHE)
    \": double quote (U+0022 QUOTATION MARK)
    \\: backslash (U+005C REVERSE SOLIDUS)
    \/: slash (U+002F SOLIDUS)
    \ooo    ASCII character in octal notation (one, two or three characters)
    \x hh   ASCII character in hexadecimal notation (e.g. \xA9)
    \x hhhh Unicode character in hexadecimal notation (e.g. \x4e00, must not be surrogate)
    \u hhhh Unicode character in hexadecimal notation (e.g. \u4e00, four digits, must not be surrogate)
    \u hhhh Unicode surrogate pair (e.g. \uD834\uDF06, must be 12 digits)
    \U hhhhhh Unicode escape sequence (e.g. \u002665, six digits, up to 0x10FFFF)
    \u {hhhhhhhh} ECMAScript 6 Unicode code point escapes (e.g. \u{1D306}, max eight digits, up to 0x10FFFF)

    NOTE: if first Unicode character is in "high surrogates" range (D800–DBFF) parser 
          will expect next input in "low surrogates" (DC00–DFFF)

    NOTE: All other escaped characters will be translated as the character itself.

*/

/*----------------------------------------------------------------------*/

/* constants */
#define ESCAPE_MAX_INPUT_LENGTH         16      /* max input length (two Unicode surrogates) */
#define ESCAPE_MAX_OUTPUT_LENGTH        4       /* utf8 max character length (https://tools.ietf.org/html/rfc3629) */

/*----------------------------------------------------------------------*/

/* escape parser result */
typedef enum {

    escape_result_continue,
    escape_result_ready

} escape_result_t;

/* escape parser state */
typedef enum {

    escape_parse_init,
    escape_parse_octal,
    escape_parse_hex,
    escape_parse_unicode,
    escape_parse_unicode_char,
    escape_parse_unicode_surrogate,
    escape_parse_unicode_ecma6

} escape_state_t;

/* escape parser data */
typedef struct {
    
    /* input buffer */
    char            input[ESCAPE_MAX_INPUT_LENGTH];
    size_t          input_pos;

    /* output buffer */
    char            output[ESCAPE_MAX_OUTPUT_LENGTH];
    size_t          output_pos;

    /* unicode conversion buffer */
    euint16_t       utf16_buffer[2];
    size_t          utf16_pos;

    /* state */
    escape_state_t  state;
    size_t          low_offset;

} escape_parser_t;

/*----------------------------------------------------------------------*/

/* parse text (do not pass first backslash to parser) */
int escape_begin(escape_parser_t* escape_parser);
int escape_parse(escape_parser_t* escape_parser, const char* text, size_t text_size, 
                 escape_result_t* result_out, size_t* size_out);

/* input sequence (input is stored in case e.g. conversion failed, backslash is included) */
const char* escape_input(escape_parser_t* escape_parser);
size_t escape_input_len(escape_parser_t* escape_parser);

/* output sequence (utf8 encoded) */
const char* escape_output(escape_parser_t* escape_parser);
size_t escape_output_len(escape_parser_t* escape_parser);

/*----------------------------------------------------------------------*/

#endif /* _ESCAPE_PARSE_H_ */


