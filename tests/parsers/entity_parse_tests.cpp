/*
    Entity parser unit tests
*/

#include "../elib_tests_config.h"

/*----------------------------------------------------------------------*/

struct _EntityParseTestParams
{
    const char*     input; 
    int             len; 
    const char*     output; 
    int             out_len;

    _EntityParseTestParams(const char* _input, int _len, const char* _output, int _out_len):
        input(_input),
        len(_len),
        output(_output),
        out_len(_out_len)
    {
    }
};

class EntityParseTest : public ::testing::TestWithParam<_EntityParseTestParams> {
};

TEST_P(EntityParseTest, entity_parse_test)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    entity_result_t entity_result;
    int err;
    size_t size_used;
    entity_parser_t entity_parser;

    /* test parameters */
    struct _EntityParseTestParams const& params = GetParam();

    /* init parser */
    err = entity_begin(&entity_parser);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* parse */
    err = entity_parse(&entity_parser, params.input + 1, params.len - 1, &entity_result, &size_used);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_EQ(entity_result, entity_result_ready);

    /* validate input */
    ASSERT_TRUE(estrncmp(params.input, entity_input(&entity_parser), size_used + 1) == 0);
    ASSERT_EQ(entity_input_len(&entity_parser), size_used + 1);

    /* validate output */
    ASSERT_EQ(entity_output_len(&entity_parser), params.out_len);
    ASSERT_TRUE(estrncmp(params.output, entity_output(&entity_parser), params.out_len) == 0);
}

/*----------------------------------------------------------------------*/

INSTANTIATE_TEST_CASE_P(entity_parse_test_number, EntityParseTest, ::testing::Values(
    _EntityParseTestParams("&#x1D306;", sizeof("&#x1D306;"), "\xF0\x9D\x8C\x86", 4),
    _EntityParseTestParams("&#119558;", sizeof("&#119558;"), "\xF0\x9D\x8C\x86", 4),
    _EntityParseTestParams("&#x0021;", sizeof("&#x0021;"), "\x21", 1),
    _EntityParseTestParams("&#x21;", sizeof("&#x21;"), "!", 1),
    _EntityParseTestParams("&#33;", sizeof("&#33;"), "!", 1)
));

INSTANTIATE_TEST_CASE_P(entity_parse_test_names, EntityParseTest, ::testing::Values(
    _EntityParseTestParams("&bnequiv;", sizeof("&bnequiv;"), "\xE2\x89\xA1\xE2\x83\xA5", 6),
    _EntityParseTestParams("&PrecedesSlantEqual;", sizeof("&PrecedesSlantEqual;"), "\xE2\x89\xBC", 3),
    _EntityParseTestParams("&CounterClockwiseContourIntegral;", sizeof("&CounterClockwiseContourIntegral;"), "\xE2\x88\xB3", 3),
    _EntityParseTestParams("&lt;", sizeof("&lt;"), "<", 1),
    _EntityParseTestParams("&gt;", sizeof("&gt;"), ">", 1),
    _EntityParseTestParams("&amp;", sizeof("&amp;"), "&", 1),
    _EntityParseTestParams("&comma;", sizeof("&comma;"), ",", 1)
));

/*----------------------------------------------------------------------*/
