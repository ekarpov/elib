/*
    UTF conversion and URL encoding helpers
*/

#include "../elib_config.h"

#include "text_format.h"

/*
    NOTE: http://en.wikipedia.org/wiki/UTF-8
*/

/*
    NOTE: http://www.unicode.org/versions/Unicode7.0.0/UnicodeStandard-7.0.pdf

Table 3-7. Well-Formed UTF-8 Byte Sequences
  -----------------------------------------------------------------------------
  |  Code Points        | First Byte | Second Byte | Third Byte | Fourth Byte |
  |  U+0000..U+007F     |     00..7F |             |            |             |
  |  U+0080..U+07FF     |     C2..DF |      80..BF |            |             |
  |  U+0800..U+0FFF     |         E0 |      A0..BF |     80..BF |             |
  |  U+1000..U+CFFF     |     E1..EC |      80..BF |     80..BF |             |
  |  U+D000..U+D7FF     |         ED |      80..9F |     80..BF |             |
  |  U+E000..U+FFFF     |     EE..EF |      80..BF |     80..BF |             |
  |  U+10000..U+3FFFF   |         F0 |      90..BF |     80..BF |      80..BF |
  |  U+40000..U+FFFFF   |     F1..F3 |      80..BF |     80..BF |      80..BF |
  |  U+100000..U+10FFFF |         F4 |      80..8F |     80..BF |      80..BF |
  -----------------------------------------------------------------------------

*/

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/* bytes required to URL-encode char */
static const char urlEncodeBytesCount[256] =
{
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,3,1,1,1,1,1,1,1,1,1,1,3,3,3,3,3,3,
    3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,3,3,3,1,
    3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,3,3,1,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
};

/* HEX encoding helper table */
static const char urlHexEncodingChars[16] =
{
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};

/* HEX decoding helper table (16 means invalid HEX character) */
static const char urlHexDecodingChars[256] =
{
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,0,1,2,3,4,5,6,7,8,9,16,16,16,16,16,16,
    16,10,11,12,13,14,15,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    16,10,11,12,13,14,15,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
};

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

/* bytes in utf8 sequence */
static const char utf8ByteCount[256] = 
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,6,6
};

/* special offset table for efficient utf8 to utf16 conversion */
static const utf32_t utf8ConversionOffsets[7] =
{
    0x00000000,
    0x00000000,
    0x00003080,
    0x000E2080,
    0x03C82080,
    0xFA082080,
    0x82082080
};

/* byte marks for utf8 encoding */
static const utf8_t utf8ByteMarks[7] =
{
    0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
};

/* replacement character */
#define UNICODE_REPLACEMENT_CHAR        0xFFFD

/* unicode surrogates */
#define UNICODE_SURROGATE_HIGH_START    0xD800
#define UNICODE_SURROGATE_HIGH_END      0xDBFF
#define UNICODE_SURROGATE_LOW_START     0xDC00
#define UNICODE_SURROGATE_LOW_END       0xDFFF

/*----------------------------------------------------------------------*/
/* 
 *      URL encoding helpers
 */
/*----------------------------------------------------------------------*/

/* required buffer sizes */
size_t url_encoded_size(const char* url_str, size_t str_len)
{
    size_t ret_size = 0;
    size_t idx;

    /* loop over string */
    if(str_len > 0)
    {
        for(idx = 0; idx < str_len; ++idx)
            ret_size += urlEncodeBytesCount[(unsigned char)url_str[idx]];

    } else
    {
        for(; *url_str != 0; ++url_str)
            ret_size += urlEncodeBytesCount[(unsigned char)*url_str];
    }

    return ret_size;
}

size_t url_decoded_size(const char* url_str, size_t str_len)
{
    size_t ret_size = str_len;
    size_t idx;

    /* loop over string */
    if(str_len > 0)
    {
        for(idx = 0; idx < str_len; ++idx)
        {
            if(url_str[idx] == '%')
            {
                ret_size += 2;
                idx += 2;
            }
        }

    } else
    {
        for(; *url_str != 0; ++url_str)
        {
            if(*url_str == '%')
            {
                ret_size += 2;
                url_str++;
                if(*url_str != 0) url_str++;
            }
        }
    }

    return ret_size;
}

/* encode and decode */
size_t url_encode(const char* str_input, size_t str_len, char* str_output, size_t* output_size)
{
    size_t in_idx, out_idx;
    size_t max_output = (output_size && *output_size > 0) ? *output_size : SIZE_MAX;
    unsigned char encodeChar;

    /* check input */
    if(str_input == 0 || str_len <= 0)
    {
        /* reset output size if needed */
        if(output_size) *output_size = 0;
        return 0;
    }

    /* validate output */
    EASSERT(str_output);
    if(str_output == 0) return 0;

    /* loop over input */
    for(in_idx = 0, out_idx = 0; in_idx < str_len && out_idx < max_output; ++in_idx, ++out_idx)
    {
        /* check if char needs to be encoded */
        if(urlEncodeBytesCount[(unsigned char)str_input[in_idx]] == 1)
        {
            /* copy as is */
            str_output[out_idx] = str_input[in_idx];

        } else
        {
            if(out_idx + 2 >= max_output)
            {
                /* not enough space to output, stop */
                break;
            }

            /* character to encode */
            encodeChar = (unsigned char)str_input[in_idx];

            /* encode */
            str_output[out_idx] = '%';
            str_output[out_idx + 1] = urlHexEncodingChars[(encodeChar >> 4) & 0x0F];
            str_output[out_idx + 2] = urlHexEncodingChars[encodeChar & 0x0F];

            /* advance */
            out_idx += 2;
        }
    }

    /* copy output size if needed */
    if(output_size) *output_size = out_idx;

    /* amount of bytes processed from input */
    return in_idx;
}

