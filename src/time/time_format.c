/*
    Date and time conversion and encoding helpers
*/

#include "../elib_config.h"

#include "time_format.h"

/*----------------------------------------------------------------------*/
/* parser constants */

/* parser state */
typedef enum {

    time_parser_state_syntax_error,
    time_parser_state_year,
    time_parser_state_month,
    time_parser_state_day,
    time_parser_state_hours,
    time_parser_state_minutes,
    time_parser_state_seconds,
    time_parser_state_mseconds,
    time_parser_state_offset,
    time_parser_state_offset_name,
    time_parser_state_weekday,
    time_parser_state_end

} time_parser_state_t;

/* parser characters */
typedef enum {

    time_parser_char_unknown,
    time_parser_char_space,
    time_parser_char_digit,
    time_parser_char_plus,
    time_parser_char_minus,
    time_parser_char_column,
    time_parser_char_dot,
    time_parser_char_T,
    time_parser_char_Z

} time_parser_char_t;

/* week day name */
static const char* _datetime_weekdays[] = 
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

/* month name */
static const char* _datetime_month_names[] = 
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

#define DATETIME_WEEKDAY_SIZE           sizeof(_datetime_weekdays) / sizeof(_datetime_weekdays[0])
#define DATETIME_MONTH_NAME_SIZE        sizeof(_datetime_month_names) / sizeof(_datetime_month_names[0])

#define DATETIME_WEEKDAY_LENGTH         3
#define DATETIME_MONTH_NAME_LENGTH      3
#define DATETIME_OFFSET_NAME_LENGTH1    2
#define DATETIME_OFFSET_NAME_LENGTH2    3
#define DATETIME_OFFSET_DIGITS_LENGTH   4

/*----------------------------------------------------------------------*/
/* parser helpers */

ELIBC_FORCE_INLINE time_parser_char_t _datetime_parse_char(const char ch)
{
    /* parse character */
    if(eisspace(ch))
        return time_parser_char_space;
    else if(eisdigit(ch))
        return time_parser_char_digit;
    else if(ch == '+')
        return time_parser_char_plus;
    else if(ch == '-')
        return time_parser_char_minus;
    else if(ch == ':')
        return time_parser_char_column;
    else if(ch == '.')
        return time_parser_char_dot;
    else if(ch == 'Z' || ch == 'z')
        return time_parser_char_Z;
    else if(ch == 'T' || ch == 't')
        return time_parser_char_T;

    return time_parser_char_unknown;
}

/* parse weekdays */
ELIBC_FORCE_INLINE int _datetime_parse_weekday(const char* weekday, size_t name_length)
{
    int idx;

    /* loop over all days */
    for(idx = 0; idx < DATETIME_WEEKDAY_SIZE; ++idx)
    {
        if(estrnicmp2(weekday, name_length, _datetime_weekdays[idx], 0) == 0) return idx;
    }

    /* unknown weekday */
    return -1;
}

/* parse month names */
ELIBC_FORCE_INLINE int _datetime_parse_month(const char* month_name, size_t name_length)
{
    int idx;

    /* loop over all names */
    for(idx = 0; idx < DATETIME_MONTH_NAME_SIZE; ++idx)
    {
        if(estrnicmp2(month_name, name_length, _datetime_month_names[idx], 0) == 0) return idx;
    }

    /* unknown month name */
    return -1;
}

/* parse offset names (RFC 822) */
ELIBC_FORCE_INLINE int _datetime_parse_offset_rfc822(const char* offset, size_t offset_length)
{
    /* match known zones */
    if(estrnicmp2(offset, offset_length, "UT", 0) == 0)         return 0; /* Universal Time */
    else if(estrnicmp2(offset, offset_length, "GMT", 0) == 0)   return 0; /* Greenwich Mean Time */
    else if(estrnicmp2(offset, offset_length, "EST", 0) == 0)   return -5 * 60; /* Eastern Time */
    else if(estrnicmp2(offset, offset_length, "EDT", 0) == 0)   return -4 * 60; /* Eastern Daylight Time */
    else if(estrnicmp2(offset, offset_length, "CST", 0) == 0)   return -6 * 60; /* Central Time */
    else if(estrnicmp2(offset, offset_length, "CDT", 0) == 0)   return -5 * 60; /* Central Daylight Time */
    else if(estrnicmp2(offset, offset_length, "MST", 0) == 0)   return -7 * 60; /* Mountain Time */
    else if(estrnicmp2(offset, offset_length, "MDT", 0) == 0)   return -6 * 60; /* Mountain Daylight Time */
    else if(estrnicmp2(offset, offset_length, "PST", 0) == 0)   return -8 * 60; /* Pacific Time */
    else if(estrnicmp2(offset, offset_length, "PDT", 0) == 0)   return -7 * 60; /* Pacific Daylight Time */

    /* unknown timezone */
    return -1;
}

