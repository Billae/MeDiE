#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <json.h>
#include <pthread.h>
#include <czmq.h>

#include "generic_storage.h"
#include "distribution_dh_s.h"
#include "mlt.h"
#include "eacl_dh.h"
#include "protocol_dh.h"
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


zsock_t *push;
zsock_t *sub;
pthread_t mlt_updater;
pthread_t eacl_sender;

#define max_id_size 21

/*pcocc*/
#define ip_manager "10.200.0.1"
/*ocre*/
//#define ip_manager "192.168.129.25"

/* path in pcocc*/
#define SRV_PATH "/home/billae/prototype_MDS/etc/server.cfg"
/*path in ocre*/
//#define SRV_PATH "/ccc/home/cont001/ocre/billae/prototype_MDS/etc/server.cfg"

/*path in pcocc*/
#define HOST_PATH "/home/billae/prototype_MDS/etc/hosts.cfg"
/*path in ocre*/
//#define HOST_PATH "/ccc/home/cont001/ocre/billae/prototype_MDS/etc/hosts.cfg"

/* path in pcocc*/
#define SCRATCH "/mnt/scratch/tmp_ack/dh/"
/*path in ocre*/
//#define SCRATCH "/ccc/home/cont001/ocre/billae/prototype_MDS/tmp/"


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

    /*init in_charge_md*/
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

    /*opening a push socket to the manager*/
    char *socket;
    asprintf(&socket, "tcp://%s:%d", ip_manager, Eacl_port);
    push = zsock_new_push(socket);
    if (push == NULL) {
            fprintf(stderr,
                "Distribution:thread_sai_sender: create zmq socket error\n");
            pthread_exit((void *)-1);
    }
    free(socket);


    /*threads initialization*/
    rc = pthread_create(&mlt_updater, NULL, &thread_mlt_updater, NULL);
    if (rc != 0) {
        fprintf(stderr,
            "Distribution:init: thread mlt updater init failed: %s\n",
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

    struct md_entry *elem;
    int i;
    for (i = 0; i < N_entry; i++) {
        while (in_charge_md[i] != NULL) {
            elem = md_entry_pop(&in_charge_md[i]);
            free(elem);
        }
        pthread_rwlock_destroy(&locks[i]);
    }
    free(in_charge_md);

    zsock_destroy(&push);

    rc = pthread_cancel(mlt_updater);
    if (rc != 0)
        fprintf(stderr, "Distribution:finalize: mlt_updater cancel failed\n");
    zsock_destroy(&sub);
    rc = pthread_join(mlt_updater, NULL);
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

            int asked_entry = json_object_get_int(entry);
            struct md_entry *to_add = malloc(sizeof(struct md_entry));
            md_entry_init(to_add, asked_entry, str_key);

            rc = -pthread_rwlock_wrlock(&locks[asked_entry]);
            if (rc != 0) {
                fprintf(stderr, "Distribution:pre_send:");
                fprintf(stderr, "lock failed: %s\n", strerror(-rc));
                return -1;
            }

            md_entry_insert(&in_charge_md[asked_entry], to_add);

            rc = -pthread_rwlock_unlock(&locks[asked_entry]);
            if (rc != 0) {
                fprintf(stderr, "Distribution:pre_send:");
                fprintf(stderr, "unlock failed: %s\n", strerror(-rc));
                return -1;
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
            int asked_entry = json_object_get_int(entry);

            rc = -pthread_rwlock_wrlock(&locks[asked_entry]);
            if (rc != 0) {
                fprintf(stderr, "Distribution:pre_send: ");
                fprintf(stderr, "lock failed: %s\n", strerror(-rc));
                return -1;
            }

            ptr = md_entry_search_md_name(&in_charge_md[asked_entry], str_key);

            rc = -pthread_rwlock_unlock(&locks[asked_entry]);
            if (rc != 0) {
                fprintf(stderr, "Distribution:pre_send: ");
                fprintf(stderr, "unlock failed: %s\n", strerror(-rc));
                return -1;
            }
            if (ptr == NULL) {
                fprintf(stderr, "Distribution:pre_send: ");
                fprintf(stderr, " search_md_entry failed\n");
                return -1;
            }
            free(ptr->md_name);
            free(ptr);
            free(str_key);
        }

    }
    return 0;
}

int cut(struct transfert_load_args *list, int p, int r)
{
    int pivot = list->servers[p], i = p-1, j = r+1;
    while (1) {
        do
            j--;
        while (list->servers[j] > pivot);
        do
            i++;
        while (list->servers[i] < pivot);
        if (i < j) {
            int tmp_srv = list->servers[i];
            int tmp_entry = list->entries[i];
            list->servers[i] = list->servers[j];
            list->entries[i] = list->entries[j];
            list->servers[j] = tmp_srv;
            list->entries[j] = tmp_entry;
        } else
            return j;
        }
}