size_t url_decode(const char* str_input, size_t str_len, char* str_output, size_t* output_size)
{
    size_t in_idx, out_idx;
    size_t max_output = (output_size && *output_size > 0) ? *output_size : SIZE_MAX;
    unsigned char decodeChar1, decodeChar2;

    /* check input */
    if(str_input == 0 || str_len <= 0)
    {
        /* reset output size if needed */
        if(output_size) *output_size = 0;
        return 0;
    }

    /* validate output */
    EASSERT(str_output);
    if(str_output == 0) return 0;

    /* loop over input */
    for(in_idx = 0, out_idx = 0; in_idx < str_len && out_idx < max_output; ++in_idx, ++out_idx)
    {
        /* check if char needs to be decoded */
        if(str_input[in_idx] != '%')
        {
            /* copy as is */
            str_output[out_idx] = str_input[in_idx];

        } else
        {
            if(in_idx + 2 >= str_len)
            {
                /* not enough input, stop */
                ETRACE("url_decode: not enough input given");
                break;
            }

            /* get characters */
            decodeChar1 = urlHexDecodingChars[(unsigned char)str_input[in_idx + 1]];
            decodeChar2 = urlHexDecodingChars[(unsigned char)str_input[in_idx + 2]];

            /* check if characters are correct */
            if(decodeChar1 != 16 && decodeChar2 != 16)
            {
                /* decode character */
                str_output[out_idx] = (decodeChar1 << 4) | decodeChar2;

            } else
            {
                ETRACE("url_decode: invalid hex character ignored");
                out_idx--;
            }

            /* advance */
            in_idx += 2;
        }
    }

    /* copy output size if needed */
    if(output_size) *output_size = out_idx;

    /* amount of bytes processed from input */
    return in_idx;
}

/*----------------------------------------------------------------------*/
/* 
 *      UTF conversion helpers
 */
/*----------------------------------------------------------------------*/

/* convert utf16 to utf32 character and advance input string */
static ELIBC_FORCE_INLINE const utf16_t* _utf16_to_utf32_impl(const utf16_t* utf16_str, size_t str_len, utf32_t* utf32, size_t* replaced_chars)
{
    /* init output */
    *utf32 = *utf16_str;

    /* handle surrogates first */
    if(*utf16_str >= UNICODE_SURROGATE_HIGH_START && *utf16_str <= UNICODE_SURROGATE_HIGH_END)
    {
        /* check if there is next character following */
        if(str_len <= 1)
        {
            ETRACE("_utf16_to_utf32_impl: not enough input given");

            /* not enough input given, replace surrogate */
            *utf32 = UNICODE_REPLACEMENT_CHAR;
            *replaced_chars += 1;
            return ++utf16_str;
        }

        /* copy high surrogate */
        *utf32 = *utf16_str - UNICODE_SURROGATE_HIGH_START;
        *utf32 <<= 10;

        /* advance */
        utf16_str++;

        /* check if there is low surrogate following */
        if(*utf16_str >= UNICODE_SURROGATE_LOW_START && *utf16_str <= UNICODE_SURROGATE_LOW_END)
        {
            /* append low surrogate */
            *utf32 += *utf16_str - UNICODE_SURROGATE_LOW_START + 0x0010000;

        } else
        {
            /* input error, replace surrogate */
            *replaced_chars += 1;
            *utf32 = UNICODE_REPLACEMENT_CHAR;

            /* return character back */
            utf16_str--;
        }

    } else if(*utf16_str >= UNICODE_SURROGATE_LOW_START && *utf16_str <= UNICODE_SURROGATE_LOW_END)
    {
        /* utf16 doesn't allow single characters in low surrogate range */
        *replaced_chars += 1;
        *utf32 = UNICODE_REPLACEMENT_CHAR;
    }

    return ++utf16_str;
}

/* utf8 bytes required to save utf32 codepoint */
static ELIBC_FORCE_INLINE int utf32_in_utf8(utf32_t utf32)
{
    if (utf32 < 0x80)
        return 1;
    else if(utf32 < 0x800)
        return 2;
    else if(utf32 < 0x10000)
        return 3;
    else if(utf32 < 0x110000)
        return 4;
    else
        /* utf16 doesn't support characters above U+10FFFF */
        return 3; /* bytes to encode replacement char */
}

/*----------------------------------------------------------------------*/

/* required buffer sizes */
size_t utf8_in_utf16(const utf8_t* utf8_str, size_t str_len)
{
    const utf8_t* str_pos = utf8_str;
    size_t utf16_len = 0;

    EASSERT(utf8_str);
    EASSERT(str_len >= 0);
    if(utf8_str == 0 || str_len < 0) return 0;

    /* loop over string */
    while(str_pos < utf8_str + str_len)
    {
        /* advance */
        utf16_len++;
        str_pos += utf8ByteCount[*str_pos];
    }

    return utf16_len;
}

