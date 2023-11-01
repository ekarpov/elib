/*
    EList unit tests
*/

#include "../elib_tests_config.h"

/*----------------------------------------------------------------------*/

#define ELIST_TEST_ITEM_SIZE        11
#define ELIST_TEST_SIZE             22000

/*----------------------------------------------------------------------*/

GTEST_TEST(elibc_elist_tests, elist_test_item_size)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    elist_t elist;
    int size_t_size = sizeof(size_t);
    int idx;

    for(idx = 1; idx <= ELIST_TEST_SIZE * size_t_size; ++idx)
    {
        elist_init(&elist, idx);
        ASSERT_EQ(elist.item_size, idx);
        ASSERT_TRUE((elist.node_size - 2) * size_t_size >= elist.item_size);

        elist_free(&elist);
    }
}

GTEST_TEST(elibc_elist_tests, elist_test_items)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    elist_t elist;
    char test_data[ELIST_TEST_ITEM_SIZE];
    const void* list_data;
    eliter_t iter = ELIST_NULL_ITERATOR;
    int err, idx;
    size_t list_size;

    elist_init(&elist, ELIST_TEST_ITEM_SIZE);

    /* add items */
    for(idx = 0; idx < ELIST_TEST_SIZE; ++idx)
    {
        /* format item data */
        ememset(test_data, idx, ELIST_TEST_ITEM_SIZE);

        /* add item */
        if(idx % 2 == 0)
        {
            err = elist_append(&elist, iter, test_data, &iter);
            ASSERT_EQ(err, ELIBC_SUCCESS);

        } else
        {
            err = elist_insert(&elist, iter, test_data, &iter);
            ASSERT_EQ(err, ELIBC_SUCCESS);
        }

        /* get data from iterator */
        list_data = elist_item(&elist, iter);
        ASSERT_TRUE(list_data != (const void*)0);
        ASSERT_BINARY_EQ(list_data, test_data, ELIST_TEST_ITEM_SIZE);

        /* format new data */
        ememset(test_data, idx + 1, ELIST_TEST_ITEM_SIZE);

        /* replace item data */
        err = elist_replace(&elist, iter, test_data);
        ASSERT_EQ(err, ELIBC_SUCCESS);

        /* get data from iterator */
        list_data = elist_item(&elist, iter);
        ASSERT_TRUE(list_data != 0);
        ASSERT_BINARY_EQ(list_data, test_data, ELIST_TEST_ITEM_SIZE);
        
        /* list size must match */
        list_size = elist_size(&elist);
        ASSERT_EQ(list_size, idx + 1);
    }

    elist_free(&elist);
}

