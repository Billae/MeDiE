#ifndef __MANAGER_H__
#define __MANAGER_H__

/**
 * @file manager.h
 * @author E. Billa
 * @brief The manager is independant and periodically update the mlt and broadcast it to the servers
 * **/

#include <czmq.h>
#include "protocol.h"
#include "mlt.h"
#include "eacl.h"


/** Initialize manager state
 * @param nb the number of available servers
 * @return 0 on success and -1 on failure
 * **/
int manager_init(int nb);


/** Finalize manager state
 * - free allocated memory
 * @return 0 on success and -1 on failure
 * **/
int manager_finalize();


/** Merge a just received sai field with the global_list.
 * All fields filled with "0" are ignored.
 * @param[in] new_list the list to merge
 * @return 0 on success and -1 on failure
 * **/
int manager_merge_eacl(uint32_t *new_list);


/** Algorithm to rebalance workload of each server
 * - calculate all server relative workload
 * - for each free server: assign a subset of overloaded servers
 *   such as both are balanced
 * - find a subset of entries in each overloaded server
 *   that match with the load it has to give (defined in the previous step)
 * - update the mlt with these load transferts
 * @return 0 on success and -1 on failure
 * **/
int manager_calculate_relab();


/** Give a subset of elements in list which
 * added to current_load become goal_load.
 * It is a greedy algorithm of giving money back (in O(n)).
 * @param current_load the starting load
 * @param goal_load the load we want to approche
 * @param list list of available loads for adding to current_load
 * @return the subset of list when the algorithm finish
 * (ie can't add any element of list without to be too far of the goal_load)
 * **/
int *manager_balance_load(int current_load, int goal_load, int *list);


#endif
