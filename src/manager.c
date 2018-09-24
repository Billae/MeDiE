#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <errno.h>
#include <math.h>
#include "manager.h"
#include "protocol.h"
#include "mlt.h"
#include "eacl.h"

/*pcocc*/
//#define ip_manager "0.0.0.0"
/*ocre*/
#define ip_manager "192.168.129.25"

/*The manager has an sai list merged from all servers eacl (each field filled
 * with "0" in an eacl is a field not supported by this server). It has also its
 * own mlt which it can update (the manager has the "true" version of the mlt).
 * **/
static uint32_t *global_list;
static struct mlt table;


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


int manager_merge_eacl(uint32_t *new_list)
{
    int i;
    for (i = 0; i < N_entry; i++) {
        if (new_list[i] != 0)
            global_list[i] = new_list[i];
    }
    return 0;
}


/*Comparator function for the sort call (ungrowing order)*/
int comparator(const void *a, const void *b)  
{
    return *(int *)b - *(int *)a;
}


/** Give a subset of elements in list which
 * added to current_load become goal_load.
 * It is a greedy algorithm of giving money back (in O(n)).
 * @param[in] current_load the starting load
 * @param[in] list list of available loads for adding to current_load
 * @param[in] list_size size of list
 * @param[in] goal_load the load we want to approche
 * @param[in] list list of available loads for adding to current_load
 * @param[out] the subset of list when the algorithm finish
 * (ie can't add any element of list without to be too far of the goal_load)
 * @return the number of element in subset
 * **/

int manager_balance_load(
    int current_load, int *list, int list_size,  int goal_load,  int *subset)
{
    int idx = 0;
    int sum = current_load;
    int last_index = -1;
    qsort(list, list_size, sizeof(int), comparator);
    int i;
    for (i = 0; i < list_size; i++) {
        if ((sum + list[i]) <= 0) {
            sum += list[i];
            subset[idx] = list[i];
            idx++;
        } else
            last_index = i;
    }

    if (last_index != -1 && fabs(sum + list[last_index]) < fabs(sum)) {
        subset[idx] = list[last_index];
        idx++;
    }

    return idx;
}


/*Give the index in list which contain value*/
int find_index(int value, int *list, int list_size)
{
    int i;
    for (i = 0; i < list_size; i++) {
        if (list[i] == value)
            return i;
    }
    return -1;
}


