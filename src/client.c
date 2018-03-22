#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <json.h>
#include <string.h>
#include "request.h"


int main(void)
{
    int rc;
    rc = DistributionInit();
    if (rc != 1) {
        fprintf(stderr, "Distribution init failed\n");
        DistributionFinalize();
        return -1;
    }

    /*create request and assign a server*/
    char *key = "Hello";
    json_object *request = create_request_create(key, "World");

    /*open the zmq socket*/
    json_object *host;
    if (!json_object_object_get_ex(request, "id_srv", &host))
        fprintf(stderr, "Error (hostname): no key found\n");

    char *socket;
    if (asprintf(&socket, "tcp://%s", json_object_get_string(host)) == -1) {
        int err = errno;
        fprintf(stderr, "Error format zmq socket name: %s\n", strerror(err));
        DistributionFinalize();
        return -1;
    }

    zsock_t *req = zsock_new_req(socket);
    if (req == NULL) {
        fprintf(stderr, "Error create zmq socket\n");
        DistributionFinalize();
        return -1;
    }
    free(socket);

    /*sending request*/
    const char *req_c = json_object_to_json_string(request);
    zstr_send(req, req_c);

    /*cleaning json object sent*/
    if (json_object_put(request) != 1)
        fprintf(stderr, "Error free request\n");

    /*receiving reply*/
    char *string = zstr_recv(req);
    json_object *reply = json_tokener_parse(string);
    zstr_free(&string);

    /*processing reply*/
    json_object *rep;
    if (!json_object_object_get_ex(reply, "repFlag", &rep))
        fprintf(stderr, "Error (reply): no key found\n");

    if (strcmp(json_object_get_string(rep), "done") == 0)
        printf("operation validated\n");
    else
        printf(" operation failed\n");


    /*cleaning*/
    if (json_object_put(reply) != 1)
        fprintf(stderr, "Error free reply\n");
    zsock_destroy(&req);

    DistributionFinalize();
    return 0;
}

