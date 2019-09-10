#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

struct queue;

/**
 * Create a new queue
 *
 * @param count     the number of elements the created queue can contain
 * @return          a pointer to a newly allocated queue on success, NULL on
 *                  error and errno is set appropriately
 *
 * @errno ENOMEM    there was not enough memory available
 */
struct queue *
queue_new(size_t count);

/**
 * Put a value in the queue
 *
 * @param queue     the queue to put \p value in
 * @param value     the value to put in \p queue
 *
 * If the queue is full, this call will discard the oldest element in \p queue
 */
void
queue_put(struct queue *queue, uint32_t value);

/**
 * Compute the mean of the elements in a queue
 *
 * @param queue     the queue whose mean to compute
 * @return          the mean of the elements in \p queue
 *
 * By convenience, the mean of an empty queue is defined to be 0
 */
uint32_t
queue_mean(struct queue *queue);

/**
 * Delete a queue and reclaim its resources
 *
 * @param queue     the queue to delete
 */
void
queue_destroy(struct queue *queue);

#endif
