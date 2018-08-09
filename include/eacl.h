#ifndef __EACL_H__
#define __EACL_H__

/**
 * @file eacl.h
 * @author E. Billa
 * @brief Entry Access Counter List structure and getter and setter
 * **/

/*each attribute of the structure is a colunm in the table
 * (i.e. an array after initialization). entry is the index,
 * access_count gives the access frequency of the entry
 * and sai is a computed value used to balance the workload*/
struct eacl {
    int *entry;
    int *access_count;
    int *sai;
};


/** Initialize the structure with size entries. Other fields are setted to 0.
 * @param[out] self the eacl to fill in
 * @param[in] size number of entries in the table
 * @return 0 on success and -1 on failure
 * **/
int eacl_init(struct eacl *self, int size);


/** Increment the access counter of an entry.
 * @param[out] self the requested eacl
 * @param[in] index the acceded entry
 * **/
void eacl_incr_access(struct eacl *self, int index);


/** Set to 0 the access counter of an entry.
 * @param[out] self the requested eacl
 * @param[in] index the entry to reset
 * **/
void eacl_reset_access_entry(struct eacl *self, int index);


/** Set to 0 all fields for an entry.
 * @param[out] self the requested eacl
 * @param[in] index the entry to reset
 * **/
void eacl_reset_all_entry(struct eacl *self, int index);


/** Compute sai values and fill the sai field of each entry.
 * @param[in] self the requested eacl
 * sai computation: (1-alpha)*sai_old + alpha*access_count
 * (alpha is a fogotten factor)
 * @return 0 on success and -1 on failure
 * **/
int eacl_calculate_sai(struct eacl *self);


/** Give the sai field of an entry.
 *  @param[in] index the entry asked
 *  @return the sai value or -1 on failure
 * **/
int eacl_read_sai(int index);


#endif
