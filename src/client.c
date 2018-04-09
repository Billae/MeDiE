#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client_api.h"

int main(int argc, char **argv)
{
    if (argv[1] == NULL || argv[2] == NULL) {
        printf("please give a number of request and a key prefix\n");
        return -1;
    }

    int nb_servers = 4;
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
    
    for(i = 0; i < atoi(argv[1]); i++) {
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

    finalize_context();
    return 0;
}
