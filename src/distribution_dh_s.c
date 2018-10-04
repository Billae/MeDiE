#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <json.h>
#include <pthread.h>
#include <czmq.h>
#include <pthread.h>

#include "distribution_dh_s.h"
#include "mlt.h"
#include "eacl.h"
#include "protocol.h"
#include "murmur3.h"
#include "md_entry.h"

/*Each server has its own mlt and its own eacl with access counter accessed
 * only in the distribution functions*/
static struct mlt table;
static struct eacl access_list;

/*Each server has its own id_srv*/
static int id_srv_self;

/*Array of linked list to know all md in charge for each server*/
static struct md_entry **in_charge_md;
static pthread_rwlock_t locks[N_entry];
/*ocre*/
#define ip_manager "192.168.129.25"
/*pcocc*/
//#define ip_manager "10.252.0.1"

/* path in pcocc*/
//#define SRV_PATH "/home/billae/prototype_MDS/etc/server.cfg"
/*path in ocre*/
#define SRV_PATH "/ccc/home/cont001/ocre/billae/prototype_MDS/etc/server.cfg"



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

    /*getting id_srv_self*/
    int max_id_size = 20;
    char *id_srv;
    id_srv = malloc(max_id_size*sizeof(*id_srv));
    if (id_srv == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution:init: id_srv malloc error: %s\n",
            strerror(err));
        return 0;
    }

    FILE *fd = fopen(SRV_PATH, "r");
    if (fd == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution:init: open server.cfg file failed: %s\n",
            strerror(err));
        free(id_srv);
        return -1;
    }
    if (fgets(id_srv, max_id_size, fd) == NULL) {
        fprintf(stderr, "Distribution:init: read config file failed\n");
        free(id_srv);
        return -1;
    }
    char *positionEntree = strchr(id_srv, '\n');
    if (positionEntree != NULL)
        *positionEntree = '\0';

    char *value = strstr(id_srv, "ID");
    value = strchr(value, '=');
    value++;
    id_srv_self = atoi(value);

    free(id_srv);
    fclose(fd);


    in_charge_md = malloc(N_entry * sizeof(struct md_entry *));
    if (in_charge_md == NULL) {
        int err = errno;
        fprintf(stderr, "Distribuion:init: in_charge malloc error: %s\n",
            strerror(err));
        return -1;
    }
    int i;
    for (i = 0; i < N_entry; i++) {
        in_charge_md[i] = NULL;
        rc = -pthread_rwlock_init(&locks[i], NULL);
        if (rc != 0) {
            fprintf(stderr, "Distribution:init: lock creation failed\n");
            return -1;
        }
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
    rc = pthread_create(&eacl_sender, NULL, &thread_sai_sender, NULL);
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

    struct md_entry *elem;
    int i;
    for (i = 0; i < N_entry; i++) {
        while (in_charge_md[i] != NULL) {
            elem = md_entry_pop(&in_charge_md[i]);
            free(elem);
        }
        pthread_rwlock_destroy(&locks[i]);
    }
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
    int state;

    int rc = mlt_get_entry(&table, json_object_get_int(entry),
        &id_srv, &ver, &state);

    if (rc != 0) {
        fprintf(stderr, "Distribution:post_receive: mlt get entry failed: %s\n",
            strerror(-rc));
        return -1;
    }

    if (state != 0)
        return -EALREADY;

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


int distribution_pre_send(json_object *reply, int global_rc)
{
    int rc;
    if (global_rc == 0) {
        /*update the in_charge_md array if needed*/
        json_object *type;
        if (!json_object_object_get_ex(reply, "reqType", &type)) {
            fprintf(stderr, "Distribution:pre_send: json extract error:");
            fprintf(stderr, "no key \"reqType\" found\n");
            return -1;
        }
        enum req_type reqType = json_object_get_int(type);

        if (reqType == RT_CREATE) {
            /*add the key in the in_charge_md array*/
            json_object *key;
            if (!json_object_object_get_ex(reply, "key", &key)) {
                fprintf(stderr, "Distribution:pre_send: json extract error:");
                fprintf(stderr, "no key \"key\" found\n");
                return -1;
            }
            char *str_key;
            asprintf(&str_key, "%s", json_object_get_string(key));

            json_object *entry;
            if (!json_object_object_get_ex(reply, "index", &entry)) {
                fprintf(stderr, "Distribution:pre_send: json extract error:");
                fprintf(stderr, "no key \"index\" found\n");
                return -1;
            }

            struct md_entry *to_add = malloc(sizeof(struct md_entry));
            md_entry_init(to_add, json_object_get_int(entry), str_key);

            struct md_entry *ptr;
            int i;
            for (i = 0; i < N_entry; i++) {
                ptr = in_charge_md[i];
                if (ptr == NULL) {
                    rc = -pthread_rwlock_wrlock(&locks[i]);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:pre_send:");
                        fprintf(stderr, "lock failed: %s\n", strerror(-rc));
                        return -1;
                    }

                    md_entry_insert(&in_charge_md[i], to_add);

                    rc = -pthread_rwlock_unlock(&locks[i]);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:pre_send:");
                        fprintf(stderr, "unlock failed: %s\n", strerror(-rc));
                        return -1;
                    }
                    break;
                }
                if (ptr->entry == json_object_get_int(entry)) {
                    rc = -pthread_rwlock_wrlock(&locks[i]);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:pre_send:");
                        fprintf(stderr, "lock failed: %s\n", strerror(-rc));
                        return -1;
                    }

                    md_entry_insert(&ptr, to_add);

                    rc = -pthread_rwlock_unlock(&locks[i]);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:pre_send:");
                        fprintf(stderr, "unlock failed: %s\n", strerror(-rc));
                        return -1;
                    }
                    break;
                }
            }
        } else if (reqType == RT_DELETE) {
            /*remove the key in the in_charge_md array*/
            json_object *key;
            if (!json_object_object_get_ex(reply, "key", &key)) {
                fprintf(stderr, "Distribution:pre_send: json extract error:");
                fprintf(stderr, "no key \"key\" found\n");
                return -1;
            }
            char *str_key;
            asprintf(&str_key, "%s", json_object_get_string(key));

            json_object *entry;
            if (!json_object_object_get_ex(reply, "index", &entry)) {
                fprintf(stderr, "Distribution:pre_send: json extract error:");
                fprintf(stderr, "no key \"index\" found\n");
                return -1;
            }
            struct md_entry *ptr;
            int i;
            for (i = 0; i < N_entry; i++) {
                ptr = in_charge_md[i];
                if (ptr == NULL) {
                    fprintf(stderr, "Distribution:pre_send:");
                    fprintf(stderr, "remove key from in_charge_md failed\n");
                    return -1;
                }
                if (ptr->entry == json_object_get_int(entry)) {
                    rc = -pthread_rwlock_wrlock(&locks[i]);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:pre_send:");
                        fprintf(stderr, "lock failed: %s\n", strerror(-rc));
                        return -1;
                    }

                    ptr = md_entry_search_md_name(&ptr, str_key);

                    rc = -pthread_rwlock_unlock(&locks[i]);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:pre_send:");
                        fprintf(stderr, "unlock failed: %s\n", strerror(-rc));
                        return -1;
                    }
                    free(ptr->md_name);
                    free(ptr);
                    break;
                }
            }
        }
    }
    return 0;
}


