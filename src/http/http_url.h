/*
    HTTP url parameters and helpers
*/

#ifndef _HTTP_URL_H_
#define _HTTP_URL_H_

/*----------------------------------------------------------------------*/

/* url parser result */
typedef struct 
{
    const char*     host;
    int             host_length;
    const char*     path;           /* path include the rest of url after host:port */
    const char*     arguments;      /* url arguments are added with ? separator */
    const char*     fragment;       /* fragment is added to url with # separator */
    unsigned short  secure;
    euint32_t       port;

} http_urlinfo_t;

/*----------------------------------------------------------------------*/
/*
    Arguments callback:
     - user data associated with parser (if set by user)
     - argument name
     - argument name length
     - argument value
     - argument value length
*/

/* parser callbacks */
typedef void (*http_urlinfo_callback_t)(void*, const char*, int, const char*, int);

/*----------------------------------------------------------------------*/

/*
    NOTE: if query parsing is needed pass callback function,
          http_urlinfo will be in parsed state at that time
*/

/* parse url (in place) */
int http_url_parse(const char* url, http_urlinfo_t* http_urlinfo,
                   void* callback_data, http_urlinfo_callback_t callback_func);

/* parse url arguments (in place) */
int http_url_parse_arguments(const char* arguments, void* callback_data, 
                             http_urlinfo_callback_t callback_func);

/*----------------------------------------------------------------------*/

#endif /* _HTTP_URL_H_ */



