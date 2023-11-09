/*
    Escape parser unit tests
*/

#include "../elib_tests_config.h"

/*----------------------------------------------------------------------*/

struct EscapeParseSingleCharParams
{
    const char* input;
    const char  ch;

    EscapeParseSingleCharParams(const char* _input, const char _ch) :
        input(_input),
        ch(_ch)
    {
    }
};

class EscapeParseSingleChar : public ::testing::TestWithParam<EscapeParseSingleCharParams> {
};

TEST_P(EscapeParseSingleChar, escape_parse_test_singlechar_tc)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    escape_result_t escape_result;
    int err;
    size_t size_used;
    escape_parser_t escape_parser;

    /* test parameters */
    struct EscapeParseSingleCharParams const& params = GetParam();

    /* init parser */
    err = escape_begin(&escape_parser);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* parse */
    err = escape_parse(&escape_parser, params.input + 1, 1, &escape_result, &size_used);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* validate input */
    ASSERT_TRUE(estrncmp(params.input, escape_input(&escape_parser), size_used + 1) == 0);
    ASSERT_TRUE(escape_input_len(&escape_parser) == size_used + 1);

    /* validate output */
    ASSERT_TRUE(escape_output(&escape_parser)[0] == params.ch);
    ASSERT_TRUE(escape_output_len(&escape_parser) == 1);
}

INSTANTIATE_TEST_CASE_P(escape_parse_test_singlechar, EscapeParseSingleChar, ::testing::Values(
    /* test supported single characters */
    EscapeParseSingleCharParams("\\b", '\b'),
    EscapeParseSingleCharParams("\\f", '\f'),
    EscapeParseSingleCharParams("\\n", '\n'),
    EscapeParseSingleCharParams("\\r", '\r'),
    EscapeParseSingleCharParams("\\t", '\t'),
    EscapeParseSingleCharParams("\\v", '\v'),
    EscapeParseSingleCharParams("\\'", '\''),
    EscapeParseSingleCharParams("\\\"", '"'),
    EscapeParseSingleCharParams("\\\\", '\\'),
    EscapeParseSingleCharParams("\\/", '/'),

    /* test unsupported single characters */
    EscapeParseSingleCharParams("\\a", 'a'),
    EscapeParseSingleCharParams("\\d", 'd')
));

/*----------------------------------------------------------------------*/

struct EscapeParseOctalParams
{
    const char* input;
    int         len;
    const char  ch;

    EscapeParseOctalParams(const char* _input, int _len, const char _ch) :
        input(_input),
        len(_len),
        ch(_ch)
    {
    }
};

class EscapeParseOctal : public ::testing::TestWithParam<EscapeParseOctalParams> {
};

TEST_P(EscapeParseOctal, escape_parse_test_octal_tc)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    escape_result_t escape_result;
    int err;
    size_t size_used;
    escape_parser_t escape_parser;

    /* test parameters */
    struct EscapeParseOctalParams const& params = GetParam();

    /* init parser */
    err = escape_begin(&escape_parser);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* parse */
    err = escape_parse(&escape_parser, params.input + 1, params.len - 1, &escape_result, &size_used);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* validate input */
    ASSERT_TRUE(estrncmp(params.input, escape_input(&escape_parser), size_used + 1) == 0);
    ASSERT_TRUE(escape_input_len(&escape_parser) == size_used + 1);

    /* validate output */
    ASSERT_TRUE(escape_output(&escape_parser)[0] == params.ch);
    ASSERT_TRUE(escape_output_len(&escape_parser) == 1);
}

INSTANTIATE_TEST_CASE_P(escape_parse_test_octal, EscapeParseOctal, ::testing::Values(
    /* test octals */
    EscapeParseOctalParams("\\1", sizeof("\\1"), 1),
    EscapeParseOctalParams("\\12", sizeof("\\12"), 12),
    EscapeParseOctalParams("\\123", sizeof("\\123"), 123),
    EscapeParseOctalParams("\\1234", sizeof("\\1234"), 123),
    EscapeParseOctalParams("\\12A4", sizeof("\\12A4"), 12),
    EscapeParseOctalParams("\\12 34", sizeof("\\12 34"), 12)
));

/*----------------------------------------------------------------------*/

struct EscapeParseParams
{
    const char* input;
    int         len;
    const char* output;
    int         out_len;

    EscapeParseParams(const char* _input, int _len, const char* _output, int _out_len) :
        input(_input),
        len(_len),
        output(_output),
        out_len(_out_len)
    {
    }
};

class EscapeParse : public ::testing::TestWithParam<EscapeParseParams> {
};

TEST_P(EscapeParse, escape_parse_test)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    escape_result_t escape_result;
    int err;
    size_t size_used;
    escape_parser_t escape_parser;

    /* test parameters */
    struct EscapeParseParams const& params = GetParam();

    /* init parser */
    err = escape_begin(&escape_parser);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* parse */
    err = escape_parse(&escape_parser, params.input + 1, params.len - 1, &escape_result, &size_used);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* validate input */
    ASSERT_TRUE(estrncmp(params.input, escape_input(&escape_parser), size_used + 1) == 0);
    ASSERT_TRUE(escape_input_len(&escape_parser) == size_used + 1);

    /* validate output */
    ASSERT_TRUE(escape_output_len(&escape_parser) == (size_t)params.out_len);
    ASSERT_TRUE(estrncmp(params.output, escape_output(&escape_parser), params.out_len) == 0);
}

/*----------------------------------------------------------------------*/

INSTANTIATE_TEST_CASE_P(escape_parse_test_hex, EscapeParse, ::testing::Values(
    /* test hex */
    EscapeParseParams("\\x21", sizeof("\\x21"), "!", 1),
    EscapeParseParams("\\x61", sizeof("\\x61"), "a", 1),
    EscapeParseParams("\\x4e00", sizeof("\\x4e00"), "\xE4\xB8\x80", 3)
));

INSTANTIATE_TEST_CASE_P(escape_parse_test_unicode, EscapeParse, ::testing::Values(
    /* test unicode */
    EscapeParseParams("\\u21", sizeof("\\u21"), "!", 1),
    EscapeParseParams("\\U000021", sizeof("\\U000021"), "!", 1),
    EscapeParseParams("\\U10FFFF", sizeof("\\U10FFFF"), "\xF4\x8F\xBF\xBF", 4),
    EscapeParseParams("\\U100FF1", sizeof("\\U100FF1"), "\xF4\x80\xBF\xB1", 4)
));

INSTANTIATE_TEST_CASE_P(escape_parse_test_unicode_surrogate, EscapeParse, ::testing::Values(
    /* test unicode */
    EscapeParseParams("\\uD834\\uDF06", sizeof("\\uD834\\uDF06"), "\xF0\x9D\x8C\x86", 4),
    EscapeParseParams("\\uDBC3\\uDFF1", sizeof("\\uD834\\uDF06"), "\xF4\x80\xBF\xB1", 4)
));

INSTANTIATE_TEST_CASE_P(escape_parse_test_unicode_ecma6, EscapeParse, ::testing::Values(
    /* test unicode */
    EscapeParseParams("\\u{000061}", sizeof("\\u{000061}"), "a", 1),
    EscapeParseParams("\\u{001D306}", sizeof("\\u{001D306}"), "\xF0\x9D\x8C\x86", 4)
));

INSTANTIATE_TEST_CASE_P(escape_parse_test_special, EscapeParse, ::testing::Values(
    /* Facebook sends JSON in this format sometimes */
    EscapeParseParams("\\u00253A", sizeof("\\u00253A"), "%", 1)
));

/*----------------------------------------------------------------------*/
