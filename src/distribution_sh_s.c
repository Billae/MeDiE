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


int distribution_signal1_action()
{
    /*create the ack file to indicate the end of the redistribution*/
    char *file_name;
    asprintf(&file_name, "%svm%dUSR", SCRATCH, id_srv_self);
    int ack = open(file_name, O_WRONLY | O_EXCL | O_CREAT , 0664);
    if (ack == -1) {
        int err = errno;
        fprintf(stderr, "Distribution:sigUSR1 handler: ");
        fprintf(stderr, "create ack file \"%s\" failed\n/:%s",
            file_name, strerror(err));
        return -1;
    }
    close(ack);

    return 0;
}


int distribution_signal2_action()
{
    /*create the ack file to indicate the end of the redistribution*/
    char *file_name;
    asprintf(&file_name, "%svm%dUSR", SCRATCH, id_srv_self);
    int ack = open(file_name, O_WRONLY | O_EXCL | O_CREAT , 0664);
    if (ack == -1) {
        int err = errno;
        fprintf(stderr, "Distribution:sigUSR2 handler: ");
        fprintf(stderr, "create ack file \"%s\" failed\n/:%s",
            file_name, strerror(err));
        return -1;
    }
    close(ack);

    return 0;
}
