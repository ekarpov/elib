/*
    XML encoder
*/

#ifndef _XML_ENCODE_H_
#define _XML_ENCODE_H_

/*----------------------------------------------------------------------*/

/* xml encoder state */
typedef struct {

    estack_t                state_stack;
    ebuffer_t*              encode_buffer;

} xml_encoder_t;

/*----------------------------------------------------------------------*/

/* encoder handle */
void    xml_encode_init(xml_encoder_t* xml_encoder);
void    xml_encode_reset(xml_encoder_t* xml_encoder);
void    xml_encode_close(xml_encoder_t* xml_encoder);

/* encode xml content */
int     xml_encode_begin(xml_encoder_t* xml_encoder, ebuffer_t* encode_buffer);
int     xml_encode_object_begin(xml_encoder_t* xml_encoder, const char* name);
int     xml_encode_object_end(xml_encoder_t* xml_encoder);
int     xml_encode_array_begin(xml_encoder_t* xml_encoder, const char* name);
int     xml_encode_array_end(xml_encoder_t* xml_encoder);
int     xml_encode_value_tag(xml_encoder_t* xml_encoder, const http_param_t* http_param, http_encoding_t encoding);
int     xml_encode_value_attribute(xml_encoder_t* xml_encoder, const http_param_t* http_param);
int     xml_encode_end(xml_encoder_t* xml_encoder);

/*----------------------------------------------------------------------*/

#endif /* _XML_ENCODE_H_ */