/*Sort in growing order the servers and adapt the change to entries*/
/*p = inf and r = size-1*/
void load_args_sort(struct transfert_load_args *list, int p, int r)
{
    int q;
    if (p < r) {
        q = cut(list, p, r);
        load_args_sort(list, p, q);
        load_args_sort(list, q+1, r);
    }
}


void *thread_load_sender(void *args)
{
    struct transfert_load_args *to_send = args;
    load_args_sort(to_send, 0, to_send->size - 1);

    /*int i;
    for (i = 0; i < to_send->size; i++)
        fprintf(stderr, "(sender: had to send: entry n %d to server %d\n",
            to_send->entries[i], to_send->servers[i]);*/

    int rc;
    int fail_rc = -1;
    FILE *fd = fopen(HOST_PATH, "r");
    if (fd == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution:thread_load_sender: ");
        fprintf(stderr, "open hosts file error: %s\n", strerror(err));
        pthread_exit(&fail_rc);
    }

    /*current_line is the line number buffered in id_srv: initially -1*/
    int current_line = -1;
    int to_send_idx = 0;

    char *id_srv;
    id_srv = malloc(max_id_size*sizeof(*id_srv));
    if (id_srv == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution:thread_load_sender: ");
        fprintf(stderr, "id_srv malloc error: %s\n", strerror(err));
        pthread_exit(&fail_rc);
    }

    /*for each index*/
    for (to_send_idx = 0; to_send_idx < to_send->size; to_send_idx++) {
        /*read until find the next server to send*/
        while (current_line != to_send->servers[to_send_idx]) {
            if (fgets(id_srv, max_id_size, fd) == NULL) {
                fprintf(stderr, "Distribution:thread_load_sender: ");
                fprintf(stderr, "read error in host file\n");
                pthread_exit(&fail_rc);
            }
            char *positionEntree = strchr(id_srv, '\n');
            if (positionEntree != NULL)
                *positionEntree = '\0';
            current_line++;
        }
        /*open a socket with the server*/
        char *socket;
        if (asprintf(&socket, "tcp://%s:%d", id_srv, Transfert_port) == -1) {
            int err = errno;
            fprintf(stderr, "Distribution: thread_load_sender: ");
            fprintf(stderr, "format zmq socket name error: %s\n", strerror(err));
            pthread_exit(&fail_rc);
        }
        zsock_t *sock = zsock_new_req(socket);
        if (sock == NULL) {
            fprintf(stderr, "Distribution:thread_load_sender: ");
            fprintf(stderr, "create zmq socket error\n");
            pthread_exit(&fail_rc);
        }
        free(socket);

        while (current_line == to_send->servers[to_send_idx]) {
            /*concat all messages associated to an entry*/
            zmsg_t *packet = zmsg_new();
            zframe_t *entry_frame = zframe_new(&to_send->entries[to_send_idx], sizeof(int));
            zmsg_append(packet, &entry_frame);

            int md_names_capacity = 80;
            char *md_names = malloc(sizeof(char) * md_names_capacity);
            strncpy(md_names, "", 1);
            int md_names_length = strlen(md_names);
            struct md_entry *ptr;

            while (in_charge_md[to_send->entries[to_send_idx]] != NULL) {
                rc = -pthread_rwlock_wrlock(&locks[to_send->entries[to_send_idx]]);
                if (rc != 0) {
                    fprintf(stderr, "Distribution:thread_load_sender: ");
                    fprintf(stderr, "lock failed: %s\n", strerror(-rc));
                    zsock_destroy(&sock);
                    pthread_exit(&fail_rc);
                }

                ptr = md_entry_pop(&in_charge_md[to_send->entries[to_send_idx]]);

                rc = -pthread_rwlock_unlock(&locks[to_send->entries[to_send_idx]]);
                if (rc != 0) {
                    fprintf(stderr, "Distribution:thread_load_sender: ");
                    fprintf(stderr, "unlock failed: %s\n", strerror(-rc));
                    zsock_destroy(&sock);
                    pthread_exit(&fail_rc);
                }

                rc = generic_del(ptr->md_name);
                if (rc == -1) {
                    fprintf(stderr, "Distribution:thread_load_sender:");
                    fprintf(stderr, "md deletion failed\n");
                    zsock_destroy(&sock);
                    pthread_exit(&fail_rc);
                }
                /* update the string length*/
                md_names_length += 1 + strlen(ptr->md_name);
                /*increase the capacity if needed*/
                if (md_names_length >= md_names_capacity) {
                    md_names_capacity = (md_names_capacity*3)/2;
                    char *tmp = realloc(md_names, md_names_capacity);
                    if (tmp == NULL) {
                        fprintf(stderr, "Distribution:thread_load_sender: ");
                        fprintf(stderr, "realloc md_names failed\n");
                        pthread_exit(&fail_rc);
                    } else
                        md_names = tmp;
                }
                /* copy a comma and the string into the buffer*/
                strncat(md_names, ",", 1);
                strncat(md_names, ptr->md_name, strlen(ptr->md_name));

                free(ptr->md_name);
                free(ptr);
            }

            md_names_length++;
            zframe_t *md_entry_size_frame = zframe_new(&md_names_length, sizeof(int));
            zmsg_append(packet, &md_entry_size_frame);

            strncat(md_names, "\0", 1);
            zframe_t *md_entry_frame = zframe_new(md_names, md_names_length);
            zmsg_append(packet, &md_entry_frame);

            /*fprintf(stderr, "packaging finished entry %d: %d -> %s\n",
                to_send->entries[to_send_idx], md_names_length, md_names);*/

            rc = zmsg_send(&packet, sock);
            if (rc != 0) {
                fprintf(stderr, "Distribution:thread_load_sender: ");
                fprintf(stderr, "md_entry send failed\n");
                zsock_destroy(&sock);
                pthread_exit(&fail_rc);
            }

            /*update the state of the entry in the mlt*/
            rc = mlt_update_state(&table, to_send->entries[to_send_idx], 0);
            if (rc != 0) {
                fprintf(stderr, "Distribution:thread_load_sender: ");
                fprintf(stderr, "mlt update failed:%s\n", strerror(-rc));
                zsock_destroy(&sock);
                pthread_exit(&fail_rc);
            }
            /*update eacl entry*/
            rc = eacl_reset_all_entry(&access_list, to_send->entries[to_send_idx]);
            if (rc != 0) {
                fprintf(stderr, "Distribution:thread_load_receiver: ");
                fprintf(stderr, "eacl update failed\n");
                zsock_destroy(&sock);
                pthread_exit(&fail_rc);
            }

            /*fprintf(stderr, "[sender: sent: entry %d\n",
                to_send->entries[to_send_idx]);*/

            /*receive the acknowledgement from the receiver*/
            zmsg_t *reply = zmsg_recv(sock);
            zframe_t *flag = zmsg_pop(reply);
            if (zframe_streq(flag, "done") != true) {
                fprintf(stderr, "Distribution:thread_load_sender: ");
                fprintf(stderr, "acknowledgement receive failed\n");
                zsock_destroy(&sock);
                pthread_exit(&fail_rc);
            }

            /*fprintf(stderr, "[sender: ack received for entry %d\n",
                to_send->entries[to_send_idx]);*/

            /*if the next entry is for the same server, stay in the send loop*/
            if ((to_send->servers[to_send_idx+1] == current_line) && (to_send_idx+1 < to_send->size))
                to_send_idx++;
            else
                break;
        }
        zsock_destroy(&sock);
    }

    fclose(fd);
    free(to_send->entries);
    free(to_send->servers);
    /*fprintf(stderr, "thread load_sender finished\n");*/
    pthread_exit(NULL);
}


