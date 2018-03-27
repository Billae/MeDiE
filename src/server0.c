#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <json.h>
#include <string.h>
#include "generic_storage.h"
#include "protocol.h"

int main(void)
{
    /*for pcocc VM*/
    //zsock_t *rep = zsock_new_rep("tcp://10.252.0.1:7410");

    /*for ocre*/
    zsock_t *rep = zsock_new_rep("tcp://192.168.129.25:7410");

//while(1)
//{

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
//}
    zsock_destroy(&rep);
    return 0;
}