GTEST_TEST(elibc_elist_tests, elist_test_iterators)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    elist_t elist;
    char test_data[ELIST_TEST_ITEM_SIZE];
    const void* list_data;
    eliter_t iter = ELIST_NULL_ITERATOR;
    int err, idx, item_count, del_count;
    size_t list_size;

    elist_init(&elist, ELIST_TEST_ITEM_SIZE);

    /* add items */
    for(idx = 0; idx < ELIST_TEST_SIZE; ++idx)
    {
        /* format item data */
        ememset(test_data, idx, ELIST_TEST_ITEM_SIZE);

        /* append item */
        err = elist_append(&elist, iter, test_data, &iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);
    }

    /* list head */
    err = elist_head(&elist, &iter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_TRUE(iter != ELIST_NULL_ITERATOR);

    /* traverse list */
    item_count = 0;
    while(iter != ELIST_NULL_ITERATOR)
    {
        /* format item data */
        ememset(test_data, item_count, ELIST_TEST_ITEM_SIZE);

        /* get data from iterator */
        list_data = elist_item(&elist, iter);
        ASSERT_TRUE(list_data != 0);
        ASSERT_BINARY_EQ(list_data, test_data, ELIST_TEST_ITEM_SIZE);

        /* next item */
        err = elist_next(&elist, iter, &iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);

        item_count++;
    }

    /* all items must have been accessed */
    ASSERT_EQ(item_count, ELIST_TEST_SIZE);

    /* list tail */
    err = elist_tail(&elist, &iter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_TRUE(iter != ELIST_NULL_ITERATOR);

    /* traverse list */
    item_count = ELIST_TEST_SIZE;
    while(iter != ELIST_NULL_ITERATOR)
    {
        item_count--;

        /* format item data */
        ememset(test_data, item_count, ELIST_TEST_ITEM_SIZE);

        /* get data from iterator */
        list_data = elist_item(&elist, iter);
        ASSERT_TRUE(list_data != 0);
        ASSERT_BINARY_EQ(list_data, test_data, ELIST_TEST_ITEM_SIZE);

        /* previous item */
        err = elist_previous(&elist, iter, &iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);
    }

    /* all items must have been accessed */
    ASSERT_EQ(item_count, 0);

    /* list head */
    err = elist_head(&elist, &iter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_NE(iter, ELIST_NULL_ITERATOR);

    /* remove every second item */
    item_count = 0;
    del_count = 0;
    while(iter != ELIST_NULL_ITERATOR)
    {
        /* format item data */
        ememset(test_data, item_count, ELIST_TEST_ITEM_SIZE);

        /* get data from iterator */
        list_data = elist_item(&elist, iter);
        ASSERT_TRUE(list_data != 0);
        ASSERT_BINARY_EQ(list_data, test_data, ELIST_TEST_ITEM_SIZE);

        if(item_count % 2 == 0)
        {
            /* remove item */
            err = elist_remove(&elist, iter, &iter);
            ASSERT_EQ(err, ELIBC_SUCCESS);

            del_count++;

        } else
        {
            /* next item */
            err = elist_next(&elist, iter, &iter);
            ASSERT_EQ(err, ELIBC_SUCCESS);
        }

        item_count++;
    }

    /* match size */
    list_size = elist_size(&elist);
    ASSERT_EQ(list_size, ELIST_TEST_SIZE - del_count);
    ASSERT_EQ(list_size, ELIST_TEST_SIZE / 2 + (ELIST_TEST_SIZE % 2));

    /* list head */
    err = elist_head(&elist, &iter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_NE(iter, ELIST_NULL_ITERATOR);

    /* validate removed data */
    idx = 1;
    while(iter != ELIST_NULL_ITERATOR)
    {
        /* format item data */
        ememset(test_data, idx, ELIST_TEST_ITEM_SIZE);

        /* get data from iterator */
        list_data = elist_item(&elist, iter);
        ASSERT_TRUE(list_data != 0);
        ASSERT_BINARY_EQ(list_data, test_data, ELIST_TEST_ITEM_SIZE);

        /* next item */
        err = elist_next(&elist, iter, &iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);

        idx += 2;
    }

    /* list head */
    err = elist_head(&elist, &iter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_NE(iter, ELIST_NULL_ITERATOR);

    /* remove all */
    while(iter != ELIST_NULL_ITERATOR)
    {
        /* remove item */
        err = elist_remove(&elist, iter, &iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);
    }

    list_size = elist_size(&elist);
    ASSERT_EQ(list_size, 0);

    /* add items again */
    for(idx = 0; idx < ELIST_TEST_SIZE; ++idx)
    {
        /* format item data */
        ememset(test_data, idx, ELIST_TEST_ITEM_SIZE);

        /* append item */
        err = elist_append(&elist, iter, test_data, &iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);
    }

    /* list tail */
    err = elist_tail(&elist, &iter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_NE(iter, ELIST_NULL_ITERATOR);

    /* remove all starting from tail */
    while(iter != ELIST_NULL_ITERATOR)
    {
        eliter_t prev_iter;

        /* previous item */
        err = elist_previous(&elist, iter, &prev_iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);

        /* remove item */
        err = elist_remove(&elist, iter, &iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);
        ASSERT_EQ(iter, ELIST_NULL_ITERATOR);

        iter = prev_iter;
    }

    list_size = elist_size(&elist);
    ASSERT_EQ(list_size, 0);

    elist_free(&elist);
}

GTEST_TEST(elibc_elist_tests, elist_test_head_removal)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    elist_t elist;
    eliter_t eiter, eiter_next;
    int err;

    elist_init(&elist, ELIST_TEST_ITEM_SIZE);

    /* add two items */
    err = elist_append(&elist, ELIST_NULL_ITERATOR, 0, 0);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    err = elist_append(&elist, ELIST_NULL_ITERATOR, 0, 0);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* delete head */
    err = elist_head(&elist, &eiter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    err = elist_remove(&elist, eiter, &eiter_next);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* check state */
    ASSERT_EQ(elist_size(&elist), 1);    

    /* second item must be there */
    err = elist_head(&elist, &eiter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_NE(elist_item(&elist, eiter), (void*)0);

    /* reset data */
    elist_reset(&elist);

    /* add three items */
    err = elist_append(&elist, ELIST_NULL_ITERATOR, 0, 0);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    err = elist_append(&elist, ELIST_NULL_ITERATOR, 0, 0);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    err = elist_append(&elist, ELIST_NULL_ITERATOR, 0, 0);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* delete head */
    err = elist_head(&elist, &eiter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    err = elist_remove(&elist, eiter, &eiter_next);
    ASSERT_EQ(err, ELIBC_SUCCESS);

    /* second item must be there */
    err = elist_head(&elist, &eiter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_NE(elist_item(&elist, eiter), (void*)0);
    err = elist_next(&elist, eiter, &eiter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    ASSERT_NE(elist_item(&elist, eiter), (void*)0);

    elist_free(&elist);
}

GTEST_TEST(elibc_elist_tests, elist_test_sort)
{
    ELIB_GTEST_MEMORY_LEAK_DETECTOR;

    elist_t elist;
    eliter_t iter = ELIST_NULL_ITERATOR;
    size_t idx, item_count;
    int item_data;
    int err;

    elist_init(&elist, sizeof(int));

    /* seed random generator */
    esrand((unsigned int)etime(0));

    /* add items in random order */
    for(idx = 0; idx < ELIST_TEST_SIZE; ++idx)
    {
        /* item data */
        item_data = erand() % ELIST_TEST_SIZE;

        /* append item */
        err = elist_append(&elist, iter, &item_data, &iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);
    }

    /* sort */
    err = esort_list(&elist, eless_int_func);

    /* validate */
    err = elist_head(&elist, &iter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    item_count = 0;
    while(iter != ELIST_NULL_ITERATOR)
    {
        item_data = *(int*)elist_item(&elist, iter);
        
        /* next item */
        err = elist_next(&elist, iter, &iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);

        item_count++;

        /* validate order */
        if(iter != ELIST_NULL_ITERATOR)
        {
            ASSERT_TRUE(item_data <= *(int*)elist_item(&elist, iter));
        }
    }

    ASSERT_EQ(item_count, ELIST_TEST_SIZE);
    ASSERT_EQ(item_count, elist_size(&elist));

    /* reset */
    elist_reset(&elist);

    /* fill items in reverse order */
    for(idx = 0; idx < ELIST_TEST_SIZE; ++idx)
    {
        /* item data */
        item_data = (int)(ELIST_TEST_SIZE - idx);

        /* append item */
        err = elist_append(&elist, iter, &item_data, &iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);
    }

    /* sort */
    err = esort_list(&elist, eless_int_func);

    /* validate */
    err = elist_head(&elist, &iter);
    ASSERT_EQ(err, ELIBC_SUCCESS);
    item_count = 0;
    while(iter != ELIST_NULL_ITERATOR)
    {
        item_data = *(int*)elist_item(&elist, iter);
        
        /* next item */
        err = elist_next(&elist, iter, &iter);
        ASSERT_EQ(err, ELIBC_SUCCESS);

        item_count++;

        /* validate order */
        if(iter != ELIST_NULL_ITERATOR)
        {
            ASSERT_TRUE(item_data <= *(int*)elist_item(&elist, iter));
        }
    }

    ASSERT_EQ(item_count, ELIST_TEST_SIZE);
    ASSERT_EQ(item_count, elist_size(&elist));

    elist_free(&elist);
}

