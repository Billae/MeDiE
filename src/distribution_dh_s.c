#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <json.h>

#include "distribution_dh_s.h"
#include "mlt.h"
#include "eacl.h"
#include "murmur3.h"

/*keep ?*/
static int nb_srv;

/*TO DO*/
int distribution_init(nb)
{
}


/*TO DO*/
int distribution_finalize()
{

}


/*TO DO*/
int distribution_post_receive(json_object *request)
{
    json_object *ver_flag;
    ver_flag = json_object_new_string("up-to-date"); /*or "out-of-date"*/
    json_object_object_add(request, "versionFlag", ver_flag);
    return 0;
}


/*TO DO*/
int distribution_pre_send(json_object *reply)
{
    return 0;
}


/*TO DO*/
int distribution_transfert_load(int entry)
{

}


/*TO DO*/
void *thread_mlt_updater(void *args)
{

}


/*TO DO*/
void *thread_eacl_sender(void *args)
{

}
