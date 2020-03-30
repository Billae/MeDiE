#include <errno.h>
#include <error.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#include "queue.h"

#ifndef COUNT
    #define COUNT 10
#endif

int
main(int argc, char *argv[])
{
    struct queue *queue;
    uint32_t *numbers;

    if (argc < 2)
        error(EX_USAGE, 0, "not enough arguments");

    numbers = calloc(argc - 1, sizeof(*numbers));
    if (numbers == NULL)
        error(EXIT_FAILURE, errno, "calloc");
    size_t i;
    for (i = 1; i < argc; i++)
        /* This is not safe, but it does not matter, this is only a demo */
        numbers[i - 1] = atoi(argv[i]);

    queue = queue_new(COUNT);
    if (queue == NULL)
        error(EXIT_FAILURE, errno, "queue_new");

    for (i = 0; i < argc - 1; i++)
        queue_put(queue, numbers[i]);

    printf("mean: %"PRIu32"\n", queue_mean(queue));

    return EXIT_SUCCESS;
}
