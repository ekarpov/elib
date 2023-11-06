/*
    HTTP parser unit tests
*/

#include "../elib_tests_config.h"

/*----------------------------------------------------------------------*/

#define HTTPPARSE_TEST_CHUNK_SIZE       333

/*----------------------------------------------------------------------*/
/* http parser callbacks */
int _http_parse_silent_callback(void* user_data, http_event_t http_event, const void* data, size_t data_size)
{
    if(http_event == http_event_syntax_error)
    {
        printf("PARSER ERROR\n");
        return ELIBC_STOP;
    }

    return ELIBC_CONTINUE;
}

int _http_parse_trace_callback(void* user_data, http_event_t http_event, const void* data, size_t data_size)
{
    http_header_t http_header;

    switch(http_event)
    {
    case http_event_version:
        fprintf(stderr, "Version: %.*s\n", (int)data_size, (const char*)data);
        break;

    case http_event_method:
        fprintf(stderr, "Method: %.*s\n", (int)data_size, (const char*)data);
        break;

    case http_event_uri:
        fprintf(stderr, "URI: %.*s\n", (int)data_size, (const char*)data);
        break;

    case http_event_status_code:
        fprintf(stderr, "Status code: %d\n", (int)*((const unsigned short*)data));
        break;

    case http_event_reason_phrase:
        fprintf(stderr, "Reason phrase: %.*s\n", (int)data_size, (const char*)data);
        break;

    case http_event_content_length:
        fprintf(stderr, "Content length: %d\n", (int)*((const euint64_t*)data));
        break;

    case http_event_content_type:
        fprintf(stderr, "Content type: %.*s\n", (int)data_size, (const char*)data);
        break;

    case http_event_header:
        http_header = *((const http_header_t*)data);
        fprintf(stderr, "Header id: %d (%s)\n", (int)http_header, http_header_name(http_header));
        break;

    case http_event_header_name:
        fprintf(stderr, "Header name: %.*s\n", (int)data_size, (const char*)data);
        break;

    case http_event_header_value:
        fprintf(stderr, "Header value: %.*s\n", (int)data_size, (const char*)data);
        break;

    case http_event_headers_ready:
        fprintf(stderr, "headers ready\n");
        break;

    case http_event_content:
        fprintf(stderr, "Content: %.*s\n", (int)data_size, (const char*)data);
        break;

    case http_event_done:
        fprintf(stderr, "DONE\n");
        break;

    case http_event_syntax_error:
        fprintf(stderr, "PARSER ERROR\n");
        return ELIBC_STOP;
        break;
    }

    return ELIBC_CONTINUE;
}

/*----------------------------------------------------------------------*/

int _http_parse_buffer(http_parser_t* http_parser, http_parse_type_t type, char* buffer, efilesize_t buffer_size)
{
    size_t processed_size;
    int ret = ELIBC_SUCCESS;

    ebuffer_t parse_buffer;

    ebuffer_init(&parse_buffer);

    /* start parser */ 
    ret = http_parse_begin(http_parser, type, &parse_buffer);

    /* process response in chunks */
    processed_size = 0;
    while(ret == ELIBC_SUCCESS && processed_size < buffer_size && !http_parse_ready(http_parser))
    {
        if(buffer_size - processed_size >= HTTPPARSE_TEST_CHUNK_SIZE)
        {
            ret = http_parse(http_parser, buffer + processed_size, HTTPPARSE_TEST_CHUNK_SIZE, 0);
        } else
        {
            ret = http_parse(http_parser, buffer + processed_size, (int)(buffer_size - processed_size), 0);
        }

        processed_size += HTTPPARSE_TEST_CHUNK_SIZE;
    }

    ebuffer_free(&parse_buffer);

    return ret;
}

/*----------------------------------------------------------------------*/

struct HttpParseTestParams
{
    const char* input_file; 
    http_parse_type_t type;

    HttpParseTestParams(const char* _input_file, http_parse_type_t _type) :
        input_file(_input_file),
        type(_type)
    {
    }
};

class HttpParseTest : public ::testing::TestWithParam<HttpParseTestParams> {
};

TEST_P(HttpParseTest, http_parse_test)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    http_parser_t http_parser;
    int ret;

    char* read_buffer = 0;
    efilesize_t buffer_size = 0;

    /* test parameters */
    struct HttpParseTestParams const& params = GetParam();

    /* load input file */
    read_buffer = elib_tests_load_file(params.input_file, &buffer_size);
    ASSERT_TRUE(read_buffer);

    /* init parser */
    http_parse_init(&http_parser, _http_parse_trace_callback, 0);

    /* process response */
    ret = _http_parse_buffer(&http_parser, params.type, read_buffer, buffer_size);
    ASSERT_EQ(ret, ELIBC_SUCCESS);

    /* parser must be in completed state */
    ret = http_parse_ready(&http_parser);
    ASSERT_EQ(ret, ELIBC_TRUE);

    /* close parser */
    http_parse_close(&http_parser);

    /* free buffer */
    efree(read_buffer);
    read_buffer = 0;
}

/*----------------------------------------------------------------------*/

INSTANTIATE_TEST_CASE_P(http_parse_test_request, HttpParseTest, ::testing::Values(
    HttpParseTestParams("data/http_request_simple.txt", http_parse_request),
    HttpParseTestParams("data/http_request_no_data.txt", http_parse_request)
));

INSTANTIATE_TEST_CASE_P(http_parse_test_response, HttpParseTest, ::testing::Values(
    HttpParseTestParams("data/http_response_simple.txt", http_parse_response)
));



