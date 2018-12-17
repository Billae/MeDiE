#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "murmur3.h"
#include "mlt.h"
#include "protocol.h"


/*This program give the entry assigned for a key.
 * asked key and number of available servers is given in argument.*/
int main(int argc, char *argv[])
{
    if (argc < 2 || argv[1] == NULL || argv[2] == NULL) {
        fprintf(stderr,
            "Please give the filename and the number of available servers\n");
        return -1;
    }

    char *name = argv[1];
    int seed = 1;
    uint32_t h_out;

    int rc;
    struct mlt table;
    rc = mlt_init(&table, N_entry, atoi(argv[2]));
    if (rc != 0) {
        fprintf(stderr, "mlt init failed\n");
        return -1;
    }

    MurmurHash3_x86_32(name, strlen(name), seed, &h_out);
    int index = h_out%N_entry;
    int num_srv, version, state;
    rc = mlt_get_entry(&table, index, &num_srv, &version, &state);
    if (rc != 0) {
        fprintf(stderr, "mlt get entry failed\n");
        return -1;
    }

    fprintf(stderr, "Key %s is assigned to entry %d for server %d\n", name, index, num_srv);
    return 0;
}
