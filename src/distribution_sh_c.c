#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <json.h>

#include "distribution_sh_c.h"
#include "murmur3.h"
#include "protocol.h"


static int nb_srv;

int distribution_init(nb)
{
    nb_srv = nb;
    return 0;
}


int distribution_finalize()
{}


int distribution_pre_send(json_object *request)
{
    /*getting the key*/
    json_object *data_key;
    if (!json_object_object_get_ex(request, "key", &data_key)) {
        fprintf(stderr,
            "Distribution:pre_send:json extract error: no key \"key\" found\n");
        return -1;
    }

    errno = 0;
    const char *key = json_object_get_string(data_key);
    if (errno == EINVAL) {
        fprintf(stderr, "Distribution:pre_send: get key error\n");
        return -1;
    }

    /*getting the request type*/
    json_object *type;
    if (!json_object_object_get_ex(request, "reqType", &type)) {
        fprintf(stderr,
            "Distribution:pre_send:json extract error: no key \"reqType\" found\n");
        return -1;
    }

    errno = 0;
    enum req_type reqType = json_object_get_int(type);
    if (errno == EINVAL) {
        fprintf(stderr, "Distribution:pre_send: get reqType error\n");
        return -1;
    }

    if (reqType == RT_CREATE) 
        distribution_assign_srv_by_key(key, request);

    return 0;
}


int distribution_post_receive(json_object *reply)
{}


int distribution_assign_srv_by_key(const char *key, json_object *request)
{
    int seed = 1;
    uint32_t num_srv;
    MurmurHash3_x86_32(key, strlen(key), seed, &num_srv);
    /*printf("server to query: %d\n", num_srv%nb_srv);*/

    json_object *host = json_object_new_int(num_srv%nb_srv);
    json_object_object_add(request, "id_srv", host);

    return 0;
}
