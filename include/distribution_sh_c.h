#ifndef __DISTRIBUTION_SH_C_H__
#define __DISTRIBUTION_SH_C_H__

/**
 * @file distribution_sh_c.h
 * @author E. Billa
 * @brief Client functions dedicated to the static hashing distribution method
 * **/

#include <json.h>


/** Initialize the available number of servers
 * **/
int init_distribution(int nb);


/** Finalize distribution context
 * **/
int finalize_distribution();


/** Operation to do before sending the request to the server
 * **/
int pre_send(json_object *request);


/** Operation to do after receiving the reply from the server
 * **/
int post_receive(json_object *reply);


/** Find the server number responsible of a key when
 * this key is already assigned **/
int *find_srv_by_key(const char *key);


/** Assign a server number to a key which is not already used
 * @param[in] key the key to store
 * @return the server number responsible of the key
 * **/
int assign_srv_by_key(const char *key);

#endif
