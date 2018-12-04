#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <json.h>

#include "distribution_sh_s.h"
#include "murmur3.h"


int distribution_init()
{
    return 0;
}


int distribution_finalize()
{
    return 0;
}


int distribution_post_receive(json_object *request)
{
    return 0;
}


int distribution_pre_send(json_object *reply, int global_rc)
{
    return 0;
}

int distribution_signal_action()
{
    return 0;
}
