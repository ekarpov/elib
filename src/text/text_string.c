/*
    Text string manipulation helpers
*/

#include "../elib_config.h"

#include "text_format.h"
#include "text_string.h"

/*----------------------------------------------------------------------*/

/* init */
void ustring_init(ustring_t* ustring)
{
    /* check input */
    EASSERT(ustring);
    if(ustring == 0) return;

    /* reset memory */
    ememset(ustring, 0, sizeof(ustring_t));

    /* init buffer */
    ebuffer_init(&ustring->data);
}

void ustring_reset(ustring_t* ustring)
{
    EASSERT(ustring);
    if(ustring)
    {
        /* reset state */
        ustring->encoding = TEXT_ENCODING_UTF8;
        ustring->length = 0;

        /* reset buffer */
        ebuffer_reset(&ustring->data);
    }
}

void ustring_free(ustring_t* ustring)
{
    EASSERT(ustring);
    if(ustring)
    {
        /* release buffer */
        ebuffer_free(&ustring->data);
    }
}

/* manipulations (NOTE: if length is zero text is assumed to be null terminated) */
int ustring_set_utf8(ustring_t* ustring, const utf8_t* text, size_t length)
{
    int err;

    /* check input */
    EASSERT(ustring);
    if(ustring == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset previous string if any */
    ustring_reset(ustring);

    /* set encoding */
    ustring->encoding = TEXT_ENCODING_UTF8;
    ustring->length = text ? length : 0;

    /* copy string */
    if(text)
    {
        /* compute length if needed */
        if(ustring->length == 0)
            ustring->length = estrlen((const char*)text);

        /* copy string */
        if(ustring->length > 0)
        {
            err = ebuffer_append(&ustring->data, text, ustring->length * sizeof(utf8_t));
            if(err != ELIBC_SUCCESS) return err;
        }
    }

    /* append end of string */
    return ebuffer_append_char(&ustring->data, 0);
}

int ustring_set_utf16(ustring_t* ustring, const utf16_t* text, size_t length)
{
    int err;

    /* check input */
    EASSERT(ustring);
    if(ustring == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset previous string if any */
    ustring_reset(ustring);

    /* set encoding */
    ustring->encoding = TEXT_ENCODING_UTF16;
    ustring->length = text ? length : 0;

    /* copy string */
    if(text)
    {
        /* compute length if needed */
        if(ustring->length == 0)
            ustring->length = ewcslen((const ewchar_t*)text);

        /* copy string */
        if(ustring->length > 0)
        {
            err = ebuffer_append(&ustring->data, text, ustring->length * sizeof(utf16_t));
            if(err != ELIBC_SUCCESS) return err;
        }
    }

    /* append end of string */
    return ebuffer_append_wchar(&ustring->data, 0);
}

int ustring_append_utf8(ustring_t* ustring, const utf8_t* text, size_t length)
{
    int err;

    /* check input */
    EASSERT(ustring);
    if(ustring == 0) return ELIBC_ERROR_ARGUMENT;

    /* if string is empty just use set */
    if(ustring->length == 0) 
        return ustring_set_utf8(ustring, text, length);

    /* ignore if string is empty */
    EASSERT(text);
    if(text == 0) return ELIBC_ERROR_ARGUMENT;

    /* compute length if needed */
    if(length == 0)
        length = estrlen((const char*)text);

    /* ignore if string is empty */
    if(length == 0) return ELIBC_SUCCESS;

    /* check string encoding */
    if(ustring->encoding == TEXT_ENCODING_UTF8)
    {
        /* update buffer position (remove last zero) */
        err = ebuffer_setpos(&ustring->data, ustring->length * sizeof(utf8_t));
        if(err != ELIBC_SUCCESS) return err;

        /* append text */
        err = ebuffer_append(&ustring->data, text, length);
        if(err != ELIBC_SUCCESS) return err;

        /* append end of string */
        err = ebuffer_append_char(&ustring->data, 0);
        if(err != ELIBC_SUCCESS) return err;

        /* update length */
        ustring->length += length;

    } else
    {
        size_t required_size, output_size;
        utf16_t* output_str;

        /* update buffer position (remove last zero) */
        err = ebuffer_setpos(&ustring->data, ustring->length * sizeof(utf16_t));
        if(err != ELIBC_SUCCESS) return err;

        /* compute required size */
        required_size = utf8_in_utf16(text, length);

        /* reserve space */
        output_str = (utf16_t*) ebuffer_append_ptr(&ustring->data, required_size * sizeof(utf16_t));
        if(output_str == 0) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;

        /* convert string */
        utf8_to_utf16(text, length, output_str, &output_size);

        /* update position in case some characters were not converted */
        if(output_size != required_size)
        {
            err = ebuffer_setpos(&ustring->data, (ustring->length + output_size) * sizeof(utf8_t));
            if(err != ELIBC_SUCCESS) return err;
        }

        /* append end of string */
        err = ebuffer_append_wchar(&ustring->data, 0);
        if(err != ELIBC_SUCCESS) return err;

        /* update length */
        ustring->length += output_size;
    }

    return ELIBC_SUCCESS;
}

int ustring_append_utf16(ustring_t* ustring, const utf16_t* text, size_t length)
{
    int err;

    /* check input */
    EASSERT(ustring);
    if(ustring == 0) return ELIBC_ERROR_ARGUMENT;

    /* if string is empty just use set */
    if(ustring->length == 0) 
        return ustring_set_utf16(ustring, text, length);

    /* ignore if string is empty */
    EASSERT(text);
    if(text == 0) return ELIBC_ERROR_ARGUMENT;

    /* compute length if needed */
    if(length == 0)
        length = ewcslen((const ewchar_t*)text);

    /* ignore if string is empty */
    if(length == 0) return ELIBC_SUCCESS;

    /* check string encoding */
    if(ustring->encoding == TEXT_ENCODING_UTF16)
    {
        /* update buffer position (remove last zero) */
        err = ebuffer_setpos(&ustring->data, ustring->length * sizeof(utf16_t));
        if(err != ELIBC_SUCCESS) return err;

        /* append text */
        err = ebuffer_append(&ustring->data, text, length * sizeof(utf16_t));
        if(err != ELIBC_SUCCESS) return err;

        /* append end of string */
        err = ebuffer_append_wchar(&ustring->data, 0);
        if(err != ELIBC_SUCCESS) return err;

        /* update length */
        ustring->length += length;

    } else
    {
        size_t required_size, output_size;
        utf8_t* output_str;

        /* update buffer position (remove last zero) */
        err = ebuffer_setpos(&ustring->data, ustring->length * sizeof(utf8_t));
        if(err != ELIBC_SUCCESS) return err;

        /* compute required size */
        required_size = utf16_in_utf8(text, length);

        /* reserve space */
        output_str = (utf8_t*) ebuffer_append_ptr(&ustring->data, required_size * sizeof(utf8_t));
        if(output_str == 0) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;

        /* convert string */
        utf16_to_utf8(text, length, output_str, &output_size);

        /* update position in case some characters were not converted */
        if(output_size != required_size)
        {
            err = ebuffer_setpos(&ustring->data, (ustring->length + output_size) * sizeof(utf8_t));
            if(err != ELIBC_SUCCESS) return err;
        }

        /* append end of string */
        err = ebuffer_append_char(&ustring->data, 0);
        if(err != ELIBC_SUCCESS) return err;

        /* update length */
        ustring->length += output_size;
    }

    /* append end of string */
    return ELIBC_SUCCESS;
}

/* format string */
int ustring_append_char(ustring_t* ustring, char ch)
{
    /* append as utf8 string */
    return ustring_append_utf8(ustring, (const utf8_t*)&ch, 1);
}

int ustring_append_int(ustring_t* ustring, int number)
{
    /* should be enough to fit signed 64 bit integer as string */
    char buffer[32]; 

    /* format string to buffer */
    esnprintf(buffer, sizeof(buffer), "%d", number);

    /* append as utf8 string */
    return ustring_append_utf8(ustring, (const utf8_t*)buffer, 0);
}

int ustring_append_uint(ustring_t* ustring, unsigned int number)
{
    /* should be enough to fit unsigned 64 bit integer as string */
    char buffer[32]; 

    /* format string to buffer */
    esnprintf(buffer, sizeof(buffer), "%u", number);

    /* append as utf8 string */
    return ustring_append_utf8(ustring, (const utf8_t*)buffer, 0);
}

int ustring_append_int64(ustring_t* ustring, eint64_t number)
{
    /* should be enough to fit signed 64 bit integer as string */
    char buffer[32]; 

    /* format string to buffer */
    esnprintf(buffer, sizeof(buffer), "%" EPRId64, number);

    /* append as utf8 string */
    return ustring_append_utf8(ustring, (const utf8_t*)buffer, 0);
}

int ustring_append_uint64(ustring_t* ustring, euint64_t number)
{
    /* should be enough to fit unsigned 64 bit integer as string */
    char buffer[32]; 

    /* format string to buffer */
    esnprintf(buffer, sizeof(buffer), "%" EPRIu64, number);

    /* append as utf8 string */
    return ustring_append_utf8(ustring, (const utf8_t*)buffer, 0);
}

int ustring_append_float(ustring_t* ustring, double number)
{
    size_t required_size;
    char dummy;
    int err;

    /* check input */
    EASSERT(ustring);
    if(ustring == 0) return ELIBC_ERROR_ARGUMENT;

    /* compute required size (in characters) */
    required_size = esnprintf(&dummy, 1, "%f", number);
    if(required_size <= 0) 
    {
        ETRACE("ustring: failed to append floating number");
        return ELIBC_ERROR_ARGUMENT;
    }

    /* check string encoding */
    if(ustring->encoding == TEXT_ENCODING_UTF8)
    {
        char* output_str;

        /* update buffer position (remove last zero) */
        err = ebuffer_setpos(&ustring->data, ustring->length * sizeof(utf8_t));
        if(err != ELIBC_SUCCESS) return err;

        /* reserve space */
        output_str = ebuffer_append_ptr(&ustring->data, required_size * sizeof(utf8_t));
        if(output_str == 0) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;

        /* format number (NOTE: string termination will be added as well) */
        esnprintf(output_str, required_size, "%f", number);

        /* update length */
        ustring->length += required_size;

    } else
    {
        ewchar_t* output_str;

        /* update buffer position (remove last zero) */
        err = ebuffer_setpos(&ustring->data, ustring->length * sizeof(utf16_t));
        if(err != ELIBC_SUCCESS) return err;

        /* reserve space */
        output_str = (ewchar_t*)ebuffer_append_ptr(&ustring->data, required_size * sizeof(ewchar_t));
        if(output_str == 0) return ELIBC_ERROR_NOT_ENOUGH_MEMORY;

        /* format number (NOTE: string termination will be added as well) */
        eswprintf(output_str, required_size, (const ewchar_t*)L"%f", number);

        /* update length */
        ustring->length += required_size;
    }

    return ELIBC_SUCCESS;
}

/* string encoding */
text_encoding_t ustring_encoding(ustring_t* ustring)
{
    EASSERT(ustring);
    return ustring ? ustring->encoding : TEXT_ENCODING_UTF8;
}

/* string length (in bytes for utf8 and in words for utf16) */
size_t ustring_length(ustring_t* ustring)
{
    EASSERT(ustring);
    return ustring ? ustring->length : 0;
}

/* string data */
const utf8_t* ustring_to_utf8(ustring_t* ustring)
{
    int err;

    /* check input */
    EASSERT(ustring);
    if(ustring == 0) return 0;

    /* check if we have any string */
    if(ustring->length == 0)
    {
        /* reset buffer */
        ebuffer_reset(&ustring->data);
        
        /* append end of string */
        err = ebuffer_append_char(&ustring->data, 0);
        if(err != ELIBC_SUCCESS) return 0;
    }

    /* check if we need to convert string first */
    if(ustring->length > 0 && ustring->encoding != TEXT_ENCODING_UTF8)
    {
        size_t required_size, output_size;
        utf8_t* output_str;

        /* compute required size */
        required_size = utf16_in_utf8((const utf16_t*)ebuffer_data(&ustring->data), ustring->length);

        /* reserve space */
        output_str = (utf8_t*) ebuffer_append_ptr(&ustring->data, required_size * sizeof(utf8_t));
        if(output_str == 0) return 0;

        /* convert string */
        utf16_to_utf8((const utf16_t*)ebuffer_data(&ustring->data), ustring->length, output_str, &output_size);

        /* update position in case some characters were not converted */
        if(output_size != required_size)
        {
            err = ebuffer_setpos(&ustring->data, (ustring->length + output_size) * sizeof(utf8_t));
            if(err != ELIBC_SUCCESS) return 0;
        }

        /* append end of string */
        err = ebuffer_append_char(&ustring->data, 0);
        if(err != ELIBC_SUCCESS) return 0;

        /* return converted string */
        return output_str;
    }

    /* return buffer data */
    return (const utf8_t*)ebuffer_data(&ustring->data);
}

const utf16_t* ustring_to_utf16(ustring_t* ustring)
{
    int err;

    /* check input */
    EASSERT(ustring);
    if(ustring == 0) return 0;

    /* check if we have any string */
    if(ustring->length == 0)
    {
        /* reset buffer */
        ebuffer_reset(&ustring->data);
        
        /* append end of string */
        err = ebuffer_append_wchar(&ustring->data, 0);
        if(err != ELIBC_SUCCESS) return 0;
    }

    /* check if we need to convert string first */
    if(ustring->length > 0 && ustring->encoding != TEXT_ENCODING_UTF16)
    {
        size_t required_size, output_size;
        utf16_t* output_str;

        /* compute required size */
        required_size = utf8_in_utf16((const utf8_t*)ebuffer_data(&ustring->data), ustring->length);

        /* reserve space */
        output_str = (utf16_t*) ebuffer_append_ptr(&ustring->data, required_size * sizeof(utf16_t));
        if(output_str == 0) return 0;

        /* convert string */
        utf8_to_utf16((const utf8_t*)ebuffer_data(&ustring->data), ustring->length, output_str, &output_size);

        /* update position in case some characters were not converted */
        if(output_size != required_size)
        {
            err = ebuffer_setpos(&ustring->data, (ustring->length + output_size) * sizeof(utf16_t));
            if(err != ELIBC_SUCCESS) return 0;
        }

        /* append end of string */
        err = ebuffer_append_wchar(&ustring->data, 0);
        if(err != ELIBC_SUCCESS) return 0;

        /* return converted string */
        return output_str;
    }

    /* return buffer data */
    return (const utf16_t*)ebuffer_data(&ustring->data);
}

/*----------------------------------------------------------------------*/
