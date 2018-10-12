#ifndef __GENERIC_STORAGE_H__
#define __GENERIC_STORAGE_H__

/**
 * @file generic_storage.h
 * @author E. Billa
 * @brief generic storage API: execute storage operations with the functions related to the type of server
 * **/


/** Store data in a location addressed by key
 * @return 0 on success and -1 on failure
 * **/
int generic_put(const char *key, const char *value);


/** Retrieve data associated to a key
 * @param[in] key key to retrieve
 * @return the associated data or NULL on failure. Caller must free() it after use.
 * **/
char *generic_get(const char *key);


/** Update data associated to a key
 * @param[in] key key associated to data to modify
 * @param[in] value new value of data
 * @return 0 on success and -1 on failure
 * **/
int generic_update(const char *key, const char *value);


/** Delete a key and data related
 * @return 0 on success and -1 on failure
 * **/
int generic_del(const char *key);

#endif
