#ifndef __DISTRIBUTION_DH_S_H__
#define __DISTRIBUTION_DH_S_H__

/**
 * @file distribution_dh_s.h
 * @author E. Billa
 * @brief Servers functions dedicated to the dynamic hashing distribution method
 * **/

#include <czmq.h>
#include <json.h>

struct transfert_load_args {
    int *entries;
    int *servers;
    int size;
};

/** Initialize distribution context
 * - init the mlt and and the eacl
 * - create a thread to maintain updated the mlt and
 *   another to send the eacl to the manager
 * @return 0 on success and -1 on failure
 * **/
int distribution_init();


/** Finalize distribution context
 * - free allocated memory
 * @return 0 on success and -1 on failure
 * **/
int distribution_finalize();


/** Operation to do after receiving the request from a client
 * - check the version number of the entry
 * - increment counter access in the eacl if the version number is valid
 * - if the version number is invalid,
 *   give the right version and information related
 * - set flags
 * @param[in,out] request the request to verify
 * @return 0 on success and -1 on failure
 * **/
int distribution_post_receive(json_object *request);


/** Operation to do before send the reply to the client
 * - set flags
 * @param[out] reply the reply to complete
 * @param[in] global_rc the global state of the request
 * @return 0 on success and -1 on failure
 * **/
int distribution_pre_send(json_object *reply, int global_rc);


/** Transfert data of an entry to another server
 * depending on the table given in argument
 * - open a req socket
 * - send md associated to the entries
 * - close the socket
 * @param[in] args the to_do list
 * @return 0 on success and -1 on failure
 * **/
void *thread_load_sender(void *args);


/** Transfert data of an entry from another server
 * depending on the table given in argument
 * - open a rep socket
 * - receive md associated to the entries
 * - close the socket
 * @param[in] args the to_do list
 * @return 0 on success and -1 on failure
 * **/
void *thread_load_receiver(void *args);


/** Thread dedicated to keeping the mlt up-to-date
 * - subscribe to the publisher socket of the manager to receive mlt
 * - wait for mlt updates and process them
 * - launch inter-server transferts due to mlt updates
 * **/
void *thread_manager_listener(void *args);


/** function called on signal SIGUSR2
 * calculate and send the sai field of the eacl to the manager.
 * **/
int distribution_signal_action();


#endif
