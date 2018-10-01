#include "mlt.h"

#include <stdio.h>

static int mlt_dump(struct mlt *mlt)
{
    int rc;
    int i;

    for (i = 0; i < mlt->size; i++) {
        int srv, ver, state;
        rc =  mlt_get_entry(mlt, i, &srv, &ver, &state);
        if (rc) {
            fprintf(stderr, "Failed to get raw %d: %s\n",
                    i, strerror(-rc));
            return rc;
        }
        printf("[%d] id_srv = %d, ver = %d, state = %d\n", i, srv, ver, state);
    }

    return 0;
}

void usage_error(const char *bin)
{
    fprintf(stderr, "Usage: %s <mlt_size> <nb_srv>\n", bin);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    struct mlt mlt = {0};
    int nb_srv;
    int size;
    int rc;
    int i;

    if (argc != 3)
        usage_error(argv[0]);

    size = atoi(argv[1]);
    nb_srv = atoi(argv[2]);

    if (size == 0 || nb_srv == 0)
        usage_error(argv[0]);

    rc = mlt_init(&mlt, size, nb_srv);
    if (rc) {
        fprintf(stderr, "mlt_init error: %s\n", strerror(-rc));
        exit(EXIT_FAILURE);
    }

    /* dump initial state */
    if (mlt_dump(&mlt))
        exit(EXIT_FAILURE);

    /* initialize random generator */
    srand(getpid() ^ time(NULL));

    /* apply size random updates */
    for (i = 0; i < size; i++) {
        int change_idx, new_srv, new_ver;

        change_idx = rand() % size;
        new_srv = rand() % nb_srv;
        new_ver = rand() % 1000;

        rc = mlt_update_entry(&mlt, change_idx, new_srv, new_ver, 0);
        if (rc) {
            fprintf(stderr, "mlt_update_entry error: %s\n", strerror(-rc));
            exit(EXIT_FAILURE);
        }
    }

    /* dump final state */
    if (mlt_dump(&mlt))
        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}

/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=4:tabstop=4:
 */
