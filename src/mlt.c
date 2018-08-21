#include "mlt.h"

#include <errno.h>
#include <pthread.h>

/** Initialize the structure. The id_srv field is filled arbitrarely
 * with a modulo operation of the number of available servers.
 * @param[out] self     The mlt to fill in.
 * @param[in] size      Number of entry in the table.
 * @param[in] nb_srv    Number of available servers.
 * @return 0 on success and -<error code> on failure.
 * **/
int mlt_init(struct mlt *self, int size, int nb_srv)
{
    int i;
    int rc;

    /* check input arguments */
    if (!self || size <= 0 || nb_srv <= 0)
        return -EINVAL;

    /* allocate and initialize arrays of MLT */
    self->id_srv = calloc(size, sizeof(int));
    if (self->id_srv == NULL)
        return -ENOMEM;

    self->n_ver = calloc(size, sizeof(int));
    if (self->n_ver == NULL) {
        rc = -ENOMEM;
        goto out_free;
    }

    self->size = size;

    /* initialize RW lock */
    rc = -pthread_rwlock_init(&self->lock, NULL);
    if (rc != 0)
        goto out_free;

    /* fill the table with the modulo value,
     * and initialize version to 0 */
    for (i = 0; i < size; i++) {
        self->id_srv[i] = i % nb_srv;
        self->n_ver[i] = 0;
    }

    return 0;

out_free:
    free(self->id_srv);
    free(self->n_ver);
    return rc;
}


/** Update the server ID and the version number of the entry.
 * @param[out] self the mlt to update
 * @param[in] mlt_idx the line in the table to update
 * @param[in] srv_idx the new ID for the entry
 * @param[in] ver the version number of the entry
 * @return 0 on success and -<error code> on failure
 *
 * **/
int mlt_update_entry(struct mlt *self, int mlt_idx, int srv_idx, int ver)
{
    int rc;

    /* check input arguments */
    if (!self || mlt_idx < 0 || srv_idx < 0)
        return -EINVAL;
    if (mlt_idx >= self->size)
        return -EOVERFLOW;

    /* lock the array for write */
    rc = -pthread_rwlock_wrlock(&self->lock);
    if (rc != 0)
        return rc;

    self->id_srv[mlt_idx] = srv_idx;
    self->n_ver[mlt_idx] = ver;

    /* unlock & return */
    return -pthread_rwlock_unlock(&self->lock);
}


/** Give the server and the version number of an entry
 * @param[in] self the requested mlt
 * @param[in] mlt_idx the entry to retrieve
 * @param[out] srv server responsible of the entry
 * @param[out] ver version number of the entry
 * @return 0 on success and -<error code> on failure
 * **/
int mlt_get_entry(struct mlt *self, int mlt_idx, int *srv, int *ver)
{
    int rc;

    /* check input arguments */
    if (!self || mlt_idx < 0 || !srv || !ver)
        return -EINVAL;
    if (mlt_idx >= self->size)
        return -EOVERFLOW;

    /* lock the array for read */
    rc = -pthread_rwlock_rdlock(&self->lock);
    if (rc != 0)
        return rc;

    *srv = self->id_srv[mlt_idx];
    *ver = self->n_ver[mlt_idx];

    /* unlock & return */
    return -pthread_rwlock_unlock(&self->lock);
}

/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=4:tabstop=4:
 */
