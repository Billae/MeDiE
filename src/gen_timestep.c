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

#ifdef DISTRIBUTION_WINDOWED
    #include "protocol_windowed.h"
#endif

/*This program generate one step of traces (i.e. one second) for one server.
 * Arguments are:
 * - srv: the id of server asked on this step
 * - n_req: the number of requests needed
 * - distinct_key: the factor of distinct key in the requests
 * - n_srv: the number of available servers  
 * - timestamp: the timestamp to be add before each requests to make traces 
 * - path: the path of the trace
*/

int main(int argc, char *argv[])
{
    if (argc < 6 || argv[1] == NULL || argv[2] == NULL || argv[3] == NULL || argv[4] == NULL || argv[5] == NULL || argv[6] == NULL) {
        fprintf(stderr,
            "Please give the server ID, the number request to create, the distinct_key factor, the number of available servers, a timestamp for this step and the path of the trace file\n");
        return -1;
    }

    int srv_id = atoi(argv[1]);
    int max_request = atoi(argv[2]);
    float factor = atof(argv[3]);
    int nb_srv = atoi(argv[4]);

    char *name;
    int seed = 1;
    uint32_t h_out;

    int rc;
    struct mlt table;
    rc = mlt_init(&table, N_entry, nb_srv);
    if (rc != 0) {
        fprintf(stderr, "mlt init failed\n");
        return -1;
    }

    FILE *fd = fopen(argv[6], "a+");
    if (fd == NULL) {
        fprintf(stderr, "open file failed: %s\n", strerror(errno));
        return -1;
    }

    int all_req=0;
    int n_key = 0;
    unsigned long int k = 0;
    while (n_key < (max_request * factor)) {
        asprintf(&name, "step%s_%ld", argv[5], k++);
        MurmurHash3_x86_32(name, strlen(name), seed, &h_out);
        int index = h_out%N_entry;

        int num_srv, version, state;
        rc = mlt_get_entry(&table, index, &num_srv, &version, &state);
        if (rc != 0) {
            fprintf(stderr, "mlt get entry failed\n");
            return -1;
        }

        if (num_srv == srv_id) {
            int req = 0;
            /*job number is n_key => one key == one job*/
            fprintf(fd, "%s,create,%s,%d\n", argv[5], name, n_key);
            req++;
            all_req++;
            while (req < 1/factor) {
                fprintf(fd, "%s,update,%s,%d\n", argv[5], name, n_key);
                req++;
                all_req++;
                if (all_req >= max_request)
                    break;
            }
            /*fprintf(fd, "delete,%s,%d\n", name, n_key);
            req++;*/
            n_key++;
        }

    }

    /*fprintf(stderr, "-%d-", all_req);*/
    fclose(fd);
    return 0;
}