/*TO DO*/
void *thread_load_sender(void *args)
{
    pthread_exit(0);
}

/*TO DO*/
void *thread_load_receiver(void *args)
{
    pthread_exit(0);
}


/*TO DO*/
void *thread_mlt_updater(void *args)
{
    /*opening a subscriber socket to the manager*/
    char *socket;
    asprintf(&socket, "tcp://%s:%d", ip_manager, Mlt_port);
    zsock_t *sub;
    sub = zsock_new_sub(socket, "");
    if (sub == NULL) {
        fprintf(stderr,
            "Distribution:thread_mlt_updater: create zmq socket error\n");
        pthread_exit((void *)-1);
    }
    int rc;
    while (1) {
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


        int i;
        for (i = 0; i < N_entry; i++) {
            fprintf(stderr, "MLT received: index=%d -- %d %d\n",
                i, temp_mlt.id_srv[i], temp_mlt.n_ver[i]);

            /*updating the mlt and fill the to_do list for sender and receiver*/
            int old_srv, old_ver, old_state;
            rc = mlt_get_entry(&table, i, &old_srv, &old_ver, &old_state);
            if (rc != 0) {
                fprintf(stderr, "Distribution:thread_mlt_updater");
                fprintf(stderr, ": get mlt failed: %s\n", strerror(-rc));
            }

            int new_srv, new_ver, new_state;
            rc = mlt_get_entry(&temp_mlt, i, &new_srv, &new_ver, &new_state);
            if (rc != 0) {
                fprintf(stderr, "Distribution:thread_mlt_updater");
                fprintf(stderr, ": get mlt failed: %s\n", strerror(-rc));
            }

            if (old_ver != new_ver) {
                if (old_srv == id_srv_self) {
                    /*add to to_send list*/
                    rc = mlt_update_entry(&table, i, new_srv, new_ver, 1);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:thread_mlt_updater:");
                        fprintf(stderr,
                            "update mlt failed: %s\n", strerror(-rc));
                    }
                } else if (new_srv == id_srv_self) {
                    /*add to to_receive list*/
                    rc = mlt_update_entry(&table, i, new_srv, new_ver, 1);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:thread_mlt_updater:");
                        fprintf(stderr,
                            "update mlt failed: %s\n", strerror(-rc));
                    }
                } else
                    rc = mlt_update_entry(&table, i, new_srv, new_ver, 0);
            }
        }
        /*launching inter-server transferts*/

        rc = mlt_destroy(&temp_mlt);
        if (rc != 0)
            fprintf(stderr, "MLT destroy failed\n");
    }

    /*cleaning*/
    zsock_destroy(&sub);
    pthread_exit(0);
}


void *thread_sai_sender(void *args)
{
    /*opening a push socket to the manager*/
    char *socket;
    /*a changer pour pcocc: l'adresse du manager*/
    asprintf(&socket, "tcp://%s:%d", ip_manager, Eacl_port);
    zsock_t *push;
    push = zsock_new_push(socket);
    if (push == NULL) {
            fprintf(stderr,
                "Distribution:thread_sai_sender: create zmq socket error\n");
            pthread_exit((void *)-1);
    }

    while (1) {
        int rc = eacl_calculate_sai(&access_list);
        if (rc != 0) {
            fprintf(stderr,
                "Distribution:thread_sai_sender: calculate SAI failed: %s\n",
                strerror(-errno));
        }

        zmsg_t *packet = zmsg_new();
        zframe_t *sai_frame = zframe_new(access_list.sai,
            sizeof(uint32_t) * access_list.size);
        zmsg_append(packet, &sai_frame);


        rc = zmsg_send(&packet, push);
        if (rc != 0)
            fprintf(stderr,
                    "Distribution:thread_sai_sender: zmsg_send failed\n");
        rc = eacl_reset_access(&access_list);
        sleep(2);

    }

    /*cleaning*/
    zsock_destroy(&push);
    pthread_exit(0);

}
