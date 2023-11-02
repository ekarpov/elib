/*
    XML parser unit tests
*/

#include "../elib_tests_config.h"

/*----------------------------------------------------------------------*/

#define XMLPARSE_TEST_CHUNK_SIZE       333

/*----------------------------------------------------------------------*/
/* xml parser callbacks */
int _xml_parse_silent_callback(void* user_data, xml_event_t xml_event, const void* data, size_t data_size)
{
    if(xml_event == xml_parse_error)
    {
        printf("PARSER ERROR\n");
        return ELIBC_STOP;
    }

    return ELIBC_CONTINUE;
}

int _xml_parse_trace_callback(void* user_data, xml_event_t xml_event, const void* data, size_t data_size)
{
    switch(xml_event)
    {

    /* special content */        
    case xml_declaration:
        printf("XML declaration: %.*s\n", (int)data_size, (const char*)data);
        break;

    case xml_pi_target:
        printf("PI target: %.*s\n", (int)data_size, (const char*)data);
        break;

    case xml_pi_content:
        printf("PI content: %.*s\n", (int)data_size, (const char*)data);
        break;

    case xml_comment:
        printf("XML comment: %.*s\n", (int)data_size, (const char*)data);
        break;

    case xml_dtd:
        printf("DTD: %.*s\n", (int)data_size, (const char*)data);
        break;
        break;

    /* content events */
    case xml_tag_begin:
        printf("-> %.*s\n", (int)data_size, (const char*)data);
        break;

    case xml_tag_end:
        printf("<- %.*s\n", (int)data_size, (const char*)data);
        break;

    case xml_tag_content:
        printf("\t%.*s\n", (int)data_size, (const char*)data);
        break;

    case xml_attribute_name:
        printf("\t%.*s = ", (int)data_size, (const char*)data);
        break;

    case xml_attribute_value:
        printf("\"%.*s\"\n", (int)data_size, (const char*)data);
        break;

    /* state events */
    case xml_parse_error:
        printf("PARSER ERROR\n");
        return ELIBC_STOP;
        break;

    default:
        EASSERT(0);
        break;
    }

    return ELIBC_CONTINUE;
}

/*----------------------------------------------------------------------*/

int _xml_parse_buffer(xml_parser_t* xml_parser, char* buffer, efilesize_t buffer_size)
{
    size_t processed_size;
    int ret = ELIBC_SUCCESS;

    ebuffer_t parse_buffer;

    ebuffer_init(&parse_buffer);

    /* start parser */ 
    ret = xml_begin(xml_parser, &parse_buffer);

    /* process response in chunks */
    processed_size = 0;
    while(ret == ELIBC_SUCCESS && processed_size < buffer_size)
    {
        if(buffer_size - processed_size >= XMLPARSE_TEST_CHUNK_SIZE)
        {
            ret = xml_parse(xml_parser, buffer + processed_size, XMLPARSE_TEST_CHUNK_SIZE);
        } else
        {
            ret = xml_parse(xml_parser, buffer + processed_size, (int)(buffer_size - processed_size));
        }

        processed_size += XMLPARSE_TEST_CHUNK_SIZE;
    }

    /* finish parser */ 
    if(ret == ELIBC_SUCCESS)
        ret = xml_end(xml_parser);

    ebuffer_free(&parse_buffer);

    return ret;
}

/*----------------------------------------------------------------------*/

GTEST_TEST(xml_parse_tests, xml_parse_test_base)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    xml_parser_t xml_parser;

    char* read_buffer = 0;
    efilesize_t buffer_size = 0;

    int ret;

    /* load input file */
    read_buffer = elib_tests_load_file("data/books.xml", &buffer_size);
    ASSERT_TRUE(read_buffer);

    /* init parser */
    xml_init(&xml_parser, _xml_parse_silent_callback, 0);

    /* decode escape characters */
    xml_decode_escapes(&xml_parser, ELIBC_TRUE);

    /* process response */
    ret = _xml_parse_buffer(&xml_parser, read_buffer, buffer_size);
    ASSERT_EQ(ret, ELIBC_SUCCESS);

    /* close parser */
    xml_close(&xml_parser);

    /* free buffer */
    efree(read_buffer);
    read_buffer = 0;
}

GTEST_TEST(xml_parse_tests, xml_parse_test_dtd)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    xml_parser_t xml_parser;

    char* read_buffer = 0;
    efilesize_t buffer_size = 0;

    int ret;

    /* try to load input file first */
    read_buffer = elib_tests_load_file("data/dtd_test.xml", &buffer_size);
    ASSERT_TRUE(read_buffer);

    /* init parser */
    xml_init(&xml_parser, _xml_parse_silent_callback, 0);

    /* decode escape characters */
    xml_decode_escapes(&xml_parser, ELIBC_TRUE);

    /* process response */
    ret = _xml_parse_buffer(&xml_parser, read_buffer, buffer_size);
    ASSERT_EQ(ret, ELIBC_SUCCESS);

    /* close parser */
    xml_close(&xml_parser);

    /* free buffer */
    efree(read_buffer);
    read_buffer = 0;
}

GTEST_TEST(xml_parse_tests, xml_parse_test_rss)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    xml_parser_t xml_parser;

    char* read_buffer = 0;
    efilesize_t buffer_size = 0;

    int ret;

    /* try to load input file first */
    read_buffer = elib_tests_load_file("data/rss_test.xml", &buffer_size);
    ASSERT_TRUE(read_buffer);

    /* init parser */
    xml_init(&xml_parser, _xml_parse_silent_callback, 0);

    /* decode escape characters */
    xml_decode_escapes(&xml_parser, ELIBC_TRUE);

    /* process response */
    ret = _xml_parse_buffer(&xml_parser, read_buffer, buffer_size);
    ASSERT_EQ(ret, ELIBC_SUCCESS);

    /* close parser */
    xml_close(&xml_parser);

    /* free buffer */
    efree(read_buffer);
    read_buffer = 0;
}

/*----------------------------------------------------------------------*/

