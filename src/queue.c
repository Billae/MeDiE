#include <errno.h>
#include <stdlib.h>

#include "queue.h"


struct queue *
queue_new(size_t count)
{
    struct queue *queue;

    queue = malloc(sizeof(*queue));
    if (queue == NULL)
        return NULL;

    queue->values = malloc(count * sizeof(*queue->values));
    if (queue->values == NULL) {
        int save_errno = errno;

        free(queue);
        errno = save_errno;
        return NULL;
    }

    queue->count = count;
    queue->used = 0;
    queue->next = 0;

    return queue;
}

void
queue_put(struct queue *queue, uint32_t value)
{
    queue->values[queue->next] = value;

    if (queue->used < queue->count)
        queue->used++;

    if (queue->next == queue->count - 1)
        queue->next = 0;
    else
        queue->next++;
}

uint32_t
queue_mean(struct queue *queue)
{
    if (queue->used == 0)
        return 0;
    size_t count = queue->used < queue->count ? queue->next : queue->count;
    uint64_t sum = 0;
    size_t i;

    for (i = 0; i < count; i++)
        sum += queue->values[i];

    return sum / count;
}

void
queue_reset(struct queue *queue)
{
    int i;
    for (i = 0; i < queue->used; i++)
        queue->values[i] = 0;

    queue->used = 0;
    queue->next = 0;
}

void
queue_destroy(struct queue *queue)
{
    free(queue->values);
    free(queue);
}
