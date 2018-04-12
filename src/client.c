#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "client_api.h"


#define BILLION  1000000000L
#define NB_REQUESTS 10000

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
    rc = init_context(nb_servers);
    if (rc < 1) {
        fprintf(stderr, "Context init failed\n");
        finalize_context();
        return -1;
    }

    /*create pattern to request different key each time*/
    char *key;
    int i = 0;
    
    struct timespec start, end;

    rc = clock_gettime(CLOCK_REALTIME, &start);
    if (rc)
        printf("Client: getting time error\n");

    for(i = 0; i < NB_REQUESTS; i++) {
        if (asprintf(&key, "%s%d",argv[2],i) == -1) {
            int err = errno;
            fprintf(stderr, "Client: generating key error:%s\n",strerror(err));
            finalize_context();
            return -1;
        }

        rc = request_create(key, data);
        if (rc != 0)
            fprintf(stderr, "Request failed\n");
    }
    
    rc = clock_gettime(CLOCK_REALTIME, &end);
    if (rc)
        printf("CLient: getting time error\n");

    double accum = (end.tv_sec - start.tv_sec)
                  + (end.tv_nsec - start.tv_nsec)/ (float) BILLION;

    printf("time = %lf\n", accum);
    finalize_context();
    return 0;
}
