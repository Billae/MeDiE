#include <stdio.h>
#include <stdlib.h>
#include "distribution.h"


const char *AssignSrvByKey(const char *key)
{
    const char *id_srv;
    /*pour les VM pcocc*/
    //id_srv = "tcp://10.252.0.1:7410";
    /*pour ocre*/
    id_srv = "tcp://192.168.129.25:7410";
    return id_srv;
}
