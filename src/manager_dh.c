#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <czmq.h>
#include <errno.h>
#include <signal.h>
#include <math.h>

#include "manager_dh.h"
#include "protocol_dh.h"
#include "mlt.h"
#include "eacl_dh.h"

/*pcocc*/
#define ip_manager "0.0.0.0"
/*ocre*/
//#define ip_manager "192.168.129.25"

/*The manager has an sai list merged from all servers eacl (each field filled
 * with "0" in an eacl is a field not supported by this server). It has also its
 * own mlt which it can update (the manager has the "true" version of the mlt).
 * **/
static uint32_t *global_list;
static struct mlt table;


/*Used to activate or not the relab computation*/
static volatile sig_atomic_t update_needed;


zsock_t *pub;
zsock_t *pull;


/*Handler for sigUSR2 signal*/
void usrHandler2(int sig)
{
    update_needed = 1;
}


/*Handler for sigINT signal*/
void intHandler(int sig)
{
    fprintf(stderr, "SigInt received, terminating...\n");
    manager_finalize();
    exit(0);
}


int manager_init(int nb)
{
    int rc;

    update_needed = 0;

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

    char *socket;

    /* Open the publisher socket to broadcast mlt updates*/

    asprintf(&socket, "tcp://%s:%d", ip_manager, Mlt_port);
    pub = zsock_new_pub(socket);
    if (pub == NULL) {
            fprintf(stderr,
                "Manager: create zmq socket pub error\n");
            return -1;
    }
    free(socket);
    socket = NULL;

    /*Open the pull socket to receive eacl updates*/

    asprintf(&socket, "tcp://%s:%d", ip_manager, Eacl_port);
    pull = zsock_new_pull(socket);
    if (pull == NULL) {
            fprintf(stderr,
                "Manager: create zmq socket pull error\n");
            return -1;
    }
    free(socket);

    return 0;
}


