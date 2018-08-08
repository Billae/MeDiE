#ifndef __DISTRIBUTION_SH_H__
#define __DISTRIBUTION_SH_H__

/**
 * @file distribution.h
 * @author E. Billa
 * @brief Function dedicated to the static hashing distribution method
 * **/


/** Initialize the available number of servers
 * **/
void init_distribution_nbsrv(int nb);


/** Find the server number responsible of a key when this key is already assigned
 * **/
int *find_srv_by_key(const char *key);


/** Assign a server number to a key which is not already used
 * @param[in] key the key to store
 * @return the server number responsible of the key
 * **/
int assign_srv_by_key(const char *key);

#endif
