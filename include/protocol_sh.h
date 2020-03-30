#ifndef __PROTOCOL_DH_H__
#define __PROTOCOL_DH_H__


/**
 * @file protocol_dh.h
 * @author E. Billa
 * @brief Protocols, structures and conventions common to client and server
 * **/

/*Client_port is the socket port to bind or connect
 * for client-server requesting*/
#ifndef Client_port
    #define Client_port 7410
#endif
/*Eacl_port is the socket port to bind or connect
 * for mlt updating between servers and manager*/

enum req_type {
    RT_CREATE   = 1,
    RT_READ     = 2,
    RT_UPDATE   = 3,
    RT_DELETE   = 4
};

#endif
