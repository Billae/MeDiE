#ifndef __EACL_WINDOWED_H__
#define __EACL_WINDOWED_H__

#include <stdint.h>

/**
 * @file eacl_indedh.h
 * @author E. Billa
 * @brief Entry Access Counter List structure and getter and setter
 * **/

/*each attribute of the structure is a colunm in the table
 * (i.e. an array after initialization).
 * - entry is the index,
 * - access_count gives the access frequency of the entry
 * - sai is a computed value used to balance the workload
 * - load_lvl is the sum of all sai and means the load for the server*/
struct eacl {
    uint32_t *access_count;
    uint32_t *sai;

    uint32_t load_lvl;
    uint32_t size;
};


/** Initialize the structure with size entries. Other fields are setted to 0.
 * @param[out] self the eacl to fill in
 * @param[in] size number of entries in the table
 * @return 0 on success and -<error code> on failure
 * **/
int eacl_init(struct eacl *self, int size);


/** Increment the access counter of an entry.
 * @param[out] self the requested eacl
 * @param[in] index the acceded entry
 * @return 0 on success and -1 on failure
 * **/
int eacl_incr_access(struct eacl *self, int index);


/** Set to 0 the access counter of all entries.
 * @param[out] self the requested eacl
 * @return 0 on success and -1 on failure
 * **/
int eacl_reset_access(struct eacl *self);


/** Set to 0 all fields for an entry.
 * @param[out] self the requested eacl
 * @param[in] index the entry to reset
 * @return 0 on success and -1 on failure
 * **/
int eacl_reset_all_entry(struct eacl *self, int index);


/** Compute sai values and fill the sai field of each entry.
 * @param[in] self the requested eacl
 * sai computation: (1-alpha)*sai_old + alpha*access_count
 * (alpha is a fogotten factor)
 * @return 0 on success and -<error code> on failure
 * **/
int eacl_calculate_sai(struct eacl *self);


/** Give the sai field of an entry.
 *  @param[in] index the entry asked
 *  @return the sai value or -<error code> on failure.
 * **/
int eacl_read_sai(struct eacl *self, int index);


/** Give the load level of the server
 *  @param[in] self the requested eacl
 *  @return the load value or -<error code> on failure.
 * **/
int eacl_read_load_lvl(struct eacl *self);


/** Destroy the structure.
 * - Desallocate memory
 * @param[out] self the eacl to destroy
 * @return 0 on success and -1 on failure**/
int eacl_destroy(struct eacl *self);


#endif
/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=4:tabstop=4:
 */
