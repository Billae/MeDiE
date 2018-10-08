#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "client_api.h"

#define BILLION  1000000000L
#define NB_REQUESTS 10000
#define MAX_SIZE_ID 20
/* path in pcocc*/
//#define ID_PATH "/home/billae/prototype_MDS/etc/colliding_id.cfg"
/*path in ocre*/
#define ID_PATH "/ccc/home/cont001/ocre/billae/prototype_MDS/etc/colliding_id.cfg"

int main(int argc, char **argv)
{
    if (argv[1] == NULL || argv[2] == NULL) {
        fprintf(stderr, "please give a number of servers and a key prefix\n");
        return -1;
    }

    int nb_servers = atoi(argv[1]);
    char *data = "a word";

    /*init context*/
    int rc;
    rc = client_init_context(nb_servers);
    if (rc < 1) {
        fprintf(stderr, "Context init failed\n");
        client_finalize_context();
        return -1;
    }

    /*create pattern to request different key each time*/
    char *key;
    int i = 0;
   /*use file for collisions*/
    FILE *fd = fopen(ID_PATH, "r");
    char *key_list[NB_REQUESTS];
    for (i = 0; i < NB_REQUESTS; i++) {
        key_list[i] = malloc(MAX_SIZE_ID*sizeof(char));
        if (fgets(key_list[i], MAX_SIZE_ID, fd) == NULL) {
            fprintf(stderr, "key reading failed\n");
            client_finalize_context();
            return -1;
        }
        char *positionEntree = strchr(key_list[i], '\n');
        if (positionEntree != NULL)
            *positionEntree = '\0';
    }



    struct timespec start, end;

    rc = clock_gettime(CLOCK_REALTIME, &start);
    if (rc)
        printf("Client: getting time error\n");

    for (i = 0; i < NB_REQUESTS; i++) {
        /*for prefix use*/
        /*    if (asprintf(&key, "%s%d", argv[2], i) == -1) {
            int err = errno;
            fprintf(stderr, "Client: generating key error:%s\n", strerror(err));
            client_finalize_context();
            return -1;
        }

        rc = client_request_create(key, data);*/
        /*for collision use*/
        rc = client_request_create(key_list[i], data);
        if (rc != 0)
            fprintf(stderr, "Request failed: %s\n", strerror(-rc));
            if (rc == -EAGAIN)
                i--;
        free(key);
    }

    rc = clock_gettime(CLOCK_REALTIME, &end);
    if (rc)
        printf("CLient: getting time error\n");

    double accum = (end.tv_sec - start.tv_sec)
                  + (end.tv_nsec - start.tv_nsec) / (float) BILLION;

// warning: the stdout stream is catched to get time value, don't flood it!
    printf("%lf", accum);
    for (i = 0; i < NB_REQUESTS; i++)
        free(key_list[i]);
    client_finalize_context();
    return 0;
}
