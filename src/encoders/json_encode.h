/*
    JSON encoder
*/

#ifndef _JSON_ENCODE_H_
#define _JSON_ENCODE_H_

/*----------------------------------------------------------------------*/

/* json encoder state */
typedef struct {

    estack_t                state_stack;
    ebuffer_t*              encode_buffer;

} json_encoder_t;

/*----------------------------------------------------------------------*/

/* encoder handle */
void    json_encode_init(json_encoder_t* json_encoder);
void    json_encode_reset(json_encoder_t* json_encoder);
void    json_encode_close(json_encoder_t* json_encoder);

/* encode json content */
int     json_encode_begin(json_encoder_t* json_encoder, ebuffer_t* encode_buffer);
int     json_encode_object_begin(json_encoder_t* json_encoder, const char* name);
int     json_encode_object_end(json_encoder_t* json_encoder);
int     json_encode_array_begin(json_encoder_t* json_encoder, const char* name);
int     json_encode_array_end(json_encoder_t* json_encoder);
int     json_encode_value_text(json_encoder_t* json_encoder, const http_param_t* http_param, http_encoding_t encoding);
int     json_encode_value_number(json_encoder_t* json_encoder, const http_param_t* http_param);
int     json_encode_end(json_encoder_t* json_encoder);

/*----------------------------------------------------------------------*/

#endif /* _JSON_ENCODE_H_ */

