/*
    ESort unit tests
*/

#include "../elib_tests_config.h"

/*----------------------------------------------------------------------*/

#define ESORT_TEST_SIZE             22000
#define ESORT_TEST_STRING_SIZE      512

/*----------------------------------------------------------------------*/

GTEST_TEST(elibc_esort_tests, esort_test_reverse)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    int* sort_buffer;
    size_t idx;
    int err;

    /* reserve array */
    sort_buffer = (int*)emalloc(sizeof(int)*ESORT_TEST_SIZE);
    ASSERT_TRUE(sort_buffer != 0);

    /* fill items in reverse order */
    for(idx = 0; idx < ESORT_TEST_SIZE; ++idx)
    {
        sort_buffer[idx] = (int)(ESORT_TEST_SIZE - idx);
    }

    /* sort */
    err = esort((char*)sort_buffer, sizeof(int), ESORT_TEST_SIZE, eless_int_func, 0);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* validate */
    for(idx = 0; idx < ESORT_TEST_SIZE - 1; ++idx)
    {
        ASSERT_TRUE(sort_buffer[idx] <= sort_buffer[idx + 1]);
    }

    /* release buffer */
    efree(sort_buffer);
}

GTEST_TEST(elibc_esort_tests, esort_test_random)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    int* sort_buffer;
    size_t idx;
    int err;

    /* reserve array */
    sort_buffer = (int*)emalloc(sizeof(int)*ESORT_TEST_SIZE);
    ASSERT_TRUE(sort_buffer != 0);

    /* seed random generator */
    esrand((unsigned int)etime(0));

    /* fill items in random order */
    for(idx = 0; idx < ESORT_TEST_SIZE; ++idx)
    {
        sort_buffer[idx] = erand() % ESORT_TEST_SIZE;
    }

    /* sort */
    err = esort((char*)sort_buffer, sizeof(int), ESORT_TEST_SIZE, eless_int_func, 0);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* validate */
    for(idx = 0; idx < ESORT_TEST_SIZE - 1; ++idx)
    {
        ASSERT_TRUE(sort_buffer[idx] <= sort_buffer[idx + 1]);
    }

    /* release buffer */
    efree(sort_buffer);
}

GTEST_TEST(elibc_esort_tests, esort_test_strings)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    char** sort_buffer;
    size_t idx;
    int err;

    /* reserve array */
    sort_buffer = (char**)emalloc(sizeof(char*)*ESORT_TEST_SIZE);
    ASSERT_TRUE(sort_buffer != 0);

    /* seed random generator */
    esrand((unsigned int)etime(0));

    /* fill items in random order */
    for(idx = 0; idx < ESORT_TEST_SIZE; ++idx)
    {
        /* reserve string */
        sort_buffer[idx] = (char*)emalloc(sizeof(char)*ESORT_TEST_STRING_SIZE);
        ASSERT_TRUE(sort_buffer[idx] != 0);

        /* init random */
        erandalnum_str(sort_buffer[idx], ESORT_TEST_STRING_SIZE - 1);
        sort_buffer[idx][ESORT_TEST_STRING_SIZE-1] = 0;
    }

    /* sort */
    err = esort((char*)sort_buffer, sizeof(char*), ESORT_TEST_SIZE, eless_str_func, 0);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* validate and free */
    for(idx = 0; idx < ESORT_TEST_SIZE; ++idx)
    {
        if(idx < ESORT_TEST_SIZE - 1)
        {
            err = estrcmp(sort_buffer[idx], sort_buffer[idx + 1]);
            ASSERT_TRUE(err <= 0);
        }

        efree(sort_buffer[idx]);
    }

    /* release buffer */
    efree(sort_buffer);
}

/*----------------------------------------------------------------------*/
