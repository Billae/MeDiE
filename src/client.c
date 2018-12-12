#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "client_api.h"

#define BILLION  1000000000L
#define MAX_SIZE 50


int main(int argc, char **argv)
{
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL) {
        fprintf(stderr, "please give a number of servers, the number of requests and the path of trace file\n");
        return -1;
    }

    int nb_servers = atoi(argv[1]);
    int nb_req = atoi(argv[2]);
    char *path = argv[3];
    char *data = "a word";

    /*init context*/
    int rc;
    rc = client_init_context(nb_servers);
    if (rc < 1) {
        fprintf(stderr, "Context init failed\n");
        client_finalize_context();
        return -1;
    }

    FILE *fd = fopen(path, "r");
    if (fd == NULL) {
        int err = errno;
        fprintf(stderr, "Client: request file %s open error:%s\n",
            path, strerror(err));
        return -1;
    }

    char *key_list[nb_req];
    int type_list[nb_req];


    char *a_line = malloc(MAX_SIZE * sizeof(char));
    int current_req;
    for (current_req = 0; current_req < nb_req; current_req++) {

        /*use file for collisions*/
/*        key_list[current_req] = malloc(MAX_SIZE*sizeof(char));
        if (fgets(key_list[current_req], MAX_SIZE, fd) == NULL) {
            fprintf(stderr, "key reading failed\n");
            client_finalize_context();
            return -1;
        }
        char *positionEntree = strchr(key_list[current_req], '\n');
        if (positionEntree != NULL)
            *positionEntree = '\0';
    }
*/
        /*for trace file*/
        if (fgets(a_line, MAX_SIZE, fd) == NULL) {
            int err = errno;
            fprintf(stderr, "Client: read request file %s failed: %s\n",
                path, strerror(err));
            client_finalize_context();
            return -1;
        }
        char *positionEntree = strchr(a_line, '\n');
        if (positionEntree != NULL)
            *positionEntree = '\0';

        /*parse the line*/
        char *type = strtok(a_line, ",");
        if (strcmp(type, "create") == 0)
            type_list[current_req] = 1;
        if (strcmp(type, "get") == 0)
            type_list[current_req] = 2;
        if (strcmp(type, "update") == 0)
            type_list[current_req] = 3;
        if (strcmp(type, "delete") == 0)
            type_list[current_req] = 4;

        char *key = strtok(NULL, ",");
        asprintf(&key_list[current_req], "%s", key);

        /*fprintf(stderr, "-%s- -%d-\n",
         * key_list[current_req], type_list[current_req]);*/
    }
    free(a_line);

    struct timespec start, end;

    rc = clock_gettime(CLOCK_REALTIME, &start);
    if (rc)
        printf("Client: getting time error\n");

    /*for collision create file*/
/*
    for (current_req = 0; current_req < nb_req; current_req++) {
        rc = client_request_create(key_list[current_req], data);
        if (rc != 0)
            fprintf(stderr, "Request failed: %s\n", strerror(-rc));
            if (rc == -EAGAIN)
                current_req--;
    }
*/
    /*for trace file*/
    for (current_req = 0; current_req < nb_req; current_req++) {
        switch (type_list[current_req]) {
        case 1: {
            rc = client_request_create(key_list[current_req], data);
            if (rc != 0)
                fprintf(stderr, "Client: Request for key %s failed: %s\n",
                    key_list[current_req], strerror(-rc));
            if (rc == -EAGAIN || rc == -EALREADY)
                current_req--;
            break;
        }

        case 2: {
            char *value;
            rc = client_request_read(key_list[current_req], &value);
            if (rc != 0)
                fprintf(stderr, "Client: Request for key %s failed: %s\n",
                    key_list[current_req], strerror(-rc));
            if (rc == -EAGAIN)
                current_req--;
            break;
        }

        case 3: {
            rc = client_request_update(key_list[current_req], data);
            if (rc != 0)
                fprintf(stderr, "Client: Request for key %s failed: %s\n",
                        key_list[current_req], strerror(-rc));
            if (rc == -EAGAIN)
                current_req--;
            break;
        }

        case 4: {
            rc = client_request_delete(key_list[current_req]);
            if (rc != 0)
                fprintf(stderr, "Client: Request for key %s failed: %s\n",
                        key_list[current_req], strerror(-rc));
            if (rc == -EAGAIN)
                current_req--;
            break;
        }

        }
    }
    rc = clock_gettime(CLOCK_REALTIME, &end);
    if (rc)
        printf("CLient: getting time error\n");

    double accum = (end.tv_sec - start.tv_sec)
                  + (end.tv_nsec - start.tv_nsec) / (float) BILLION;

// warning: the stdout stream is catched to get time value, don't flood it!
    printf("%lf", accum);

    for (current_req = 0; current_req < nb_req; current_req++)
        free(key_list[current_req]);

    client_finalize_context();
    fclose(fd);
    return 0;
}
