#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <czmq.h>
#include "protocol.h"
#include "mlt.h"
#include "eacl.h"

/**
 * @file manager.h
 * @author E. Billa
 * @brief The manager is independant and periodically update the mlt and broadcast it to the servers
 * **/

/*The manager has an eacl merged from all servers eacl (each field filled with "0" in an eacl is a field not supported by this server). It has also its own mlt which it can update (the manager has the "true" version of the mlt).**/
static eacl_s global_list;
static mlt_s table;


/*variable indicating when a relab computation is needed (periodically setted to 1)*/
int update_needed;

/** Initialize manager state
 * - init the mlt and the eacl
 * - create the timer thread (to enable periodic updates) and set update_needed to 0
 * @return 0 on success and -1 on failure
 * **/
int init_manager();


/** Merge a just received eacl with the global_list. All fields filled with "0" are ignored.
 * @param[in] new_list the list to merge
 * @return 0 on success and -1 on failure
 * **/
int receive_eacl(eacl_s *new_list);


/** Algorithm to rebalance workload of each server
 * - calculate all server relative workload
 * - for each free server: assign a subset of overloaded servers such as both are balanced
 * - find a subset of entries in each overloaded server that match with the load it has to give (defined in the previous step)
 * - update the mlt with these load transferts
 * @return 0 on success and -1 on failure
 * **/
int calculate_relab();


/** Give a subset of elements in list which added to current_load become goal_load.
 * It is a greedy algorithm of giving money back (in O(n)).
 * @param current_load the starting load
 * @param goal_load the load we want to approche
 * @param list list of available loads for adding to current_load
 * @return the subset of list when the algorithm finish (ie can't add any element of list without to be too far of the goal_load)
 * **/
int *balance_load(int current_load, int goal_load, int *list);


/** Thread which receive eacl and then update the mlt using the relab computation 
 * - open the publisher socket (to broadcast mlt)
 * - connect to all server with a subscriber socket (to get eacls)
 * - check for eacl updates
 * - when the update_needed variable is to 1 launch a relab compute and a mlt update
 * **/
void *thread_manager(void *args);


/** Thread which will periodically wake up to set the update_needed variable to 1
 * **/
void *thread_timer(void *args);

#endif