void *thread_load_receiver(void *args)
{
    struct transfert_load_args *to_receive = args;

    /*int i;
    for (i = 0; i < to_receive->size; i++)
        fprintf(stderr, "(receiver: had to receive: entry n %d from server %d\n",
            to_receive->entries[i], to_receive->servers[i]);*/

    int rc;
    int fail_rc = -1;

    /*open the rep socket*/
    char *socket;
    if (asprintf(&socket, "tcp://0.0.0.0:%d", Transfert_port) == -1) {
        int err = errno;
        fprintf(stderr, "Distribution: thread_load_receiver: ");
        fprintf(stderr, "format zmq socket name error: %s\n", strerror(err));
        pthread_exit(&fail_rc);
    }

    zsock_t *sock = zsock_new_rep(socket);
    if (sock == NULL) {
        fprintf(stderr, "Distribution:thread_load_receiver: ");
        fprintf(stderr, "create zmq socket error\n");
        pthread_exit(&fail_rc);
    }
    free(socket);

    int to_receive_idx = 0;
    while (to_receive_idx < to_receive->size) {
        /*receive each message of the to_do list*/
        zmsg_t *packet = zmsg_recv(sock);
        zframe_t *entry_frame = zmsg_pop(packet);
        byte *tmp_entry = zframe_data(entry_frame);
        int entry;
        memcpy(&entry, tmp_entry, sizeof(int));

        zframe_t *md_entry_size_frame = zmsg_pop(packet);
        byte *tmp_md_entry_size = zframe_data(md_entry_size_frame);
        int md_entry_size;
        memcpy(&md_entry_size, tmp_md_entry_size, sizeof(int));

        zframe_t *md_names_frame = zmsg_pop(packet);
        byte *tmp_md_names = zframe_data(md_names_frame);
        char *md_names = malloc(md_entry_size);
        memcpy(md_names, tmp_md_names, md_entry_size);
        /*fprintf(stderr, "received entry %d, md_size: %d, received mdlist: %s\n",
            entry, md_entry_size, md_names);*/

        char *md_name = strtok(md_names, ",");

        /*add a new md_entry*/
        while (md_name != NULL) {
            struct md_entry *to_add = malloc(sizeof(struct md_entry));
            char *name;
            rc = asprintf(&name, "%s", md_name);
            if (rc < 0) {
                fprintf(stderr, "Distribution:thread_load_receiver:");
                fprintf(stderr, "get md_name failed: %s\n", strerror(-rc));
                zsock_destroy(&sock);
                pthread_exit(&fail_rc);
            }
            md_entry_init(to_add, entry, name);
            md_name = strtok(NULL, ",");

            rc = -pthread_rwlock_wrlock(&locks[entry]);
            if (rc != 0) {
                fprintf(stderr, "Distribution:thread_load_receiver: ");
                fprintf(stderr, "lock failed: %s\n", strerror(-rc));
                zsock_destroy(&sock);
                pthread_exit(&fail_rc);
            }

            md_entry_insert(&in_charge_md[entry], to_add);

            rc = -pthread_rwlock_unlock(&locks[entry]);
            if (rc != 0) {
                fprintf(stderr, "Distribution:thread_load_receiver:");
                fprintf(stderr, "unlock failed: %s\n", strerror(-rc));
                zsock_destroy(&sock);
                pthread_exit(&fail_rc);
            }
            rc = generic_put(in_charge_md[entry]->md_name, "a word");
            if (rc == -1) {
                fprintf(stderr, "Distribution:thread_load_receiver:");
                fprintf(stderr, "md creation failed\n");
                zsock_destroy(&sock);
                pthread_exit(&fail_rc);
            }
        }

        /*update the state of the entry in the mlt*/
        rc = mlt_update_state(&table, entry, 0);
        if (rc != 0) {
            fprintf(stderr, "Distribution:thread_load_receiver: ");
            fprintf(stderr, "mlt update failed:%s\n", strerror(-rc));
            zsock_destroy(&sock);
            pthread_exit(&fail_rc);
        }
        /*update eacl entry*/
        rc = eacl_reset_all_entry(&access_list, entry);
        if (rc != 0) {
            fprintf(stderr, "Distribution:thread_load_receiver: ");
            fprintf(stderr, "eacl update failed\n");
            zsock_destroy(&sock);
            pthread_exit(&fail_rc);
        }

        /*fprintf(stderr, "[receiver: received: entry n %d\n", entry);*/

        /*send an acknowledgement to the sender*/
        zmsg_t *reply = zmsg_new();
        zframe_t *flag = zframe_new("done", 4);
        zmsg_append(reply, &flag);

        zmsg_send(&reply, sock);

        to_receive_idx++;
    }

    zsock_destroy(&sock);
    free(to_receive->entries);
    free(to_receive->servers);
    /*fprintf(stderr, "thread load_receiver finished\n");*/
    pthread_exit(NULL);
}


