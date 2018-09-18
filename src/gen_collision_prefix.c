#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "murmur3.h"
#include "mlt.h"

#define N_entry 80
#define iter 100000

/* path in pcocc*/
//#define PATH "/home/billae/prototype_MDS/etc/colliding_id.cfg"
/*path in ocre*/
#define PATH "/ccc/home/cont001/ocre/billae/prototype_MDS/etc/colliding_id.cfg"

/*This program generate prefixs which produce an access to the entry 0.
 * hostname and number of available servers are given in argument.
 * It is used to create collision in dynamic hashing.*/
int main(int argc, char *argv[])
{
    if (argc < 2 || argv[1] == NULL || argv[2] == NULL) {
        fprintf(stderr,
            "Please give the hostname and the number of available servers\n");
        return -1;
    }

    char *name;
    int seed = 1;
    uint32_t h_out;

    int i, rc;
    struct mlt table;
    rc = mlt_init(&table, N_entry, atoi(argv[2]));
    if (rc != 0) {
        fprintf(stderr, "mlt init failed\n");
        return -1;
    }

    FILE *fd = fopen(PATH, "w+");
    if (fd == NULL) {
        fprintf(stderr, "open file failed: %s\n", strerror(errno));
        return -1;
    }

    for (i = 0; i < iter; i++) {
        asprintf(&name, "%s%d", argv[1], i);
        MurmurHash3_x86_32(name, strlen(name), seed, &h_out);
        int index = h_out%N_entry;

        int num_srv, version;
        rc = mlt_get_entry(&table, index, &num_srv, &version);
        if (rc != 0) {
            fprintf(stderr, "mlt get entry failed\n");
            return -1;
        }
        if (num_srv == 0)
            fprintf(fd, "%s\n", name);
    }

    fclose(fd);
    return 0;
}
