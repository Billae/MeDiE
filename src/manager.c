#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <errno.h>
#include <pthread.h>

#include "manager.h"
#include "protocol.h"
#include "mlt.h"
#include "eacl.h"


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

    rc = eacl_init(&global_list, N_entry);
    if (rc != 0) {
        fprintf(stderr,
            "Manager:init: eacl init error: %s\n", strerror(-rc));
        return -1;
    }

    update_needed = 0;

    /*thread initialization*/
    pthread_t timer;
    rc = pthread_create(&timer, NULL, &thread_timer, NULL);
    if (rc != 0) {
        fprintf(stderr,
            "Manager:init: thread timer init failed: %s\n",
            strerror(rc));
        return -1;
    }

    return 0;
}


/*TO DO*/
int manager_finalize()
{
    int rc;

    rc = mlt_destroy(&table);
    if (rc != 0)
        fprintf(stderr, "Manager:finalize: mlt_destroy failed\n");

    rc = eacl_destroy(&global_list);
    if (rc != 0)
        fprintf(stderr, "Manager:finalize: eacl_destroy failed\n");

    return 0;
}


/*TO DO*/
int manager_receive_eacl(struct eacl *new_list)
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


/*TO DO*/
void *thread_timer(void *args)
{
    return 0;
}


/** Receive eacl and then update the mlt
 * using the relab computation
 * - open the publisher socket (to broadcast mlt)
 * - open a pull socket to get eacls
 * - launch the timer thread
 * - check for eacl updates
 * - when the update_needed variable is to 1 launch a relab compute
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

    while (1) {
        time_t start = time(NULL);
        zmsg_t *packet;
        while ((time(NULL) - start) < 5) {
            /*receiving eacls*/
            packet = zmsg_recv(pull);
            if (packet == NULL)
                fprintf(stderr, "Manager: zmq receive failed\n");

            struct eacl temp_eacl;
            rc = eacl_init(&temp_eacl, N_entry);

            zframe_t *access_count_frame = zmsg_pop(packet);
            byte *temp = zframe_data(access_count_frame);
            memcpy(temp_eacl.access_count, temp, sizeof(uint32_t)*N_entry);

            zframe_t *sai_frame = zmsg_pop(packet);
            temp = zframe_data(sai_frame);
            memcpy(temp_eacl.sai, temp, sizeof(uint32_t)*N_entry);

            fprintf(stderr, "eacl received:%d %d\n",
                temp_eacl.access_count[0], temp_eacl.sai[0]);
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
