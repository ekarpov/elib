/*
    WBXML decoder
    Refer to WBXML documentation: http://www.w3.org/1999/06/NOTE-wbxml-19990624/
*/

#include "../elib_config.h"

#include "wbxml_types.h"
#include "wbxml_decoder.h"

/*----------------------------------------------------------------------*/
/* flags */
#define WBXML_FLAG_MASK_RESET           0xFF00
#define WBXML_FLAG_ERROR                0x0001

/*----------------------------------------------------------------------*/
/* special token code flags */
#define WBXML_TOKEN_IS_LITERAL          0x80000000

/*----------------------------------------------------------------------*/
/* state decoders */
int _wbxml_decoder_version(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_stringtable(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_body(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_codepage(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_tag_no_content(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_attributes(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_attribute(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_opaque_data(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_multibyte(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_string(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_jump_to_multibyte(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_jump_to_string(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);

/* state decoder helpers */
int _wbxml_decoder_common_token(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_begin_tag(wbxml_decoder_t* wbxml_decoder, euint8_t tag_token);
int _wbxml_decoder_end_tag(wbxml_decoder_t* wbxml_decoder);
int _wbxml_decoder_begin_attribute(wbxml_decoder_t* wbxml_decoder, euint8_t attribute_token);
int _wbxml_decoder_end_attribute(wbxml_decoder_t* wbxml_decoder);

/* helpers */
int _wbxml_jump_to_state(wbxml_decoder_t* wbxml_decoder, wbxml_state_t wbxml_state,
                         const euint8_t* input, size_t input_size, size_t* input_pos);
int _wbxml_decoder_multibyte_decoded(wbxml_decoder_t* wbxml_decoder);
int _wbxml_decoder_string_decoded(wbxml_decoder_t* wbxml_decoder, const euint8_t* string, size_t string_length);
int _wbxml_decoder_entity_decoded(wbxml_decoder_t* wbxml_decoder, euint32_t entity_code);
int _wbxml_decoder_literal_decoded(wbxml_decoder_t* wbxml_decoder);
int _wbxml_decoder_string_from_idx(wbxml_decoder_t* wbxml_decoder, euint32_t string_index, euint8_t** string, euint32_t* string_length);
int _wbxml_decoder_report_string_idx(wbxml_decoder_t* wbxml_decoder, euint32_t string_index);
int _wbxml_decoder_syntax_error(wbxml_decoder_t* wbxml_decoder);

/*----------------------------------------------------------------------*/
/* worker methods */
/*----------------------------------------------------------------------*/
ELIBC_FORCE_INLINE int _wbxml_push_state(wbxml_decoder_t* wbxml_decoder, wbxml_state_t wbxml_state)
{
    /* push old state */
    int err = estack_push(&wbxml_decoder->state_stack, wbxml_decoder->wbxml_state);
    if(err == ELIBC_SUCCESS)
    {
        /* replace state */
        wbxml_decoder->wbxml_state = wbxml_state;
    }

    return err;
}

ELIBC_FORCE_INLINE int _wbxml_pop_state(wbxml_decoder_t* wbxml_decoder)
{
    EASSERT(estack_size(&wbxml_decoder->state_stack) > 0);
    if(estack_size(&wbxml_decoder->state_stack) == 0) return ELIBC_ERROR_ARGUMENT;

    /* replace state */
    wbxml_decoder->wbxml_state = *(wbxml_state_t*)estack_top(&wbxml_decoder->state_stack);

    /* pop old state */
    return estack_pop(&wbxml_decoder->state_stack);
}

ELIBC_FORCE_INLINE int _wbxml_push_tag(wbxml_decoder_t* wbxml_decoder, euint32_t wbxml_tag)
{
    /* push old tag */
    int err = estack_push(&wbxml_decoder->tag_stack, wbxml_decoder->wbxml_tag);
    if(err == ELIBC_SUCCESS)
    {
        /* replace tag */
        wbxml_decoder->wbxml_tag = wbxml_tag;
    }

    return err;
}

ELIBC_FORCE_INLINE int _wbxml_pop_tag(wbxml_decoder_t* wbxml_decoder)
{
    EASSERT(estack_size(&wbxml_decoder->tag_stack) > 0);
    if(estack_size(&wbxml_decoder->tag_stack) == 0) return ELIBC_ERROR_ARGUMENT;

    /* replace tag */
    wbxml_decoder->wbxml_tag = *(euint32_t*)estack_top(&wbxml_decoder->tag_stack);

    /* pop old tag */
    return estack_pop(&wbxml_decoder->tag_stack);
}

ELIBC_FORCE_INLINE int _wbxml_append_char(wbxml_decoder_t* wbxml_decoder, char wbxml_char)
{
    if(wbxml_decoder->decode_buffer->pos + 1 < ebuffer_size(wbxml_decoder->decode_buffer))
    {
        /* just copy to avoid extra function call */
        (ebuffer_data(wbxml_decoder->decode_buffer))[wbxml_decoder->decode_buffer->pos] = wbxml_char;
        wbxml_decoder->decode_buffer->pos++;

        return ELIBC_SUCCESS;

    } else
    {
        return ebuffer_append_char(wbxml_decoder->decode_buffer, wbxml_char);
    }
}

/*----------------------------------------------------------------------*/

/* decoder handle */
void    wbxml_init(wbxml_decoder_t* wbxml_decoder, wbxml_callback_t decoder_callback, void* user_data)
{
    EASSERT(wbxml_decoder);
    EASSERT(decoder_callback);

    if(wbxml_decoder == 0 || decoder_callback == 0) return;

    /* reset all fields */
    ememset(wbxml_decoder, 0, sizeof(wbxml_decoder_t));

    /* init stacks */
    estack_init(&wbxml_decoder->state_stack, sizeof(wbxml_state_t));
    estack_init(&wbxml_decoder->tag_stack, sizeof(euint32_t));

    /* reserve default space (ignore error, if memory allocation fails we will notice this later) */
    estack_reserve(&wbxml_decoder->state_stack, PARSER_DEFAULT_STACK_SIZE);
    estack_reserve(&wbxml_decoder->tag_stack, PARSER_DEFAULT_STACK_SIZE);

    /* copy callback */
    wbxml_decoder->callback = decoder_callback;
    wbxml_decoder->callback_data = user_data;
    wbxml_decoder->callback_return = ELIBC_CONTINUE;

    /* init state decoders */
    wbxml_decoder->decoders[wbxml_state_version] = (wbxml_state_decode_t)_wbxml_decoder_version;
    wbxml_decoder->decoders[wbxml_state_public_id] = (wbxml_state_decode_t)_wbxml_decoder_jump_to_multibyte;
    wbxml_decoder->decoders[wbxml_state_charset] = (wbxml_state_decode_t)_wbxml_decoder_jump_to_multibyte;
    wbxml_decoder->decoders[wbxml_state_stringtable_length] = (wbxml_state_decode_t)_wbxml_decoder_jump_to_multibyte;
    wbxml_decoder->decoders[wbxml_state_stringtable] = (wbxml_state_decode_t)_wbxml_decoder_stringtable;
    wbxml_decoder->decoders[wbxml_state_body] = (wbxml_state_decode_t)_wbxml_decoder_body;
    wbxml_decoder->decoders[wbxml_state_codepage] = (wbxml_state_decode_t)_wbxml_decoder_codepage;
    wbxml_decoder->decoders[wbxml_state_tag_no_content] = (wbxml_state_decode_t)_wbxml_decoder_tag_no_content;
    wbxml_decoder->decoders[wbxml_state_attributes] = (wbxml_state_decode_t)_wbxml_decoder_attributes;
    wbxml_decoder->decoders[wbxml_state_attribute] = (wbxml_state_decode_t)_wbxml_decoder_attribute;
    wbxml_decoder->decoders[wbxml_state_opaque_data_length] = (wbxml_state_decode_t)_wbxml_decoder_jump_to_multibyte;
    wbxml_decoder->decoders[wbxml_state_opaque_data] = (wbxml_state_decode_t)_wbxml_decoder_opaque_data;
    wbxml_decoder->decoders[wbxml_state_extension_token_int] = (wbxml_state_decode_t)_wbxml_decoder_jump_to_multibyte;
    wbxml_decoder->decoders[wbxml_state_extension_token_str] = (wbxml_state_decode_t)_wbxml_decoder_jump_to_string;

    wbxml_decoder->decoders[wbxml_state_multibyte] = (wbxml_state_decode_t)_wbxml_decoder_multibyte;
    wbxml_decoder->decoders[wbxml_state_string] = (wbxml_state_decode_t)_wbxml_decoder_string;
    wbxml_decoder->decoders[wbxml_state_string_index] = (wbxml_state_decode_t)_wbxml_decoder_jump_to_multibyte;
    wbxml_decoder->decoders[wbxml_state_entity] = (wbxml_state_decode_t)_wbxml_decoder_jump_to_multibyte;
    wbxml_decoder->decoders[wbxml_state_literal] = (wbxml_state_decode_t)_wbxml_decoder_jump_to_multibyte;

#ifdef _ELIBC_DEBUG
    /* check that all decoders were set */
    {
        int i;
        for(i = 0; i < wbxml_state_count; ++i)
        {
            EASSERT1(wbxml_decoder->decoders[i], "wbxml_decoder: state decoder was not set");
        }
    }
#endif /* _ELIBC_DEBUG */
}

void    wbxml_close(wbxml_decoder_t* wbxml_decoder)
{
    /* free buffers */
    if(wbxml_decoder)
    {
        estack_free(&wbxml_decoder->state_stack);
        estack_free(&wbxml_decoder->tag_stack);
        efree(wbxml_decoder->string_table);
    }
}

/* decode binary stream */
int     wbxml_begin(wbxml_decoder_t* wbxml_decoder, ebuffer_t* decode_buffer)
{
    EASSERT(wbxml_decoder);
    EASSERT(decode_buffer);
    if(wbxml_decoder == 0 || decode_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* copy buffer reference */
    wbxml_decoder->decode_buffer = decode_buffer;

    /* reset decoder state */
    estack_reset(&wbxml_decoder->state_stack);
    estack_reset(&wbxml_decoder->tag_stack);
    ebuffer_reset(wbxml_decoder->decode_buffer);
    wbxml_decoder->table_size = 0;
    wbxml_decoder->flags &= WBXML_FLAG_MASK_RESET;

    /* beginning state */
    wbxml_decoder->wbxml_state = wbxml_state_version;

    return ELIBC_SUCCESS;
}

int     wbxml_decode(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size)
{
    size_t input_pos;
    int err;

    EASSERT(wbxml_decoder);
    EASSERT(wbxml_decoder->decode_buffer);
    if(wbxml_decoder == 0 || wbxml_decoder->decode_buffer == 0) return ELIBC_ERROR_ARGUMENT;

    /* check input */
    if(input == 0 && input_size > 0) return ELIBC_ERROR_ARGUMENT;

    /* reset error */
    err = ELIBC_SUCCESS;

    /* process input */
    for(input_pos = 0; input_pos < input_size && err == ELIBC_SUCCESS && wbxml_decoder->callback_return == ELIBC_CONTINUE; ++input_pos)
    {
        /* process input based on state */
        EASSERT((int)wbxml_decoder->wbxml_state < wbxml_state_count);
        err = wbxml_decoder->decoders[wbxml_decoder->wbxml_state](wbxml_decoder, input, input_size, &input_pos);

        /* stop if error */
        if(wbxml_decoder->flags & WBXML_FLAG_ERROR)
        {
            /* report syntax error */
            if(wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_decoder_error, 0, input_pos) != ELIBC_CONTINUE)
            {
                /* stop */
                return ELIBC_ERROR_PARSER_INVALID_INPUT;
            }

            /* reset error flag and continue */
            wbxml_decoder->flags &= ~((unsigned short)WBXML_FLAG_ERROR);
        }
    }

    return err;
}

int     wbxml_end(wbxml_decoder_t* wbxml_decoder)
{
    EASSERT(wbxml_decoder);
    if(wbxml_decoder == 0) return ELIBC_ERROR_ARGUMENT;

    /* validate decoder state */
    if(estack_size(&wbxml_decoder->state_stack) != 0 || wbxml_decoder->wbxml_state != wbxml_state_body ||
       estack_size(&wbxml_decoder->tag_stack) != 0 || wbxml_decoder->wbxml_tag != 0)
    {
        ETRACE("wbxml_decoder: decoder finished in incomplete state, not enough input was given");
        return ELIBC_ERROR_PARSER_INVALID_INPUT;
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* state decoders */
int _wbxml_decoder_version(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    EUNUSED(input_size);

    /* report version */
    wbxml_decoder->callback_return = wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_document_version, 0, input[*input_pos]);

    /* move to next state */
    wbxml_decoder->wbxml_state = wbxml_document_public_id;

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_stringtable(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    /* copy string table */
    while(wbxml_decoder->read_pos < wbxml_decoder->table_size && *input_pos < input_size)
    {
        /* update string count if end of string found */
        if(input[*input_pos] == wbxml_decoder->string_term)
        {
            wbxml_decoder->string_count++;
        }

        wbxml_decoder->string_table[wbxml_decoder->read_pos] = input[*input_pos];
        wbxml_decoder->read_pos++;
        ++(*input_pos);
    }

    /* move to next state if whole data read */
    if(wbxml_decoder->read_pos == wbxml_decoder->table_size)
    {
        wbxml_decoder->wbxml_state = wbxml_state_body;
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_common_token(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    EUNUSED(input_size);

    /* check known tokens first */
    switch(input[*input_pos])
    {
    case WBXML_TOKEN_SWITCH_PAGE:
        /* move to codepage state */
        return _wbxml_push_state(wbxml_decoder, wbxml_state_codepage);
        break;

    case WBXML_TOKEN_STR_I:
        /* reset buffer */
        ebuffer_reset(wbxml_decoder->decode_buffer);

        /* inline string reading */
        return _wbxml_push_state(wbxml_decoder, wbxml_state_string);
        break;

    case WBXML_TOKEN_STR_T:
        /* table string reading */
        return _wbxml_push_state(wbxml_decoder, wbxml_state_string_index);
        break;

    case WBXML_TOKEN_ENTITY:
        /* process entity */
        return _wbxml_push_state(wbxml_decoder, wbxml_state_entity);
        break;

    case WBXML_TOKEN_LITERAL:
    case WBXML_TOKEN_LITERAL_C:
    case WBXML_TOKEN_LITERAL_A:
    case WBXML_TOKEN_LITERAL_AC:
        /* copy literal token */
        wbxml_decoder->literal_token = input[*input_pos];

        /* process literal */
        return _wbxml_push_state(wbxml_decoder, wbxml_state_literal);
        break;

    case WBXML_TOKEN_PI:
        /* report processing instruction */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_decoder_pi, 0, 0);
        break;

    case WBXML_TOKEN_OPAQUE:
        /* opaque data reading */
        return _wbxml_push_state(wbxml_decoder, wbxml_state_opaque_data);
        break;

    /*** extension tokens ***/

    case WBXML_TOKEN_EXT_0:
        /* report token */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_ext_token_0, 0, 0);
        break;

    case WBXML_TOKEN_EXT_1:
        /* report token */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_ext_token_1, 0, 0);
        break;

    case WBXML_TOKEN_EXT_2:
        /* report token */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_ext_token_2, 0, 0);
        break;

    case WBXML_TOKEN_EXT_I_0:
        /* extension token reading */
        wbxml_decoder->ext_token = 0;
        return _wbxml_push_state(wbxml_decoder, wbxml_state_extension_token_int);
        break;

    case WBXML_TOKEN_EXT_I_1:
        /* extension token reading */
        wbxml_decoder->ext_token = 1;
        return _wbxml_push_state(wbxml_decoder, wbxml_state_extension_token_int);
        break;

    case WBXML_TOKEN_EXT_I_2:
        /* extension token reading */
        wbxml_decoder->ext_token = 2;
        return _wbxml_push_state(wbxml_decoder, wbxml_state_extension_token_int);
        break;

    case WBXML_TOKEN_EXT_T_0:
        /* extension token reading */
        wbxml_decoder->ext_token = 0;
        return _wbxml_push_state(wbxml_decoder, wbxml_state_extension_token_str);
        break;

    case WBXML_TOKEN_EXT_T_1:
        /* extension token reading */
        wbxml_decoder->ext_token = 0;
        return _wbxml_push_state(wbxml_decoder, wbxml_state_extension_token_str);
        break;

    case WBXML_TOKEN_EXT_T_2:
        /* extension token reading */
        wbxml_decoder->ext_token = 0;
        return _wbxml_push_state(wbxml_decoder, wbxml_state_extension_token_str);
        break;


    default:
        ETRACE("wbxml_decoder: unknown common token");
        EASSERT(0);
        return ELIBC_ERROR_INTERNAL;
        break;
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_begin_tag(wbxml_decoder_t* wbxml_decoder, euint8_t tag_token)
{
    euint8_t tag_id;
    int err;

    /* get tag identity from token */
    tag_id = (tag_token & WBXML_TAG_MASK);

    /* push to stack */
    err = _wbxml_push_tag(wbxml_decoder, tag_id);
    if(err != ELIBC_SUCCESS) return err;

    /* check if tag is literal (should be already encoded) */
    if(tag_id != WBXML_TOKEN_LITERAL)
    {
        /* report tag start */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_tag_begin, 0, tag_id);
    } else
    {
        euint8_t* string;
        euint32_t string_length;

        /* mark current tag id as literal */
        wbxml_decoder->wbxml_tag |= WBXML_TOKEN_IS_LITERAL;

        /* find literal from index */
        err = _wbxml_decoder_string_from_idx(wbxml_decoder, wbxml_decoder->literal_index, &string, &string_length);
        if(err != ELIBC_SUCCESS) return err;

        /* report literal */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_tag_begin_string, string, string_length);
    }

    /* stop decoder if requested by client */
    if(wbxml_decoder->callback_return != ELIBC_CONTINUE) return ELIBC_SUCCESS;

    /* check if tag has content or attributes */
    if(tag_token & (WBXML_TAG_WITH_CONTENT | WBXML_TAG_WITH_ATTRIBUTES))
    {
        /* jump to attributes state if needed */
        if(tag_token & WBXML_TAG_WITH_ATTRIBUTES)
        {
            /* add special state in case we need to report tag end just after attribute list */
            if((tag_token & WBXML_TAG_WITH_CONTENT) == 0)
            {
                err = _wbxml_push_state(wbxml_decoder, wbxml_state_tag_no_content);
                if(err != ELIBC_SUCCESS) return err;
            }

            return _wbxml_push_state(wbxml_decoder, wbxml_state_attributes);
        }

    } else
    {
        /* report tag end */
        return _wbxml_decoder_end_tag(wbxml_decoder);
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_end_tag(wbxml_decoder_t* wbxml_decoder)
{
    /* check if current tag is literal */
    if(wbxml_decoder->wbxml_tag & WBXML_TOKEN_IS_LITERAL)
    {
        euint8_t* string;
        euint32_t string_length;
        int err;

        /* find literal from index */
        err = _wbxml_decoder_string_from_idx(wbxml_decoder, wbxml_decoder->literal_index, &string, &string_length);
        if(err != ELIBC_SUCCESS) return err;

        /* report literal */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_tag_end_string, string, string_length);

    } else
    {
        /* just report tag id */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_tag_end, 0, wbxml_decoder->wbxml_tag);
    }

    /* pop tag */
    _wbxml_pop_tag(wbxml_decoder);

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_body(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    /* process input */
    switch(input[*input_pos])
    {
    case WBXML_TOKEN_SWITCH_PAGE:
    case WBXML_TOKEN_STR_I:
    case WBXML_TOKEN_STR_T:
    case WBXML_TOKEN_ENTITY:
    case WBXML_TOKEN_LITERAL:
    case WBXML_TOKEN_LITERAL_C:
    case WBXML_TOKEN_LITERAL_A:
    case WBXML_TOKEN_LITERAL_AC:
    case WBXML_TOKEN_PI:
    case WBXML_TOKEN_EXT_0:
    case WBXML_TOKEN_EXT_1:
    case WBXML_TOKEN_EXT_2:
    case WBXML_TOKEN_EXT_I_0:
    case WBXML_TOKEN_EXT_I_1:
    case WBXML_TOKEN_EXT_I_2:
    case WBXML_TOKEN_EXT_T_0:
    case WBXML_TOKEN_EXT_T_1:
    case WBXML_TOKEN_EXT_T_2:
    case WBXML_TOKEN_OPAQUE:
        return _wbxml_decoder_common_token(wbxml_decoder, input, input_size, input_pos);
        break;

    case WBXML_TOKEN_END:
        /* report tag end */
        return _wbxml_decoder_end_tag(wbxml_decoder);
        break;

    default:
        /* begin tag */
        return _wbxml_decoder_begin_tag(wbxml_decoder, input[*input_pos]);
        break;
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_tag_no_content(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    EUNUSED(input_size);

    /* make sure input is END token */
    if(input[*input_pos] != WBXML_TOKEN_END)
    {
        ETRACE("wbxml_decoder: unexpected token while processing empty tag end");
        EASSERT(0);
        return ELIBC_ERROR_INTERNAL;
    }

    /* pop state */
    _wbxml_pop_state(wbxml_decoder);

    /* report tag end */
    return _wbxml_decoder_end_tag(wbxml_decoder);
}

int _wbxml_decoder_begin_attribute(wbxml_decoder_t* wbxml_decoder, euint8_t attribute_token)
{
    // TODO:
    return ELIBC_SUCCESS;
}

int _wbxml_decoder_end_attribute(wbxml_decoder_t* wbxml_decoder)
{
    // TODO:
    return ELIBC_SUCCESS;
}

int _wbxml_decoder_attributes(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    /* process input */
    switch(input[*input_pos])
    {
    case WBXML_TOKEN_SWITCH_PAGE:
    case WBXML_TOKEN_STR_I:
    case WBXML_TOKEN_STR_T:
    case WBXML_TOKEN_ENTITY:
    case WBXML_TOKEN_LITERAL:
    case WBXML_TOKEN_LITERAL_C:
    case WBXML_TOKEN_LITERAL_A:
    case WBXML_TOKEN_LITERAL_AC:
    case WBXML_TOKEN_PI:
    case WBXML_TOKEN_EXT_0:
    case WBXML_TOKEN_EXT_1:
    case WBXML_TOKEN_EXT_2:
    case WBXML_TOKEN_EXT_I_0:
    case WBXML_TOKEN_EXT_I_1:
    case WBXML_TOKEN_EXT_I_2:
    case WBXML_TOKEN_EXT_T_0:
    case WBXML_TOKEN_EXT_T_1:
    case WBXML_TOKEN_EXT_T_2:
    case WBXML_TOKEN_OPAQUE:
        // TODO: check if all tags are supported
        return _wbxml_decoder_common_token(wbxml_decoder, input, input_size, input_pos);
        break;

    case WBXML_TOKEN_END:
        /* pop state */
        _wbxml_pop_state(wbxml_decoder);

        /* check if we need to report empty tag end */
        if(wbxml_decoder->wbxml_state == wbxml_state_tag_no_content)
        {
            /* process state */
            return _wbxml_decoder_tag_no_content(wbxml_decoder, input, input_size, input_pos);
        }
        break;

    default:
        /* begin attribute */
        return _wbxml_decoder_begin_attribute(wbxml_decoder, input[*input_pos]);
        break;
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_attribute(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    /* process input */
    switch(input[*input_pos])
    {
    case WBXML_TOKEN_SWITCH_PAGE:
    case WBXML_TOKEN_STR_I:
    case WBXML_TOKEN_STR_T:
    case WBXML_TOKEN_ENTITY:
    case WBXML_TOKEN_LITERAL:
    case WBXML_TOKEN_LITERAL_C:
    case WBXML_TOKEN_LITERAL_A:
    case WBXML_TOKEN_LITERAL_AC:
    case WBXML_TOKEN_PI:
    case WBXML_TOKEN_EXT_0:
    case WBXML_TOKEN_EXT_1:
    case WBXML_TOKEN_EXT_2:
    case WBXML_TOKEN_EXT_I_0:
    case WBXML_TOKEN_EXT_I_1:
    case WBXML_TOKEN_EXT_I_2:
    case WBXML_TOKEN_EXT_T_0:
    case WBXML_TOKEN_EXT_T_1:
    case WBXML_TOKEN_EXT_T_2:
    case WBXML_TOKEN_OPAQUE:
        // TODO: check if all tags are supported
        return _wbxml_decoder_common_token(wbxml_decoder, input, input_size, input_pos);
        break;

    case WBXML_TOKEN_END:
        // TODO: fail with syntax error
        break;

    default:
        // TODO: fail with syntax error ?
        break;
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_opaque_data(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    /* copy opaque data */
    while(wbxml_decoder->read_pos < wbxml_decoder->opaque_size && *input_pos < input_size)
    {
        /* copy data */
        wbxml_decoder->opaque_data[wbxml_decoder->read_pos] = input[*input_pos];
        wbxml_decoder->read_pos++;
        ++(*input_pos);
    }

    /* pop state if whole data read */
    if(wbxml_decoder->read_pos == wbxml_decoder->opaque_size)
    {
        /* report opaque data */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_decoder_opaque_data, wbxml_decoder->opaque_data, wbxml_decoder->opaque_size);

        /* pop state */
        _wbxml_pop_state(wbxml_decoder);
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_codepage(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    EUNUSED(input_size);

    /* report codepage switch */
    wbxml_decoder->callback_return =
            wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_document_codepage, 0, input[*input_pos]);

    /* pop state */
    return _wbxml_pop_state(wbxml_decoder);
}

int _wbxml_decoder_multibyte(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    /* process input */
    while(*input_pos < input_size)
    {
        /* check if number has continuation */
        if(input[*input_pos] & WBXML_MULTIBYTE_CONTINUE)
        {
            /* reset continuation flag and append number */
            wbxml_decoder->mb_uint_value += (WBXML_MULTIBYTE_CONTENT_MASK & (input[*input_pos]));

            /* move value 7 bits to the left */
            wbxml_decoder->mb_uint_value <<= 7;

            /* next */
            ++(*input_pos);

        } else
        {
            /* append last 7 bits */
            wbxml_decoder->mb_uint_value += input[*input_pos];

            /* pop state */
            _wbxml_pop_state(wbxml_decoder);

            /* report multibyte ready */
            return _wbxml_decoder_multibyte_decoded(wbxml_decoder);
        }
    }
    return ELIBC_SUCCESS;
}

int _wbxml_decoder_string(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    int err;

    /* read string */
    while(*input_pos < input_size && input[*input_pos] != wbxml_decoder->string_term)
    {
        /* copy data */
        err = _wbxml_append_char(wbxml_decoder, input[*input_pos]);
        if(err != ELIBC_SUCCESS) return err;

        /* next */
        ++(*input_pos);
    }

    /* report string if string term found */
    if(input[*input_pos] == wbxml_decoder->string_term)
    {
        /* pop state */
        _wbxml_pop_state(wbxml_decoder);

        /* repoort string */
        return _wbxml_decoder_string_decoded(wbxml_decoder, (euint8_t*)ebuffer_data(wbxml_decoder->decode_buffer), ebuffer_pos(wbxml_decoder->decode_buffer));
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_jump_to_multibyte(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    int err;

    /* jump to multibyte number decoder */
    err = _wbxml_push_state(wbxml_decoder, wbxml_state_multibyte);
    if(err != ELIBC_SUCCESS) return err;

    /* reset multibyte decoder buffer */
    wbxml_decoder->mb_uint_value = 0;

    /* process input */
    return _wbxml_decoder_multibyte(wbxml_decoder, input, input_size, input_pos);
}

int _wbxml_decoder_jump_to_string(wbxml_decoder_t* wbxml_decoder, const euint8_t* input, size_t input_size, size_t* input_pos)
{
    int err;

    /* jump to string reading */
    err = _wbxml_push_state(wbxml_decoder, wbxml_state_string);
    if(err != ELIBC_SUCCESS) return err;

    /* reset buffer */
    ebuffer_reset(wbxml_decoder->decode_buffer);

    /* process input */
    return _wbxml_decoder_string(wbxml_decoder, input, input_size, input_pos);
}

/*----------------------------------------------------------------------*/
/* helpers */
int _wbxml_decoder_multibyte_decoded(wbxml_decoder_t* wbxml_decoder)
{
    /* check state */
    switch(wbxml_decoder->wbxml_state)
    {
    case wbxml_state_public_id:
        /* check if public id is encoded as literal */
        if(wbxml_decoder->mb_uint_value == WBXML_PUBLICID_LITERAL)
        {
            /* table string reading */
            return _wbxml_push_state(wbxml_decoder, wbxml_state_string_index);
        }

        /* report public id */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_document_public_id, 0, wbxml_decoder->mb_uint_value);

        /* start reading charset */
        wbxml_decoder->wbxml_state = wbxml_state_charset;
        break;

    case wbxml_state_charset:
        /* report charset */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_document_charset, 0, wbxml_decoder->mb_uint_value);

        /* TODO: get string termination from character set */
        wbxml_decoder->string_term = 0;

        /* start reading string table length */
        wbxml_decoder->wbxml_state = wbxml_state_stringtable_length;
        break;

    case wbxml_state_stringtable_length:
        /* copy table size */
        wbxml_decoder->table_size = wbxml_decoder->mb_uint_value;

        /* check if there is string table in the document */
        if(wbxml_decoder->table_size)
        {
            /* reserve memory for table */
            void* tmp = 0;
            if(wbxml_decoder->string_table)
            {
                tmp = erealloc(wbxml_decoder->string_table, wbxml_decoder->table_size);

            } else
            {
                tmp = emalloc(wbxml_decoder->table_size);
            }

            /* check result and copy memory reference */
            if(tmp == 0) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;
            wbxml_decoder->string_table = tmp;

            /* reset table read position */
            wbxml_decoder->read_pos = 0;
            wbxml_decoder->string_count = 0;

            /* move to table read */
            wbxml_decoder->read_pos = 0;
            wbxml_decoder->wbxml_state = wbxml_state_stringtable;

        } else
        {
            /* move to next state if string table is empty */
            wbxml_decoder->wbxml_state = wbxml_state_body;
        }
        break;

    case wbxml_state_entity:
        /* pop state */
        _wbxml_pop_state(wbxml_decoder);

        /* report decoded entity */
        return _wbxml_decoder_entity_decoded(wbxml_decoder, wbxml_decoder->mb_uint_value);
        break;

    case wbxml_state_string_index:
        /* pop state */
        _wbxml_pop_state(wbxml_decoder);

        /* report string from table */
        return _wbxml_decoder_report_string_idx(wbxml_decoder, wbxml_decoder->mb_uint_value);
        break;

    case wbxml_state_literal:
        /* pop state */
        _wbxml_pop_state(wbxml_decoder);

        /* copy encoded literal */
        wbxml_decoder->literal_index = wbxml_decoder->mb_uint_value;

        /* process literal */
        return _wbxml_decoder_literal_decoded(wbxml_decoder);
        break;

    case wbxml_state_opaque_data_length:
        /* reset data buffer */
        ebuffer_reset(wbxml_decoder->decode_buffer);

        /* reserve memory */
        if(ebuffer_reserve(wbxml_decoder->decode_buffer, wbxml_decoder->mb_uint_value) != ELIBC_SUCCESS)
        {
            return ELIBC_ERROR_NOT_ENOUGH_MEMORY;
        }

        /* init opaque buffer to read */
        wbxml_decoder->opaque_data = (euint8_t*)ebuffer_data(wbxml_decoder->decode_buffer);
        wbxml_decoder->opaque_size = wbxml_decoder->mb_uint_value;

        /* move to data read */
        wbxml_decoder->read_pos = 0;
        wbxml_decoder->wbxml_state = wbxml_state_opaque_data;
        break;

    case wbxml_state_extension_token_int:
        /* report extension token number */
        if(wbxml_decoder->ext_token == 0)
        {
            wbxml_decoder->callback_return =
                    wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_ext_token_integer_0, 0, wbxml_decoder->mb_uint_value);
        } else if(wbxml_decoder->ext_token == 1)
        {
            wbxml_decoder->callback_return =
                    wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_ext_token_integer_1, 0, wbxml_decoder->mb_uint_value);
        } else
        {
            wbxml_decoder->callback_return =
                    wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_ext_token_integer_2, 0, wbxml_decoder->mb_uint_value);
        }

        /* pop state */
        _wbxml_pop_state(wbxml_decoder);
        break;

    default:
        ETRACE("wbxml_decoder: unexpected state after parsing multibyte integer");
        EASSERT(0);
        return ELIBC_ERROR_INTERNAL;
        break;
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_string_decoded(wbxml_decoder_t* wbxml_decoder, const euint8_t* string, size_t string_length)
{
    /* check state */
    switch(wbxml_decoder->wbxml_state)
    {
    case wbxml_state_body:
        /* report string content */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_tag_content_string, string, string_length);
        break;

    case wbxml_state_attribute:
        /* report string attribute */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_attribute_value_string, string, string_length);
        break;

    case wbxml_state_public_id:
        /* report public id as string */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_document_public_id_string, string, string_length);

        /* move to next state */
        wbxml_decoder->wbxml_state = wbxml_state_charset;
        break;

    case wbxml_state_extension_token_str:
        /* report extension token string */
        if(wbxml_decoder->ext_token == 0)
        {
            wbxml_decoder->callback_return =
                    wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_ext_token_string_0, string, string_length);
        } else if(wbxml_decoder->ext_token == 1)
        {
            wbxml_decoder->callback_return =
                    wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_ext_token_string_1, string, string_length);
        } else
        {
            wbxml_decoder->callback_return =
                    wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_ext_token_string_2, string, string_length);
        }

        /* pop state */
        _wbxml_pop_state(wbxml_decoder);
        break;

    default:
        ETRACE("wbxml_decoder: unexpected state after parsing string");
        EASSERT(0);
        return ELIBC_ERROR_INTERNAL;
        break;
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_entity_decoded(wbxml_decoder_t* wbxml_decoder, euint32_t entity_code)
{
    /* check state */
    switch(wbxml_decoder->wbxml_state)
    {
    case wbxml_state_body:
        /* report entity content */
        wbxml_decoder->callback_return =
                wbxml_decoder->callback(wbxml_decoder->callback_data, wbxml_tag_content_entity, 0, entity_code);
        break;

    default:
        ETRACE("wbxml_decoder: unexpected state after parsing entity");
        EASSERT(0);
        return ELIBC_ERROR_INTERNAL;
        break;
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_literal_decoded(wbxml_decoder_t* wbxml_decoder)
{
    /* check state */
    switch(wbxml_decoder->wbxml_state)
    {
    case wbxml_state_body:
        /* process tag */
        return _wbxml_decoder_begin_tag(wbxml_decoder, wbxml_decoder->literal_token);
        break;

    case wbxml_state_attributes:
        /* process attribute */
        return _wbxml_decoder_begin_attribute(wbxml_decoder, wbxml_decoder->literal_token);
        break;

    default:
        ETRACE("wbxml_decoder: unexpected state after parsing literal");
        EASSERT(0);
        return ELIBC_ERROR_INTERNAL;
        break;
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_string_from_idx(wbxml_decoder_t* wbxml_decoder, euint32_t string_index, euint8_t** string, euint32_t* string_length)
{
    euint8_t* string_pos;
    euint32_t string_count;

    /* check if string index is out of range */
    if(string_index >= wbxml_decoder->string_count)
    {
        ETRACE("wbxml_decoder: string table index is out of range");
        return _wbxml_decoder_syntax_error(wbxml_decoder);
    }

    /* find string start */
    string_pos = wbxml_decoder->string_table;
    string_count = 0;
    while(string_count < string_index && string_pos < wbxml_decoder->string_table + wbxml_decoder->table_size)
    {
        ++string_pos;
        if(*string_pos == wbxml_decoder->string_term)
        {
            ++string_count;
            ++string_pos;
        }
    }

    /* check if found just in case */
    if(string_pos >= wbxml_decoder->string_table + wbxml_decoder->table_size)
    {
        ETRACE("wbxml_decoder: string table seems to be corrupted");
        EASSERT(0);
        return ELIBC_ERROR_INTERNAL;
    }

    /* copy return string */
    *string = string_pos;

    /* find string length */
    *string_length = 0;
    while(string_pos[*string_length] != wbxml_decoder->string_term &&
          string_pos + *string_length < wbxml_decoder->string_table + wbxml_decoder->table_size)
    {
        ++(*string_length);
    }

    /* check if found just in case */
    if(string_pos + *string_length >= wbxml_decoder->string_table + wbxml_decoder->table_size)
    {
        ETRACE("wbxml_decoder: string table seems to be corrupted");
        EASSERT(0);
        return ELIBC_ERROR_INTERNAL;
    }

    return ELIBC_SUCCESS;
}

int _wbxml_decoder_report_string_idx(wbxml_decoder_t* wbxml_decoder, euint32_t string_index)
{
    euint8_t* string;
    euint32_t string_length;
    int err;

    /* find string from index */
    err = _wbxml_decoder_string_from_idx(wbxml_decoder, string_index, &string, &string_length);
    if(err != ELIBC_SUCCESS) return err;

    /* report string */
    return _wbxml_decoder_string_decoded(wbxml_decoder, string, string_length);
}

int _wbxml_decoder_syntax_error(wbxml_decoder_t* wbxml_decoder)
{
    /* mark error */
    wbxml_decoder->flags |= WBXML_FLAG_ERROR;

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/



