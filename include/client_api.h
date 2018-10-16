#ifndef __CLIENT_API_H__
#define __CLIENT_API_H__

/**
 * @file client_api.h
 * @author E. Billa
 * @brief user API to create a client context
 * **/

#include <json.h>
#include <czmq.h>


/** Initialize the client context:
 *      - connect to nb_srv available servers and keep connexions in a static tab
 *      - initialize the available number of servers in distribution
 * @param[in] nb_srv the number of server available to connect on
 * @return a positive number on success and a negative one on failure.
 * The number (positive or negative) indicates how many servers is connected.
 * 0 is also a failure return value
 * **/
int client_init_context(int nb_srv);


/** Clean context: close all open ZMQ connexion and free the server list
 * **/
void client_finalize_context();


/** Initialize a ZMQ connexion to the host as a request socket
 * @param[in] id_srv the server to connect on
 * **/
zsock_t *client_init_connexion(const char *id_srv);


/** Generic following of a request processing
 * @param[in] request the request to send
 * @param[out] reply the answer to the request
 * @return 0 on succes or -<error code> on failure
 * **/
int client_request(json_object *request, json_object **reply);


/** Ask for the creation of a new key-value pair in the storage system
 * @param[in] key the key where data has to be stored
 * @param[in] data the data to store
 * @return 0 on succes or -<error code> on failure
 * **/
int client_request_create(const char *key, const char *data);


/** Ask for the retrieval of the value associated in the storage system
 * @param[in] key the key to retrieve
 * @param[out] returnvalue the data associated to key
 * @return 0 on succes or -<error code> on failure
 * **/
int client_request_read(const char *key, char **return_value);


/** Ask for the update of a key-value pair in the storage system
 * @param[in] key the key associated to data to update
 * @param[in] data the new value of data to store
 * @return 0 on succes or -<error code> on failure
 * **/
int client_request_update(const char *key, const char *data);


/** Ask for the deletion of a key-value pair in the storage system
 * @param[in] key the key to delete
 * @return 0 on succes or -<error code> on failure
 * **/
int client_request_delete(const char *key);


#endif