size_t utf16_in_utf8(const utf16_t* utf16_str, size_t str_len)
{
    const utf16_t* str_pos = utf16_str;
    size_t utf8_len = 0;
    size_t replaced_chars = 0;
    utf32_t utf32;

    EASSERT(utf16_str);
    EASSERT(str_len >= 0);
    if(utf16_str == 0 || str_len < 0) return 0;

    /* loop over string */
    while(str_pos < utf16_str + str_len)
    {
        /* convert to utf32 character */
        str_pos = _utf16_to_utf32_impl(str_pos, str_len - (str_pos - utf16_str), &utf32, &replaced_chars);

        /* bytes required */
        utf8_len += utf32_in_utf8(utf32);
    }

    return utf8_len;
}

/* conversion */
size_t utf8_to_utf16(const utf8_t* utf8_str, size_t str_len, utf16_t* utf16_str, size_t* output_size)
{
    size_t in_idx = 0;
    size_t out_idx = 0;
    size_t max_output = (output_size && *output_size > 0) ? *output_size : SIZE_MAX;
    utf32_t utf32;
    size_t replaced_chars = 0;

    EASSERT(utf8_str);
    EASSERT(utf16_str);
    EASSERT(str_len >= 0);
    if(utf8_str == 0 || utf16_str == 0 || str_len < 0) return 0;

    /* loop over input */
    while(in_idx < str_len)
    {
        /* utf8 bytes to process */
        int bytesCount = utf8ByteCount[utf8_str[in_idx]];

        /* check if enough input */
        if(in_idx + bytesCount > str_len)
        {
            ETRACE("utf8_to_utf16: not enough input given");
            break;
        }

        /* stop if maximum output size */
        if(out_idx > max_output) break;

        /* use 32bits char at first */
        utf32 = 0;

        /* convert */
        switch(bytesCount)
        {
        case 6: utf32 += utf8_str[in_idx]; utf32 <<= 6; ++in_idx;
        case 5: utf32 += utf8_str[in_idx]; utf32 <<= 6; ++in_idx;
        case 4: utf32 += utf8_str[in_idx]; utf32 <<= 6; ++in_idx;
        case 3: utf32 += utf8_str[in_idx]; utf32 <<= 6; ++in_idx;
        case 2: utf32 += utf8_str[in_idx]; utf32 <<= 6; ++in_idx;
        case 1: utf32 += utf8_str[in_idx]; ++in_idx;
        };

        /* remove all extra bits added during conversion */
        utf32 -= utf8ConversionOffsets[bytesCount];

        /*
            NOTE: UTF-8 was restricted by RFC3629 (http://tools.ietf.org/html/rfc3629)  to end at U+10FFFF, 
                  in order to match the constraints of the UTF-16 character encoding
        */

        /* check if result will fit to utf16 */
        if(utf32 <= 0x10FFFF)
        {
            /* copy result */
            utf16_str[out_idx] = (utf16_t)utf32;
        } else
        {
            /* replace */
            replaced_chars++;
            utf16_str[out_idx] = UNICODE_REPLACEMENT_CHAR;
        }

        /* advance counters */
        out_idx += 1;
    }

    if(replaced_chars > 0)
    {
        ETRACE("utf8_to_utf16: %d too large codepoints replaced", replaced_chars);
    }
            
    /* copy output size if needed */
    if(output_size) *output_size = out_idx;

    /* amount of bytes processed from input */
    return in_idx;
}

size_t utf16_to_utf8(const utf16_t* utf16_str, size_t str_len, utf8_t* utf8_str, size_t* output_size)
{
    const utf16_t* str_pos = utf16_str;
    size_t out_idx = 0;
    size_t max_output = (output_size && *output_size > 0) ? *output_size : SIZE_MAX;
    utf32_t utf32;
    size_t replaced_chars = 0;
    size_t bytesCount = 0;

    EASSERT(utf8_str);
    EASSERT(utf16_str);
    EASSERT(str_len >= 0);
    if(utf8_str == 0 || utf16_str == 0 || str_len < 0) return 0;

    /* loop over string */
    while(str_pos < utf16_str + str_len)
    {
        /* convert to utf32 character */
        const utf16_t* next_pos = _utf16_to_utf32_impl(str_pos, str_len - (str_pos - utf16_str), &utf32, &replaced_chars);

        /* utf8 bytes to write */
        bytesCount = utf32_in_utf8(utf32);

        /* stop if maximum output size */
        if(out_idx + bytesCount > max_output) break;

        /* convert utf32 to utf8 */
        EASSERT(bytesCount <= 4);
        switch (bytesCount)
        {
        case 4: utf8_str[out_idx + 3] = (utf8_t)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 3: utf8_str[out_idx + 2] = (utf8_t)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 2: utf8_str[out_idx + 1] = (utf8_t)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 1: utf8_str[out_idx] = (utf8_t)(utf32 | utf8ByteMarks[bytesCount]);
        }

        /* advance */
        str_pos = next_pos;
        out_idx += bytesCount;
    }

    if(replaced_chars > 0)
    {
        ETRACE("utf16_to_utf8: %d codepoints replaced", replaced_chars);
    }

    /* copy output size if needed */
    if(output_size) *output_size = out_idx;

    /* amount of bytes processed from input */
    return (str_pos - utf16_str);
}

