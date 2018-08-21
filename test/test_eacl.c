#include "eacl.h"

#include <stdio.h>
#include <stdlib.h>

static int eacl_dump_sai(struct eacl *eacl)
{
    int rc;
    int i;

    for (i = 0; i < eacl->size; i++) {
        rc =  eacl_read_sai(eacl, i);
        if (rc < 0) {
            fprintf(stderr, "Failed to get sai %d: %s\n",
                    i, strerror(-rc));
            return rc;
        }
        printf("[%d] sai = %d\n", i, rc);
    }

    return 0;
}

void usage_error(const char *bin)
{
    fprintf(stderr, "Usage: %s <eacl_size>\n", bin);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    struct eacl eacl = {0};
    int nb_srv;
    int size;
    int rc;
    int i;

    if (argc != 2)
        usage_error(argv[0]);

    size = atoi(argv[1]);

    if (size <= 0)
        usage_error(argv[0]);

    rc = eacl_init(&eacl, size);
    if (rc) {
        fprintf(stderr, "eacl_init error: %s\n", strerror(-rc));
        exit(EXIT_FAILURE);
    }

    /* dump initial state */
    if (eacl_dump_sai(&eacl))
        exit(EXIT_FAILURE);

    /* initialize random generator */
    srand(getpid() ^ time(NULL));

    /* apply random increments */
    for (i = 0; i < size; i++) {
        int idx = rand() % size;

        eacl_incr_access(&eacl, idx);
    }

    /* updates sai's and dump final state */
    rc = eacl_calculate_sai(&eacl);
    if (rc) {
        fprintf(stderr, "eacl_calculate_sai error: %s\n", strerror(-rc));
        exit(EXIT_FAILURE);
    }

    /* dump final state */
    if (eacl_dump_sai(&eacl))
        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}

/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=4:tabstop=4:
 */
