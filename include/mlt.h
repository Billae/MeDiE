#ifndef __MLT_H__
#define __MLT_H__

#include <stdlib.h>

/**
 * @file mlt.h
 * @author E. Billa
 * @brief Metadata Lookup Table structure and getter and setter
 * **/


/* Each attribute of the structure is a colunm in the table
 * (i.e. an array after initialization).
 *  - id_srv gives the server responsible of this entry;
 *  - n_ver indicates the latest version number of the entry;
 *  - size stores the allocated array size.
 *  - lock protects the whole table for current accesses.
 *    NB: as access gain is per raw, this could be optimized by
 *    using a lock per raw instead of a global MLT lock.
 */
struct mlt {
    int             *id_srv;
    int             *n_ver;

    int             size;
    pthread_rwlock_t lock;
};


/** Initialize the structure. The id_srv field is filled arbitrarely
 * with a modulo operation of the number of available servers.
 * @param[out] self the mlt to fill in
 * @param[in] size number of entry in the table
 * @param[in] nb_srv number of available servers
 * @return 0 on success and -<error code> on failure
 * **/
int mlt_init(struct mlt *self, int size, int nb_srv);


/** Update the server ID and the version number of the entry.
 * @param[out] self the mlt to update
 * @param[in] mlt_idx the line in the table to update
 * @param[in] srv_idx the new ID for the entry
 * @param[in] ver the version number of the entry
 * @return 0 on success and -<error code> on failure
 * **/
int mlt_update_entry(struct mlt *self, int mlt_idx, int srv_idx, int ver);


/** Give the server and the version number of an entry
 * @param[in] self the requested mlt
 * @param[in] mlt_idx the entry to retrieve
 * @param[out] srv server responsible of the entry
 * @param[out] ver version number of the entry
 * @return 0 on success and -<error code> on failure
 * **/
int mlt_get_entry(struct mlt *self, int mlt_idx, int *srv, int *ver);


/**
 * Destroy the structure. 
 * - Desallocate memory
 * @param[out] self the mlt to destroy
 * @return 0 on success and -1 on failure**/
int mlt_destroy(struct mlt *self);


#endif
/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=4:tabstop=4:
 */
