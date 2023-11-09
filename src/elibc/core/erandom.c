/*
    Random generator helpers
*/

#include "../elibc_config.h"
#include "../stdlib/estdlib.h"

#include "eassert.h"
#include "erandom.h"

/*----------------------------------------------------------------------*/
/* data */
static const char ELIBC_ALPHABET_ARRAY[] = "abcdefghijklmnopqrstuvwxyz";
static const char ELIBC_ALNUM_ARRAY[] = "0123456789abcdefghijklmnopqrstuvwxyz";

/*----------------------------------------------------------------------*/
/* random characters */
char _erandom_char_alpha()
{
    return ELIBC_ALPHABET_ARRAY[erand() % (sizeof(ELIBC_ALPHABET_ARRAY) / sizeof(ELIBC_ALPHABET_ARRAY[0]) - 1)];
}

char _erandom_char_alnum()
{
    return ELIBC_ALNUM_ARRAY[erand() % (sizeof(ELIBC_ALNUM_ARRAY) / sizeof(ELIBC_ALNUM_ARRAY[0]) - 1)];
}

/*----------------------------------------------------------------------*/
/* random characters */
char erandalpha()
{
    /* get random character */
    return _erandom_char_alpha();
}

char erandalnum()
{
    /* get random character */
    return _erandom_char_alnum();
}

/*----------------------------------------------------------------------*/
/* random strings */
void erandalpha_str(char* str_out, size_t str_len)
{
    size_t idx;

    EASSERT(str_out);
    if(str_out == 0 || str_len == 0) return;

    /* fill string */
    for(idx = 0; idx < str_len; ++idx)
    {
        str_out[idx] = _erandom_char_alpha();
    }
}

void erandalnum_str(char* str_out, size_t str_len)
{
    size_t idx;

    EASSERT(str_out);
    if(str_out == 0 || str_len == 0) return;

    /* fill string */
    for(idx = 0; idx < str_len; ++idx)
    {
        str_out[idx] = _erandom_char_alnum();
    }
}

void erand_str(char* str_out, size_t str_len, const char* alphabet, size_t alphabet_len)
{
    size_t idx;

    EASSERT(str_out);
    EASSERT(alphabet);
    EASSERT(alphabet_len);
    if(str_out == 0 || str_len == 0 || alphabet == 0 || alphabet_len == 0) return;

    /* seed random generator */
    esrand((unsigned int)etime(0));

    /* fill string */
    for(idx = 0; idx < str_len; ++idx)
    {
        str_out[idx] = alphabet[erand() % alphabet_len];
    }
}

/*----------------------------------------------------------------------*/

