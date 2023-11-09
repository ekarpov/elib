/*
    Text string manipulation helpers
*/

#ifndef _TEXT_STRING_H_
#define _TEXT_STRING_H_

/*----------------------------------------------------------------------*/

/* text encodings */
typedef enum {

    TEXT_ENCODING_UTF8 = 0,           
    TEXT_ENCODING_UTF16

} text_encoding_t;

/*----------------------------------------------------------------------*/

/* unicode string */
typedef struct
{
    text_encoding_t     encoding;
    size_t              length;

    ebuffer_t           data;

} ustring_t;

/*----------------------------------------------------------------------*/

/*
    NOTE: length is assumed to be in bytes (sizeof(utf8_t)) for utf8 strings and in
          words (sizeof(utf16_t)) for utf16 strings
*/

/*----------------------------------------------------------------------*/

/* init */
void ustring_init(ustring_t* ustring);
void ustring_reset(ustring_t* ustring);
void ustring_free(ustring_t* ustring);

/* manipulations (NOTE: if length is zero text is assumed to be null terminated) */
int ustring_set_utf8(ustring_t* ustring, const utf8_t* text, size_t length);
int ustring_set_utf16(ustring_t* ustring, const utf16_t* text, size_t length);
int ustring_append_utf8(ustring_t* ustring, const utf8_t* text, size_t length);
int ustring_append_utf16(ustring_t* ustring, const utf16_t* text, size_t length);

/* format string */
int ustring_append_char(ustring_t* ustring, char ch);
int ustring_append_int(ustring_t* ustring, int number);
int ustring_append_uint(ustring_t* ustring, unsigned int number);
int ustring_append_int64(ustring_t* ustring, eint64_t number);
int ustring_append_uint64(ustring_t* ustring, euint64_t number);
int ustring_append_float(ustring_t* ustring, double number);

/* string format */
text_encoding_t ustring_encoding(ustring_t* ustring);

/* string length (in bytes for utf8 and in words for utf16) */
size_t ustring_length(ustring_t* ustring);

/* string data */
const utf8_t*   ustring_to_utf8(ustring_t* ustring);
const utf16_t*  ustring_to_utf16(ustring_t* ustring);

/*----------------------------------------------------------------------*/
/* text helpers (use inline functions instead of macro for type checking) */
inline int ustring_set_text(ustring_t* ustring, const char* text, size_t length)
{
    return ustring_set_utf8(ustring, (const utf8_t*)text, length);
}

inline int ustring_set_textw(ustring_t* ustring, const ewchar_t* text, size_t length)
{
    return ustring_set_utf16(ustring, (const utf16_t*)text, length);
}

inline int ustring_append_text(ustring_t* ustring, const char* text, size_t length)
{
    return ustring_append_utf8(ustring, (const utf8_t*)text, length);
}

inline int ustring_append_textw(ustring_t* ustring, const ewchar_t* text, size_t length)
{
    return ustring_append_utf16(ustring, (const utf16_t*)text, length);
}

/* string data as text */
inline const char* ustring_to_text(ustring_t* ustring)
{
    return (const char*)ustring_to_utf8(ustring);
}

inline const ewchar_t* ustring_to_textw(ustring_t* ustring)
{
    return (const ewchar_t*)ustring_to_utf16(ustring);
}

/*----------------------------------------------------------------------*/

#endif /* _TEXT_STRING_H_ */

