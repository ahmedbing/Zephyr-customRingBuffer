#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int ring_buffer_data_t;

typedef enum {
    RING_BUFFER_SUCCESS = 0,
    RING_BUFFER_ERROR_FULL,
    RING_BUFFER_ERROR_EMPTY,
    RING_BUFFER_ERROR_INIT_FAILED,
    RING_BUFFER_ERROR_INVALID_CAPACITY,
    RING_BUFFER_ERROR_INVALID_ARGUMENT
} ring_buffer_status_t;

struct ring_buffer {
    ring_buffer_data_t *buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
};

/**
 * @brief Initialize a ring buffer instance with caller-provided storage.
 *
 * This function prepares the ring buffer for use by assigning the backing
 * storage area, setting the capacity, and resetting internal indices/state.
 *
 * @param rb Pointer to the ring buffer instance to initialize.
 * @param storage Pointer to the caller-allocated storage array.
 * @param capacity Maximum number of elements the buffer can store.
 *
 * @return RING_BUFFER_STATUS_OK on success, otherwise an error status.
 */
ring_buffer_status_t ring_buffer_init(struct ring_buffer *rb,
                                            ring_buffer_data_t *storage,
                                            size_t capacity);

/**
 * @brief Clear all contents of the ring buffer.
 *
 * This function resets the buffer to an empty state without modifying the
 * underlying storage memory.
 *
 * @param rb Pointer to the ring buffer instance.
 */
void ring_buffer_clear(struct ring_buffer *rb);

/**
 * @brief Insert one element into the ring buffer.
 *
 * Stores a new element at the write position if space is available.
 *
 * @param rb Pointer to the ring buffer instance.
 * @param data Element to be written into the buffer.
 *
 * @return RING_BUFFER_STATUS_OK on success, otherwise an error status
 *         such as full buffer or invalid argument.
 */
ring_buffer_status_t ring_buffer_put(struct ring_buffer *rb,
                                           ring_buffer_data_t data);

/**
 * @brief Remove one element from the ring buffer.
 *
 * Retrieves the oldest available element from the buffer if the buffer is
 * not empty.
 *
 * @param rb Pointer to the ring buffer instance.
 * @param data Pointer to the output location for the retrieved element.
 *
 * @return RING_BUFFER_STATUS_OK on success, otherwise an error status
 *         such as empty buffer or invalid argument.
 */
ring_buffer_status_t ring_buffer_get(struct ring_buffer *rb,
                                           ring_buffer_data_t *data);

/**
 * @brief Remove and return all currently stored elements in FIFO order.
 *
 * Copies all available elements from the ring buffer into the
 * caller-provided output array and removes them from the buffer.
 *
 * @param rb Pointer to the ring buffer instance.
 * @param out_buffer Pointer to the destination array.
 * @param out_capacity Capacity of the destination array.
 * @param drained_count Pointer to the variable that receives the number of
 *        copied elements.
 * @return RING_BUFFER_SUCCESS on success, otherwise an error status such as
 *         empty buffer or invalid argument.
 */
ring_buffer_status_t ring_buffer_drain(struct ring_buffer *rb,
                                       ring_buffer_data_t *out_buffer,
                                       size_t out_capacity,
                                       size_t *drained_count);

/**
 * @brief Check whether the ring buffer is empty.
 *
 * @param rb Pointer to the ring buffer instance.
 *
 * @return true if the buffer contains no elements, otherwise false.
 */
bool ring_buffer_is_empty(const struct ring_buffer *rb);

/**
 * @brief Check whether the ring buffer is full.
 *
 * @param rb Pointer to the ring buffer instance.
 *
 * @return true if the buffer has reached its capacity, otherwise false.
 */
bool ring_buffer_is_full(const struct ring_buffer *rb);

/**
 * @brief Get the current number of stored elements.
 *
 * @param rb Pointer to the ring buffer instance.
 *
 * @return Number of valid elements currently stored in the buffer.
 */
size_t ring_buffer_get_size(const struct ring_buffer *rb);

#ifdef __cplusplus
}
#endif
