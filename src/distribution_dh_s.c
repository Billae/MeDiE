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

#define max_id_size 21

/*pcocc*/
//#define ip_manager "10.252.0.1"
/*ocre*/
#define ip_manager "192.168.129.25"

/* path in pcocc*/
//#define SRV_PATH "/home/billae/prototype_MDS/etc/server.cfg"
/*path in ocre*/
#define SRV_PATH "/ccc/home/cont001/ocre/billae/prototype_MDS/etc/server.cfg"

/*path in pcocc*/
//#define HOST_PATH "/home/billae/prototype_MDS/etc/hosts.cfg"
/*path in ocre*/
#define HOST_PATH "/ccc/home/cont001/ocre/billae/prototype_MDS/etc/hosts.cfg"

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
            int i, found;
            found = 0;
            for (i = 0; i < N_entry; i++) {
                ptr = in_charge_md[i];

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
                    found = 1;
                    break;
                }
            }
            if (found == 0) {
                    fprintf(stderr, "Distribution:pre_send:");
                    fprintf(stderr, "remove key from in_charge_md failed\n");
                    return -1;
            }

        }
    }
    return 0;
}


/*Sort in growing order the servers and adapt the change to entries*/
void load_args_sort(struct transfert_load_args *list)
{
    int i, change;
    change = 1;
    while (change) {
        change = 0;
        for (i = 0; i < list->size; i++) {
            if (list->servers[i] > list->servers[i+1]) {
                change = 1;
                int tmp_srv = list->servers[i];
                int tmp_entry = list->entries[i];
                list->servers[i] = list->servers[i+1];
                list->entries[i] = list->entries[i+1];
                list->servers[i+1] = tmp_srv;
                list->entries[i+1] = tmp_entry;
            }
        }
    }
}


void *thread_load_sender(void *args)
{
    struct transfert_load_args *to_send = args;
    load_args_sort(to_send);

    fprintf(stderr, "load_sender: entries to give\n");
    int i;
    for (i = 0; i < to_send->size; i++)
        fprintf(stderr, "(sender: entry n %d to server %d\n",
            to_send->entries[i], to_send->servers[i]);

    int rc;
    int fail_rc = -1;
    FILE *fd = fopen(HOST_PATH, "r");
    if (fd == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution:thread_load_sender: ");
        fprintf(stderr, "open hosts file error: %s\n", strerror(err));
        pthread_exit(&fail_rc);
    }

    int current_line = 0;
    int to_send_idx = 0;
    int current_srv = to_send->servers[to_send_idx];

    char *id_srv;
    id_srv = malloc(max_id_size*sizeof(*id_srv));
    if (id_srv == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution:thread_load_sender: ");
        fprintf(stderr, "id_srv malloc error: %s\n", strerror(err));
        pthread_exit(&fail_rc);
    }

    while (fgets(id_srv, max_id_size, fd) != NULL && to_send_idx < to_send->size) {

        char *positionEntree = strchr(id_srv, '\n');
        if (positionEntree != NULL)
            *positionEntree = '\0';
        if (current_line == current_srv) {
            /*the srv_id read is one on the to_do list*/
            char *socket;
            if (asprintf(&socket, "tcp://%s:%d", id_srv, Transfert_port) == -1) {
                int err = errno;
                fprintf(stderr, "Distribution: thread_load_sender: ");
                fprintf(stderr, "format zmq socket name error: %s\n",
                        strerror(err));
                pthread_exit(&fail_rc);
            }

            zsock_t *sock = zsock_new_req(socket);
            if (sock == NULL) {
                fprintf(stderr, "Distribution:thread_load_sender: ");
                fprintf(stderr, "create zmq socket error\n");
                pthread_exit(&fail_rc);
            }
            free(socket);

            while (current_srv == current_line) {
                /*send all messages associated to this server*/
                zmsg_t *packet = zmsg_new();
                zframe_t *entry_frame = zframe_new(
                    &to_send->entries[to_send_idx], sizeof(int));
                zmsg_append(packet, &entry_frame);

                /*md_names is all md associated to an entry*/
                char *md_names = NULL;
                struct md_entry *ptr;
                int i;
                for (i = 0; i < N_entry; i++) {
                    if (in_charge_md[i]->entry == to_send->entries[to_send_idx]) {
                        while (in_charge_md[i] != NULL) {

                            rc = -pthread_rwlock_wrlock(&locks[i]);
                            if (rc != 0) {
                                fprintf(stderr, "Distribution:thread_load_sender: ");
                                fprintf(stderr, "lock failed: %s\n", strerror(-rc));
                                pthread_exit(&fail_rc);
                            }

                            ptr = md_entry_pop(&in_charge_md[i]);

                            rc = -pthread_rwlock_unlock(&locks[i]);
                            if (rc != 0) {
                                fprintf(stderr, "Distribution:thread_load_sender: ");
                                fprintf(stderr, "unlock failed: %s\n", strerror(-rc));
                                pthread_exit(&fail_rc);
                            }

                            int alloc_size = strlen(md_names) + strlen(ptr->md_name);
                            md_names = realloc(md_names, alloc_size);
                            if (md_names == NULL) {
                                fprintf(stderr, "Distribution:thread_load_sender: ");
                                fprintf(stderr, "realloc md_names failed\n");
                                pthread_exit(&fail_rc);
                            }
                            rc = snprintf(md_names, alloc_size, "%s,%s", md_names, ptr->md_name);
                            if (rc < 0) {
                                fprintf(stderr, "Distribution:thread_load_sender: ");
                                fprintf(stderr, "sprintf md_names failed\n");
                                pthread_exit(&fail_rc);
                            }
                            free(ptr->md_name);
                            free(ptr);
                        }
                        break;
                    }
                }
                zframe_t *md_entry_frame = zframe_new(&md_names, strlen(md_names));
                zmsg_append(packet, &md_entry_frame);

                rc = zmsg_send(&packet, sock);
                if (rc != 0) {
                    fprintf(stderr, "Distribution:thread_load_sender: ");
                    fprintf(stderr, "md_entry send failed\n");
                    pthread_exit(&fail_rc);
                }

                /*update the state of the entry in the mlt*/
                rc = mlt_update_state(&table, to_send_idx, 0);
                if (rc != 0) {
                    fprintf(stderr, "Distribution:thread_load_sender: ");
                    fprintf(stderr, "mlt update failed:%s\n", strerror(-rc));
                    pthread_exit(&fail_rc);
                }
                /*update eacl entry*/
                rc = eacl_reset_all_entry(&access_list, to_send_idx);
                if (rc != 0) {
                    fprintf(stderr, "Distribution:thread_load_receiver: ");
                    fprintf(stderr, "eacl update failed\n");
                    pthread_exit(&fail_rc);
                }

                /*receive the acknowledgement from the receiver*/
                zmsg_t *reply = zmsg_recv(sock);
                zframe_t *flag = zmsg_pop(reply);
                if (zframe_streq(flag, "done") != 0) {
                    fprintf(stderr, "Distribution:thread_load_receiver: ");
                    fprintf(stderr, "acknowledgement receive failed\n");
                    pthread_exit(&fail_rc);
                }

                /*next entry in the to_do list*/
                to_send_idx++;
                current_srv = to_send->servers[to_send_idx];
            }
            zsock_destroy(&sock);

        }
        current_line++;
    }

    fclose(fd);
    free(to_send->entries);
    free(to_send->servers);
    fprintf(stderr, "thread load_sender finished\n");
    pthread_exit(NULL);
}


