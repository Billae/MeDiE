#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <json.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "generic_storage.h"
#include "protocol.h"

#ifdef DISTRIBUTION_SH
    #include "distribution_sh_s.h"
#endif
#ifdef DISTRIBUTION_DH
    #include "distribution_dh_s.h"
#endif


/* path in pcocc*/
//#define PREFIX "/mnt/server/"
/*path in ocre*/
#define PREFIX "/ccc/home/cont001/ocre/billae/prototype_MDS/"


#define max_id_size 21
/*counter to have load of each server and its lock*/
static volatile sig_atomic_t access_count;

static int id_self;

zsock_t *rep;

void int_handler(int sig)
{
    int rc;
    fprintf(stderr, "Server: SigInt received, terminating...\n");
    rc = distribution_finalize();
    if (rc != 0)
        fprintf(stderr, "Server: distribution finalize failed\n");
    zsock_destroy(&rep);
    exit(0);
}

/*catch the load of the server*/
void usr1_handler(int sig)
{
    /*open the result file*/
    char *result_path;
    asprintf(&result_path, "%sload%d", PREFIX, id_self);

    int fd_res = open(result_path, O_WRONLY | O_APPEND | O_CREAT, 0664);
    if (fd_res == -1) {
        int err = errno;
        fprintf(stderr, "Server:signal handler: open error: %s\n",
            strerror(err));
        free(result_path);
        return;
    }
    free(result_path);

    char *load;
    asprintf(&load, "%d\n", access_count);

    if ((write(fd_res, load, strlen(load))) < 0) {
        int err = errno;
        fprintf(stderr, "Server:signal handler: ");
        fprintf(stderr, "write error: %s\n", strerror(err));
        return;
    }
    free(load);

    access_count = 0;

    close(fd_res);
    return;
}


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
        free(socket);

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
        free(name);
    }

    rc = distribution_init(atoi(argv[1]));
    if (rc != 0) {
        fprintf(stderr, "Server: distribution init failed\n");
        zsock_destroy(&rep);
        return -1;
    }

    /*fill id_self*/
    char *srv_path;
    asprintf(&srv_path, "%setc/server.cfg", PREFIX);
    char *id_str = malloc(max_id_size);
    if (id_str == NULL) {
        int err = errno;
        fprintf(stderr, "Server: id_str malloc error: %s\n",
            strerror(err));
        return -1;
    }

    int fd_srv = open(srv_path, O_RDONLY);
    if (fd_srv == -1) {
        int err = errno;
        fprintf(stderr, "Server: open error: %s\n",
            strerror(err));
        free(srv_path);
        return -1;
    }

    free(srv_path);

    rc = read(fd_srv, id_str, max_id_size);
    if (rc < 0) {
        fprintf(stderr, "Server: read config file failed\n");
        return -1;
    }

    char *positionEntree = strchr(id_str, '\n');
    if (positionEntree != NULL)
        *positionEntree = '\0';

    char *value = strstr(id_str, "ID");
    value = strchr(value, '=');
    value++;
    id_self = atoi(value);

    free(id_str);
    close(fd_srv);


    int global_rc;

    /*launch the perf watcher*/
    access_count = 0;

    struct sigaction act_usr1;
    act_usr1.sa_handler = usr1_handler;
    rc = sigaction(SIGUSR1, &act_usr1, NULL);
    if (rc != 0)
        fprintf(stderr, "Server: can't catch SIGUSR1\n");

    struct sigaction act_int;
    act_int.sa_handler = int_handler;
    rc = sigaction(SIGINT, &act_int, NULL);
    if (rc != 0)
        fprintf(stderr, "Server: can't catch SIGINT\n");


while (1) {

    global_rc = 0;

    /*receiving request*/
    char *string = zstr_recv(rep);

    if (string != NULL) {
        json_object *request = json_tokener_parse(string);
        zstr_free(&string);

        access_count++;

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
}
    rc = distribution_finalize();
    if (rc != 0)
        fprintf(stderr, "Server: distribution finalize failed\n");

    zsock_destroy(&rep);
    return 0;
}
