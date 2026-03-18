#include <zephyr/ztest.h>
#include <ring_buffer.h>

#define TEST_BUFFER_CAPACITY 4U

/**
 * @brief Verify that a newly initialized ring buffer is empty.
 *
 * This test checks the expected initial state after successful initialization:
 * - buffer is empty
 * - buffer is not full
 * - size is zero
 */
ZTEST(ring_buffer, test_init_buffer_is_empty)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_true(ring_buffer_is_empty(&rb),
                 "Buffer should be empty after initialization");
    zassert_false(ring_buffer_is_full(&rb),
                  "Buffer should not be full after initialization");
    zassert_equal(ring_buffer_get_size(&rb), 0U,
                  "Buffer size should be zero after initialization");
}

/**
 * @brief Verify that inserting one element updates the buffer state correctly.
 *
 * Expected behavior:
 * - put succeeds
 * - buffer is no longer empty
 * - size becomes one
 */
ZTEST(ring_buffer, test_put_one_element)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];
    const size_t expected_size = 1U;

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_equal(ring_buffer_put(&rb, 10),
                  RING_BUFFER_SUCCESS,
                  "Put operation failed");

    zassert_false(ring_buffer_is_empty(&rb),
                  "Buffer should not be empty after one put");
    zassert_equal(ring_buffer_get_size(&rb), expected_size,
                  "Buffer size should be one after one put");
}

/**
 * @brief Verify that retrieving one element returns the inserted value.
 *
 * Expected behavior:
 * - put succeeds
 * - get succeeds
 * - returned value matches inserted value
 * - buffer becomes empty again
 */
ZTEST(ring_buffer, test_get_one_element)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];
    ring_buffer_data_t value = 0;

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_equal(ring_buffer_put(&rb, 42),
                  RING_BUFFER_SUCCESS,
                  "Put operation failed");
    zassert_equal(ring_buffer_get(&rb, &value),
                  RING_BUFFER_SUCCESS,
                  "Get operation failed");

    zassert_equal(value, 42,
                  "Returned value does not match inserted value");
    zassert_true(ring_buffer_is_empty(&rb),
                 "Buffer should be empty after removing the only element");
    zassert_equal(ring_buffer_get_size(&rb), 0U,
                  "Buffer size should be zero after removing the only element");
}

/**
 * @brief Verify that drain succeeds and returns all elements in FIFO order.
 *
 * Expected behavior:
 * - drain succeeds
 * - drained_count matches stored element count
 * - output buffer contains values in FIFO order
 * - ring buffer becomes empty after drain
 */
ZTEST(ring_buffer, test_drain_success)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];
    ring_buffer_data_t out_buffer[TEST_BUFFER_CAPACITY] = {0};
    size_t drained_count = 0U;

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_equal(ring_buffer_put(&rb, 10), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 20), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 30), RING_BUFFER_SUCCESS, NULL);

    zassert_equal(ring_buffer_drain(&rb, out_buffer, TEST_BUFFER_CAPACITY, &drained_count),
                                    RING_BUFFER_SUCCESS, "Drain should succeed");

    zassert_equal(drained_count, 3U, "Drained count should match with size");
    zassert_equal(out_buffer[0], 10, "First drained element should be 10");
    zassert_equal(out_buffer[1], 20, "Second drained element should be 20");
    zassert_equal(out_buffer[2], 30, "Third drained element should be 30");

    zassert_true(ring_buffer_is_empty(&rb), "Buffer should be empty after drain");
    zassert_equal(ring_buffer_get_size(&rb), 0U, "Buffer size should be zero after drain");
}

/**
 * @brief Verify that drain fails with invalid arguments.
 *
 * Expected behavior:
 * - drain returns RING_BUFFER_ERROR_INVALID_ARGUMENT
 */
ZTEST(ring_buffer, test_drain_fails_with_invalid_argument)
{
    ring_buffer_data_t out_buffer[TEST_BUFFER_CAPACITY];
    size_t drained_count = 0U;

zassert_equal(ring_buffer_drain(NULL, out_buffer, TEST_BUFFER_CAPACITY, &drained_count),
                  RING_BUFFER_ERROR_INVALID_ARGUMENT,
                  "Drain should fail with NULL ring buffer pointer");
}

/**
 * @brief Verify that drain fails when output capacity is too small.
 *
 * Expected behavior:
 * - drain returns RING_BUFFER_ERROR_INVALID_CAPACITY
 * - drained_count is set to zero
 * - ring buffer contents remain unchanged
 */
ZTEST(ring_buffer, test_drain_fails_with_invalid_capacity)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];
    ring_buffer_data_t out_buffer[2];
    size_t drained_count = 99U;

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_equal(ring_buffer_put(&rb, 1), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 2), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 3), RING_BUFFER_SUCCESS, NULL);

    zassert_equal(ring_buffer_drain(&rb, out_buffer, 2U, &drained_count),
                  RING_BUFFER_ERROR_INVALID_CAPACITY,
                  "Drain should fail when output capacity is too small");

    zassert_equal(drained_count, 0U,
                  "Drained count should be zero on invalid capacity");
    zassert_equal(ring_buffer_get_size(&rb), 3U,
                  "Buffer contents should remain unchanged after failed drain");
}