/* utf32 required buffer sizes */
size_t utf32_in_utf16(const utf32_t* utf32_str, size_t str_len)
{
    size_t in_idx = 0;
    size_t out_len = 0;
    utf32_t utf32;

    EASSERT(utf32_str);
    EASSERT(str_len >= 0);
    if(utf32_str == 0 || str_len < 0) return 0;

    /* loop over string */
    while(in_idx < str_len)
    {
        /* input value */
        utf32 = utf32_str[in_idx++];

        /* check if utf32 needs to be split into surrogates */
        if(utf32 > 0xFFFF && utf32 <= 0x10FFFF)
            out_len += 2;
        else
            out_len += 1;
    }

    return out_len;
}

size_t utf16_in_utf32(const utf16_t* utf16_str, size_t str_len)
{
    const utf16_t* str_pos = utf16_str;
    size_t out_len = 0;
    size_t replaced_chars = 0;
    utf32_t utf32;

    EASSERT(utf16_str);
    EASSERT(str_len >= 0);
    if(utf16_str == 0 || str_len < 0) return 0;

    /* loop over string */
    while(str_pos < utf16_str + str_len)
    {
        /* convert to utf32 character */
        str_pos = _utf16_to_utf32_impl(str_pos, str_len - (str_pos - utf16_str), &utf32, &replaced_chars);

        /* output size */
        out_len++;
    }

    /* required size */
    return out_len;
}

/* utf32 conversion */
size_t utf16_to_utf32(const utf16_t* utf16_str, size_t str_len, utf32_t* utf32_str, size_t* output_size)
{
    const utf16_t* str_pos = utf16_str;
    size_t max_output = (output_size && *output_size > 0) ? *output_size : SIZE_MAX;
    size_t out_idx = 0;
    size_t replaced_chars = 0;
    utf32_t utf32;

    EASSERT(utf16_str);
    EASSERT(str_len >= 0);
    EASSERT(utf32_str);
    if(utf16_str == 0 || str_len < 0 || utf32_str == 0) return 0;

    /* loop over string */
    while(str_pos < utf16_str + str_len && out_idx < max_output)
    {
        /* convert to utf32 character */
        str_pos = _utf16_to_utf32_impl(str_pos, str_len - (str_pos - utf16_str), &utf32, &replaced_chars);

        /* copy output */
        utf32_str[out_idx++] = utf32;
    }

    if(replaced_chars > 0)
    {
        ETRACE("utf16_to_utf32: %d codepoints replaced", replaced_chars);
    }

    /* copy output size if needed */
    if(output_size) *output_size = out_idx;

    /* amount of bytes processed from input */
    return (str_pos - utf16_str);
}

size_t utf32_to_utf16(const utf32_t* utf32_str, size_t str_len, utf16_t* utf16_str, size_t* output_size)
{
    size_t max_output = (output_size && *output_size > 0) ? *output_size : SIZE_MAX;
    size_t in_idx = 0;
    size_t out_idx = 0;
    utf32_t utf32;
    size_t replaced_chars = 0;

    EASSERT(utf32_str);
    EASSERT(utf16_str);
    EASSERT(str_len >= 0);
    if(utf16_str == 0 || str_len < 0 || utf32_str == 0) return 0;

    /* loop over string */
    while(in_idx < str_len && out_idx < max_output)
    {
        /* input value */
        utf32 = utf32_str[in_idx++];

        /* check if value will fit into utf16 */
        if(utf32 <= 0xFFFF)
        {
            /* copy as is */
            utf16_str[out_idx++] = (utf16_t)utf32;

        } else if(utf32 <= 0x10FFFF)
        {
            /* stop if maximum output size */
            if(out_idx + 2 > max_output) break;

            /* split into surrogates */
            utf32 -= 0x0010000;

            /* top ten bits into first surrogate */
            utf16_str[out_idx++] = ((utf32 & 0x00FFC00) >> 10) + UNICODE_SURROGATE_HIGH_START;

            /* low ten bits into second surrogate */
            utf16_str[out_idx++] = (utf32 & 0x00003FF) + UNICODE_SURROGATE_LOW_START;

        } else
        {
            /* maximum range for utf16 is 0x10FFFF */
            replaced_chars += 1;
            utf16_str[out_idx++] = UNICODE_REPLACEMENT_CHAR;
        }
    }

    if(replaced_chars > 0)
    {
        ETRACE("utf32_to_utf16: %d codepoints replaced", replaced_chars);
    }

    /* copy output size if needed */
    if(output_size) *output_size = out_idx;

    /* amount of bytes processed from input */
    return in_idx;
}

