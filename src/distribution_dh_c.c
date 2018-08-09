#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <json.h>

#include "distribution_dh_c.h"
#include "mlt.h"
#include "protocol.h"
#include "murmur3.h"


static int nb_srv;

/*TO DO*/
int init_distribution(nb)
{
    nb_srv = nb;
}


/*TO DO*/
int finalize_distribution()
{

}


/*TO DO*/
int pre_send(json_object *request)
{

}


/*TO DO*/
int post_receive(json_object *reply)
{

}


/*TO DO*/
int assign_srv_by_key(const char *key)
{
    int seed = 1;
    uint32_t num_srv;
    MurmurHash3_x86_32(key, strlen(key), seed, &num_srv);
    /*printf("server to query: %d\n", num_srv%nb_srv);*/

    return num_srv%nb_srv;
}
