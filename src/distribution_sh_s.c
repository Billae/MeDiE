#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <json.h>

#include "distribution_sh_s.h"
#include "murmur3.h"


int init_distribution(nb)
{}


int finalize_distribution()
{}


int distribution_post_receive(json_object *request)
{}


int distribution_pre_send(json_object *reply)
{}