/* validate */
int is_valid_utf8(const utf8_t* utf8_str, size_t str_len)
{
    size_t in_idx = 0;

    EASSERT(utf8_str);
    EASSERT(str_len >= 0);
    if(utf8_str == 0 || str_len < 0) return ELIBC_FALSE;

    /* loop over input */
    while(in_idx < str_len)
    {
        /* utf8 bytes to process */
        int bytesCount = utf8ByteCount[utf8_str[in_idx]];

        /* check if enough input */
        if(in_idx + bytesCount > str_len) return ELIBC_FALSE;

        /* validate */
        switch(bytesCount)
        {
        case 6: if((utf8_str[in_idx] & 0xC0) != 0x80) return ELIBC_FALSE; ++in_idx;  
        case 5: if((utf8_str[in_idx] & 0xC0) != 0x80) return ELIBC_FALSE; ++in_idx; 
        case 4: if((utf8_str[in_idx] & 0xC0) != 0x80) return ELIBC_FALSE; ++in_idx; 
        case 3: if((utf8_str[in_idx] & 0xC0) != 0x80) return ELIBC_FALSE; ++in_idx; 
        case 2: if((utf8_str[in_idx] & 0xC0) != 0x80) return ELIBC_FALSE; ++in_idx; 
        case 1: ++in_idx;
        };
    }

    return ELIBC_TRUE;
}

/* utf8 character offset to byte offset */
size_t utf8_offset(const utf8_t* utf8_str, size_t str_len, size_t chars_offset)
{
    size_t in_idx = 0;

    EASSERT(utf8_str);
    if(utf8_str == 0) return 0;

    /* loop over input */
    while(in_idx < str_len && chars_offset > 0)
    {
        /* utf8 bytes to process */
        int bytesCount = utf8ByteCount[utf8_str[in_idx]];

        /* check if enough input */
        if(in_idx + bytesCount > str_len) return str_len;

        /* advance */
        in_idx += bytesCount;
        chars_offset--;
    }

    return in_idx;
}

/*----------------------------------------------------------------------*/
/*
    Combined functions to convert utf16 to and from utf8 url-encoded strings
*/

/* implementation helpers (NOTE: if output is not given function just computes required output size) */
static size_t utf8url_to_utf16_impl(const char* utf8url_str, size_t str_len, utf16_t* utf16_str, size_t* output_size)
{
    size_t in_idx = 0;
    size_t out_idx = 0;
    size_t max_output = (output_size && *output_size > 0) ? *output_size : SIZE_MAX;
    utf32_t utf32;
    size_t replaced_chars = 0;
    size_t buffPos = 0;
    utf8_t utf8_buff[6];
    size_t bytesCount = 0;
    size_t ignoreOut = 0;
    size_t processedInput = 0;

    /* loop over input */
    while(in_idx < str_len)
    {
        /* stop if maximum output size */
        if(out_idx > max_output) break;

        EASSERT(buffPos < 6);

        /* check if char needs to be url-decoded first */
        if(utf8url_str[in_idx] != '%')
        {
            /* use as is */
            utf8_buff[buffPos] = utf8url_str[in_idx];

            /* advance */
            in_idx += 1;

        } else
        {
            unsigned char decodeChar1, decodeChar2;

            if(in_idx + 2 >= str_len)
            {
                /* not enough input, stop */
                ETRACE("utf8url_to_utf16: not enough input given");
                break;
            }

            /* get characters */
            decodeChar1 = urlHexDecodingChars[(unsigned char)utf8url_str[in_idx + 1]];
            decodeChar2 = urlHexDecodingChars[(unsigned char)utf8url_str[in_idx + 2]];

            /* check if characters are correct */
            if(decodeChar1 != 16 && decodeChar2 != 16)
            {
                /* decode character */
                utf8_buff[buffPos] = (decodeChar1 << 4) | decodeChar2;

            } else
            {
                /* character is not valid so skip full utf8 character */
                ETRACE("utf8url_to_utf16: invalid hex character while url-decoding");
                utf8_buff[buffPos] = 0;
                ignoreOut = 1;
            }

            /* advance */
            in_idx += 3;
        }

        /* check if this is first utf8 character */
        if(buffPos == 0)
        {
            /* get required byte count */
            bytesCount = utf8ByteCount[utf8_buff[buffPos]];
            EASSERT(bytesCount <= 6);
        }

        /* advance buffer position */
        buffPos++;

        /* check if we have enough characters in buffer to convert */
        if(buffPos == bytesCount)
        {
            /* use 32bits char at first */
            utf32 = 0;

            /* convert */
            buffPos = 0;
            switch(bytesCount)
            {
            case 6: utf32 += utf8_buff[buffPos]; utf32 <<= 6; ++buffPos;
            case 5: utf32 += utf8_buff[buffPos]; utf32 <<= 6; ++buffPos;
            case 4: utf32 += utf8_buff[buffPos]; utf32 <<= 6; ++buffPos;
            case 3: utf32 += utf8_buff[buffPos]; utf32 <<= 6; ++buffPos;
            case 2: utf32 += utf8_buff[buffPos]; utf32 <<= 6; ++buffPos;
            case 1: utf32 += utf8_buff[buffPos];
            };

            /* remove all extra bits added during conversion */
            utf32 -= utf8ConversionOffsets[bytesCount];

            /*
                NOTE: UTF-8 was restricted by RFC3629 (http://tools.ietf.org/html/rfc3629)  to end at U+10FFFF,
                      in order to match the constraints of the UTF-16 character encoding
            */

            /* copy output if needed */
            if(utf16_str)
            {
                /* check if result will fit to utf16 and not ignored */
                if(utf32 <= 0x10FFFF && !ignoreOut)
                {
                    /* copy result */
                    utf16_str[out_idx] = (utf16_t)utf32;

                } else
                {
                    /* replace */
                    replaced_chars++;
                    utf16_str[out_idx] = UNICODE_REPLACEMENT_CHAR;
                }
            }

            /* advance counters */
            out_idx += 1;
            processedInput = in_idx;

            /* reset counters */
            buffPos = 0;
            bytesCount = 0;
            ignoreOut = 0;
        }
    }

    if(replaced_chars > 0)
    {
        ETRACE("utf8url_to_utf16: %d codepoints replaced", replaced_chars);
    }

    /* copy output size if needed */
    if(output_size) *output_size = out_idx;

    /* amount of bytes processed from input */
    return processedInput;
}

