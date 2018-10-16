#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "client_api.h"

int main(int argc, char *argv[])
{
    if (argv[1] == NULL) {
        fprintf(stderr, "please give a number of servers\n");
        return -1;
    }

    int nb_servers = atoi(argv[1]);

    /*init context*/
    int rc;
    rc = client_init_context(nb_servers);
    if (rc < 1) {
        fprintf(stderr, "Context init failed\n");
        client_finalize_context();
        return -1;
    }

    char *key = "test";
    char *data = "data";
    rc = client_request_create(key, data);
    if (rc != 0)
        fprintf(stderr, "Request created failed: %s\n", strerror(-rc));

    fprintf(stderr, "Request created\n");
    char *reply = NULL;
    rc = client_request_read(key, &reply);
    if (rc != 0)
        fprintf(stderr, "Request read failed: %s\n", strerror(-rc));
    else
        fprintf(stderr, "request: %s\n", reply);

    char *change = "another";
    rc = client_request_update(key, change);
    if (rc != 0)
        fprintf(stderr, "Request update failed: %s\n", strerror(-rc));

    fprintf(stderr, "Request updated\n");
    char *reply1 = NULL;
    rc = client_request_read(key, &reply1);
    if (rc != 0)
        fprintf(stderr, "Request read failed: %s\n", strerror(-rc));
    else
        fprintf(stderr, "request: %s\n", reply1);

    rc = client_request_delete(key);
    if (rc != 0)
        fprintf(stderr, "Request delete failed: %s\n", strerror(-rc));

    fprintf(stderr, "Request deleted\n");

    client_finalize_context();
    return 0;
}
