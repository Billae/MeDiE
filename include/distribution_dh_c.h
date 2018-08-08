#ifndef __DISTRIBUTION_DH_C_H__
#define __DISTRIBUTION_DH_C_H__

#include <json.h>
#include "mlt.h"
#include "protocol.h"

/**
 * @file distribution_dh_c.h
 * @author E. Billa
 * @brief Clients functions dedicated to the dynamic hashing distribution method
 * **/


/*Each client has its own mlt accessed only in the distribution functions*/
static mlt_s table;


/** Initialize distribution context
 * - init the mlt
 * @param[in] nb number of available servers
 * @return 0 on success and -1 on failure
 * **/
int init_distribution(int nb);


/** Finalize distributino context
 * - free allocated memory
 * @return 0 on success and -1 on failure
 * **/
int finalize_distribution();


/** Operations to do before sending the request to the server
 * - add fields relative to distribution method (version number)
 * @param[in,out] request the request to verify
 * @return 0 on succes or -1 on failure 
 * **/
int pre_send(json_object *request);


/** Operations to do after receiving the request from the server
 *  - check the flag out-of-date
 * @param[in] request the request to verify
 * @return 0 on succes or -1 on failure 
 * **/
int post_receive(json_object *request);


/** Find the server number responsible of a key when this key is already assigned
 * **/
int *find_srv_by_key(const char *key);


/** Assign a server number to a key which is not already used
 * @param key the key to store
 * @return the server number responsible of the key
 * **/
int assign_srv_by_key(const char *key);

#endif
