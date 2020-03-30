#ifndef __PROTOCOL_DH_H__
#define __PROTOCOL_DH_H__


/**
 * @file protocol_dh.h
 * @author E. Billa
 * @brief Protocols, structures and conventions common to client and server
 * **/

#include "mlt.h"

/*N_entry defines the mlt size*/
#ifndef N_entry
    #define N_entry 100
#endif


/*Client_port is the socket port to bind or connect
 * for client-server requesting*/
#ifndef Client_port
    #define Client_port 7410
#endif
/*Eacl_port is the socket port to bind or connect
 * for mlt updating between servers and manager*/
#ifndef Eacl_port
    #define Eacl_port 7411
#endif
/*Mlt_port is the socket port to bind or connect
 * for mlt updating between servers and manager*/
#ifndef Mlt_port
    #define Mlt_port 7412
#endif
/*Transfert_port is the socket port to bind or connect
 * for servers to transfert load*/
#ifndef Transfert_port
    #define Transfert_port 7413
#endif


/*w_factor is the performance factor of each server*/
#ifndef w_factor
    #define w_factor 1
#endif

enum req_type {
    RT_CREATE   = 1,
    RT_READ     = 2,
    RT_UPDATE   = 3,
    RT_DELETE   = 4
};

#endif
