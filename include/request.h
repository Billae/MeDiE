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
 * @return request in json format
 * **/
json_object* create_request(char* key);

/**
 * Create request of type CREATE
 * @param key key to store data
 * @param data data to store
 * @return request in json format
 * **/
json_object* create_request_create(char* key, char* data);

#endif
