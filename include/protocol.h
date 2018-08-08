#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__


#include "mlt.h"

/**
 * @file protocol.h
 * @author E. Billa
 * @brief Protocols, structures and conventions common to client and server
 * **/

/*N_entry defines the mlt size*/
#define N_entry 100


enum req_type {
    RT_CREATE   = 1,
    RT_READ     = 2,
    RT_UPDATE   = 3,
    RT_DELETE   = 4
};

#endif
