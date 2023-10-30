/*
    HTTP authentication helpers
*/

#ifndef _HTTP_AUTH_H_
#define _HTTP_AUTH_H_

/*----------------------------------------------------------------------*/

/* compute basic http authorization header value */
int http_basicauth_length(const char* user, const char* password, size_t* length_out);
int http_basicauth_format(const char* user, const char* password, char* output);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_AUTH_H_ */

