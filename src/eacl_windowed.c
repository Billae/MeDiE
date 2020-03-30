#include "eacl_windowed.h"

#include <errno.h>
#include <stdlib.h>
#include "queue.h"

/*alpha is a forgotten factor (0<alpha<1)*/
#ifndef ALPHA
    #define ALPHA (0.5)
#endif
/*window_size is the number of step to take into account in the sai computation (a step is usually 5min)*/
#ifndef WINDOW_SIZE
    #define WINDOW_SIZE 12
#endif

/** Initialize the structure with size entries. Other fields are setted to 0.
 * @param[out] self the eacl to fill in
 * @param[in] size number of entries in the table
 * @return 0 on success and -<error code> on failure
 * **/
int eacl_init(struct eacl *self, int size)
{
    int rc;

    /* check input arguments */
    if (!self || size <= 0)
        return -EINVAL;

    /* allocate and initialize arrays */
    self->access_count = calloc(size, sizeof(uint32_t));
    if (self->access_count == NULL)
        return -ENOMEM;

    self->sai = calloc(size, sizeof(uint32_t));
    if (self->sai == NULL) {
        rc = -ENOMEM;
        goto out_free;
    }

    self->old_accesses = calloc(size, sizeof(struct queue *));
    if (self->old_accesses == NULL) {
        rc = -ENOMEM;
        goto out_free;
    }

    int i, j;
    for (i = 0; i < size; i++) {
        self->old_accesses[i] = queue_new(WINDOW_SIZE);
        if (self->old_accesses[i] == NULL) {
            rc = ENOMEM;
            goto queue_free;
        }
    }

    self->load_lvl = 0;
    self->size = size;

    /* Note: as arrays are allocated with calloc,
     * they are already filled with 0 */
    return 0;

queue_free:
    for (j = 0; j < i; j++)
        queue_destroy(self->old_accesses[j]);
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


/** Set to 0 the access counter of all entries.
 * @param[out] self the requested eacl
 * @return 0 on success and -1 on failure
 * **/
int eacl_reset_access(struct eacl *self)
{
    if (!self)
        return -1;

    int i;
    for (i = 0; i < self->size; i++) {
        queue_put(self->old_accesses[i], self->access_count[i]);
        self->access_count[i] = 0;
    }

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
    queue_reset(self->old_accesses[index]);
    return 0;
}


/** Compute sai values and fill the sai field of each entry.
 * Then, compute the load_lvl.
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

    self->load_lvl = 0;
    for (i = 0; i < self->size; i++) {
        self->sai[i] = (1 - ALPHA) * queue_mean(self->old_accesses[i]) + ALPHA * self->access_count[i];
        self->load_lvl += self->sai[i];
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


int eacl_read_load_lvl(struct eacl *self)
{
    if (!self)
        return -EINVAL;

    return self->load_lvl;
}


int eacl_destroy(struct eacl *self)
{
    free(self->access_count);
    free(self->sai);
    int j;
    for (j = 0; j < self->size; j++)
        queue_destroy(self->old_accesses[j]);
    free(self->old_accesses);
    return 0;
}

/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=4:tabstop=4:
 */
