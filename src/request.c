#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <json.h>
#include <errno.h>
#include <string.h>

#include "request.h"

#ifdef DISTRIBUTION_SH
    #include "distribution_sh_c.h"
    #include "protocol_sh.h"
#endif

#ifdef DISTRIBUTION_DH
    #include "distribution_dh_c.h"
    #include "protocol_dh.h"
#endif

#ifdef DISTRIBUTION_INDEDH
    #include "distribution_indedh_c.h"
    #include "protocol_indedh.h"
#endif



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
        fprintf(stderr,
                "Request:create_request: request ID generation error: %s\n",
                strerror(err));
        return NULL;
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
    if (request == NULL)
        return NULL;

    enum req_type reqType = RT_CREATE;
    json_object *type = json_object_new_int(reqType);
    json_object_object_add(request, "reqType", type);

    json_object *str = json_object_new_string(data);
    json_object_object_add(request, "data", str);

    return request;
}


json_object *create_request_read(const char *key)
{
    json_object *request = create_request(key);
    if (request == NULL)
        return NULL;

    enum req_type reqType = RT_READ;
    json_object *type = json_object_new_int(reqType);
    json_object_object_add(request, "reqType", type);

    return request;
}


json_object *create_request_update(const char *key, const char *data)
{
    json_object *request = create_request(key);
    if (request == NULL)
        return NULL;

    enum req_type reqType = RT_UPDATE;
    json_object *type = json_object_new_int(reqType);
    json_object_object_add(request, "reqType", type);

    json_object *str = json_object_new_string(data);
    json_object_object_add(request, "data", str);

    return request;
}


json_object *create_request_delete(const char *key)
{
    json_object *request = create_request(key);
    if (request == NULL)
        return NULL;

    enum req_type reqType = RT_DELETE;
    json_object *type = json_object_new_int(reqType);
    json_object_object_add(request, "reqType", type);

    return request;
}
