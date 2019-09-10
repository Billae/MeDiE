#ifndef __DISTRIBUTION_WINDOWED_C_H__
#define __DISTRIBUTION_WINDOWED_C_H__

/**
 * @file distribution_indedh_c.h
 * @author E. Billa
 * @brief Clients functions dedicated to the independant dynamic hashing distribution method
 * **/

#include <json.h>
#include "mlt.h"
#include "protocol_windowed.h"


/** Initialize distribution context
 * - init the mlt
 * @param[in] nb number of available servers
 * @return 0 on success and -1 on failure
 * **/
int distribution_init(int nb);


/** Finalize distributino context
 * - free allocated memory
 * @return 0 on success and -1 on failure
 * **/
int distribution_finalize();


/** Operations to do before sending the request to the server
 * - add fields relative to distribution method (version number)
 * @param[in,out] request the request to verify
 * @return 0 on succes or -1 on failure
 * **/
int distribution_pre_send(json_object *request);


/** Operations to do after receiving the reply from the server
 *  - check the flag out-of-date and update if needed
 * @param[in] request the request to verify
 * @return 0 on succes or -1 on failure
 * **/
int distribution_post_receive(json_object *reply);


/** Add the id_srv field in the request
 * @param[in] key the key to store
 * @param[out] request the request to fill in
 * @return 0 on success and -1 on failure
 * **/
int distribution_assign_srv_by_key(const char *key, json_object *request);

#endif
