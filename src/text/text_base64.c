/*
    Base64 text decoding and encoding
*/

#include "../elib_config.h"

#include "text_base64.h"

/*----------------------------------------------------------------------*/
/* base64 encoding table */
static const euint8_t base64_encodng_table[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* base64 decoding table */
static const euint8_t base64_decoding_table[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,62,0,0,0,63,52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,0,
    26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/*----------------------------------------------------------------------*/

/* encode and decode */
size_t base64_encode(const euint8_t* buffer_input, size_t input_size, euint8_t* buffer_output, size_t* output_size)
{
    size_t in_idx, out_idx;
    size_t max_output = (output_size && *output_size > 0) ? *output_size : base64_encoded_size(input_size);

    /* check input */
    if(buffer_input == 0 || input_size <= 0) 
    {
        /* reset output size if needed */
        if(output_size) *output_size = 0;
        return 0;
    }

    /* validate output */
    EASSERT(buffer_output);
    if(buffer_output == 0) return 0;

    /* loop over input */
    for(in_idx = 0, out_idx = 0; in_idx + 3 <= input_size && out_idx + 4 <= max_output; in_idx += 3, out_idx += 4)
    {
        /* convert 3 input bytes to 4 output */
        euint32_t triple = (buffer_input[in_idx] << 0x10) +
                           (buffer_input[in_idx + 1] << 0x08) +
                            buffer_input[in_idx + 2];

        buffer_output[out_idx] =     base64_encodng_table[(triple >> 3 * 6) & 0x3F];
        buffer_output[out_idx + 1] = base64_encodng_table[(triple >> 2 * 6) & 0x3F];
        buffer_output[out_idx + 2] = base64_encodng_table[(triple >> 1 * 6) & 0x3F];
        buffer_output[out_idx + 3] = base64_encodng_table[(triple >> 0 * 6) & 0x3F];
    }

    /* check if there is anything left */
    if(in_idx < input_size && out_idx + 4 <= max_output)
    {
        /* append padding if needed */
        euint32_t triple = (buffer_input[in_idx] << 0x10);
        triple += (in_idx + 2 <= input_size) ? (buffer_input[in_idx + 1] << 0x08) : 0;
        triple += (in_idx + 3 <= input_size) ? buffer_input[in_idx + 2] : 0;

        buffer_output[out_idx] = base64_encodng_table[(triple >> 3 * 6) & 0x3F];
        buffer_output[out_idx + 1] = base64_encodng_table[(triple >> 2 * 6) & 0x3F];
        buffer_output[out_idx + 2] = (in_idx + 2 <= input_size) ? base64_encodng_table[(triple >> 1 * 6) & 0x3F] : '=';
        buffer_output[out_idx + 3] = (in_idx + 3 <= input_size) ? base64_encodng_table[(triple >> 0 * 6) & 0x3F] : '=';

        /* update counters */
        in_idx = input_size;
        out_idx += 4;
    }

    /* copy output size if needed */
    if(output_size) *output_size = out_idx;

    /* amount of bytes processed from input */
    return in_idx;
}

size_t base64_decode(const euint8_t* buffer_input, size_t input_size, euint8_t* buffer_output, size_t* output_size)
{
    size_t in_idx, out_idx;
    size_t max_output = (output_size && *output_size > 0) ? *output_size : base64_decoded_size(input_size);

    /* loop over input */
    for(in_idx = 0, out_idx = 0; in_idx + 4 <= input_size && out_idx + 3 <= max_output; in_idx += 4, out_idx += 3)
    {
        /* convert 4 input bytes to 3 output */
        euint32_t triple = (base64_decoding_table[buffer_input[in_idx]] << 3 * 6)
            + (base64_decoding_table[buffer_input[in_idx + 1]] << 2 * 6)
            + (base64_decoding_table[buffer_input[in_idx + 2]] << 1 * 6)
            + (base64_decoding_table[buffer_input[in_idx + 3]] << 0 * 6);

        buffer_output[out_idx] = (triple >> 2 * 8) & 0xFF;
        buffer_output[out_idx + 1] = (triple >> 1 * 8) & 0xFF;
        buffer_output[out_idx + 2] = (triple >> 0 * 8) & 0xFF;
    }

    /* copy output size if needed */
    if(output_size)
    {
        *output_size = out_idx;

        /* ignore padding if any */
        if(in_idx >= 4)
        {
            if(buffer_input[in_idx - 2] == '=') *output_size -= 1;
            if(buffer_input[in_idx - 1] == '=') *output_size -= 1;
        }
    }

    /* amount of bytes processed from input */
    return in_idx;
}

/*----------------------------------------------------------------------*/


