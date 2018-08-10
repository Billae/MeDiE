#include <stdio.h>
#include <stdlib.h>
#include <json.h>
#include <errno.h>
#include "distribution_dh_c.h"
#include "request.h"
#include "protocol.h"

/*TO DO*/
json_object *create_request(const char *key)
{
    json_object *request = json_object_new_object();

    json_object *data_key = json_object_new_string(key);
    json_object_object_add(request, "key", data_key);

    /*ID request generation*/
    static int id;
    char *strID;
    if (asprintf(&strID, "%d", id) == -1) {
        int err = errno;
        fprintf(stderr, "Request:create_request: request ID generation error: %s\n", strerror(err));
    }
    json_object *reqID = json_object_new_string(strID);
    json_object_object_add(request, "reqID", reqID);
    id++;

    free(strID);
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

    /*find the server number with the distribution method*/
    json_object *host = json_object_new_int(distribution_assign_srv_by_key(key));
    json_object_object_add(request, "id_srv", host);

    return request;
}
