/*
    Date and time conversion and encoding helpers
*/

#ifndef _TIME_FORMAT_H_
#define _TIME_FORMAT_H_

/*----------------------------------------------------------------------*/

/*
    Unix time:      https://en.wikipedia.org/wiki/Unix_time
    ISO-8601:       https://en.wikipedia.org/wiki/ISO_8601
    RFC 1123:       https://tools.ietf.org/html/rfc1123 (adopted from RFC-822 Section 5 https://tools.ietf.org/html/rfc822#section-5)
*/

/*----------------------------------------------------------------------*/

/* date and time format */
typedef enum {

    DATETIME_FORMAT_UNIXTIME = 0,       /* seconds since 1 of January 1970 */    
    DATETIME_FORMAT_ISO8601,            /* only combined date and time supported */
    DATETIME_FORMAT_RFC1123,            /* military timezones not supported */
    DATETIME_FORMAT_TWITTER             /* Twitter UTC date format */

} time_format_t;

/*----------------------------------------------------------------------*/

/* date and time */
typedef struct
{
    /* date */
    unsigned short  year;
    unsigned short  month;          /* 0-11, months since January */
    unsigned short  day;

    /* time */
    unsigned short  hours;
    unsigned short  minutes;
    unsigned short  seconds;

    /* time zone offset */
    signed short    offset;         /* in minutes */

    /* day of week (optional) */
    unsigned short  weekday;        /* 0-6, days since Sunday */

} datetime_t;

/*----------------------------------------------------------------------*/

/* 
    NOTE: - output_size is in and out parameter, if set to non zero value it will
            limit maximum output size, on output contains size of the output used
          - output_size is optional, may be null
          - functions return ELIBC_SUCCEESS if successful or error code otherwise
*/

/* parse and format */
int datetime_parse(time_format_t format, const char* str_input, size_t str_len, datetime_t* datetime);
int datetime_format(time_format_t format, const datetime_t* datetime, char* str_output, size_t* output_size);

/*----------------------------------------------------------------------*/

/* convert times */
int datetime_to_unixtime(const datetime_t* datetime, etime_t* unixtime);
int unixtime_to_datetime(const etime_t unixtime, datetime_t* datetime);

/*----------------------------------------------------------------------*/
/* names (English) */
const char* datetime_weekday(int weekday);
const char* datetime_month_name(int month);

/*----------------------------------------------------------------------*/

#endif /* _TIME_FORMAT_H_ */

