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
int distribution_init();


/** Finalize distribution context
 * **/
int distribution_finalize();


/** Operation to do after receiving the request from the client
 * **/
int distribution_post_receive(json_object *request);


/** Operation to do before sending the reply to the client
 * **/
int distribution_pre_send(json_object *reply, int global_rc);


/** function called on signal SIGUSR1
* do nothing and create the ack file
* **/
int distribution_signal1_action();


/** function called on signal SIGUSR2
* do nothing and create the ack file
* **/
int distribution_signal2_action();


#endif
