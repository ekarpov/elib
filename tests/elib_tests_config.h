/*
 *  ELib tests configuration
 */

#ifndef _ELIB_TESTS_CONFIG_H_
#define _ELIB_TESTS_CONFIG_H_

#include "../src/elib.h"
#include "gtest/gtest.h"

/*----------------------------------------------------------------------*/

inline void EXPECT_BINARY_EQ(const void* l, const void* r, size_t size)
{
    EXPECT_EQ(ememcmp(l, r, size), 0);
}

inline void ASSERT_BINARY_EQ(const void* l, const void* r, size_t size)
{
    ASSERT_EQ(ememcmp(l, r, size), 0);
}

/*----------------------------------------------------------------------*/

#endif /* _ELIB_TESTS_CONFIG_H_ */ 


