/*
 *  ELib tests configuration
 */

#ifndef _ELIB_TESTS_CONFIG_H_
#define _ELIB_TESTS_CONFIG_H_

#include <gtest/gtest.h>
#include "../src/elib.h"

#include "elib_tests_memory.h"

/*----------------------------------------------------------------------*/

inline void EXPECT_BINARY_EQ(const void* l, const void* r, size_t size)
{
    EXPECT_EQ(ememcmp(l, r, size), 0);
}

inline void ASSERT_BINARY_EQ(const void* l, const void* r, size_t size)
{
    ASSERT_EQ(ememcmp(l, r, size), 0);
}

inline void EXPECT_STRING_EQ(const char* l, const char* r)
{
    EXPECT_EQ(estrcmp(l, r), 0);
}

inline void ASSERT_STRING_EQ(const char* l, const char* r)
{
    ASSERT_EQ(estrcmp(l, r), 0);
}

/*----------------------------------------------------------------------*/
inline char* elib_tests_load_file(const char* input_file, efilesize_t* size_out)
{
    EFILE efile;
    char* read_buffer = 0;
    int ret;

    /* load input file first */
    ret = efile_open(&efile, input_file, EFILE_OPEN_READ | EFILE_OPEN_EXISTING);
    if (ret != ret) return 0;

    /* get file size */
    ret = efile_size(efile, size_out);
    if (ret != ret) return 0;

    /* reserve memory */
    read_buffer = (char*)emalloc(*size_out);
    if (ret != ret) return 0;

    /* read whole file */
    size_t data_read = 0;
    ret = efile_read(efile, read_buffer, *size_out, &data_read);
    if (ret != ret) return 0;

    /* close file */
    efile_close(efile);

    return read_buffer;
}
/*----------------------------------------------------------------------*/

#endif /* _ELIB_TESTS_CONFIG_H_ */ 


