/*
    UTF conversion and URL encoding helpers
*/

#ifndef _TEXT_FORMAT_H_
#define _TEXT_FORMAT_H_

/*----------------------------------------------------------------------*/

/* 
    NOTE: - output_size is in and out parameter, if set to non zero value it will
            limit maximum output size, on output contains size of the output used
          - output_size is optional, may be null
          - functions return amount of input buffer processed
*/

/*----------------------------------------------------------------------*/

/*
    URL encode/decode function
*/

/* required buffer sizes (if str_len is zero url_str is assumed to be null-terminated) */
size_t url_encoded_size(const char* url_str, size_t str_len);
size_t url_decoded_size(const char* url_str, size_t str_len);

/* encode and decode (if str_len is zero str_input is assumed to be null-terminated) */
size_t url_encode(const char* str_input, size_t str_len, char* str_output, size_t* output_size);
size_t url_decode(const char* str_input, size_t str_len, char* str_output, size_t* output_size);

/*----------------------------------------------------------------------*/

/*
    UTF conversion
*/

/* required buffer sizes */
size_t utf8_in_utf16(const utf8_t* utf8_str, size_t str_len);
size_t utf16_in_utf8(const utf16_t* utf16_str, size_t str_len);

/* conversion */
size_t utf8_to_utf16(const utf8_t* utf8_str, size_t str_len, utf16_t* utf16_str, size_t* output_size);
size_t utf16_to_utf8(const utf16_t* utf16_str, size_t str_len, utf8_t* utf8_str, size_t* output_size);

/* utf32 required buffer sizes */
size_t utf32_in_utf16(const utf32_t* utf32_str, size_t str_len);
size_t utf16_in_utf32(const utf16_t* utf16_str, size_t str_len);

/* utf32 conversion */
size_t utf16_to_utf32(const utf16_t* utf16_str, size_t str_len, utf32_t* utf32_str, size_t* output_size);
size_t utf32_to_utf16(const utf32_t* utf32_str, size_t str_len, utf16_t* utf16_str, size_t* output_size);

/* validate */
int is_valid_utf8(const utf8_t* utf8_str, size_t str_len);

/* utf8 character offset to byte offset */
size_t utf8_offset(const utf8_t* utf8_str, size_t str_len, size_t chars_offset);

/*----------------------------------------------------------------------*/

/*
    Combined functions to convert utf16 to and from utf8 url-encoded strings
*/

/* required buffer sizes */
size_t utf8url_in_utf16(const char* utf8url_str, size_t str_len);
size_t utf16_in_utf8url(const utf16_t* utf16_str, size_t str_len);

/* conversion */
size_t utf8url_to_utf16(const char* utf8url_str, size_t str_len, utf16_t* utf16_str, size_t* output_size);
size_t utf16_to_utf8url(const utf16_t* utf16_str, size_t str_len, char* utf8url_str, size_t* output_size);

/*----------------------------------------------------------------------*/

/*
    HEX string to number conversion
*/

/* conversion */
size_t hexstr_to_uint16(const char* hex_str, size_t str_len, euint16_t* num_out);
size_t hexstr_to_uint32(const char* hex_str, size_t str_len, euint32_t* num_out);

/* validate */
int is_valid_hexchar(unsigned char ch);

/*----------------------------------------------------------------------*/

/*
    Escaped utf8 string
*/

/* required buffer sizes */
size_t utf8_in_utf8esc(const utf8_t* utf8_str, size_t str_len);
size_t utf16_in_utf8esc(const utf16_t* utf16_str, size_t str_len);

/* conversion */
size_t utf8_to_utf8esc(const utf8_t* utf8url_str, size_t str_len, char* utf8esc_str, size_t* output_size);
size_t utf16_to_utf8esc(const utf16_t* utf16_str, size_t str_len, char* utf8esc_str, size_t* output_size);

/*----------------------------------------------------------------------*/

#endif /* _TEXT_FORMAT_H_ */



