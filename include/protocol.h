#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__


/**
 * @file protocol.h
 * @author E. Billa
 * @brief Protocols, structures and conventions common to client and server
 * **/

#include "mlt.h"

/*N_entry defines the mlt size*/
#define N_entry 100


/*Client_port is the socket port to bind or connect
 * for client-server requesting*/
#define Client_port 7410
/*Eacl_port is the socket port to bind or connect
 * for mlt updating between servers and manager*/
#define Eacl_port 7411
/*Mlt_port is the socket port to bind or connect
 * for mlt updating between servers and manager*/
#define Mlt_port 7412
/*Transfert_port is the socket port to bind or connect
 * for servers to transfert load*/
#define Transfert_port 7413


enum req_type {
    RT_CREATE   = 1,
    RT_READ     = 2,
    RT_UPDATE   = 3,
    RT_DELETE   = 4
};

#endif
