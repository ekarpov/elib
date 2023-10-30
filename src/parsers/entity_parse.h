/*
    Entity parser 
*/

#ifndef _ENTITY_PARSE_H_
#define _ENTITY_PARSE_H_

/*----------------------------------------------------------------------*/

/* constants */
#define ENTITY_MAX_INPUT_LENGTH             33     /* max supported entity length */
#define ENTITY_MAX_OUTPUT_LENGTH            12

/*----------------------------------------------------------------------*/

/*
    NOTE: supported entities are in the following format:
          
          &#nnnnnnn;    Unicode character in decimal notation (e.g. &#119558; up to 1114111)
          &#xhhhhhh;    Unicode character in hexadecimal notation (e.g. &#x002665; up to 0x10FFFF)
          &name;        entity name (max 31 chars, &lt; or &longleftrightarrow;)
*/

/*----------------------------------------------------------------------*/

/* entity parser result */
typedef enum {

    entity_result_continue,
    entity_result_ready,
    entity_result_not_entity

} entity_result_t;

/* entity parser state */
typedef enum {

    entity_parse_init,
    entity_parse_decimal,
    entity_parse_hex,
    entity_parse_name

} entity_state_t;

/* entity parser data */
typedef struct {
    
    /* input buffer */
    char            input[ENTITY_MAX_INPUT_LENGTH];
    size_t          input_pos;

    /* output buffer */
    char            output[ENTITY_MAX_OUTPUT_LENGTH];
    size_t          output_pos;

    /* state */
    entity_state_t  state;

} entity_parser_t;

/*----------------------------------------------------------------------*/

/* parse text (do not pass first ampersand to parser) */
int entity_begin(entity_parser_t* entity_parser);
int entity_parse(entity_parser_t* entity_parser, const char* text, size_t text_size, 
                 entity_result_t* result_out, size_t* size_out);

/* input sequence (input is stored in case e.g. conversion failed or entity not found, ampersand is included) */
const char* entity_input(entity_parser_t* entity_parser);
size_t entity_input_len(entity_parser_t* entity_parser);

/* output sequence (utf8 encoded) */
const char* entity_output(entity_parser_t* entity_parser);
size_t entity_output_len(entity_parser_t* entity_parser);

/*----------------------------------------------------------------------*/

#endif /* _ENTITY_PARSE_H_ */

