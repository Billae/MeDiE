#ifndef __REQUEST_H__
#define __REQUEST_H__

/**
 * @file request.h
 * @author E. Billa
 * @brief module for managing request in json
 * **/

#include<json.h>


/**
 * Create base of request commom to all requests
 * @param key key to manage (store, retrieve, delete...)
 * @return request in json format. Caller must free() it after use.
 * **/
json_object* create_request(const char *key);


/**
 * Create request of type CREATE (1)
 * @param key key to store data
 * @param data data to store
 * @return request in json format. Caller must free() it after use.
 * **/
json_object* create_request_create(const char *key, const char *data);


/**
 * Create request of type READ (2)
 * @param key key to read data
 * @return request in json format. Caller must free() it after use.
 * **/
json_object* create_request_read(const char *key);


/**
 * Create request of type UPDATE (3)
 * @param key key to update data
 * @param data data to store
 * @return request in json format. Caller must free() it after use.
 * **/
json_object* create_request_update(const char *key, const char *data);


/**
 * Create request of type DELETE (4)
 * @param key key to delete
 * @return request in json format. Caller must free() it after use.
 * **/
json_object* create_request_delete(const char *key);


#endif
