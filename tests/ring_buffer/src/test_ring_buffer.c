#include <zephyr/ztest.h>
#include <ring_buffer.h>

#define TEST_BUFFER_CAPACITY 4

/*
* Test Case: Initialize the ring buffer and verify it's empty   
*/
ZTEST(ring_buffer, test_init_buffer_is_empty)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_true(ring_buffer_is_empty(&rb), "Buffer should be empty after init");
    zassert_false(ring_buffer_is_full(&rb), "Buffer should not be full after init");
    zassert_equal(ring_buffer_get_size(&rb), 0U, "Buffer size should be zero after init");
}

ZTEST_SUITE(ring_buffer, NULL, NULL, NULL, NULL, NULL);