#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "distribution.h"

#include "murmur3.h"

static int nb_srv;

void init_distribution_nbsrv(nb)
{
    nb_srv = nb;
}


int assign_srv_by_key(const char *key)
{
    int seed = 1;
    uint32_t num_srv;
    MurmurHash3_x86_32(key, strlen(key), seed, &num_srv);
    printf("server to query: %d\n", num_srv%nb_srv);

    return num_srv%nb_srv;
}