void *thread_mlt_updater(void *args)
{
    /*opening a subscriber socket to the manager*/
    char *socket;
    asprintf(&socket, "tcp://%s:%d", ip_manager, Mlt_port);
    sub = zsock_new_sub(socket, "");
    if (sub == NULL) {
        fprintf(stderr,
            "Distribution:thread_mlt_updater: create zmq socket error\n");
        pthread_exit((void *)-1);
    }
    free(socket);

    int rc;
    while (1) {

        zmsg_t *packet = zmsg_recv(sub);

        struct mlt temp_mlt;
        rc = mlt_init(&temp_mlt, N_entry, 1);
        if (rc != 0)
            fprintf(stderr,
                "Distribution:thread_mlt_updater: temp_mlt init failed\n");

        zframe_t *id_srv_frame = zmsg_pop(packet);
        byte *temp = zframe_data(id_srv_frame);
        memcpy(temp_mlt.id_srv, temp, sizeof(uint32_t) * N_entry);
        zframe_destroy(&id_srv_frame);

        zframe_t *n_ver_frame = zmsg_pop(packet);
        temp = zframe_data(n_ver_frame);
        memcpy(temp_mlt.n_ver, temp, sizeof(uint32_t) * N_entry);
        zframe_destroy(&n_ver_frame);

        zmsg_destroy(&packet);

        /*to do lists given to receiver and sender threads*/
        struct transfert_load_args to_receive;
        to_receive.entries = malloc(N_entry * sizeof(int));
        to_receive.servers = malloc(N_entry * sizeof(int));
        to_receive.size = 0;
        struct transfert_load_args to_send;
        to_send.entries = malloc(N_entry * sizeof(int));
        to_send.servers = malloc(N_entry * sizeof(int));
        to_send.size = 0;

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
                    to_send.servers[to_send.size] = new_srv;
                    to_send.entries[to_send.size] = i;
                    to_send.size++;
                    rc = mlt_update_entry(&table, i, new_srv, new_ver, 1);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:thread_mlt_updater:");
                        fprintf(stderr,
                            "update mlt failed: %s\n", strerror(-rc));
                    }
                } else if (new_srv == id_srv_self) {
                    /*add to to_receive list*/
                    to_receive.servers[to_receive.size] = old_srv;
                    to_receive.entries[to_receive.size] = i;
                    to_receive.size++;
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
        pthread_t load_sender;
        pthread_t load_receiver;
        rc = pthread_create(&load_sender, NULL, &thread_load_sender, &to_send);
        if (rc != 0) {
            fprintf(stderr, "Distribution:thread_mlt_updater: ");
            fprintf(stderr,
                "thread load_sender init failed: %s\n", strerror(-rc));
        }
        rc = pthread_create(
            &load_receiver, NULL, &thread_load_receiver, &to_receive);
        if (rc != 0) {
            fprintf(stderr, "Distribution:thread_mlt_updater: ");
            fprintf(stderr,
                "thread load_receiver init failed: %s\n", strerror(-rc));
        }

        rc = pthread_join(load_sender, NULL);
        if (rc != 0) {
            fprintf(stderr, "Distribution:thread_mlt_updater: ");
            fprintf(stderr,
                "thread load_sender join failed: %s\n", strerror(-rc));
        }

        rc = pthread_join(load_receiver, NULL);
        if (rc != 0) {
            fprintf(stderr, "Distribution:thread_mlt_updater: ");
            fprintf(stderr,
                "thread load_receiver join failed: %s\n", strerror(-rc));
        }


        rc = mlt_destroy(&temp_mlt);
        if (rc != 0)
            fprintf(stderr, "MLT destroy failed\n");
 
        /*create the ack file to indicate the end of the redistribution*/
        char *file_name;
        asprintf(&file_name, "%svm%dUSR-1", SCRATCH, id_srv_self);
        int ack = open(file_name, O_WRONLY | O_EXCL | O_CREAT , 0664);
        if (ack == -1) {
            int err = errno;
            fprintf(stderr, "Server:sigUSR2 handler: ");
            fprintf(stderr, "create ack file \"%s\" failed\n/:%s",
                file_name, strerror(err));
        }
        close(ack);
   }

    /*cleaning*/
    zsock_destroy(&sub);
    pthread_exit(0);
}