/**
 * @brief Verify that put fails when the buffer is full.
 *
 * Expected behavior:
 * - filling the buffer up to capacity succeeds
 * - additional put returns RING_BUFFER_ERROR_FULL
 * - size remains unchanged
 */
ZTEST(ring_buffer, test_put_fails_when_full)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_equal(ring_buffer_put(&rb, 1), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 2), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 3), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 4), RING_BUFFER_SUCCESS, NULL);

    zassert_true(ring_buffer_is_full(&rb),
                 "Buffer should be full after reaching capacity");
    zassert_equal(ring_buffer_get_size(&rb), TEST_BUFFER_CAPACITY,
                  "Buffer size should equal capacity");

    zassert_equal(ring_buffer_put(&rb, 5),
                  RING_BUFFER_ERROR_FULL,
                  "Put should fail when buffer is full");

    zassert_equal(ring_buffer_get_size(&rb), TEST_BUFFER_CAPACITY,
                  "Buffer size should remain unchanged after failed put");
}

/**
 * @brief Verify that get fails when the buffer is empty.
 *
 * Expected behavior:
 * - get on an empty buffer returns RING_BUFFER_ERROR_EMPTY
 * - size remains zero
 */
ZTEST(ring_buffer, test_get_fails_when_empty)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];
    ring_buffer_data_t value = 0;

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_equal(ring_buffer_get(&rb, &value),
                  RING_BUFFER_ERROR_EMPTY,
                  "Get should fail when buffer is empty");

    zassert_equal(ring_buffer_get_size(&rb), 0U,
                  "Buffer size should remain zero");
}

/**
 * @brief Verify FIFO behavior is preserved after wraparound.
 *
 * This test fills the buffer, removes elements from the front, inserts new
 * elements to force tail wraparound, and verifies that retrieval still follows
 * correct FIFO order.
 */
ZTEST(ring_buffer, test_wrap_around_preserves_fifo_order)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];
    ring_buffer_data_t value = 0;

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_equal(ring_buffer_put(&rb, 10), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 20), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 30), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 40), RING_BUFFER_SUCCESS, NULL);

    zassert_equal(ring_buffer_get(&rb, &value), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(value, 10, "Expected first FIFO value to be 10");

    zassert_equal(ring_buffer_get(&rb, &value), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(value, 20, "Expected second FIFO value to be 20");

    zassert_equal(ring_buffer_put(&rb, 50), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 60), RING_BUFFER_SUCCESS, NULL);

    zassert_equal(ring_buffer_get_size(&rb), TEST_BUFFER_CAPACITY,
                  "Buffer size should return to full capacity");
    zassert_true(ring_buffer_is_full(&rb),
                 "Buffer should be full after wraparound refill");

    zassert_equal(ring_buffer_get(&rb, &value), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(value, 30, "Expected next FIFO value to be 30");

    zassert_equal(ring_buffer_get(&rb, &value), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(value, 40, "Expected next FIFO value to be 40");

    zassert_equal(ring_buffer_get(&rb, &value), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(value, 50, "Expected next FIFO value to be 50");

    zassert_equal(ring_buffer_get(&rb, &value), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(value, 60, "Expected next FIFO value to be 60");

    zassert_true(ring_buffer_is_empty(&rb),
                 "Buffer should be empty after all elements are removed");
}

/**
 * @brief Verify that initialization fails with a NULL ring buffer pointer.
 */
ZTEST(ring_buffer, test_init_fails_with_null_buffer)
{
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];

    zassert_equal(ring_buffer_init(NULL, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_ERROR_INIT_FAILED,
                  "Initialization should fail with NULL ring buffer pointer");
}

/**
 * @brief Verify that initialization fails with NULL storage.
 */
ZTEST(ring_buffer, test_init_fails_with_null_storage)
{
    struct ring_buffer rb;

    zassert_equal(ring_buffer_init(&rb, NULL, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_ERROR_INIT_FAILED,
                  "Initialization should fail with NULL storage pointer");
}

/**
 * @brief Verify that initialization fails with zero capacity.
 */
ZTEST(ring_buffer, test_init_fails_with_zero_capacity)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];

    zassert_equal(ring_buffer_init(&rb, storage, 0U),
                  RING_BUFFER_ERROR_INIT_FAILED,
                  "Initialization should fail with zero capacity");
}

/**
 * @brief Verify that put fails with a NULL ring buffer pointer.
 */
ZTEST(ring_buffer, test_put_fails_with_null_buffer)
{
    zassert_equal(ring_buffer_put(NULL, 123),
                  RING_BUFFER_ERROR_INVALID_ARGUMENT,
                  "Put should fail with NULL ring buffer pointer");
}

/**
 * @brief Verify that get fails with a NULL ring buffer pointer.
 */
ZTEST(ring_buffer, test_get_fails_with_null_buffer)
{
    ring_buffer_data_t value = 0;

    zassert_equal(ring_buffer_get(NULL, &value),
                  RING_BUFFER_ERROR_INVALID_ARGUMENT,
                  "Get should fail with NULL ring buffer pointer");
}

/**
 * @brief Verify that get fails with a NULL output pointer.
 */
ZTEST(ring_buffer, test_get_fails_with_null_output_pointer)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_equal(ring_buffer_get(&rb, NULL),
                  RING_BUFFER_ERROR_INVALID_ARGUMENT,
                  "Get should fail with NULL output pointer");
}

/**
 * @brief Verify that clear resets the buffer state to empty.
 *
 * Expected behavior:
 * - size becomes zero
 * - buffer reports empty
 * - buffer reports not full
 */
ZTEST(ring_buffer, test_clear_resets_buffer_state)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_equal(ring_buffer_put(&rb, 1), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 2), RING_BUFFER_SUCCESS, NULL);

    ring_buffer_clear(&rb);

    zassert_true(ring_buffer_is_empty(&rb),
                 "Buffer should be empty after clear");
    zassert_false(ring_buffer_is_full(&rb),
                  "Buffer should not be full after clear");
    zassert_equal(ring_buffer_get_size(&rb), 0U,
                  "Buffer size should be zero after clear");
}

