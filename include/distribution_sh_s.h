#ifndef __DISTRIBUTION_SH_S_H__
#define __DISTRIBUTION_SH_S_H__

/**
 * @file distribution_sh_s.h
 * @author E. Billa
 * @brief Server functions dedicated to the static hashing distribution method
 * **/

#include <json.h>


/** Initialize the available number of servers
 * **/
int init_distribution(int nb);


/** Finalize distribution context
 * **/
int finalize_distribution();


/** Operation to do after receiving the request from the client 
 * **/
int post_receive(json_object *request);


/** Operation to do before sending the reply to the client
 * **/
int pre_send(json_object *reply);


#endif
