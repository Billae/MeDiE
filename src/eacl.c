#include "eacl.h"

#include <errno.h>
#include <stdlib.h>

/** Initialize the structure with size entries. Other fields are setted to 0.
 * @param[out] self the eacl to fill in
 * @param[in] size number of entries in the table
 * @return 0 on success and -<error code> on failure
 * **/
int eacl_init(struct eacl *self, int size)
{
    int i;
    int rc;

    /* check input arguments */
    if (!self || size <= 0)
        return -EINVAL;

    /* allocate and initialize arrays */
    self->access_count = calloc(size, sizeof(int));
    if (self->access_count == NULL)
        return -ENOMEM;

    self->sai = calloc(size, sizeof(int));
    if (self->sai == NULL) {
        rc = -ENOMEM;
        goto out_free;
    }

    self->size = size;

    /* Note: as arrays are allocated with calloc,
     * they are already filled with 0 */

    return 0;

out_free:
    free(self->access_count);
    free(self->sai);
    return rc;
}


/** Increment the access counter of an entry.
 * @param[out] self the requested eacl
 * @param[in] index the accessed entry
 * @return 0 on success and -1 on failure
 * **/
int eacl_incr_access(struct eacl *self, int index)
{
    if (!self || index < 0 || index >= self->size)
        return -1;

    self->access_count[index]++;
    return 0;
}

/** Set to 0 the access counter of an entry.
 * @param[out] self the requested eacl
 * @param[in] index the entry to reset
 * @return 0 on success and -1 on failure
 * **/
int eacl_reset_access_entry(struct eacl *self, int index)
{
    if (!self || index < 0 || index >= self->size)
        return -1;

    self->access_count[index] = 0;
    return 0;
}

/** Set to 0 all fields for an entry.
 * @param[out] self the requested eacl
 * @param[in] index the entry to reset
 * @return 0 on success and  -1 on failure
 * **/
int eacl_reset_all_entry(struct eacl *self, int index)
{
    if (!self || index < 0 || index >= self->size)
        return -1;

    self->access_count[index] = 0;
    self->sai[index] = 0;
    return 0;
}


/** Compute sai values and fill the sai field of each entry.
 * @param[in] self the requested eacl
 * sai computation: (1-alpha)*sai_old + alpha*access_count
 * (alpha is a fogotten factor)
 * @return 0 on success and -<error code> on failure
 * **/
int eacl_calculate_sai(struct eacl *self)
{
    int i;

    /* return in case of error */
    if (!self)
        return -EINVAL;

    for (i = 0; i < self->size; i++) {
        /* FIXME
         * For this initial code draft we use
         *  sai = access_count, but this has to be changed for the
         *  actual formula. */
        self->sai[i] = self->access_count[i];
    }

    return 0;
}


/** Give the sai field of an entry.
 *  @param[in] index the entry asked
 *  @return the sai value or -<error code> on failure.
 * **/
int eacl_read_sai(struct eacl *self, int index)
{
    if (!self || index < 0 || index >= self->size)
        return -EINVAL;

    return self->sai[index];
}

/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=4:tabstop=4:
 */
