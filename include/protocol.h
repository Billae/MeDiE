#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

/**
 * @file protocol.h
 * @author E. Billa
 * @brief Protocols, structures and conventions common to client and server
 * **/


enum req_type {
    RT_CREATE   = 1,
    RT_READ     = 2,
    RT_UPDATE   = 3,
    RT_DELETE   = 4
};


#endif
