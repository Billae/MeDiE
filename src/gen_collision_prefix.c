#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "murmur3.h"
#include "mlt.h"


#ifdef DISTRIBUTION_DH
    #include "protocol_dh.h"
#endif

#ifdef DISTRIBUTION_INDEDH
    #include "protocol_indedh.h"
#endif



#define factor_distinct_key (0.01)

/*max_request is the mean number of request received by one server (among 4) in real traces*/
#define max_request 2000000

/* path in pcocc*/
#define PATH "/home/billae/prototype_MDS/etc/colliding_id.cfg"
/*path in ocre*/
//#define PATH "/ccc/home/cont001/ocre/billae/prototype_MDS/etc/colliding_id.cfg"

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

    int i = 0;
    int rc;
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

    unsigned long int k = 0;
    int n_key = 0;
    while (n_key < (max_request * factor_distinct_key)) {
        asprintf(&name, "%s%ld", argv[1], k++);
        MurmurHash3_x86_32(name, strlen(name), seed, &h_out);
        int index = h_out%N_entry;

        int num_srv, version, state;
        rc = mlt_get_entry(&table, index, &num_srv, &version, &state);
        if (rc != 0) {
            fprintf(stderr, "mlt get entry failed\n");
            return -1;
        }
        if (num_srv == 0) {
            i = 0;
            fprintf(fd, "create,%s\n", name);
            i++;
            while (i < (1/factor_distinct_key)) {
                fprintf(fd, "update,%s\n", name);
                i++;
            }
            /*fprintf(fd, "delete,%s\n", name);*/
            n_key++;
        }
    }

    fclose(fd);
    return 0;
}
