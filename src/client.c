#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <json.h>
#include <string.h>
#include "request.h"


int main (void)
{
    char *key= "Hello";
    
    json_object *request = create_request_create(key, "World");

    //open the zmq socket
    json_object *host;
     if(!json_object_object_get_ex(request, "id_srv", &host))
        printf("Error (hostname): no key found\n");
    zsock_t *req = zsock_new_req(json_object_get_string(host));
    // sending request
    const char *req_c = json_object_to_json_string(request);
    zstr_send(req, req_c);

    //cleaning json object sent
    if(json_object_put(request) != 1)
        printf("error free reply");

    //receiving reply
    char *string = zstr_recv(req);
    json_object *reply = json_tokener_parse(string);
    zstr_free(&string);

    //processing reply
    json_object *rep;
    if(!json_object_object_get_ex(reply, "repFlag", &rep))
        printf("Error (reply): no key found\n");
    if(strcmp(json_object_get_string(rep),"done")==0)
        printf("operation validated\n");
    
    //cleaning
    if(json_object_put(reply) != 1)
        printf("error free reply");
    zsock_destroy(&req);

    return 0;
}

