#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <json.h>
#include <string.h>
#include "generic_storage.h"
#include "protocol.h"

int main(int argc, char **argv)
{
    /*ocre usage need different port for each server*/
    if (argv[1] == NULL || strcmp(argv[1], "o") != 0 && strcmp(argv[1], "p") != 0) {
        fprintf(stderr, "please give a server type o or p (and port if type is o)\n");
        return -1;
    }

    zsock_t *rep;

    if (strcmp(argv[1], "p") == 0) {
    /*for pcocc VM*/
        rep = zsock_new_rep("tcp://0.0.0.0:7410");
    }

    else {
    /*for ocre*/
        if (argv[2] == NULL) {
            fprintf(stderr, "please give a server port\n");
            return -1;
        }

        char *name;
        asprintf(&name, "tcp://192.168.129.25:%s", argv[2]);
        rep = zsock_new_rep(name);
        if (rep == NULL) {
            fprintf(stderr, "Error create zmq socket\n");
            return -1;
        }
    }


while (1) {

    int global_rc = 1;

    /*receiving request*/
    char *string = zstr_recv(rep);
    json_object *request = json_tokener_parse(string);
    zstr_free(&string);

    /*processing*/
    json_object *key;
    if (!json_object_object_get_ex(request, "key", &key))
        fprintf(stderr, "Error: no key found\n");

    json_object *type;
    if (!json_object_object_get_ex(request, "reqType", &type))
        fprintf(stderr, "Error: no key found\n");

    enum req_type reqType = json_object_get_int(type);

    switch (reqType) {

    case RT_CREATE: //create
    {   json_object *data;
        if (!json_object_object_get_ex(request, "data", &data))
            fprintf(stderr, "Error: no key found\n");
        int rc;
        rc = generic_put(json_object_get_string(data),
                json_object_get_string(key));
        if (rc != 1) {
            fprintf(stderr, "Error generic storage operation\n");
            global_rc = -1;
            break;
        }
    }

    case RT_UPDATE: //update
        break;

    case RT_DELETE: //delete
        break;

    default: //get
        break;
    }

    /*creating reply and send*/

    json_object *repFlag;
    if (global_rc == 1)
        repFlag = json_object_new_string("done");
    else
        repFlag = json_object_new_string("aborted");


    json_object_object_add(request, "repFlag", repFlag);

    const char *rep_c = json_object_to_json_string(request);
    zstr_send(rep, rep_c);

    /*cleaning*/
    if (json_object_put(request) != 1)
        fprintf(stderr, "Error free request");
}

    zsock_destroy(&rep);
    return 0;
}
