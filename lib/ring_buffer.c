#include "ring_buffer.h"

/*
 *   Static Functions
 */
static size_t ring_buffer_index(const size_t index, const size_t capacity)
{
    return index % capacity;
}

/*
 *   Ring Buffer API Implementations
 */
ring_buffer_status_t ring_buffer_init( struct ring_buffer *rb, 
                                       ring_buffer_data_t *storage,
                                       size_t capacity)
{
    if (!rb || !storage || capacity == 0) {
        return RING_BUFFER_ERROR_INIT_FAILED;
    }
    rb->buffer = storage;
    rb->capacity = capacity;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    return RING_BUFFER_SUCCESS;
}

void ring_buffer_clear(struct ring_buffer *rb){
    if (rb) {
        rb->head = 0;
        rb->tail = 0;
        rb->count = 0;
    }
}

ring_buffer_status_t ring_buffer_put(struct ring_buffer *rb, ring_buffer_data_t data){
    if (!rb) {
        return RING_BUFFER_ERROR_INVALID_ARGUMENT;
    }
    if (rb->count == rb->capacity) {
        return RING_BUFFER_ERROR_FULL;
    }

    rb->buffer[rb->tail] = data;
    rb->tail = ring_buffer_index(rb->tail + 1U, rb->capacity);
    rb->count++;
    return RING_BUFFER_SUCCESS;
}

ring_buffer_status_t ring_buffer_get(struct ring_buffer *rb, ring_buffer_data_t *data){
    if (!rb || !data) {
        return RING_BUFFER_ERROR_INVALID_ARGUMENT;
    }
    if (rb->count == 0U) {
        return RING_BUFFER_ERROR_EMPTY;
    }

    *data = rb->buffer[rb->head];
    rb->head = ring_buffer_index(rb->head + 1U, rb->capacity);
    rb->count--;
    return RING_BUFFER_SUCCESS;
}

ring_buffer_status_t ring_buffer_drain(struct ring_buffer *rb,
                                       ring_buffer_data_t *out_buffer,
                                       size_t out_capacity,
                                       size_t *drained_count)
{
    if (!rb || !out_buffer || !drained_count) {
        return RING_BUFFER_ERROR_INVALID_ARGUMENT;
    }

    if (rb->count == 0U) {
        *drained_count = 0U;
        return RING_BUFFER_ERROR_EMPTY;
    }

    if (out_capacity < rb->count) {
        *drained_count = 0U;
        return RING_BUFFER_ERROR_INVALID_CAPACITY;
    }

    size_t count_to_drain = rb->count;
    for (size_t idx = 0U; idx < count_to_drain; ++idx) {
        ring_buffer_status_t status = ring_buffer_get(rb, &out_buffer[idx]);
        if (status != RING_BUFFER_SUCCESS) {
            *drained_count = idx;
            return status;
        }
    }

    *drained_count = count_to_drain;

    return RING_BUFFER_SUCCESS;
}

bool ring_buffer_is_empty(const struct ring_buffer *rb){
    if(!rb){
        return true;
    }
    return (rb->count == 0);
}

bool ring_buffer_is_full(const struct ring_buffer *rb){
    if(!rb){
        return false;
    }
    return (rb->count == rb->capacity);
}

size_t ring_buffer_get_size(const struct ring_buffer *rb){
    if(!rb){
        return 0;
    }
    return (rb->count);
}