void *thread_load_receiver(void *args)
{
    struct transfert_load_args *to_receive = args;
    load_args_sort(to_receive);

    fprintf(stderr, "load_receiver: entries to receive\n");
    int i;
    for (i = 0; i < to_receive->size; i++)
        fprintf(stderr, "receiver: entry n %d to server %d\n",
            to_receive->entries[i], to_receive->servers[i]);

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
        /*receive each message in the to_do list*/
        zmsg_t *packet = zmsg_recv(sock);
        zframe_t *entry_frame = zmsg_pop(packet);
        byte *tmp_entry = zframe_data(entry_frame);
        int entry;
        memcpy(&entry, tmp_entry, sizeof(int));

        zframe_t *md_names_frame = zmsg_pop(packet);
        byte *tmp_md_names = zframe_data(md_names_frame);
        char *md_names = malloc(sizeof(*tmp_md_names));
        memcpy(md_names, tmp_md_names, sizeof(*tmp_md_names));

        char *md_name = strtok(md_names, ",");

        /*add a new md_entry*/
        int i;
        for (i = 0; i < N_entry; i++) {

            if (in_charge_md[i] == NULL) {
                while (md_name != NULL) {
                    struct md_entry *to_add = malloc(sizeof(struct md_entry));
                    char *name;
                    rc = asprintf(&name, "%s", md_name);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:thread_load_receiver:");
                        fprintf(stderr, "get md_name failed: %s\n", strerror(-rc));
                        pthread_exit(&fail_rc);
                    }
                    md_entry_init(to_add, entry, name);
                    md_name = strtok(NULL, ",");

                    rc = -pthread_rwlock_wrlock(&locks[i]);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:thread_load_receiver: ");
                        fprintf(stderr, "lock failed: %s\n", strerror(-rc));
                        pthread_exit(&fail_rc);
                    }

                    md_entry_insert(&in_charge_md[i], to_add);

                    rc = -pthread_rwlock_unlock(&locks[i]);
                    if (rc != 0) {
                        fprintf(stderr, "Distribution:thread_load_receiver:");
                        fprintf(stderr, "unlock failed: %s\n", strerror(-rc));
                        pthread_exit(&fail_rc);
                    }
                }
                break;
            }
        }

        /*update the state of the entry in the mlt*/
        rc = mlt_update_state(&table, entry, 0);
        if (rc != 0) {
            fprintf(stderr, "Distribution:thread_load_receiver: ");
            fprintf(stderr, "mlt update failed:%s\n", strerror(-rc));
            pthread_exit(&fail_rc);
        }
        /*update eacl entry*/
        rc = eacl_reset_all_entry(&access_list, entry);
        if (rc != 0) {
            fprintf(stderr, "Distribution:thread_load_receiver: ");
            fprintf(stderr, "eacl update failed\n");
            pthread_exit(&fail_rc);
        }

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
    fprintf(stderr, "thread load_receiver finished\n");
    pthread_exit(NULL);
}


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
