#ifndef __DISTRIBUTION_H__
#define __DISTRIBUTION_H__

/**
 * @file distribution.h
 * @author E. Billa
 * @brief Functions using data distribution across servers
 * **/


/** Find the server responsible of a key when this key is already assigned **/
char* FindSrvByKey(char* key);


/** Assign a server to a key which is not already used **/
char* AssignSrvByKey(char* key);

#endif
