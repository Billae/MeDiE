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
int init_distribution(nb)
{
    nb_srv = nb;
}


/*TO DO*/
int finalize_distribution()
{

}


/*TO DO*/
int post_receive(json_object *request)
{

}


/*TO DO*/
int pre_send(json_object *reply)
{

}


/*TO DO*/
int transfert_load(int entry)
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
