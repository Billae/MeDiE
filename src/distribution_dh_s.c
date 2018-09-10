#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <json.h>
#include <pthread.h>
#include <czmq.h>

#include "distribution_dh_s.h"
#include "mlt.h"
#include "eacl.h"
#include "protocol.h"
#include "murmur3.h"


int distribution_init(nb)
{
    int rc;

    rc = mlt_init(&table, N_entry, nb);
    if (rc != 0) {
        fprintf(stderr,
            "Distribution:init: mlt init error: %s\n", strerror(-rc));
        return -1;
    }

    rc = eacl_init(&access_list, N_entry);
    if (rc != 0) {
        fprintf(stderr,
            "Distribution:init: eacl init error: %s\n", strerror(-rc));
        return -1;
    }

    /*threads initialization*/
    pthread_t mlt_updater;
    rc = pthread_create(&mlt_updater, NULL, &thread_mlt_updater, NULL);
    if (rc != 0) {
        fprintf(stderr,
            "Distribution:init: thread mlt updater init failed: %s\n",
            strerror(-rc));
        return -1;
    }

    pthread_t eacl_sender;
    rc = pthread_create(&eacl_sender, NULL, &thread_eacl_sender, NULL);
    if (rc != 0) {
        fprintf(stderr,
            "Distribution:init: thread eacl sender init failed: %s\n",
            strerror(-rc));
        return -1;
    }

    return 0;
}


int distribution_finalize()
{
    int rc;

    rc = mlt_destroy(&table);
    if (rc != 0)
        fprintf(stderr, "Distribution:finalize: mlt_destroy failed\n");

    rc = eacl_destroy(&access_list);
    if (rc != 0)
        fprintf(stderr, "Distribution:finalize: eacl_destroy failed\n");
    return 0;
}


int distribution_post_receive(json_object *request)
{
    /*verifying entry version*/
    json_object *entry;
    if (!json_object_object_get_ex(request, "index", &entry)) {
        fprintf(stderr,
            "Distribution:post_receive: json extract error: no key \"index\" found\n");
        return -1;
    }

    json_object *entry_ver;
    if (!json_object_object_get_ex(request, "entryVersion", &entry_ver)) {
        fprintf(stderr,
            "Distribution:post_receive: json extract error: no key \"entryVersion\" found\n");
        return -1;
    }

    int id_srv;
    int ver;

    int rc = mlt_get_entry(&table, json_object_get_int(entry), &id_srv, &ver);

    if (rc != 0)
        return -1;

    json_object *ver_flag;

    /*mlt out of date*/
    if (json_object_get_int(entry_ver) != ver) {
        ver_flag = json_object_new_string("out-of-date");
        json_object_object_add(request, "versionFlag", ver_flag);

        /*adding the right version*/
        json_object *srv_r = json_object_new_int(id_srv);
        json_object *version_r = json_object_new_int(ver);

        json_object_object_add(request, "newSrv", srv_r);
        json_object_object_add(request, "newVersion", version_r);
        return -EAGAIN;
    }

    ver_flag = json_object_new_string("up-to-date");
    json_object_object_add(request, "versionFlag", ver_flag);

    /*incrementing counter access*/
    rc = eacl_incr_access(&access_list, json_object_get_int(entry));
    if (rc != 0)
        fprintf(stderr,
            "Distribution:post_receive: eacl_incr_access failed\n");
    return 0;
}


int distribution_pre_send(json_object *reply)
{
    return 0;
}


/*TO DO*/
int distribution_transfert_load(int entry)
{
    return 0;
}


/*TO DO*/
void *thread_mlt_updater(void *args)
{
    /*opening a subscriber socket to the manager*/
    char *socket;
    /*a changer pour pcocc: l'adresse du manager*/
    asprintf(&socket, "tcp://192.168.129.25:%d", Mlt_port);
    zsock_t *sub;
    sub = zsock_new_sub(socket, "");
    if (sub == NULL) {
            fprintf(stderr,
                "Distribution:thread_mlt_updater: create zmq socket error\n");
            pthread_exit((void *)-1);
    }
    int rc;
    while(1)
    {
        struct mlt temp_mlt;
        rc = mlt_init(&temp_mlt, N_entry, 1);
        if (rc != 0)
            fprintf(stderr,
                "Distribution:thread_mlt_updater: temp_mlt init failed\n");

        zmsg_t *packet = zmsg_recv(sub);
        zframe_t *id_srv_frame = zmsg_pop(packet);
        byte *temp = zframe_data(id_srv_frame);
        memcpy(temp_mlt.id_srv, temp, sizeof(uint32_t) * N_entry);

        zframe_t *n_ver_frame = zmsg_pop(packet);
        temp = zframe_data(n_ver_frame);
        memcpy(temp_mlt.n_ver, temp, sizeof(uint32_t) * N_entry);


        fprintf(stderr,
            "MLT received: %d %d\n", temp_mlt.id_srv[0], temp_mlt.n_ver[0]);
        /*updating the mlt*/
        /*launching inter-server transferts*/
    }

    /*cleaning*/
    zsock_destroy(&sub);
    pthread_exit(0);
}


void *thread_eacl_sender(void *args)
{
    /*opening a push socket to the manager*/
    char *socket;
    /*a changer pour pcocc: l'adresse du manager*/
    asprintf(&socket, "tcp://192.168.129.25:%d", Eacl_port);
    zsock_t *push;
    push = zsock_new_push(socket);
    if (push == NULL) {
            fprintf(stderr,
                "Distribution:thread_eacl_sender: create zmq socket error\n");
            pthread_exit((void *)-1);
    }

    while (1) {
        int rc = eacl_calculate_sai(&access_list);
        if (rc != 0) {
            fprintf(stderr,
                "Distribution:thread_eacl_sender: calculate SAI failed: %s\n",
                strerror(-errno));
        }

        zmsg_t *packet = zmsg_new();
        zframe_t *access_count_frame = zframe_new(access_list.access_count,
            sizeof(uint32_t) * access_list.size);
        zmsg_append(packet, &access_count_frame);

        zframe_t *sai_frame = zframe_new(access_list.sai,
            sizeof(uint32_t) * access_list.size);
        zmsg_append(packet, &sai_frame);


        rc = zmsg_send(&packet, push);
        if (rc != 0)
            fprintf(stderr, "Distribution:thread_eacl_sender: zmsg_send failed\n");
        rc = eacl_reset_access(&access_list);
        sleep(2);

    }

    /*cleaning*/
    zsock_destroy(&push);
    pthread_exit(0);

}