int _datetime_parser_validate(const datetime_t* datetime)
{
    int err = ELIBC_SUCCESS;

    /* validate */
    if(datetime->year > 9999)
    {
        ETRACE("datetime_parse: invalid year value");
        err = ELIBC_ERROR_PARSER_INVALID_INPUT;
    }
    if(datetime->month > 11)
    {
        ETRACE("datetime_parse: invalid month value");
        err = ELIBC_ERROR_PARSER_INVALID_INPUT;
    }
    if(datetime->day > 31)
    {
        ETRACE("datetime_parse: invalid day value");
        err = ELIBC_ERROR_PARSER_INVALID_INPUT;
    }
    if(datetime->hours > 23)
    {
        ETRACE("datetime_parse: invalid hours value");
        err = ELIBC_ERROR_PARSER_INVALID_INPUT;
    }
    if(datetime->minutes > 59)
    {
        ETRACE("datetime_parse: invalid minutes value");
        err = ELIBC_ERROR_PARSER_INVALID_INPUT;
    }
    if(datetime->seconds > 59)
    {
        ETRACE("datetime_parse: invalid seconds value");
        err = ELIBC_ERROR_PARSER_INVALID_INPUT;
    }
    if(datetime->offset < -12*60 || datetime->offset > 12*60)
    {
        ETRACE("datetime_parse: invalid zone offset value");
        err = ELIBC_ERROR_PARSER_INVALID_INPUT;
    }

    return err;
}

/*----------------------------------------------------------------------*/
/* unixtime */
/*----------------------------------------------------------------------*/

int _datetime_parse_unixtime(const char* str_input, size_t str_len, datetime_t* datetime)
{
    char parse_buffer[32]; 
    size_t buffer_pos = 0;
    size_t str_idx;

    etime_t unixtime = 0;

    /* copy to internal buffer */
    for(str_idx = 0; str_idx < str_len && str_idx + 1 < sizeof(parse_buffer); ++str_idx)
    {
        /* skip spaces if any */
        if(eisspace(str_input[str_idx])) continue;

        /* stop if not a digit */
        if(!eisdigit(str_input[str_idx])) break;

        /* copy to buffer */
        parse_buffer[buffer_pos] = str_input[str_idx];
        buffer_pos++;
    }

    /* convert to integer */
    unixtime = eatoi2(parse_buffer, buffer_pos);

    /* convert unixtime to datetime */
    return unixtime_to_datetime(unixtime, datetime);
}