static size_t utf16_to_utf8url_impl(const utf16_t* utf16_str, size_t str_len, char* utf8url_str, size_t* output_size)
{
    const utf16_t* str_pos = utf16_str;
    size_t out_idx = 0;
    size_t max_output = (output_size && *output_size > 0) ? *output_size : SIZE_MAX;
    utf32_t utf32;
    size_t replaced_chars = 0;
    size_t bytesCount = 0;
    utf8_t utf8_buff[4];
    size_t idx, out_pos;

    /* loop over string */
    while(str_pos  < utf16_str + str_len)
    {
        /* convert to utf32 character */
        const utf16_t* next_pos = _utf16_to_utf32_impl(str_pos, str_len - (str_pos - utf16_str), &utf32, &replaced_chars);

        /* utf8 bytes to write */
        bytesCount = utf32_in_utf8(utf32);

        /* convert utf32 to utf8 */
        EASSERT(bytesCount <= 4);
        switch (bytesCount)
        {
        case 4: utf8_buff[3] = (utf8_t)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 3: utf8_buff[2] = (utf8_t)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 2: utf8_buff[1] = (utf8_t)((utf32 | 0x80) & 0xBF); utf32 >>= 6;
        case 1: utf8_buff[0] = (utf8_t)(utf32 | utf8ByteMarks[bytesCount]);
        }

        /* get current positon */
        out_pos = out_idx;

        /* stop if maximum output size (assume that all characters will need to be converted) */
        if(out_pos + bytesCount * 3 > max_output) break;

        /* encode buffer to output string */
        for(idx = 0; idx < bytesCount; ++idx)
        {
            /* check if char needs to be encoded */
            if(urlEncodeBytesCount[(unsigned char)utf8_buff[idx]] == 1)
            {
                /* copy as is */
                if(utf8url_str)
                {
                    utf8url_str[out_pos] = utf8_buff[idx];
                }

                /* advance */
                out_pos += 1;

            } else
            {
                /* encode */
                if(utf8url_str)
                {
                    /* character to encode */
                    unsigned char encodeChar = (unsigned char)utf8_buff[idx];

                    utf8url_str[out_pos] = '%';
                    utf8url_str[out_pos + 1] = urlHexEncodingChars[(encodeChar >> 4) & 0x0F];
                    utf8url_str[out_pos + 2] = urlHexEncodingChars[encodeChar & 0x0F];
                }

                /* advance */
                out_pos += 3;
            }
        }

        /* advance */
        str_pos = next_pos;
        out_idx = out_pos;
    }

    if(replaced_chars > 0)
    {
        ETRACE("utf16_to_utf8url: %d codepoints replaced", replaced_chars);
    }

    /* copy output size if needed */
    if(output_size) *output_size = out_idx;

    /* amount of bytes processed from input */
    return (str_pos - utf16_str);
}

/* required buffer sizes */
size_t utf8url_in_utf16(const char* utf8url_str, size_t str_len)
{
    size_t output_size = 0;

    EASSERT(utf8url_str);
    EASSERT(str_len >= 0);
    if(utf8url_str == 0 || str_len < 0) return 0;

    /* pass to implementation */
    utf8url_to_utf16_impl(utf8url_str, str_len, 0, &output_size);

    /* required size */
    return output_size;
}

size_t utf16_in_utf8url(const utf16_t* utf16_str, size_t str_len)
{
    size_t output_size = 0;

    EASSERT(utf16_str);
    EASSERT(str_len >= 0);
    if(utf16_str == 0 || str_len < 0) return 0;

    /* pass to implementation */
    utf16_to_utf8url_impl(utf16_str, str_len, 0, &output_size);

    /* required size */
    return output_size;
}

/* conversion */
size_t utf8url_to_utf16(const char* utf8url_str, size_t str_len, utf16_t* utf16_str, size_t* output_size)
{
    EASSERT(utf8url_str);
    EASSERT(utf16_str);
    EASSERT(str_len >= 0);
    if(utf8url_str == 0 || utf16_str == 0 || str_len < 0) return 0;

    /* pass to implementation */
    return utf8url_to_utf16_impl(utf8url_str, str_len, utf16_str, output_size);
}

size_t utf16_to_utf8url(const utf16_t* utf16_str, size_t str_len, char* utf8url_str, size_t* output_size)
{
    EASSERT(utf8url_str);
    EASSERT(utf16_str);
    EASSERT(str_len >= 0);
    if(utf8url_str == 0 || utf16_str == 0 || str_len < 0) return 0;

    /* pass to implementation */
    return utf16_to_utf8url_impl(utf16_str, str_len, utf8url_str, output_size);
}