/**
 * @brief Verify that a cleared buffer can be reused correctly.
 */
ZTEST(ring_buffer, test_clear_allows_buffer_reuse)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];
    ring_buffer_data_t value = 0;

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_equal(ring_buffer_put(&rb, 11), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_put(&rb, 22), RING_BUFFER_SUCCESS, NULL);

    ring_buffer_clear(&rb);

    zassert_equal(ring_buffer_put(&rb, 33), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_get(&rb, &value), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(value, 33,
                  "Buffer should be reusable after clear");
}

/**
 * @brief Verify correct operation with buffer capacity of one.
 *
 * This is an important edge case because head and tail wrap on every access.
 */
ZTEST(ring_buffer, test_capacity_one_behavior)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[1];
    ring_buffer_data_t value = 0;

    zassert_equal(ring_buffer_init(&rb, storage, 1U),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed for capacity-one buffer");

    zassert_true(ring_buffer_is_empty(&rb), NULL);

    zassert_equal(ring_buffer_put(&rb, 99), RING_BUFFER_SUCCESS, NULL);
    zassert_true(ring_buffer_is_full(&rb), NULL);
    zassert_equal(ring_buffer_get_size(&rb), 1U, NULL);

    zassert_equal(ring_buffer_put(&rb, 100),
                  RING_BUFFER_ERROR_FULL,
                  "Put should fail when capacity-one buffer is full");

    zassert_equal(ring_buffer_get(&rb, &value), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(value, 99, "Expected stored value to be returned");

    zassert_true(ring_buffer_is_empty(&rb), NULL);
}

/**
 * @brief Verify size tracking across multiple put and get operations.
 */
ZTEST(ring_buffer, test_size_tracking_across_operations)
{
    struct ring_buffer rb;
    ring_buffer_data_t storage[TEST_BUFFER_CAPACITY];
    ring_buffer_data_t value = 0;

    zassert_equal(ring_buffer_init(&rb, storage, TEST_BUFFER_CAPACITY),
                  RING_BUFFER_SUCCESS,
                  "Initialization failed");

    zassert_equal(ring_buffer_get_size(&rb), 0U, NULL);

    zassert_equal(ring_buffer_put(&rb, 1), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_get_size(&rb), 1U, NULL);

    zassert_equal(ring_buffer_put(&rb, 2), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_get_size(&rb), 2U, NULL);

    zassert_equal(ring_buffer_get(&rb, &value), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_get_size(&rb), 1U, NULL);

    zassert_equal(ring_buffer_get(&rb, &value), RING_BUFFER_SUCCESS, NULL);
    zassert_equal(ring_buffer_get_size(&rb), 0U, NULL);
}

/**
 * @brief Verify query helper behavior with NULL ring buffer pointer.
 *
 * This test reflects the current API behavior:
 * - ring_buffer_is_empty(NULL) returns true
 * - ring_buffer_is_full(NULL) returns false
 * - ring_buffer_get_size(NULL) returns zero
 */
ZTEST(ring_buffer, test_query_helpers_with_null_buffer)
{
    zassert_true(ring_buffer_is_empty(NULL),
                 "NULL buffer should be treated as empty by current API");
    zassert_false(ring_buffer_is_full(NULL),
                  "NULL buffer should not be treated as full by current API");
    zassert_equal(ring_buffer_get_size(NULL), 0U,
                  "NULL buffer size should be reported as zero");
}

ZTEST_SUITE(ring_buffer, NULL, NULL, NULL, NULL, NULL);