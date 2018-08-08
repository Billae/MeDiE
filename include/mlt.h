#ifndef __MLT_H__
#define __MLT_H__

/**
 * @file mlt.h
 * @author E. Billa
 * @brief Metadata Lookup Table structure and getter and setter
 * **/


/*each attribute of the structure is a colunm in the table (i.e. an array after initialization). entry is the index, id_srv gives the server responsible of this entry and n_ver indicates the latest version number of the entry*/
typedef struct mlt
{
    int *entry;
    int *id_srv;
    int *n_ver;
} mlt_s;


/** Initialize the structure. The id_srv field is filled arbitrarely with a modulo operation of the number of available servers.
 * @param[out] self the mlt to fill in
 * @param[in] size number of entry in the table
 * @param[in] nb_srv number of available servers
 * @return 0 on success and -1 on failure
 * **/
int mlt_init(mlt_s *self, int size, int nb_srv);


/** Update the server ID and the version number of the entry.
 * @param[out] self the mlt to update
 * @param[in] entry the line in the table to update
 * @param[in] new_srv the new ID for the entry
 * @param[in] ver the version number of the entry
 * @return 0 on success and -1 on failure
 * **/
int mlt_update_entry(mlt_s *self, int entry, int new_srv, int ver);


/** Give the server and the version number of an entry
 * @param[in] self the requested mlt
 * @param[in] entry the entry to retrieve
 * @param[out] srv server responsible of the entry
 * @param[out] ver version number of the entry
 * @return 0 on success and -1 on failure
 * **/
int mlt_get_entry(mlt_s *self, int entry, int *srv, int *ver);


#endif
