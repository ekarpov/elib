/*
    WBXML decoder
    Refer to WBXML documentation: http://www.w3.org/1999/06/NOTE-wbxml-19990624/
*/

#ifndef _WBXML_DECODER_H_
#define _WBXML_DECODER_H_

/*----------------------------------------------------------------------*/
/* decoder events */
typedef enum {

    /* properties reported as integer parameter */
    wbxml_document_version,
    wbxml_document_public_id,
    wbxml_document_charset,             /* refer: http://www.iana.org/assignments/character-sets/character-sets.xhtml */
    wbxml_document_codepage,

    /* properties reported as string parameter */
    wbxml_document_public_id_string,

    wbxml_tag_begin,                    /* tag id as integer parameter */
    wbxml_tag_begin_string,             /* tag name as string parameter */
    wbxml_tag_content_string,           /* string as data and length as integer parameter */
    wbxml_tag_content_entity,           /* entity code as integer parameter */
    wbxml_tag_end,                      /* tag id as integer parameter */
    wbxml_tag_end_string,               /* tag name as string parameter */

    wbxml_attribute_begin,              /* attribute id as integer parameter */
    wbxml_attribute_begin_string,       /* attribute name as string parameter */
    wbxml_attribute_value_string,       /* string as data and length as integer parameter */
    wbxml_attribute_value_token,        /* value token id as integer parameter */
    wbxml_attribute_end,                /* attribute id as integer parameter */
    wbxml_attribute_end_string,         /* attribute name as string parameter */

    wbxml_decoder_opaque_data,          /* data and length as parameters */
    wbxml_decoder_pi,                   /* no parameters */

    /* extension tokens */
    wbxml_ext_token_0,                  /* no parameters */
    wbxml_ext_token_1,                  /* no parameters */
    wbxml_ext_token_2,                  /* no parameters */
    wbxml_ext_token_integer_0,          /* token as integer parameter */
    wbxml_ext_token_integer_1,          /* token as integer parameter */
    wbxml_ext_token_integer_2,          /* token as integer parameter */
    wbxml_ext_token_string_0,           /* token as string parameter */
    wbxml_ext_token_string_1,           /* token as string parameter */
    wbxml_ext_token_string_2,           /* token as string parameter */

    wbxml_decoder_error

} wbxml_event_t;

/*----------------------------------------------------------------------*/

/*
    Callback parameters:
     - user data associated with decoder (if set by user)
     - decoder event
     - data parameter (depends on event type)
     - integer parameter (depends on event type)
     Return: ELIBC_CONTINUE to continue or ELIBC_STOP to stop decoder
*/

/* wbxml decoder callback */
typedef int (*wbxml_callback_t)(void*, wbxml_event_t, const void*, size_t);

/*----------------------------------------------------------------------*/
/* decoder internal states */
typedef enum {

    wbxml_state_version,
    wbxml_state_public_id,
    wbxml_state_charset,
    wbxml_state_stringtable_length,
    wbxml_state_stringtable,
    wbxml_state_body,
    wbxml_state_codepage,
    wbxml_state_tag_no_content,
    wbxml_state_attributes,
    wbxml_state_attribute,
    wbxml_state_opaque_data_length,
    wbxml_state_opaque_data,
    wbxml_state_extension_token_str,
    wbxml_state_extension_token_int,

    wbxml_state_multibyte,
    wbxml_state_string,
    wbxml_state_string_index,
    wbxml_state_entity,
    wbxml_state_literal,

    wbxml_state_count       /* must be the last */

} wbxml_state_t;

/*----------------------------------------------------------------------*/

/* state decoders */
typedef int (*wbxml_state_decode_t)(void*, const euint8_t*, size_t, size_t*);

/*----------------------------------------------------------------------*/
/* decoder */
typedef struct {

    /* state decoders */
    wbxml_state_decode_t    decoders[wbxml_state_count];

    /* state stack */
    estack_t                state_stack;
    wbxml_state_t           wbxml_state;

    /* tag stack */
    estack_t                tag_stack;
    euint32_t               wbxml_tag;
    euint8_t                wbxml_attribute;

    /* string termination symbol */
    euint8_t                string_term;

    /* decoded string table */
    euint8_t*               string_table;
    euint32_t               string_count;
    euint32_t               table_size;

    /* decoded multi byte integer value */
    euint32_t               mb_uint_value;

    /* decoded literal */
    euint8_t                literal_token;
    euint32_t               literal_index;

    /* decoded opaque data */
    euint8_t*               opaque_data;
    euint32_t               opaque_size;

    /* extension token number */
    euint8_t                ext_token;

    /* read counter */
    euint32_t               read_pos;

    /* data buffer */
    ebuffer_t*              decode_buffer;

    /* callback pointers */
    wbxml_callback_t        callback;
    void*                   callback_data;

    /* callback return value */
    int                     callback_return;

    /* decoder state */
    unsigned short          flags;

} wbxml_decoder_t;

/*----------------------------------------------------------------------*/

/* decoder handle */
void    wbxml_init(wbxml_decoder_t* wbxml_decoder, wbxml_callback_t decoder_callback, void* user_data);
void    wbxml_close(wbxml_decoder_t* wbxml_decoder);

/* decode binary stream */
int     wbxml_begin(wbxml_decoder_t* wbxml_decoder, ebuffer_t* decode_buffer);
int     wbxml_decode(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size);
int     wbxml_end(wbxml_decoder_t* wbxml_decoder);

/*----------------------------------------------------------------------*/

#endif /* _WBXML_DECODER_H_ */


