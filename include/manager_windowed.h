#ifndef __MANAGER_WINDOWED_H__
#define __MANAGER_WINDOWED_H__

/**
 * @file manager_indedh.h
 * @author E. Billa
 * @brief The manager is independant and update the mlt when a server ask it and broadcast it to the servers
 * **/

#include <czmq.h>
#include "protocol_windowed.h"
#include "mlt.h"
#include "eacl_windowed.h"


/** Initialize manager state
 * @param[in] nb the number of available servers
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
 * @param[in] nb number of available servers
 * @return 0 on success and -1 on failure
 * **/
int manager_calculate_relab(int nb);


#endif
