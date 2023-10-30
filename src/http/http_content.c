/*
    HTTP content
*/

#include "../elib_config.h"

#include "http_content.h"

/*----------------------------------------------------------------------*/

/* reset content */
void http_content_reset(http_content_t* http_content)
{
    /* reset content */
    EASSERT(http_content);
    if(http_content)
    {
        ememset(http_content, 0, sizeof(http_content_t));
    }
}

/* set content (NOTE: data is not copied so all pointers must be valid until content is no longer needed) */
int http_content_set(http_content_t* http_content, http_content_t* http_content_from)
{
    /* check input */
    EASSERT(http_content);
    EASSERT(http_content_from);
    if(http_content == 0 || http_content_from == 0) return ELIBC_ERROR_ARGUMENT;

    /* copy content */
    ememcpy(http_content, http_content_from, sizeof(http_content_t));

    return ELIBC_SUCCESS;
}

int http_content_set_iface(http_content_t* http_content, http_content_iface_t* http_content_iface)
{
    /* check input */
    EASSERT(http_content);
    EASSERT(http_content_iface);
    if(http_content == 0 || http_content_iface == 0) return ELIBC_ERROR_ARGUMENT;

    /* validate interface */
    if(http_content_iface->http_content_type == 0 ||
       http_content_iface->http_content_size == 0 ||
       http_content_iface->http_content_read == 0)
    {
        EASSERT1(0, "http_content: content interface is not valid");
        return ELIBC_ERROR_ARGUMENT;
    }

    /* reset previous content */
    http_content_reset(http_content);

    /* copy content interface */
    http_content->content_iface = *http_content_iface;

    return ELIBC_SUCCESS;
}

int http_content_set_data(http_content_t* http_content, const char* content_type, const char* content_data, euint64_t content_size)
{
    /* check input */
    EASSERT(http_content);
    if(http_content == 0) return ELIBC_ERROR_ARGUMENT;

    /* check if data is set */
    if(content_size != 0 && content_data == 0)
    {
        EASSERT1(0, "http_content: content data is not valid");
        return ELIBC_ERROR_ARGUMENT;
    }

    /* reset previous content */
    http_content_reset(http_content);

    /* copy content reference */
    http_content->content_type = content_type;
    http_content->content_data = content_data;
    http_content->content_size = content_size;

    return ELIBC_SUCCESS;
}

/* content properties */
const char* http_content_type(const http_content_t* http_content)
{
    /* check input */
    EASSERT(http_content);
    if(http_content == 0) return 0;

    /* check if content interface is set */
    if(http_content->content_iface.http_content_type)
    {
        /* pass to content interface */
        return http_content->content_iface.http_content_type(http_content->content_iface.user_data);
    
    } else
    {
        /* use provided value if set */
        return http_content->content_type;
    }
}

euint64_t   http_content_size(const http_content_t* http_content)
{
    /* check input */
    EASSERT(http_content);
    if(http_content == 0) return 0;

    /* check if content interface is set */
    if(http_content->content_iface.http_content_type)
    {
        /* pass to content interface */
        return http_content->content_iface.http_content_size(http_content->content_iface.user_data);
    
    } else
    {
        /* use provided value if set */
        return http_content->content_size;
    }
}

/* read content */
int http_content_read(http_content_t* http_content, char* buffer, size_t buffer_size, size_t* buffer_used)
{
    /* check if content interface is set */
    if(http_content->content_iface.http_content_read)
    {
        /* pass to content interface */
        return http_content->content_iface.http_content_read(http_content->content_iface.user_data, buffer, buffer_size, buffer_used);
    
    } else
    {
        /* data left */
        size_t data_left = (size_t)(http_content->content_size - http_content->content_offset);

        /* read size */
        size_t read_size = (data_left <= buffer_size) ? data_left : buffer_size;

        /* read from content reference */
        if(read_size > 0)
        {
            /* copy content */
            ememcpy(buffer, http_content->content_data + http_content->content_offset, read_size);

            /* update offset */
            http_content->content_offset += read_size;

            /* buffer used */
            if(buffer_used) *buffer_used = read_size;
        }

        return ELIBC_SUCCESS;
    }
}

/*----------------------------------------------------------------------*/
