#define _GNU_SOURCE

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
    int rc;
    /*ocre usage need different port for each server*/
    if (argv[1] == NULL || argv[2] == NULL ||
        (strcmp(argv[2], "o") != 0 && strcmp(argv[2], "p") != 0)) {

        fprintf(stderr,
            "please give the number of available servers and a server type o or p\n");
        return -1;
    }

    zsock_t *rep;

    if (strcmp(argv[2], "p") == 0) {
        /*for pcocc VM*/
        char *socket;
        if (asprintf(&socket, "tcp://0.0.0.0:%d", Client_port) == -1) {
            int err = errno;
            fprintf(stderr,
                "Server: format zmq socket name error: %s\n", strerror(err));
            return -1;
        }

        rep = zsock_new_rep(socket);
        if (rep == NULL) {
            fprintf(stderr, "Server: create zmq socket error\n");
            return -1;
        }

    }

    else {
        /*for ocre*/
        char *name;
        asprintf(&name, "tcp://192.168.129.25:%d", Client_port);
        rep = zsock_new_rep(name);
        if (rep == NULL) {
            fprintf(stderr, "Server: create zmq socket error\n");
            return -1;
        }
    }

    rc = distribution_init(atoi(argv[1]));
    if (rc != 0) {
        fprintf(stderr, "Server: distribution init failed\n");
        zsock_destroy(&rep);
        return -1;
    }

    int global_rc;

while (1) {

    global_rc = 0;

    /*receiving request*/
    char *string = zstr_recv(rep);
    json_object *request = json_tokener_parse(string);
    zstr_free(&string);

    /*call the distribution processing*/
    rc = distribution_post_receive(request);
    if (rc == -EAGAIN) {
        /*mlt out of date*/
        global_rc = -EAGAIN;
        goto reply;
    } else if (rc == -EALREADY) {
        /*operation on mlt. wait and retry*/
        global_rc = -EALREADY;
        goto reply;
    } else if (rc < 0) {
        fprintf(stderr, "Server: distribution_post_receive failed\n");
        global_rc = -1;
        goto reply;
    }

    /*processing*/
    json_object *key;
    if (!json_object_object_get_ex(request, "key", &key))
        fprintf(stderr, "Server: json extract error: no key \"key\" found\n");

    json_object *type;
    if (!json_object_object_get_ex(request, "reqType", &type))
        fprintf(stderr,
                "Server: json extract error: no key \"reqType\" found\n");

    enum req_type reqType = json_object_get_int(type);

    switch (reqType) {

    case RT_CREATE: /*create*/
    {
        json_object *data;
        if (!json_object_object_get_ex(request, "data", &data))
            fprintf(stderr,
                "Server: json extract error: no key \"data\" found\n");
        rc = generic_put(json_object_get_string(key),
        json_object_get_string(data));
        if (rc != 0) {
            fprintf(stderr, "Server: generic storage operation error\n");
            global_rc = -1;
        }
        break;
    }

    case RT_UPDATE: /*update*/
    {
        json_object *data;
        if (!json_object_object_get_ex(request, "data", &data))
            fprintf(stderr,
                "Server: json extract error: no key \"data\" found\n");
        rc = generic_update(json_object_get_string(key),
        json_object_get_string(data));
        if (rc != 0) {
            fprintf(stderr, "Server: generic storage operation error\n");
            global_rc = -1;
        }
        break;
    }

    case RT_DELETE: /*delete*/
    {
        rc = generic_del(json_object_get_string(key));
        if (rc != 0) {
            fprintf(stderr, "Server: generic storage operation error\n");
            global_rc = -1;
        }
        break;
    }

    default: /*get*/
    {
        char *get_value = generic_get(json_object_get_string(key));
        if (get_value == NULL) {
            fprintf(stderr, "Server: generic storage operation error\n");
            global_rc = -1;
        }
        json_object *get_value_reply = json_object_new_string(get_value);
        json_object_object_add(request, "getValue", get_value_reply);
        break;
    }
    }
    /*creating reply and send*/
reply:
    rc = distribution_pre_send(request, global_rc);
    if (rc != 0) {
        fprintf(stderr, "Server: distribution_pre_send failed\n");
        global_rc = -1;
    }

    json_object *repFlag;
    if (global_rc == 0)
        repFlag = json_object_new_string("done");
    else if (global_rc == -EAGAIN)
        repFlag = json_object_new_string("update&retry");
    else if (global_rc == -EALREADY)
        repFlag = json_object_new_string("wait&retry");
    else
        repFlag = json_object_new_string("aborted");


    json_object_object_add(request, "repFlag", repFlag);

    const char *rep_c = json_object_to_json_string(request);
    zstr_send(rep, rep_c);

    /*cleaning*/
    if (json_object_put(request) != 1)
        fprintf(stderr, "Server: free request error\n");
}

    rc = distribution_finalize();
    if (rc != 0)
        fprintf(stderr, "Server: distribution finalize failed\n");

    zsock_destroy(&rep);
    return 0;
}
