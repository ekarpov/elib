/*
    Base64 text decoding and encoding
*/

#ifndef _TEXT_BASE64_H_
#define _TEXT_BASE64_H_

/*----------------------------------------------------------------------*/

/* required buffer sizes */
#define base64_encoded_size(input_size)     (4 * (((input_size) + 2) / 3))
#define base64_decoded_size(input_size)     (3 * ((input_size) / 4))

/* 
    NOTE: - output_size is in and out parameters, if set to non zero value it will
            limit maximum output size, on output contains size of the output used
          - output_size is optional, may be null
          - functions return amount of input buffer processed
*/

/* encode and decode */
size_t base64_encode(const euint8_t* buffer_input, size_t input_size, euint8_t* buffer_output, size_t* output_size);
size_t base64_decode(const euint8_t* buffer_input, size_t input_size, euint8_t* buffer_output, size_t* output_size);

/*----------------------------------------------------------------------*/

#endif /* _TEXT_BASE64_H_ */

