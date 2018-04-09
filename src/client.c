#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client_api.h"

int main(int argc, char **argv)
{
    if (argv[1] == NULL || argv[2] == NULL) {
        printf("please give a key and data to store\n");
        return -1;
    }

    int nb_servers = 4;

    /*init context*/
    int rc;
    rc = init_context(nb_servers);
    if (rc < 1) {
        fprintf(stderr, "Context init failed\n");
        finalize_context();
        return -1;
    }

    /*create pattern to request different key each time*/

    /**/

    rc = request_create(argv[1], argv[2]);
    if (rc != 0)
        fprintf(stderr, "Request failed\n");

    finalize_context();
    return 0;
}
