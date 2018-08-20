#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <json.h>
#include <string.h>
#include <errno.h>

#include "generic_storage.h"
#include "protocol.h"

#ifdef DISTRIBUTION_SH
    #include "distribution_sh_s.h"
#endif
#ifdef DISTRIBUTION_DH
    #include "distribution_dh_s.h"
#endif

/*TO DO*/
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
            fprintf(stderr, "Server: create zmq socket error\n");
            return -1;
        }
    }
    distribution_init();

while (1) {

    int global_rc = 1;

    /*receiving request*/
    char *string = zstr_recv(rep);
    json_object *request = json_tokener_parse(string);
    zstr_free(&string);

    /*call the distribution processing*/
    distribution_post_receive(request);
    
    /*processing*/
    json_object *key;
    if (!json_object_object_get_ex(request, "key", &key))
        fprintf(stderr, "Server: json extract error: no key \"key\" found\n");

    json_object *type;
    if (!json_object_object_get_ex(request, "reqType", &type))
        fprintf(stderr, "Server: json extract error: no key \"reqType\" found\n");

    enum req_type reqType = json_object_get_int(type);

    switch (reqType) {

    case RT_CREATE: //create
    {   json_object *data;
        if (!json_object_object_get_ex(request, "data", &data))
            fprintf(stderr, "Server: json extract error: no key \"data\" found\n");
        int rc;
        rc = generic_put(json_object_get_string(data),
                json_object_get_string(key));
        if (rc != 1) {
            fprintf(stderr, "Server: generic storage operation error\n");
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

    distribution_pre_send(request);
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
        fprintf(stderr, "Server: free request error\n");
}

    distribution_finalize();
    zsock_destroy(&rep);
    return 0;
}
