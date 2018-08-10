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
int distribution_init(int nb);


/** Finalize distribution context
 * **/
int distribution_finalize();


/** Operation to do before sending the request to the server
 * **/
int distribution_pre_send(json_object *request);


/** Operation to do after receiving the reply from the server
 * **/
int distribution_post_receive(json_object *reply);


/** Find the server number responsible of a key when
 * this key is already assigned **/
int distribution_find_srv_by_key(const char *key);


/** Assign a server number to a key which is not already used
 * @param[in] key the key to store
 * @return the server number responsible of the key
 * **/
int distribution_assign_srv_by_key(const char *key);

#endif
