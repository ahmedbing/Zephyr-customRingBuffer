#include "ring_buffer.h"

/*
    Static Functions
*/
static size_t ring_buffer_index(const size_t index, const size_t capacity)
{
    return index % capacity;
}

/*
    Ring Buffer API Implementations
*/
const ring_buffer_status_t ring_buffer_init( struct ring_buffer *rb, 
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
    rb = NULL;
}

const ring_buffer_status_t ring_buffer_put(struct ring_buffer *rb, ring_buffer_data_t data){
    if (!rb || rb->count >= rb->capacity) {
        return RING_BUFFER_ERROR_FULL;
    }
    rb->buffer[ring_buffer_index(rb->tail, rb->capacity)] = data;
    rb->tail = ring_buffer_index(rb->tail + 1, rb->capacity);
    rb->count++;
    return RING_BUFFER_SUCCESS;
}

const ring_buffer_status_t ring_buffer_get(struct ring_buffer *rb, ring_buffer_data_t *data){
    if (!rb || rb->count == 0) {
        return RING_BUFFER_ERROR_EMPTY;
    }
    *data = rb->buffer[ring_buffer_index(rb->head, rb->capacity)];
    rb->head = ring_buffer_index(rb->head + 1, rb->capacity);
    rb->count--;
    return RING_BUFFER_SUCCESS;
}

const bool ring_buffer_is_empty(const struct ring_buffer *rb){
    if(!rb){
        return true;
    }
    return (rb->count == 0);
}

const bool ring_buffer_is_full(const struct ring_buffer *rb){
    if(!rb){
        return false;
    }
    return (rb->count >= rb->capacity);
}

const size_t ring_buffer_get_size(const struct ring_buffer *rb){
    if(!rb){
        return 0;
    }
    return (rb->count);
}