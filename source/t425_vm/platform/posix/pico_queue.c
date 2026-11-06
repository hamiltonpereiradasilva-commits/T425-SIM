#include "pico/util/queue.h"

#include <stdlib.h>
#include <string.h>

bool queue_init(queue_t *queue, size_t element_size, size_t element_count)
{
    if (queue == 0 || element_size == 0u || element_count == 0u)
        return false;

    queue->data = calloc(element_count, element_size);
    if (queue->data == 0)
        return false;

    queue->element_size = element_size;
    queue->element_count = element_count;
    queue->head = 0u;
    queue->tail = 0u;
    queue->used = 0u;
    return true;
}

void queue_free(queue_t *queue)
{
    if (queue == 0)
        return;

    free(queue->data);
    queue->data = 0;
    queue->element_size = 0u;
    queue->element_count = 0u;
    queue->head = 0u;
    queue->tail = 0u;
    queue->used = 0u;
}

bool queue_try_add(queue_t *queue, const void *element)
{
    if (queue == 0 || element == 0 || queue_is_full(queue))
        return false;

    memcpy(queue->data + queue->tail * queue->element_size,
           element, queue->element_size);
    queue->tail = (queue->tail + 1u) % queue->element_count;
    queue->used++;
    return true;
}

bool queue_try_remove(queue_t *queue, void *element)
{
    if (queue == 0 || element == 0 || queue_is_empty(queue))
        return false;

    memcpy(element, queue->data + queue->head * queue->element_size,
           queue->element_size);
    queue->head = (queue->head + 1u) % queue->element_count;
    queue->used--;
    return true;
}

bool queue_try_peek(const queue_t *queue, void *element)
{
    if (queue == 0 || element == 0 || queue_is_empty(queue))
        return false;

    memcpy(element, queue->data + queue->head * queue->element_size,
           queue->element_size);
    return true;
}

bool queue_is_empty(const queue_t *queue)
{
    return queue == 0 || queue->used == 0u;
}

bool queue_is_full(const queue_t *queue)
{
    return queue != 0 && queue->used >= queue->element_count;
}
