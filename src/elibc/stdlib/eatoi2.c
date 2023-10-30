/*
 *  Fixed size string to integer conversion
*/

#include "../elibc_config.h"
#include "../core/eassert.h"

#include "estdlib.h"

/*----------------------------------------------------------------------*/

/*
    NOTE: standard function requires zero terminated string
*/
int eatoi2(const char* str_in, size_t str_length)
{
    int retval = 0;
    int sign = 1;
    size_t pos = 0;

    /* check input */
    EASSERT(str_in);
    if(str_in == 0) return 0;

    /* compute length if not set */
    if(str_length == 0 && str_in != 0) str_length = estrlen(str_in);

    /* skip spaces first */
    while(pos < str_length && eisspace(str_in[pos])) ++pos;

    /* convert sign if any */
    if(pos < str_length && str_in[pos] == '-')
    {
        sign = -1;
        ++pos;

    } else if(pos < str_length && str_in[pos] == '+')
    {
        ++pos;
    }

    /* convert */
    while(pos < str_length && eisdigit(str_in[pos]))
    {
        retval *= 10;
        retval += str_in[pos] - '0';

        ++pos;
    }

    return sign * retval;
}

int ewatoi2(const ewchar_t* str_in, size_t str_length)
{
    int retval = 0;
    int sign = 1;
    size_t pos = 0;

    /* check input */
    EASSERT(str_in);
    if(str_in == 0) return 0;

    /* compute length if not set */
    if(str_length == 0 && str_in != 0) str_length = ewcslen(str_in);

    /* skip spaces first */
    while(pos < str_length && eiswspace(str_in[pos])) ++pos;

    /* convert sign if any */
    if(pos < str_length && str_in[pos] == L'-')
    {
        sign = -1;
        ++pos;

    } else if(pos < str_length && str_in[pos] == L'+')
    {
        ++pos;
    }

    /* convert */
    while(pos < str_length && eiswdigit(str_in[pos]))
    {
        retval *= 10;
        retval += str_in[pos] - L'0';

        ++pos;
    }

    return sign * retval;
}

/*----------------------------------------------------------------------*/
