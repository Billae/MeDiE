#include <stdio.h>
#include <stdlib.h>
#include <json.h>
#include <errno.h>
#include "distribution.h"
#include "request.h"
#include "protocol.h"


json_object *create_request(const char *key)
{
    json_object *request = json_object_new_object();

    json_object *data_key = json_object_new_string(key);
    json_object_object_add(request, "key", data_key);

    /*ID request generation*/
    int id = 0;
    char strID[10];
    if (snprintf(strID,10, "%d", id) < 0){
        int err = errno;
        fprintf(stderr," %s Error: request ID generation: %s", strID, strerror(err));
    }

    json_object *reqID = json_object_new_string(strID);
    json_object_object_add(request, "reqID", reqID);
    id++;

    return request;
}


json_object *create_request_create(const char *key, const char *data)
{
    json_object *request = create_request(key);

    /*request type: 1 -> create*/
    enum req_type reqType = RT_CREATE;
    json_object *type = json_object_new_int(reqType);
    json_object *str = json_object_new_string(data);

    json_object_object_add(request, "data", str);
    json_object_object_add(request, "reqType", type);

    /*find the server with the distribution method*/
    json_object *hostname = json_object_new_string(AssignSrvByKey(key));
    json_object_object_add(request, "id_srv", hostname);

    return request;
}
