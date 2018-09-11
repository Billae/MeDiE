#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <json.h>

#include "distribution_dh_c.h"
#include "mlt.h"
#include "protocol.h"
#include "murmur3.h"


/*Each client has its own mlt accessed only in the distribution functions*/
static struct mlt table;


int distribution_init(nb)
{
    int rc = mlt_init(&table, N_entry, nb);
    if (rc != 0) {
        fprintf(stderr, "Distribution:init: mlt init error: %s\n",
            strerror(-rc));
        return -1;
    }
    return 0;
}


int distribution_finalize()
{
    int rc = mlt_destroy(&table);
    if (rc != 0)
        fprintf(stderr, "Distribution:finalize: mlt_destroy failed\n");
    return 0;

}


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

    if (reqType == RT_CREATE) {
        int rc = distribution_assign_srv_by_key(key, request);
        if (rc != 0) {
            fprintf(stderr,
                "Distribution:pre_send: assignation server failed\n");
            return -1;
        }
    }
    return 0;
}


int distribution_post_receive(json_object *reply)
{
    /*verifying the version number*/
    json_object *ver_flag;
    if (!json_object_object_get_ex(reply, "versionFlag", &ver_flag)) {
        fprintf(stderr,
            "Distribution:post_receive: json extract error: no key \"version_flag\" found\n");
        return -1;
    }

    /*mlt out of date*/
    if (strcmp(json_object_get_string(ver_flag), "up-to-date") !=  0) {
        /* updating the local mlt*/
        json_object *srv_true;
        if (!json_object_object_get_ex(reply, "newSrv", &srv_true)) {
            fprintf(stderr,
                "Distribution:post_receive: json extract error: no key \"newSrv\" found\n");
            return -1;
        }

        json_object *ver_true;
        if (!json_object_object_get_ex(reply, "newVersion", &ver_true)) {
            fprintf(stderr,
                "Distribution:post_receive: json extract error: no key \"newVersion\" found\n");
            return -1;
        }

        json_object *index;
        if (!json_object_object_get_ex(reply, "index", &index)) {
            fprintf(stderr,
                "Distribution:post_receive: json extract error: no key \"index\" found\n");
            return -1;
        }

        int rc = mlt_update_entry(&table, json_object_get_int(index),
                json_object_get_int(srv_true), json_object_get_int(ver_true));
        if (rc != 0) {
            fprintf(stderr,
                "Distribution:post_receive: mlt update failed: %s\n",
                strerror(-rc));
            return -1;
        }

    }
    return 0;
}


int distribution_assign_srv_by_key(const char *key, json_object *request)
{
    int seed = 1;
    uint32_t h_out;
    MurmurHash3_x86_32(key, strlen(key), seed, &h_out);
    int index = h_out%N_entry;

    int num_srv;
    int version;
    int rc = mlt_get_entry(&table, index, &num_srv, &version);
    if (rc != 0) {
        fprintf(stderr,
            "Distribution:assign_srv_by_key: mlt error: %s\n",
                strerror(-rc));
        return -1;
    }


    json_object *host = json_object_new_int(num_srv);
    json_object_object_add(request, "id_srv", host);

    json_object *ind = json_object_new_int(index);
    json_object_object_add(request, "index", ind);


    json_object *ver = json_object_new_int(version);
    json_object_object_add(request, "entryVersion", ver);

    return 0;
}
