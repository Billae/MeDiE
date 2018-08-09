#ifndef __DISTRIBUTION_DH_S_H__
#define __DISTRIBUTION_DH_S_H__

/**
 * @file distribution_dh_s.h
 * @author E. Billa
 * @brief Servers functions dedicated to the dynamic hashing distribution method
 * **/

#include <czmq.h>
#include <json.h>
#include "mlt.h"
#include "eacl.h"


/*Each server has its own mlt and its own eacl with access counter accessed
 * only in the distribution functions*/
static struct mlt table;
static struct eacl access_list;


/** Initialize distribution context
 * - init the mlt and and the eacl
 * - create a thread to maintain updated the mlt and
 *   another to send the eacl to the manager
 * @return 0 on success and -1 on failure
 * **/
int init_distribution();


/** Finalize distribution context
 * - free allocated memory
 * @return 0 on success and -1 on failure
 * **/
int finalize_distribution();


/** Operation to do after receiving the request from a client
 * and before send it the reply
 * - check the version number of the entry
 * - increment counter access in the eacl if the version number is valid
 * - set flags
 * @param[in,out] request the request to verify
 * @return 0 on success and -1 on failure
 * **/
int post_receive(json_object *request);


/** Operation to do before send the reply to the client
 * - set flags
 * @param[out] reply the reply to complete
 * @return 0 on success and -1 on failure
 * **/
int pre_send(json_object *reply);


/** Transfert data of an entry to another server (or reverse case)
 * depending on the new mlt
 * - open a socket: if free server a rep socket and
 *   if overloaded server a req socket
 * - send or receive data associated with the entry
 * - close the socket
 * @return 0 on success and -1 on failure
 * **/
int transfert_load(int entry);


/** Thread dedicated to keeping the mlt up-to-date
 * - subscribe to the publisher socket of the manager to receive mlt
 * - wait for mlt updates and process them
 * - launch inter-server transferts due to mlt updates
 * **/
void *thread_mlt_updater(void *args);


/** Thread which will periodically wake up and send the eacl to the manager.
 * - open a publisher socket to send eacl
 * - periodically send the eacl to the manager
 * **/
void *thread_eacl_sender(void *args);


#endif
