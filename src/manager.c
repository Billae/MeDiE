#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <errno.h>

#include "manager.h"
#include "protocol.h"
#include "mlt.h"
#include "eacl.h"


/*The manager has an sai list merged from all servers eacl (each field filled
 * with "0" in an eacl is a field not supported by this server). It has also its
 * own mlt which it can update (the manager has the "true" version of the mlt).
 * **/
static uint32_t *global_list;
static struct mlt table;


/*TO DO*/
int manager_init(nb)
{
    int rc;
    rc = mlt_init(&table, N_entry, nb);
    if (rc != 0) {
        fprintf(stderr,
            "Manager:init: mlt init error: %s\n", strerror(-rc));
        return -1;
    }

    global_list = calloc(N_entry, sizeof(uint32_t));
    if (global_list == NULL) {
        fprintf(stderr,
            "Manager:init: global sai init error\n");
        return -1;
    }

    return 0;
}


int manager_finalize()
{
    int rc;

    rc = mlt_destroy(&table);
    if (rc != 0)
        fprintf(stderr, "Manager:finalize: mlt_destroy failed\n");

    free(global_list);

    return 0;
}


/*TO DO*/
int manager_merge_eacl(uint32_t *new_list)
{
    return 0;
}

/*TO DO*/
int manager_calculate_relab()
{
    return 0;
}


/*TO DO*/
int *manager_balance_load(int current_load, int goal_load, int *list)
{
    return 0;
}


/** Receive eacl and then update the mlt
 * using the relab computation
 * - open the publisher socket (to broadcast mlt)
 * - open a pull socket to get eacls
 * - check for eacl updates
 * - periodically launch a relab compute
 *   and a mlt update
 * **/

int main(int argc, char *argv[])
{
    if (argv[1] == NULL) {
        fprintf(stderr,
            "please give the number of available servers\n");
        return -1;
    }


    int rc;
    rc = manager_init(atoi(argv[1]));
    if (rc != 0) {
        fprintf(stderr, "Manager: init failed\n");
        return -1;
    }

    char *socket;

    /* Open the publisher socket to broadcast mlt updates*/

    /*a changer pour pcocc: 0.0.0.0*/
    asprintf(&socket, "tcp://192.168.129.25:%d", Mlt_port);
    zsock_t *pub;
    pub = zsock_new_pub(socket);
    if (pub == NULL) {
            fprintf(stderr,
                "Manager: create zmq socket pub error\n");
            return -1;
    }


    /*Open the pull socket to receive eacl updates*/

    /*a changer pour pcocc: 0.0.0.0*/
    asprintf(&socket, "tcp://192.168.129.25:%d", Eacl_port);
    zsock_t *pull;
    pull = zsock_new_pull(socket);
    if (pull == NULL) {
            fprintf(stderr,
                "Manager: create zmq socket pull error\n");
            return -1;
    }
    free(socket);

    while (1) {

        time_t start = time(NULL);
        zmsg_t *packet;
        while ((time(NULL) - start) < 5) {

            /*receiving eacls*/
            packet = zmsg_recv(pull);
            if (packet == NULL)
                fprintf(stderr, "Manager: zmq receive failed\n");

            uint32_t *temp_sai = calloc(N_entry, sizeof(uint32_t));

            zframe_t *sai_frame = zmsg_pop(packet);
            byte *temp = zframe_data(sai_frame);
            memcpy(temp_sai, temp, sizeof(uint32_t)*N_entry);

            fprintf(stderr, "sai received:%d\n", temp_sai[0]);

            rc = manager_merge_eacl(temp_sai);
            if (rc != 0)
                fprintf(stderr, "Manager: merge eacl with global failed\n");
            free(temp_sai);
            fprintf(stderr, "global sai updated: %d\n", global_list[0]);
        }
        zmsg_destroy(&packet);


        /*RELAB*/
        rc = manager_calculate_relab();
        if (rc != 0)
            fprintf(stderr, "Manager: relab computation failed\n");

        /*sending MLT*/
        packet = zmsg_new();
        zframe_t *id_srv_frame = zframe_new(table.id_srv,
            sizeof(uint32_t) * table.size);
        zmsg_append(packet, &id_srv_frame);

        zframe_t *n_ver_frame = zframe_new(table.n_ver,
            sizeof(uint32_t) * table.size);
        zmsg_append(packet, &n_ver_frame);


        rc = zmsg_send(&packet, pub);
        if (rc != 0)
            fprintf(stderr, "Manager: zmsg_send failed\n");
    }

    /*cleaning*/
    zsock_destroy(&pub);
    zsock_destroy(&pull);
    manager_finalize();
    return 0;
}
