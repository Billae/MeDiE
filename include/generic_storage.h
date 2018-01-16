#ifndef __GENERIC_STORAGE_H__
#define __GENERIC_STORAGE_H__

/**
 * @file generic_storage.h
 * @author E. Billa
 * @brief generic storage API: execute storage operations with the functions related to the type of server
 * **/


/** Store data in a location addressed by key 
 * @return 0 if successful
 * **/
int generic_put(const char* data, const char* key);


/** Retrieve data associated to a key
 * @param key key to retrieve
 * @return the associated data
 * **/
char* generic_get(char* key);


/** Delete a key and data related
 * @return 0 if successful
 * **/
int generic_del(char* key);

#endif
