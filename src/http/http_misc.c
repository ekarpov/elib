/*
    HTTP misc helpers
*/

#include "../elib_config.h"

#include "http_misc.h"

/*----------------------------------------------------------------------*/
/* data */
static const char HTTP_CONTENT_BOUNDARY_ALPHABET[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

/*----------------------------------------------------------------------*/

/* format content boundary */
void http_format_content_boundary(char* boundary_out, size_t boundary_length)
{
    EASSERT(boundary_out);
    EASSERT(boundary_length);
    if(boundary_out == 0 || boundary_length == 0) return;

    /* generate random boundary */
    erand_str(boundary_out, 
              boundary_length, 
              HTTP_CONTENT_BOUNDARY_ALPHABET,
              sizeof(HTTP_CONTENT_BOUNDARY_ALPHABET) - 1);
}


/*----------------------------------------------------------------------*/


