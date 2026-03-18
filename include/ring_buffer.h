#ifndef RING_BUFFER_H
#define RING_BUFFER_H

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
    RING_BUFFER_ERROR_INVALID_CAPACITY
} ring_buffer_status_t;

struct ring_buffer {
    ring_buffer_data_t *buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
};

const ring_buffer_status_t ring_buffer_init(struct ring_buffer *rb, ring_buffer_data_t *storage, size_t capacity);

void ring_buffer_clear(struct ring_buffer *rb);

const ring_buffer_status_t ring_buffer_put(struct ring_buffer *rb, ring_buffer_data_t data);

const ring_buffer_status_t ring_buffer_get(struct ring_buffer *rb, ring_buffer_data_t *data);

const bool ring_buffer_is_empty(const struct ring_buffer *rb);

const bool ring_buffer_is_full(const struct ring_buffer *rb);

const size_t ring_buffer_get_size(const struct ring_buffer *rb);

#ifdef __cplusplus
}
#endif
#endif // RING_BUFFER_H
