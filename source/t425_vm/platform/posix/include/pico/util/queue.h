#ifndef T425_POSIX_PICO_QUEUE_H
#define T425_POSIX_PICO_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint8_t *data;
    size_t element_size;
    size_t element_count;
    size_t head;
    size_t tail;
    size_t used;
} queue_t;

bool queue_init(queue_t *queue, size_t element_size, size_t element_count);
void queue_free(queue_t *queue);
bool queue_try_add(queue_t *queue, const void *element);
bool queue_try_remove(queue_t *queue, void *element);
bool queue_try_peek(const queue_t *queue, void *element);
bool queue_is_empty(const queue_t *queue);
bool queue_is_full(const queue_t *queue);

#endif
