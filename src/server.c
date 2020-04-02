#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <json.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "generic_storage.h"

#ifdef DISTRIBUTION_SH
    #include "distribution_sh_s.h"
    #include "protocol_sh.h"
    #ifndef PREFIX
        #define PREFIX "/mnt/result/sh/"
    #endif
#endif

#ifdef DISTRIBUTION_DH
    #include "distribution_dh_s.h"
    #include "protocol_dh.h"
    #ifndef PREFIX
        #define PREFIX "/mnt/result/dh/"
    #endif
#endif

#ifdef DISTRIBUTION_INDEDH
    #include "distribution_indedh_s.h"
    #include "protocol_indedh.h"
    #ifndef PREFIX
        #define PREFIX "/mnt/result/indedh/"
    #endif
#endif

#ifdef DISTRIBUTION_WINDOWED
    #include "distribution_windowed_s.h"
    #include "protocol_windowed.h"
    #ifndef PREFIX
        #define PREFIX "/mnt/result/windowed/"
    #endif
#endif


/* path in pcocc*/
#ifndef SRV_PATH
    #define SRV_PATH "/home/billae/prototype_MDS/etc/server.cfg"
#endif
/*path in ocre*/
//#define SRV_PATH "/ccc/home/cont001/ocre/billae/prototype_MDS/etc/server.cfg"

/*path in ocre*/
//#define PREFIX "/ccc/home/cont001/ocre/billae/prototype_MDS/"


#ifndef max_id_size
    #define max_id_size 21
#endif
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

/*Both USR1 and USR2 catch the load level and the call distribution signal_handler*/
void usr1_handler(int sig)
{
    /*open the result file*/
    char *result_path;
    asprintf(&result_path, "%s/server/load%d", PREFIX, id_self);

    int fd_res = open(result_path, O_WRONLY | O_APPEND | O_CREAT, 0664);
    if (fd_res == -1) {
        int err = errno;
        fprintf(stderr, "Server:sigUSR1 handler: open %s error: %s\n",
            result_path, strerror(err));
        free(result_path);
        return;
    }
    free(result_path);

    char *load;
    asprintf(&load, "%d\n", access_count);

    if ((write(fd_res, load, strlen(load))) < 0) {
        int err = errno;
        fprintf(stderr, "Server:sigUSR1 handler: ");
        fprintf(stderr, "write in %s error: %s\n", result_path, strerror(err));
        return;
    }
    free(load);
    access_count = 0;

    close(fd_res);

    int rc;
    rc = distribution_signal1_action();
    if (rc != 0)
        fprintf(stderr, "Server:sigUSR1 handler: distribution signal action failed\n");
}


/*send eacl to manager*/
void usr2_handler(int sig)
{
/*    fprintf(stderr, "entering in signal 2 handler\n");*/

    /*open the result file*/
    char *result_path;
    asprintf(&result_path, "%s/server/load%d", PREFIX, id_self);

    int fd_res = open(result_path, O_WRONLY | O_APPEND | O_CREAT, 0664);
    if (fd_res == -1) {
        int err = errno;
        fprintf(stderr, "Server:sigUSR2 handler: open %s error: %s\n",
            result_path, strerror(err));
        free(result_path);
        return;
    }
    free(result_path);

    char *load;
    asprintf(&load, "%d\n", access_count);

    if ((write(fd_res, load, strlen(load))) < 0) {
        int err = errno;
        fprintf(stderr, "Server:sigUSR2 handler: ");
        fprintf(stderr, "write in %s error: %s\n", result_path, strerror(err));
        return;
    }
    free(load);

    access_count = 0;

    close(fd_res);

    int rc;
    rc = distribution_signal2_action();
    if (rc != 0)
        fprintf(stderr, "Server:sigUSR2 handler: distribution signal action failed\n");
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
            fprintf(stderr, "Server: create zmq socket %s error\n", socket);
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
            fprintf(stderr, "Server: create zmq socket %s error\n", name);
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
    char *id_str = malloc(max_id_size);
    if (id_str == NULL) {
        int err = errno;
        fprintf(stderr, "Server: id_str malloc error: %s\n",
            strerror(err));
        return -1;
    }

    int fd_srv = open(SRV_PATH, O_RDONLY);
    if (fd_srv == -1) {
        int err = errno;
        fprintf(stderr, "Server: open %s error: %s\n",
            SRV_PATH, strerror(err));
        return -1;
    }

    rc = read(fd_srv, id_str, max_id_size);
    if (rc < 0) {
        int err = errno;
        fprintf(stderr, "Server: read config file %s failed: %s\n",
            SRV_PATH, strerror(err));
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

    /*perf counter*/
    access_count = 0;

    struct sigaction act_usr1;
    memset(&act_usr1, 0, sizeof(struct sigaction));
    act_usr1.sa_handler = usr1_handler;
    rc = sigaction(SIGUSR1, &act_usr1, NULL);
    if (rc != 0)
        fprintf(stderr, "Server: can't catch SIGUSR1\n");

    struct sigaction act_usr2;
    memset(&act_usr2, 0, sizeof(struct sigaction));
    act_usr2.sa_handler = usr2_handler;
    rc = sigaction(SIGUSR2, &act_usr2, NULL);
    if (rc != 0)
        fprintf(stderr, "Server: can't catch SIGUSR2\n");

    struct sigaction act_int;
    memset(&act_int, 0, sizeof(struct sigaction));
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
            rc = generic_put(json_object_get_string(key), json_object_get_string(data));
            if (rc != 0) {
                fprintf(stderr, "Server: generic storage operation \"put\" error\n");
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
            rc = generic_update(json_object_get_string(key), json_object_get_string(data));
            if (rc != 0) {
                fprintf(stderr, "Server: generic storage operation \"update\" error\n");
                global_rc = -1;
            }
            break;
        }

        case RT_DELETE: /*delete*/
        {
            rc = generic_del(json_object_get_string(key));
            if (rc != 0) {
                fprintf(stderr, "Server: generic storage operation \"del\" error\n");
                global_rc = -1;
            }
            break;
        }

        default: /*get*/
        {
            char *get_value = generic_get(json_object_get_string(key));
            if (get_value == NULL) {
                fprintf(stderr, "Server: generic storage operation \"get\" error\n");
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
            fprintf(stderr, "Server: distribution_pre_send failed for key %s\n",
                json_object_get_string(key));
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
