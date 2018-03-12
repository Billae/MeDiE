#ifndef __DISTRIBUTION_H__
#define __DISTRIBUTION_H__

/**
 * @file distribution.h
 * @author E. Billa
 * @brief Functions using data distribution across servers
 * **/


/** Initialize the servers list **/
int DistributionInit();


/** Add a server to the servers list
 * @param name the server to add
 * **/
int AddServerToList(char *name);


/** Clean distribution features: free servers list **/
void DistributionFinalize();


/** Find the server responsible of a key when this key is already assigned **/
const char *FindSrvByKey(const char *key);


/** Assign a server to a key which is not already used **/
const char *AssignSrvByKey(const char *key);

#endif