int manager_finalize()
{
    int rc;

    rc = mlt_destroy(&table);
    if (rc != 0)
        fprintf(stderr, "Manager:finalize: mlt_destroy failed\n");

    free(global_list);
    zsock_destroy(&pub);
    zsock_destroy(&pull);

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
        if ((sum + list[i]) <= goal_load) {
            if (list[i] == 0)
                break;
            sum += list[i];
            subset[idx] = list[i];
            idx++;
        } else
            last_index = i;
    }

    if (last_index != -1 && abs(sum + list[last_index]) < abs(sum)) {
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

    int current_srv;
    for (current_srv = 0; current_srv < nb; current_srv++)
        all[current_srv] = 0;

    int sum_all = 0;
    int current_entry;
    for (current_entry = 0; current_entry < N_entry; current_entry++) {
        int srv, version, state;
        rc = mlt_get_entry(&table, current_entry, &srv, &version, &state);
        if (rc != 0) {
            fprintf(stderr, "Manager:calculate_relab: ");
            fprintf(stderr, "mlt get entry %d failed\n", current_entry);
            goto free_all;
        }
        all[srv] += global_list[current_entry];
        sum_all += global_list[current_entry];
    }
    int sum_w = w_factor * nb;


    /*load[i] is the relative load of each server*/
    int *load = malloc(sizeof(int) * nb);
    if (load == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc load failed\n");
        goto free_all;
    }

    /*create two subset: L is for large load (load[i]>0)
     * and S is for small load (load[i]<0)*/
    /*subset_l is filled with loads*/
    int *subset_l = calloc(nb, sizeof(int));
    if (subset_l == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc subset_l failed\n");
        goto free_load;
    }
    /*subset_s is only index*/
    int *subset_s = calloc(nb, sizeof(int));
    if (subset_s == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc subset_s failed\n");
        goto free_subset_l;
    }

    int size_l = 0;
    int size_s = 0;
    for (current_srv = 0; current_srv < nb; current_srv++) {
        /*balanced is the balanced weight for the server i*/
        int balanced = (sum_all * w_factor) / sum_w;
        load[current_srv] = all[current_srv] - balanced;
        /*fprintf(stderr, "manager: load of server %d: %d\n",
            current_srv, load[current_srv]);*/
        if (load[current_srv] > 0) {
            subset_l[size_l] = load[current_srv];
            size_l++;
        } else if (load[current_srv] < 0) {
            subset_s[size_s] = current_srv;
            size_s++;
        }
    }


    /*target is the index of idle server to give load*/
    int *target = malloc(sizeof(int) * nb);
    if (target == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc target failed\n");
        goto free_subset_s;
    }
    for (current_srv = 0; current_srv < nb; current_srv++)
        target[current_srv] = current_srv;


    int *subset_c = malloc(sizeof(int) * size_l);
    int size_c = 0;
    if (subset_c == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc subset_c failed\n");
        goto free_target;
    }

    /*find which server in S will receive entries from which subset in L*/
    int srv_in_s;
    for (srv_in_s = 0; srv_in_s < size_s; srv_in_s++) {
        size_c = manager_balance_load
            (load[subset_s[srv_in_s]], subset_l, size_l, 0, subset_c);

        /*fprintf(stderr, "loads to give to server %d:\n", subset_s[srv_in_s]);*/
        int srv_in_c;
        for (srv_in_c = 0; srv_in_c < size_c; srv_in_c++) {
            /*fill target*/
            int index = find_index(subset_c[srv_in_c], load, nb);
            target[index] = subset_s[srv_in_s];
            /*remove from subset_l server which give load*/
            int srv_in_l;
            for (srv_in_l = 0; srv_in_l < size_l; srv_in_l++) {
                if (subset_l[srv_in_l] == subset_c[srv_in_c]) {
                    subset_l[srv_in_l] = subset_l[size_l-1];
                    size_l--;
                    break;
                }
            }
            /*fprintf(stderr, "-%d-", subset_c[srv_in_c]);*/
        }
        /*fprintf(stderr, "\n");*/
    }
    free(subset_c);

    /*find for each server in L
     * a subset of entries matching with the load to give*/
    for (current_srv = 0; current_srv < nb; current_srv++) {
        if (load[current_srv] > 0 && target[current_srv] != current_srv) {
            int size_srv = 0;
            int *subset_srv = malloc(sizeof(int) * N_entry);
            if (subset_srv == NULL) {
                fprintf(stderr,
                    "Manager:calculate_relab: malloc subset_srv failed\n");
                goto free_target;
            }

            int current_entry;
            /*find loads for the server current_srv*/
            for (current_entry = 0; current_entry < N_entry; current_entry++) {
                int srv, version, state;
                rc = mlt_get_entry(&table, current_entry,
                        &srv, &version, &state);
                if (rc != 0) {
                    fprintf(stderr, "Manager:calculate_relab: ");
                    fprintf(stderr, "mlt get entry %d failed\n", current_entry);
                    free(subset_srv);
                    goto free_target;
                }
                if (srv == current_srv) {
                    subset_srv[size_srv] = global_list[current_entry];
                    size_srv++;
                }
            }

            int *subset_sai = malloc(sizeof(int) * size_srv);
            if (subset_sai == NULL) {
                fprintf(stderr,
                    "Manager:calculate_relab: malloc subset_sai failed\n");
                free(subset_srv);
                goto free_target;
            }
            int size_sai = 0;
            /*pick a subset of sai to obtain load*/
            size_sai = manager_balance_load
                (0, subset_srv, size_srv, load[current_srv], subset_sai);

            /*fprintf(stderr,
                "List of SAI of server %d to obtain the load to give:\n",
                current_srv);*/
            /*update the MLT*/
            int sai_in_subset;
            for (sai_in_subset = 0; sai_in_subset < size_sai; sai_in_subset++) {
                /*fprintf(stderr, "%d, ", subset_sai[sai_in_subset]);*/
                for (current_entry = 0; current_entry < N_entry; current_entry++) {
                    if (global_list[current_entry] == subset_sai[sai_in_subset]) {
                        int srv, version, state;
                        rc = mlt_get_entry(&table, current_entry,
                                &srv, &version, &state);
                         if (rc != 0) {
                            fprintf(stderr, "Manager:calculate_relab: ");
                            fprintf(stderr, "mlt get entry %d failed\n", current_entry);
                            free(subset_sai);
                            free(subset_srv);
                            goto free_target;
                        }
                        if (srv == current_srv) {
                            rc = mlt_update_entry(&table, current_entry,
                                    target[current_srv], version + 1, 0);
                            if (rc != 0) {
                                fprintf(stderr, "Manager:calculate_relab: ");
                                fprintf(stderr, "mlt update entry %d failed\n", current_entry);
                                free(subset_sai);
                                free(subset_srv);
                                goto free_target;
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
    free(subset_s);
    free(subset_l);
    free(load);
    free(all);
    return 0;

free_target:
    free(target);
free_subset_s:
    free(subset_s);
free_subset_l:
    free(subset_l);
free_load:
    free(load);
free_all:
    free(all);
    return -1;
}


/** Receive eacl and then update the mlt
 * using the relab computation
 * - open the publisher socket (to broadcast mlt)
 * - open a pull socket to get eacls
 * - check for eacl updates
 * - launch a relab compute if a SIGUSR2 is received
 *   and then update and send the mlt
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

    struct sigaction act_int;
    memset(&act_int, 0, sizeof(struct sigaction));
    act_int.sa_handler = intHandler;
    rc = sigaction(SIGINT, &act_int, NULL);
    if (rc != 0)
        fprintf(stderr, "Manager: can't catch SIGINT\n");

    struct sigaction act_usr2;
    memset(&act_usr2, 0, sizeof(struct sigaction));
    act_usr2.sa_handler = usrHandler2;
    rc = sigaction(SIGUSR2, &act_usr2, NULL);
    if (rc != 0)
        fprintf(stderr, "Manager: can't catch SIGUSR2\n");

    while (1) {
        while (update_needed == 0) {
            sleep(1);
        }

        int nb_rcv;
        for (nb_rcv = 0; nb_rcv < nb_srv; nb_rcv++) {
            /*receiving eacls*/
            zmsg_t *packet = zmsg_recv(pull);
            if (packet == NULL)
                fprintf(stderr, "Manager: zmq receive failed\n");
            else {
                uint32_t *temp_sai = calloc(N_entry, sizeof(uint32_t));

                zframe_t *sai_frame = zmsg_pop(packet);
                byte *temp = zframe_data(sai_frame);
                memcpy(temp_sai, temp, sizeof(uint32_t)*N_entry);
                zframe_destroy(&sai_frame);
                /*fprintf(stderr, "sai received:%d\n", temp_sai[0]);*/

                rc = manager_merge_eacl(temp_sai);
                if (rc != 0)
                    fprintf(stderr, "Manager: merge eacl with global failed\n");
                free(temp_sai);
                /*fprintf(stderr, "global sai updated: %d\n", global_list[0]);*/
                zmsg_destroy(&packet);
            }
        }

        /*RELAB*/
        rc = manager_calculate_relab(nb_srv);
        if (rc != 0)
            fprintf(stderr, "Manager: relab computation failed\n");

        /*sending MLT*/
        zmsg_t *packet = zmsg_new();
        zframe_t *id_srv_frame = zframe_new(table.id_srv,
            sizeof(uint32_t) * table.size);
        zmsg_append(packet, &id_srv_frame);

        zframe_t *n_ver_frame = zframe_new(table.n_ver,
            sizeof(uint32_t) * table.size);
        zmsg_append(packet, &n_ver_frame);

        rc = zmsg_send(&packet, pub);
        if (rc != 0)
            fprintf(stderr, "Manager: zmsg_send new MLT failed\n");
        update_needed = 0;
    }

    /*cleaning*/
    manager_finalize();
    return 0;
}
