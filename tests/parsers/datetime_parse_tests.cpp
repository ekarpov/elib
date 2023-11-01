/*
    Datetime parser unit tests
*/

#include "../elib_tests_config.h"

/*----------------------------------------------------------------------*/

GTEST_TEST(datetome_parse_tests, date_parse_test_twitter)
{
    datetime_t datetime;
    char format_buffer[64];
    int err;

    /* parse */
    err = datetime_parse(DATETIME_FORMAT_TWITTER, "Wed Aug 27 13:08:45 +0000 2008", 0, &datetime);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_EQ(datetime.weekday, 3);
    ASSERT_EQ(datetime.month, 7);
    ASSERT_EQ(datetime.day, 27);
    ASSERT_EQ(datetime.hours, 13);
    ASSERT_EQ(datetime.minutes, 8);
    ASSERT_EQ(datetime.seconds, 45);
    ASSERT_EQ(datetime.offset, 0);
    ASSERT_EQ(datetime.year, 2008);

    /* format */
    err = datetime_format(DATETIME_FORMAT_TWITTER, &datetime, format_buffer, 0);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_STRING_EQ(format_buffer, "Wed Aug 27 13:08:45 +0000 2008");

    /* parse */
    err = datetime_parse(DATETIME_FORMAT_TWITTER, "Thu Apr 06 15:28:43 +0000 2017", 0, &datetime);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_EQ(datetime.weekday, 4);
    ASSERT_EQ(datetime.month, 3);
    ASSERT_EQ(datetime.day, 6);
    ASSERT_EQ(datetime.hours, 15);
    ASSERT_EQ(datetime.minutes, 28);
    ASSERT_EQ(datetime.seconds, 43);
    ASSERT_EQ(datetime.offset, 0);
    ASSERT_EQ(datetime.year, 2017);

    /* format */
    err = datetime_format(DATETIME_FORMAT_TWITTER, &datetime, format_buffer, 0);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_STRING_EQ(format_buffer, "Thu Apr 06 15:28:43 +0000 2017");
}

GTEST_TEST(datetome_parse_tests, date_parse_test_rss)
{
    datetime_t datetime;
    char format_buffer[64];
    etime_t unixtime;
    int err;

    /* parse */
    err = datetime_parse(DATETIME_FORMAT_RFC1123, "Tue, 25 Jul 2017 15:25:32 +0300", 0, &datetime);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_EQ(datetime.weekday, 2);
    ASSERT_EQ(datetime.month, 6);
    ASSERT_EQ(datetime.day, 25);
    ASSERT_EQ(datetime.hours, 15);
    ASSERT_EQ(datetime.minutes, 25);
    ASSERT_EQ(datetime.seconds, 32);
    ASSERT_EQ(datetime.offset, 180);
    ASSERT_EQ(datetime.year, 2017);

    /* format */
    err = datetime_format(DATETIME_FORMAT_RFC1123, &datetime, format_buffer, 0);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_STRING_EQ(format_buffer, "25 Jul 2017 15:25:32 +0300");

    /* convert time */
    err = datetime_to_unixtime(&datetime, &unixtime);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_EQ(unixtime, 1500985532);
}

/*----------------------------------------------------------------------*/

