/*
    HTTP authentication helpers
*/

#include "../elib_config.h"

#include "../text/text_base64.h"
#include "http_auth.h"

/*----------------------------------------------------------------------*/

/* compute basic http authorization header value */
int http_basicauth_length(const char* user, const char* password, size_t* length_out)
{
    size_t slen;

    /* check input */
    EASSERT(user);
    EASSERT(password);
    EASSERT(length_out);
    if(user == 0 || password == 0 || length_out == 0) return ELIBC_ERROR_ARGUMENT;

    /* size to be encoded */
    slen = estrlen(user) + 1 + estrlen(password);

    /* Basic + space + base64(user + semicolon + password) */
    *length_out = 5 + 1 + base64_encoded_size(slen);

    return ELIBC_SUCCESS;
}

int http_basicauth_format(const char* user, const char* password, char* output)
{
    euint8_t tmp_buff[3];
    size_t slen, sencode, output_size, tmp_size;

    /* check input */
    EASSERT(user);
    EASSERT(password);
    EASSERT(output);
    if(user == 0 || password == 0 || output == 0) return ELIBC_ERROR_ARGUMENT;

    /* copy prefix */
    estrcpy(output, "Basic ");
    output += 6;

    /* encode user */
    slen = estrlen(user);

    /*
        NOTE: base64 encodes 3 input bytes into 4 output bytes
              in order to avoid temporary buffer we encode first
              part of user name that will not require padding and
              then combine rest with column and beginning of password
    */
    if(slen >= 3)
    {
        sencode = slen - (slen % 3);
        output_size = 0;
        user += base64_encode((const euint8_t*)user, sencode, (euint8_t*)output, &output_size);
        output += output_size;
    }

    /* fill temporary buffer */
    tmp_size = 0;
    while(*user != 0 && tmp_size < 3) tmp_buff[tmp_size++] = *(user++);
    tmp_buff[tmp_size++] = ':';
    while(*password != 0 && tmp_size < 3) tmp_buff[tmp_size++] = *(password++);

    EASSERT(*user == 0);

    /* encode temporary buffer */
    output_size = 0;
    base64_encode(tmp_buff, tmp_size, (euint8_t*)output, &output_size);
    output += output_size;

    /* encode password */
    if(*password != 0)
    {
        slen = estrlen(password);
        output_size = 0;
        base64_encode((const euint8_t*)password, slen, (euint8_t*)output, &output_size);
    }

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