int distribution_signal1_action()
{
    /*create the ack file to indicate the end of the redistribution*/
    char *file_name;
    asprintf(&file_name, "%svm%dUSR-0", SCRATCH, id_srv_self);
    int ack = open(file_name, O_WRONLY | O_EXCL | O_CREAT , 0664);
    if (ack == -1) {
        int err = errno;
        fprintf(stderr, "Distribution:sigUSR1 handler: ");
        fprintf(stderr, "create ack file \"%s\" failed\n/:%s",
            file_name, strerror(err));
        return -1;
    }
    close(ack);
    return 0;
}


int distribution_signal2_action()
{
    int rc = eacl_calculate_sai(&access_list);
    if (rc != 0) {
        fprintf(stderr,
            "Distribution:send_sai: calculate SAI failed: %s\n",
            strerror(-rc));
        return -1;
    }

    zmsg_t *packet = zmsg_new();
    zframe_t *sai_frame = zframe_new(access_list.sai,
        sizeof(uint32_t) * access_list.size);
    zmsg_append(packet, &sai_frame);


    rc = zmsg_send(&packet, push);
    if (rc != 0) {
        fprintf(stderr,
            "Distribution:send_sai: zmsg_send failed\n");
        return -1;
    }
    rc = eacl_reset_access(&access_list);

    return 0;
}