int _datetime_format_unixtime(const datetime_t* datetime, char* str_output, size_t* output_size)
{
    char convert_buffer[32];
    etime_t unixtime = 0;
    size_t str_len;
    int err;

    /* convert to unixtime first */
    err = datetime_to_unixtime(datetime, &unixtime);
    if(err != ELIBC_SUCCESS) return err;

    /* convert to string */
    esprintf(convert_buffer, "%lld", unixtime);

    /* get length */
    str_len = estrlen(convert_buffer);

    /* double check */
    EASSERT(str_len + 1 < sizeof(convert_buffer));

    /* check if maxsize is in use */
    if(output_size && *output_size > 0 && str_len + 1 > *output_size)
    {
        /* maximum size, more space needed */
        *output_size = str_len + 1;

        return ELIBC_ERROR_ARGUMENT;
    }

    /* copy */
    estrcpy(str_output, convert_buffer);

    /* output size */
    if(output_size) *output_size = str_len + 1;

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* ISO-8601 */
/*----------------------------------------------------------------------*/

/*
    Examples: 2016-04-29T18:31:37+00:00
              2016-04-29T18:31:37Z
              20160429T183137Z
              2015-11-22T13:33:04.000Z
*/

/* parse */
int _datetime_parse_iso8601(const char* str_input, size_t str_len, datetime_t* datetime)
{
    char parse_buffer[16];
    size_t buffer_pos = 0;
    size_t str_idx;

    time_parser_state_t parse_state = time_parser_state_year;
    time_parser_char_t  parse_char;

    signed short offset_sign = 1;

    /* loop over all characters */
    for(str_idx = 0; str_idx < str_len && parse_state != time_parser_state_end; ++str_idx)
    {
        /* parse character */
        parse_char = _datetime_parse_char(str_input[str_idx]);

        /* skip leading spaces if any */
        if(buffer_pos == 0 && parse_char == time_parser_char_space) continue;

        /* double check temp buffer */
        EASSERT(buffer_pos < sizeof(parse_buffer));
        if(buffer_pos >= sizeof(parse_buffer)) return ELIBC_ERROR_INTERNAL;

        /* copy character to buffer */
        parse_buffer[buffer_pos] = str_input[str_idx];
        buffer_pos++;

        /* ignore certain characters */
        if((parse_char == time_parser_char_minus && parse_state != time_parser_state_offset) ||
           parse_char == time_parser_char_column)
        {
            /* ignore character */
            buffer_pos--;

            continue;
        }

        /* check state */
        switch(parse_state)
        {
        case time_parser_state_year:
            /* check character type */
            if(parse_char == time_parser_char_digit)
            {
                /* check if ready */
                if(buffer_pos == 4)
                {
                    /* parse year */
                    datetime->year = eatoi2(parse_buffer, buffer_pos);

                    /* next state */
                    buffer_pos = 0;
                    parse_state = time_parser_state_month;
                }

            } else
            {
                /* unexpected character */
                parse_state = time_parser_state_syntax_error;
            }

            break;

        case time_parser_state_month:
            /* check character type */
            if(parse_char == time_parser_char_digit)
            {
                /* check if ready */
                if(buffer_pos == 2)
                {
                    /* parse month */
                    datetime->month = eatoi2(parse_buffer, buffer_pos);

                    /* 
                        NOTE: ISO 8601 months are from 1-12 but we need 0-11
                    */
                    datetime->month--;

                    /* next state */
                    buffer_pos = 0;
                    parse_state = time_parser_state_day;
                }

            } else
            {
                /* unexpected character */
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_day:
            /* check character type */
            if(parse_char == time_parser_char_digit)
            {
                /* check if ready */
                if(buffer_pos == 2)
                {
                    /* parse day */
                    datetime->day = eatoi2(parse_buffer, buffer_pos);

                    /* next state */
                    buffer_pos = 0;
                    parse_state = time_parser_state_hours;
                }

            } else
            {
                /* unexpected character */
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_hours:
            /* must start with T */
            if(buffer_pos == 1)
            {
                /* check character */
                if(parse_char != time_parser_char_T)
                    parse_state = time_parser_state_syntax_error;

            } else if(parse_char == time_parser_char_digit)
            {
                /* check if ready */
                if(buffer_pos == 3)
                {
                    parse_buffer[buffer_pos] = 0;

                    /* parse hours */
                    datetime->hours = eatoi(parse_buffer + 1);

                    /* next state */
                    buffer_pos = 0;
                    parse_state = time_parser_state_minutes;
                }
            } else
            {
                /* unexpected character */
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_minutes:
            /* check character type */
            if(parse_char == time_parser_char_digit)
            {
                /* check if ready */
                if(buffer_pos == 2)
                {
                    /* parse minutes */
                    datetime->minutes = eatoi2(parse_buffer, buffer_pos);

                    /* next state */
                    buffer_pos = 0;
                    parse_state = time_parser_state_seconds;
                }

            } else
            {
                /* unexpected character */
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_seconds:
            /* check character type */
            if(parse_char == time_parser_char_digit)
            {
                /* check if ready */
                if(buffer_pos == 2)
                {
                    /* parse seconds */
                    datetime->seconds = eatoi2(parse_buffer, buffer_pos);

                    /* next state */
                    buffer_pos = 0;
                    parse_state = time_parser_state_offset;
                }

            } else
            {
                /* unexpected character */
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_mseconds:
            /* check character type */
            if(parse_char == time_parser_char_digit)
            {
                /* check if ready */
                if(buffer_pos == 3)
                {
                    /* NOTE: ignore milliseconds */

                    /* next state */
                    buffer_pos = 0;
                    parse_state = time_parser_state_offset;
                }
            } else
            {
                /* unexpected character */
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_offset:
            /* check character type */
            if(parse_char == time_parser_char_dot)
            {
                /* must be first */
                if(buffer_pos == 1)
                {
                    /* next state */
                    buffer_pos = 0;
                    parse_state = time_parser_state_mseconds;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }

            } else if(parse_char == time_parser_char_Z)
            {
                /* zero offset */
                datetime->offset = 0;

                /* parser ready but there must be only single character */
                parse_state = (buffer_pos == 1) ? time_parser_state_end : time_parser_state_syntax_error;

            } else if(parse_char == time_parser_char_plus)
            {
                /* must be first */
                if(buffer_pos != 1) parse_state = time_parser_state_syntax_error;

                /* positive offset */
                offset_sign = 1;

                /* ignore */
                buffer_pos--;

            } else if(parse_char == time_parser_char_minus)
            {
                /* must be first */
                if(buffer_pos != 1) parse_state = time_parser_state_syntax_error;

                /* negative offset */
                offset_sign = -1;

                /* ignore */
                buffer_pos--;

            } else if(parse_char == time_parser_char_digit)
            {
                /* parse offset */
                if(buffer_pos == 2)
                {
                    /* parse hours */
                    datetime->offset = 60 * offset_sign * eatoi2(parse_buffer, buffer_pos);

                } else if(buffer_pos == 4)
                {
                    parse_buffer[buffer_pos] = 0;

                    /* parse minutes */
                    datetime->offset += offset_sign * eatoi(parse_buffer + 2);

                    /* parser ready */
                    parse_state = time_parser_state_end;
                }

            } else
            {
                /* unexpected character */
                parse_state = time_parser_state_syntax_error;
            }

            break;
        }

        /* report error if any */
        if(parse_state == time_parser_state_syntax_error)
        {
            ETRACE2("datetime_parse: syntax error at %d, unexpected character \'%c\'", str_idx, str_input[str_idx]);
            return ELIBC_ERROR_PARSER_INVALID_INPUT;
        }

    }

    return _datetime_parser_validate(datetime);
}

/* format */
int _datetime_format_iso8601(const datetime_t* datetime, char* str_output, size_t* output_size)
{
    char offset_buffer[16];
    int offset, offset_hours, offset_minutes;

    /* output must fit 10 + 1 + 8 + 6 = 25 characters plus zero terminate */
    if(output_size)
    {
        /* check if we have enough space */
        if(*output_size != 0 && *output_size < 26) 
        {
            /* set required size */
            *output_size = 26;

            return ELIBC_ERROR_ARGUMENT;
        }

        /* output size will be always the same */
        *output_size = 26;
    }

    /* remove sign */
    offset = (datetime->offset > 0) ? datetime->offset : -datetime->offset;

    /* offset hours */
    offset_hours = 0;
    while((offset_hours + 1) * 60 <= offset) offset_hours++;

    /* offset minutes */
    offset_minutes = offset - offset_hours * 60;

    /* format offset */
    if(datetime->offset >= 0)
        esprintf(offset_buffer, "+%.2d:%.2d", offset_hours, offset_minutes);
    else
        esprintf(offset_buffer, "-%.2d:%.2d", offset_hours, offset_minutes);

    /* format */
    esprintf(str_output, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d%.6s", datetime->year, (datetime->month + 1), datetime->day, 
        datetime->hours, datetime->minutes, datetime->seconds, offset_buffer);

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* RFC 1123 */
/*----------------------------------------------------------------------*/

/*
    Examples: Thu, 01 Dec 1994 16:00:00 GMT
              
*/

int _datetime_parse_rfc1123(const char* str_input, size_t str_len, datetime_t* datetime)
{
    char parse_buffer[16];
    size_t buffer_pos = 0;
    size_t str_idx;

    time_parser_state_t parse_state = time_parser_state_weekday;
    time_parser_char_t  parse_char;

    signed short offset_sign = 1;

    /* loop over all characters */
    for(str_idx = 0; str_idx < str_len && parse_state != time_parser_state_end; ++str_idx)
    {
        /* parse character */
        parse_char = _datetime_parse_char(str_input[str_idx]);

        /* skip leading spaces if any */
        if(buffer_pos == 0 && parse_char == time_parser_char_space) continue;

        /* double check temp buffer */
        EASSERT(buffer_pos < sizeof(parse_buffer));
        if(buffer_pos >= sizeof(parse_buffer)) return ELIBC_ERROR_INTERNAL;

        /* copy character to buffer */
        parse_buffer[buffer_pos] = str_input[str_idx];
        buffer_pos++;

        /* check state */
        switch(parse_state)
        {
        case time_parser_state_weekday:
            if(buffer_pos > DATETIME_WEEKDAY_LENGTH)
            {
                /* parse week day */
                datetime->weekday = _datetime_parse_weekday(parse_buffer, DATETIME_WEEKDAY_LENGTH);
                
                /* week day must end with comma */
                if(datetime->weekday != -1)
                {
                    /* skip spaces if any */
                    while(str_idx < str_len && eisspace(str_input[str_idx])) ++str_idx;

                    /* check if we have comma */
                    if(str_idx < str_len && str_input[str_idx] == ',')
                    {
                        /* jump to next state */
                        parse_state = time_parser_state_day;
                        buffer_pos = 0;

                    } else
                    {
                        /* syntax error */
                        parse_state = time_parser_state_syntax_error;
                    }

                } else
                {
                    /* if weekday not set, ignore input and re-start parser */
                    str_idx = -1; /* extra 1 for loop end */
                    buffer_pos = 0;
               }

            } else if(!eisalpha(str_input[str_idx]))
            {
                /* assume week day not set */
                parse_state = time_parser_state_day;
                str_idx = -1; /* extra 1 for loop end */
                buffer_pos = 0;
            }
            break;

        case time_parser_state_day:
            if(buffer_pos > 2)
            {
                /* convert */
                datetime->day = eatoi2(parse_buffer, buffer_pos);

                /* space must follow */
                if(parse_char == time_parser_char_space)
                {
                    /* next state */
                    parse_state = time_parser_state_month;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }

            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_month:
            if(buffer_pos > DATETIME_MONTH_NAME_LENGTH)
            {
                /* parse month */
                datetime->month = _datetime_parse_month(parse_buffer, DATETIME_MONTH_NAME_LENGTH);

                /* validate */
                if(datetime->month == -1)
                {
                    ETRACE("datetime_parse: syntax error, unknown month name");
                    return ELIBC_ERROR_PARSER_INVALID_INPUT;
                }

                /* space must follow */
                if(parse_char == time_parser_char_space)
                {
                    /* next state */
                    parse_state = time_parser_state_year;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }

            } else if(!eisalpha(str_input[str_idx]))
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_year:
            if(buffer_pos > 4 || (buffer_pos > 2 && parse_char == time_parser_char_space))
            {
                /* convert */
                datetime->year = eatoi2(parse_buffer, buffer_pos);

                /* space must follow */
                if(parse_char == time_parser_char_space)
                {
                    /* next state */
                    parse_state = time_parser_state_hours;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }

            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_hours:
            if(buffer_pos > 2)
            {
                /* convert */
                datetime->hours = eatoi2(parse_buffer, buffer_pos);

                /* column must follow */
                if(parse_char == time_parser_char_column)
                {
                    /* next state */
                    parse_state = time_parser_state_minutes;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }


            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_minutes:
            if(buffer_pos > 2)
            {
                /* convert */
                datetime->minutes = eatoi2(parse_buffer, buffer_pos);

                /* reset buffer */
                buffer_pos = 0;

                /* if column follows next state is seconds, if space then offset */
                if(parse_char == time_parser_char_column)
                {
                    /* next state */
                    parse_state = time_parser_state_seconds;

                } else if(parse_char == time_parser_char_space)
                {
                    /* next state */
                    parse_state = time_parser_state_offset_name;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }

            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_seconds:
            if(buffer_pos > 2)
            {
                /* convert */
                datetime->seconds = eatoi2(parse_buffer, buffer_pos);

                /* space must follow */
                if(parse_char == time_parser_char_space)
                {
                    /* next state */
                    parse_state = time_parser_state_offset_name;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }


            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_offset_name:
            if(buffer_pos == DATETIME_OFFSET_NAME_LENGTH1)
            {
                /* parse offset */
                datetime->offset = _datetime_parse_offset_rfc822(parse_buffer, DATETIME_OFFSET_NAME_LENGTH1);

                if(datetime->offset != -1)
                {
                    /* parse ready */
                    parse_state = time_parser_state_end;
                }

            } else if(buffer_pos == DATETIME_OFFSET_NAME_LENGTH2)
            {
                /* parse offset */
                datetime->offset = _datetime_parse_offset_rfc822(parse_buffer, DATETIME_OFFSET_NAME_LENGTH2);

                if(datetime->offset != -1)
                {
                    /* parse ready */
                    parse_state = time_parser_state_end;

                } else
                {
                    parse_buffer[buffer_pos] = 0;

                    /* unknown time zone offset */
                    ETRACE1("datetime_parse: invalid time zone offset name %s", parse_buffer);
                    return ELIBC_ERROR_PARSER_INVALID_INPUT;
                }
                
            } else if(!eisalpha(str_input[str_idx]))
            {
                /* assume offset is in digits format */
                parse_state = time_parser_state_offset;
                str_idx -= (buffer_pos + 1); /* extra 1 for loop end */
                buffer_pos = 0;
            }
            break;

        case time_parser_state_offset:
            if(parse_char == time_parser_char_plus)
            {
                /* must be first */
                if(buffer_pos != 1) parse_state = time_parser_state_syntax_error;

                /* positive offset */
                offset_sign = 1;

                /* ignore */
                buffer_pos--;

            } else if(parse_char == time_parser_char_minus)
            {
                /* must be first */
                if(buffer_pos != 1) parse_state = time_parser_state_syntax_error;

                /* negative offset */
                offset_sign = -1;

                /* ignore */
                buffer_pos--;

            } else if(buffer_pos == DATETIME_OFFSET_DIGITS_LENGTH)
            {
                /* NOTE: offset is in format HHMM */

                /* parse offset */
                datetime->offset = offset_sign * 60 * eatoi2(parse_buffer, 2) + eatoi2(parse_buffer + 2, 2);

                /* parser ready */
                parse_state = time_parser_state_end;

            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;
        }

        /* report error if any */
        if(parse_state == time_parser_state_syntax_error)
        {
            ETRACE2("datetime_parse: syntax error at %d, unexpected character \'%c\'", str_idx, str_input[str_idx]);
            return ELIBC_ERROR_PARSER_INVALID_INPUT;
        }
    }

    return _datetime_parser_validate(datetime);
}

int _datetime_format_rfc1123(const datetime_t* datetime, char* str_output, size_t* output_size)
{
    char offset_buffer[16];
    int offset, offset_hours, offset_minutes;

    /* output must fit 2 + 1 + 3 + 1 + 4 + 1 + 8 + 1 + 5 = 26 characters plus zero terminate */
    if(output_size)
    {
        /* check if we have enough space */
        if(*output_size != 0 && *output_size < 27) 
        {
            /* set required size */
            *output_size = 27;

            return ELIBC_ERROR_ARGUMENT;
        }

        /* output size will be always the same */
        *output_size = 27;
    }

    /* remove sign */
    offset = (datetime->offset > 0) ? datetime->offset : -datetime->offset;

    /* offset hours */
    offset_hours = 0;
    while((offset_hours + 1) * 60 <= offset) offset_hours++;

    /* offset minutes */
    offset_minutes = offset - offset_hours * 60;

    /* format offset */
    if(datetime->offset >= 0)
        esprintf(offset_buffer, "+%.2d%.2d", offset_hours, offset_minutes);
    else
        esprintf(offset_buffer, "-%.2d%.2d", offset_hours, offset_minutes);

    /* format */
    esprintf(str_output, "%.2d %.3s %.4d %.2d:%.2d:%.2d %.5s", datetime->day, datetime_month_name(datetime->month), datetime->year, 
        datetime->hours, datetime->minutes, datetime->seconds, offset_buffer);

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* Twitter */
/*----------------------------------------------------------------------*/

/*
    Examples: Wed Aug 27 13:08:45 +0000 2008
              Thu Apr 06 15:28:43 +0000 2017
              
*/

int _datetime_parse_twitter(const char* str_input, size_t str_len, datetime_t* datetime)
{
    char parse_buffer[16];
    size_t buffer_pos = 0;
    size_t str_idx;

    time_parser_state_t parse_state = time_parser_state_weekday;
    time_parser_char_t  parse_char;

    signed short offset_sign = 1;

    /* loop over all characters */
    for(str_idx = 0; str_idx < str_len && parse_state != time_parser_state_end; ++str_idx)
    {
        /* parse character */
        parse_char = _datetime_parse_char(str_input[str_idx]);

        /* skip leading spaces if any */
        if(buffer_pos == 0 && parse_char == time_parser_char_space) continue;

        /* double check temp buffer */
        EASSERT(buffer_pos < sizeof(parse_buffer));
        if(buffer_pos >= sizeof(parse_buffer)) return ELIBC_ERROR_INTERNAL;

        /* copy character to buffer */
        parse_buffer[buffer_pos] = str_input[str_idx];
        buffer_pos++;

        /* check state */
        switch(parse_state)
        {
        case time_parser_state_weekday:
            if(buffer_pos > DATETIME_WEEKDAY_LENGTH)
            {
                /* parse week day */
                datetime->weekday = _datetime_parse_weekday(parse_buffer, DATETIME_WEEKDAY_LENGTH);
                
                /* validate */
                if(datetime->weekday == -1)
                {
                    ETRACE("datetime_parse: syntax error, unknown weekday");
                    return ELIBC_ERROR_PARSER_INVALID_INPUT;
                }

                /* space must follow */
                if(parse_char == time_parser_char_space)
                {
                    /* next state */
                    parse_state = time_parser_state_month;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }

            } else if(!eisalpha(str_input[str_idx]))
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_month:
            if(buffer_pos > DATETIME_MONTH_NAME_LENGTH)
            {
                /* parse month */
                datetime->month = _datetime_parse_month(parse_buffer, DATETIME_MONTH_NAME_LENGTH);

                /* validate */
                if(datetime->month == -1)
                {
                    ETRACE("datetime_parse: syntax error, unknown month name");
                    return ELIBC_ERROR_PARSER_INVALID_INPUT;
                }

                /* space must follow */
                if(parse_char == time_parser_char_space)
                {
                    /* next state */
                    parse_state = time_parser_state_day;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }

            } else if(!eisalpha(str_input[str_idx]))
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_day:
            if(buffer_pos > 2)
            {
                /* convert */
                datetime->day = eatoi2(parse_buffer, buffer_pos);

                /* space must follow */
                if(parse_char == time_parser_char_space)
                {
                    /* next state */
                    parse_state = time_parser_state_hours;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }

            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_hours:
            if(buffer_pos > 2)
            {
                /* convert */
                datetime->hours = eatoi2(parse_buffer, buffer_pos);

                /* column must follow */
                if(parse_char == time_parser_char_column)
                {
                    /* next state */
                    parse_state = time_parser_state_minutes;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }


            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_minutes:
            if(buffer_pos > 2)
            {
                /* convert */
                datetime->minutes = eatoi2(parse_buffer, buffer_pos);

                /* column must follow */
                if(parse_char == time_parser_char_column)
                {
                    /* next state */
                    parse_state = time_parser_state_seconds;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }

            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_seconds:
            if(buffer_pos > 2)
            {
                /* convert */
                datetime->seconds = eatoi2(parse_buffer, buffer_pos);

                /* space must follow */
                if(parse_char == time_parser_char_space)
                {
                    /* next state */
                    parse_state = time_parser_state_offset;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }


            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_offset:
            if(parse_char == time_parser_char_plus)
            {
                /* must be first */
                if(buffer_pos != 1) parse_state = time_parser_state_syntax_error;

                /* positive offset */
                offset_sign = 1;

                /* ignore */
                buffer_pos--;

            } else if(parse_char == time_parser_char_minus)
            {
                /* must be first */
                if(buffer_pos != 1) parse_state = time_parser_state_syntax_error;

                /* negative offset */
                offset_sign = -1;

                /* ignore */
                buffer_pos--;

            } else if(buffer_pos > DATETIME_OFFSET_DIGITS_LENGTH)
            {
                /* NOTE: offset is in format HHMM */

                /* parse offset */
                datetime->offset = offset_sign * 60 * eatoi2(parse_buffer, 2) + eatoi2(parse_buffer + 2, 2);

                /* space must follow */
                if(parse_char == time_parser_char_space)
                {
                    /* next state */
                    parse_state = time_parser_state_year;
                    buffer_pos = 0;

                } else
                {
                    /* syntax error */
                    parse_state = time_parser_state_syntax_error;
                }

            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        case time_parser_state_year:
            if(buffer_pos == 4)
            {
                /* convert */
                datetime->year = eatoi2(parse_buffer, buffer_pos);

                /* parser ready */
                parse_state = time_parser_state_end;

            } else if(parse_char != time_parser_char_digit)
            {
                parse_state = time_parser_state_syntax_error;
            }
            break;

        }

        /* report error if any */
        if(parse_state == time_parser_state_syntax_error)
        {
            ETRACE2("datetime_parse: syntax error at %d, unexpected character \'%c\'", str_idx, str_input[str_idx]);
            return ELIBC_ERROR_PARSER_INVALID_INPUT;
        }
    }

    return _datetime_parser_validate(datetime);
}

int _datetime_format_twitter(const datetime_t* datetime, char* str_output, size_t* output_size)
{
    char offset_buffer[16];
    int offset, offset_hours, offset_minutes;

    /* output must fit 3 + 1 + 3 + 1 + 2 + 1 + 8 + 1 + 5 + 1 + 4 = 30 characters plus zero terminate */
    if(output_size)
    {
        /* check if we have enough space */
        if(*output_size != 0 && *output_size < 31) 
        {
            /* set required size */
            *output_size = 31;

            return ELIBC_ERROR_ARGUMENT;
        }

        /* output size will be always the same */
        *output_size = 31;
    }

    /* remove sign */
    offset = (datetime->offset > 0) ? datetime->offset : -datetime->offset;

    /* offset hours */
    offset_hours = 0;
    while((offset_hours + 1) * 60 <= offset) offset_hours++;

    /* offset minutes */
    offset_minutes = offset - offset_hours * 60;

    /* format offset */
    if(datetime->offset >= 0)
        esprintf(offset_buffer, "+%.2d%.2d", offset_hours, offset_minutes);
    else
        esprintf(offset_buffer, "-%.2d%.2d", offset_hours, offset_minutes);

    /* format */
    esprintf(str_output, "%.3s %.3s %.2d %.2d:%.2d:%.2d %.5s %.4d", datetime_weekday(datetime->weekday), datetime_month_name(datetime->month), 
        datetime->day, datetime->hours, datetime->minutes, datetime->seconds, offset_buffer, datetime->year);

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* validation */

/*----------------------------------------------------------------------*/

/* parse and format */
int datetime_parse(time_format_t format, const char* str_input, size_t str_len, datetime_t* datetime)
{
    /* check input */
    EASSERT(datetime);
    if(datetime == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset output */
    ememset(datetime, 0, sizeof(datetime_t));

    /* compute input length if needed */
    if(str_input != 0 && str_len == 0) 
        str_len = estrlen(str_input);

    /* ignore if input is empty */
    if(str_input == 0 || str_len == 0) return ELIBC_SUCCESS;

    /* check format */
    switch(format)
    {
    case DATETIME_FORMAT_UNIXTIME:
        return _datetime_parse_unixtime(str_input, str_len, datetime);
        break;

    case DATETIME_FORMAT_ISO8601:
        return _datetime_parse_iso8601(str_input, str_len, datetime);
        break;

    case DATETIME_FORMAT_RFC1123:
        return _datetime_parse_rfc1123(str_input, str_len, datetime);
        break;

    case DATETIME_FORMAT_TWITTER:
        return _datetime_parse_twitter(str_input, str_len, datetime);
        break;
    }

    ETRACE("datetime_parse: unsupported format requested");
    return ELIBC_ERROR_ARGUMENT;
}

int datetime_format(time_format_t format, const datetime_t* datetime, char* str_output, size_t* output_size)
{
    /* check input */
    EASSERT(datetime);
    EASSERT(str_output);
    if(datetime == 0 || str_output == 0) return ELIBC_ERROR_ARGUMENT;

    /* check date format */
    if(datetime->year > 9999 ||
       datetime->month > 11 ||
       datetime->day > 31 ||
       datetime->hours > 24 ||
       datetime->minutes > 60 ||
       datetime->seconds > 60 ||
       datetime->offset < -12*60 ||
       datetime->offset > 12*60)
    {
        /* reset output size */
        if(output_size) *output_size = 0;

        ETRACE("datetime_format: date is not valid");
        return ELIBC_ERROR_ARGUMENT;
    }

    /* check format */
    switch(format)
    {
    case DATETIME_FORMAT_UNIXTIME:
        return _datetime_format_unixtime(datetime, str_output, output_size);
        break;

    case DATETIME_FORMAT_ISO8601:
        return _datetime_format_iso8601(datetime, str_output, output_size);
        break;

    case DATETIME_FORMAT_RFC1123:
        return _datetime_format_rfc1123(datetime, str_output, output_size);
        break;

    case DATETIME_FORMAT_TWITTER:
        return _datetime_format_twitter(datetime, str_output, output_size);
        break;
    }

    ETRACE("datetime_format: unsupported format requested");
    return ELIBC_ERROR_ARGUMENT;
}

/*----------------------------------------------------------------------*/

/*
    NOTE: ignore milliseconds
*/

/* convert times */
int datetime_to_unixtime(const datetime_t* datetime, etime_t* unixtime)
{
    etm_t timeinfo;

    /* check input */
    EASSERT(datetime);
    EASSERT(unixtime);
    if(datetime == 0 || unixtime == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset time info */
    ememset(&timeinfo, 0, sizeof(etm_t));

    /* fill tm structure */
    timeinfo.tm_year = datetime->year - 1900;
    timeinfo.tm_mon = datetime->month;
    timeinfo.tm_mday = datetime->day;
    timeinfo.tm_hour = datetime->hours;
    timeinfo.tm_min = datetime->minutes;
    timeinfo.tm_sec = datetime->seconds;
    
    /* Zero (0) to indicate that standard time is in effect. */
    timeinfo.tm_isdst = 0; 

    /* convert */
    *unixtime = etimegm(&timeinfo);

    /* remove offset */
    *unixtime -= 60 * datetime->offset;

    return ELIBC_SUCCESS;
}

int unixtime_to_datetime(const etime_t unixtime, datetime_t* datetime)
{
    etm_t* ptm;

    /* check input */
    EASSERT(datetime);
    EASSERT(unixtime);
    if(datetime == 0 || unixtime == 0) return ELIBC_ERROR_ARGUMENT;

    /* reset output */
    ememset(datetime, 0, sizeof(datetime_t));

    /* convert */
    ptm = egmtime(&unixtime);
    if(ptm == 0)
    {
        ETRACE("unixtime_to_datetime: failed to convert unixtime");
        return ELIBC_ERROR_INTERNAL;
    }

    /* fill output */
    datetime->year = ptm->tm_year + 1900;
    datetime->month = ptm->tm_mon;
    datetime->day = ptm->tm_mday;
    datetime->hours = ptm->tm_hour;
    datetime->minutes = ptm->tm_min;
    datetime->seconds = ptm->tm_sec;
    datetime->weekday = ptm->tm_wday;

    /* unixtime is in UTC */
    datetime->offset = 0;

    return ELIBC_SUCCESS;
}

/*----------------------------------------------------------------------*/
/* names (English) */
const char* datetime_weekday(int weekday)
{
    if(weekday >= 0 && weekday < DATETIME_WEEKDAY_SIZE) return _datetime_weekdays[weekday];

    EASSERT(0);
    return 0;
}

const char* datetime_month_name(int month)
{
    if(month >= 0 && month < DATETIME_MONTH_NAME_SIZE) return _datetime_month_names[month];

    EASSERT(0);
    return 0;
}

/*----------------------------------------------------------------------*/
