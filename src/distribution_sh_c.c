#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <json.h>

#include "distribution_sh_c.h"
#include "murmur3.h"

static int nb_srv;

int distribution_init(nb)
{
    nb_srv = nb;
    return 0;
}


int distribution_finalize()
{}


int distribution_pre_send(json_object *request)
{}


int distribution_post_receive(json_object *reply)
{}


int distribution_assign_srv_by_key(const char *key)
{
    int seed = 1;
    uint32_t num_srv;
    MurmurHash3_x86_32(key, strlen(key), seed, &num_srv);
    /*printf("server to query: %d\n", num_srv%nb_srv);*/

    return num_srv%nb_srv;
}