/*----------------------------------------------------------------------*/

/*
    HEX string to number conversion
*/

/* conversion (at most 4 characters will be used from hex_str) */
size_t hexstr_to_uint16(const char* hex_str, size_t str_len, euint16_t* num_out)
{
    size_t idx;
    char dch;

    EASSERT(hex_str);
    EASSERT(str_len);
    EASSERT(num_out);
    if(hex_str == 0 || str_len == 0 || num_out < 0) return 0;

    /* reset output */
    *num_out = 0;

    /* process input */
    for(idx = 0; idx < str_len && idx < 4; ++idx)
    {
        /* convert character */
        dch = urlHexDecodingChars[(unsigned char)hex_str[idx]];

        /* validate character */
        if(dch != 16)
        {
            /* convert character */
            *num_out = ((*num_out) << 4) | dch;

        } else
        {
            ETRACE("hexstr_to_uint16: invalid hex character ignored");

            /* assume zero */
            *num_out = (*num_out) << 4;
        }
    }

    return idx;
}

size_t hexstr_to_uint32(const char* hex_str, size_t str_len, euint32_t* num_out)
{
    size_t idx;
    char dch;

    EASSERT(hex_str);
    EASSERT(str_len);
    EASSERT(num_out);
    if(hex_str == 0 || str_len == 0 || num_out < 0) return 0;

    /* reset output */
    *num_out = 0;

    /* process input */
    for(idx = 0; idx < str_len && idx < 8; ++idx)
    {
        /* convert character */
        dch = urlHexDecodingChars[(unsigned char)hex_str[idx]];

        /* validate character */
        if(dch != 16)
        {
            /* convert character */
            *num_out = ((*num_out) << 4) | dch;

        } else
        {
            ETRACE("hexstr_to_uint32: invalid hex character ignored");

            /* assume zero */
            *num_out = (*num_out) << 4;
        }
    }

    return idx;
}

/* validate */
int is_valid_hexchar(unsigned char ch)
{
    return (urlHexDecodingChars[(unsigned char)ch] != 16);
}

/*----------------------------------------------------------------------*/

/*
    Escaped utf8 string
*/

/* convert utf16 to utf32 character and advance input string */
static ELIBC_FORCE_INLINE size_t _utf32_escape_size_impl(utf32_t* utf32, size_t* replaced_chars)
{
    /* check if character needs to be replaced */
    if((*utf32) > 0x10FFFF)
    {
        (*utf32) = UNICODE_REPLACEMENT_CHAR;
        *replaced_chars += 1;
    }

    return ((*utf32) > 0xFFFF) ? 6 : 4;
}

static ELIBC_FORCE_INLINE void _utf32_escape_impl(utf32_t utf32, size_t charCount, char* utf8esc_str, size_t out_pos)
{
    /* escape mark */
    utf8esc_str[out_pos++] = '\\';
    utf8esc_str[out_pos++] = 'u';

    if(charCount == 6)
    {
        utf8esc_str[out_pos++] = urlHexEncodingChars[(unsigned char)((utf32 & 0xF00000) >> 20) & 0x0F];
        utf8esc_str[out_pos++] = urlHexEncodingChars[(unsigned char)((utf32 & 0x0F0000) >> 16) & 0x0F];
    }

    utf8esc_str[out_pos++] = urlHexEncodingChars[(unsigned char)((utf32 & 0xF000) >> 12) & 0x0F];
    utf8esc_str[out_pos++] = urlHexEncodingChars[(unsigned char)((utf32 & 0x0F00) >> 8) & 0x0F];
    utf8esc_str[out_pos++] = urlHexEncodingChars[(unsigned char)((utf32 & 0x00F0) >> 4) & 0x0F];
    utf8esc_str[out_pos++] = urlHexEncodingChars[(unsigned char)((utf32 & 0x000F)) & 0x0F];
}