/*TO DO*/
int manager_calculate_relab(int nb)
{
    fprintf(stderr, "Manager calculate_relab\n");
    int rc;

    /*all[i] is the sum of all sai of entries managed by the server i*/
    int *all = malloc(sizeof(int) * nb);
    if (all == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc all failed\n");
        return -1;
    }

    int i;
    for (i = 0; i < nb; i++)
        all[i] = 0;
    
    int sum_all = 0;
    for (i = 0; i < N_entry; i++) {
        int srv, version;
        rc = mlt_get_entry(&table, i, &srv, &version);
        if (rc != 0) {
            fprintf(stderr, "Manager:calculate_relab: mlt get entry failed\n");
            free(all);
            return -1;
        }
        all[srv] += global_list[i];
        sum_all += global_list[i];
    }
    int sum_w = w_factor * nb;


    /*load[i] is the relative load of each server*/
    int *load = malloc(sizeof(int) * nb);
    if (load == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc load failed\n");
        free(all);
        return -1;
    }

    /*create two subset: L is for large load (load[i]>0)
     * and S is for small load (load[i]<0)*/
    /*subset_l is filled with loads*/
    int *subset_l = calloc(nb, sizeof(int));
    if (subset_l == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc subset_l failed\n");
        free(all);
        return -1;
    }
    /*subset_s is only index*/
    int *subset_s = calloc(nb, sizeof(int));
    if (subset_s == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc subset_s failed\n");
        free(all);
        free(subset_l);
        return -1;
    }

    int size_l = 0;
    int size_s = 0;
    for (i = 0; i < nb; i++) {
        /*balanced is the balanced weight for the server i*/
        int balanced = (sum_all * w_factor) / sum_w;
        load[i] = all[i] - balanced;
        if (load[i] > 0) {
            subset_l[size_l] = load[i];
            size_l++;
        } else if (load[i] < 0) {
            subset_s[size_s] = i;
            size_s++;
        }
    }


    /*target is the index of idle server to give load*/
    int *target = malloc(sizeof(int) * nb);
    if (target == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc target failed\n");
        free(all);
        free(subset_l);
        free(subset_s);
        return -1;
    }
    for (i = 0; i < nb; i++)
        target[i] = i;


    int *subset_c = malloc(sizeof(int) * nb);
    int size_c = 0;
    if (subset_c == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc subset_c failed\n");
        free(all);
        free(subset_l);
        free(subset_s);
        free(target);
        return -1;
    }

    /*find which server in S will receive entries from which subset in L*/
    for (i = 0; i < size_s; i++) {
        size_c = manager_balance_load
            (load[subset_s[i]], subset_l, size_l, 0, subset_c);
        if (size_c == 0) {
            fprintf(stderr, "manage_balance_load() failed\n");
            free(all);
            free(subset_l);
            free(subset_s);
            free(target);
            return -1;
        }

        fprintf(stderr, "loads to give to server %d:\n", subset_s[i]);
        int j;
        for (j = 0; j < size_c; j++) {
            /*fill target*/
            int index = find_index(subset_c[j], load, nb);
            target[index] = subset_s[i];
            /*remove from subset_l server which give load*/
            int k;
            for (k = 0; k < size_l; k++) {
                if (subset_l[k] == subset_c[j]) {
                    subset_l[k] = subset_l[size_l-1];
                    size_l--;
                    break;
                }
            }
            fprintf(stderr, "-%d-", subset_c[j]);
        }
        fprintf(stderr, "\n");
    }
    free(subset_c);

    /*find for each server in L a subset of entries matching with the load to give*/
    for (i = 0; i < nb; i++) {
        if (load[i] > 0 && target[i] != i) {
            int size_srv = 0;
            int *subset_srv = malloc(sizeof(int) * N_entry);
            if (subset_srv == NULL) {
                fprintf(stderr,
                    "Manager:calculate_relab: malloc subset_srv failed\n");
                free(all);
                free(subset_l);
                free(subset_s);
                free(target);
                free(subset_c);
                return -1;
            }

            int j;
            /*find loads for the server i*/
            for (j = 0; j < N_entry; j++) {
                int srv, version;
                rc = mlt_get_entry(&table, i, &srv, &version);
                if (rc != 0) {
                    fprintf(stderr,
                        "Manager:calculate_relab: mlt get entry failed\n");
                    free(all);
                    free(subset_l);
                    free(subset_s);
                    free(target);
                    free(subset_c);
                    free(subset_srv);
                    return -1;
                }
                if (srv == i) {
                    subset_srv[size_srv] = global_list[j];
                    size_srv++;
                }
            }

            int *subset_sai = malloc(sizeof(int) * size_srv);
            int size_sai = 0;
            /*pick a subset of sai to obtain load*/
            size_sai = manager_balance_load
                (0, subset_srv, size_srv, load[i], subset_sai);
            if (size_sai == 0) {
                fprintf(stderr, "manage_balance_load() failed\n");
                free(all);
                free(subset_l);
                free(subset_s);
                free(target);
                return -1;
            }


            fprintf(stderr, "List of SAI to obtain the load to give:\n");
            /*update the MLT*/
            for (j = 0; j < size_sai; j++) {
                int k;
                fprintf(stderr, "%d, ", subset_sai[i]);
                for (k = 0; k < N_entry; k++) {
                    if (global_list[k] == subset_sai[j]) {
                        int srv, version;
                        rc = mlt_get_entry(&table, i, &srv, &version);
                         if (rc != 0) {
                            fprintf(stderr,
                                "Manager:calculate_relab: mlt get entry failed\n");
                            free(all);
                            free(subset_l);
                            free(subset_s);
                            free(target);
                            free(subset_sai);
                            free(subset_srv);
                            return -1;
                        }
                        if (srv == i) {
                            rc = mlt_update_entry
                                (&table, k, target[i], version + 1);
                            if (rc != 0) {
                                fprintf(stderr,
                                    "Manager:calculate_relab: mlt update entry failed\n");
                                free(all);
                                free(subset_l);
                                free(subset_s);
                                free(target);
                                free(subset_sai);
                                free(subset_srv);
                                return -1;
                            }
                        }
                    }
                }
            }
            free(subset_sai);
            free(subset_srv);
        }
    }

    free(target);
    free(subset_l);
    free(subset_s);
    free(all);
    free(load);
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
    int nb_srv = atoi(argv[1]);

    int rc;
    rc = manager_init(nb_srv);
    if (rc != 0) {
        fprintf(stderr, "Manager: init failed\n");
        return -1;
    }

    char *socket;

    /* Open the publisher socket to broadcast mlt updates*/

    asprintf(&socket, "tcp://%s:%d", ip_manager, Mlt_port);
    zsock_t *pub;
    pub = zsock_new_pub(socket);
    if (pub == NULL) {
            fprintf(stderr,
                "Manager: create zmq socket pub error\n");
            return -1;
    }


    /*Open the pull socket to receive eacl updates*/

    asprintf(&socket, "tcp://%s:%d", ip_manager, Eacl_port);
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
        rc = manager_calculate_relab(nb_srv);
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