/* implementation helpers (NOTE: if output is not given function just computes required output size) */
static size_t utf8_to_utf8esc_impl(const utf8_t* utf8_str, size_t str_len, char* utf8esc_str, size_t* output_size)
{
    size_t in_idx = 0;
    size_t out_idx = 0;
    size_t max_output = (output_size && *output_size > 0) ? *output_size : SIZE_MAX;
    size_t bytesCount = 0;
    size_t charCount = 0;
    size_t replaced_chars = 0;
    utf32_t utf32;

    /* process input */
    while(in_idx < str_len)
    {
        /* get byte count */
        bytesCount = utf8ByteCount[utf8_str[in_idx]];

        /* check if enough input */
        if(in_idx + bytesCount > str_len)
        {
            break;
        }

        /* check if we need to escape */
        if(bytesCount == 1)
        {
            /* stop if maximum output size */
            if(out_idx + 1 > max_output) break;

            /* copy as is */
            if(utf8esc_str)
            {
                utf8esc_str[out_idx] = utf8_str[in_idx];
            }

            /* advance */
            out_idx += 1;

        } else
        {
            /* convert to utf32 */
            utf32 = 0;

            /* convert */
            switch(bytesCount)
            {
            case 6: utf32 += utf8_str[in_idx]; utf32 <<= 6; ++in_idx;
            case 5: utf32 += utf8_str[in_idx]; utf32 <<= 6; ++in_idx;
            case 4: utf32 += utf8_str[in_idx]; utf32 <<= 6; ++in_idx;
            case 3: utf32 += utf8_str[in_idx]; utf32 <<= 6; ++in_idx;
            case 2: utf32 += utf8_str[in_idx]; utf32 <<= 6; ++in_idx;
            case 1: utf32 += utf8_str[in_idx]; ++in_idx;
            };

            /* remove all extra bits added during conversion */
            utf32 -= utf8ConversionOffsets[bytesCount];

            /* character count */
            charCount = _utf32_escape_size_impl(&utf32, &replaced_chars);

            /* stop if maximum output size */
            if(out_idx + 2 + charCount > max_output) break;

            /* check if string is provided */
            if(utf8esc_str)
            {
                /* escape unicode character */
                _utf32_escape_impl(utf32, charCount, utf8esc_str, out_idx);
            }

            /* advance */
            out_idx += (2 + charCount);
        }

        /* advance */
        in_idx += bytesCount;
    }

    if(replaced_chars > 0)
    {
        ETRACE("utf8_to_utf8esc: %d codepoints replaced", replaced_chars);
    }

    /* copy output size if needed */
    if(output_size) *output_size = out_idx;

    /* amount of bytes processed from input */
    return in_idx;
}

static size_t utf16_to_utf8esc_impl(const utf16_t* utf16_str, size_t str_len, char* utf8esc_str, size_t* output_size)
{
    const utf16_t* str_pos = utf16_str;
    size_t max_output = (output_size && *output_size > 0) ? *output_size : SIZE_MAX;
    utf32_t utf32;
    size_t replaced_chars = 0;
    size_t bytesCount = 0;
    size_t charCount = 0;
    size_t out_pos;

    out_pos = 0;

    /* loop over string */
    while(str_pos < utf16_str + str_len)
    {
        /* convert to utf32 character */
        const utf16_t* next_pos = _utf16_to_utf32_impl(str_pos, str_len - (str_pos - utf16_str), &utf32, &replaced_chars);

        /* utf8 bytes to write */
        bytesCount = utf32_in_utf8(utf32);

        if(bytesCount == 1)
        {
            /* stop if maximum output size */
            if(out_pos + 1 > max_output) break;

            /* copy as is */
            if(utf8esc_str)
            {
                utf8esc_str[out_pos] = (utf8_t)(utf32 | utf8ByteMarks[bytesCount]);
            }

            /* advance */
            out_pos += 1;

        } else
        {
            /* character count */
            charCount = _utf32_escape_size_impl(&utf32, &replaced_chars);

            /* stop if maximum output size */
            if(out_pos + 2 + charCount > max_output) break;

            /* check if string is provided */
            if(utf8esc_str)
            {
                /* escape unicode character */
                _utf32_escape_impl(utf32, charCount, utf8esc_str, out_pos);
            }

            /* advance */
            out_pos += (2 + charCount);
        }

        /* advance */
        str_pos = next_pos;
    }

    if(replaced_chars > 0)
    {
        ETRACE("utf16_to_utf8esc: %d codepoints replaced", replaced_chars);
    }

    /* copy output size if needed */
    if(output_size) *output_size = out_pos;

    /* amount of bytes processed from input */
    return (str_pos - utf16_str);
}

/* required buffer sizes */
size_t utf8_in_utf8esc(const utf8_t* utf8_str, size_t str_len)
{
    size_t output_size = 0;

    EASSERT(utf8_str);
    EASSERT(str_len >= 0);
    if(utf8_str == 0 || str_len < 0) return 0;

    /* pass to implementation */
    utf8_to_utf8esc_impl(utf8_str, str_len, 0, &output_size);

    /* required size */
    return output_size;
}

size_t utf16_in_utf8esc(const utf16_t* utf16_str, size_t str_len)
{
    size_t output_size = 0;

    EASSERT(utf16_str);
    EASSERT(str_len >= 0);
    if(utf16_str == 0 || str_len < 0) return 0;

    /* pass to implementation */
    utf16_to_utf8esc_impl(utf16_str, str_len, 0, &output_size);

    /* required size */
    return output_size;
}

/* conversion */
size_t utf8_to_utf8esc(const utf8_t* utf8_str, size_t str_len, char* utf8esc_str, size_t* output_size)
{
    EASSERT(utf8esc_str);
    EASSERT(utf8_str);
    EASSERT(str_len >= 0);
    if(utf8esc_str == 0 || utf8_str == 0 || str_len < 0) return 0;

    /* pass to implementation */
    return utf8_to_utf8esc_impl(utf8_str, str_len, utf8esc_str, output_size);
}

size_t utf16_to_utf8esc(const utf16_t* utf16_str, size_t str_len, char* utf8esc_str, size_t* output_size)
{
    EASSERT(utf8esc_str);
    EASSERT(utf16_str);
    EASSERT(str_len >= 0);
    if(utf8esc_str == 0 || utf16_str == 0 || str_len < 0) return 0;

    /* pass to implementation */
    return utf16_to_utf8esc_impl(utf16_str, str_len, utf8esc_str, output_size);
}

/*----------------------------------------------------------------------*/


