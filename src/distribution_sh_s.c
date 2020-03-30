#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <json.h>

#include "distribution_sh_s.h"
#include "murmur3.h"


/*Each server has its own id_srv*/
static int id_srv_self;


#ifndef max_id_size
    #define max_id_size 21
#endif

/* path in pcocc*/
#ifndef SRV_PATH
    #define SRV_PATH "/home/billae/prototype_MDS/etc/server.cfg"
#endif
/*path in ocre*/
//#define SRV_PATH "/ccc/home/cont001/ocre/billae/prototype_MDS/etc/server.cfg"

/* path in pcocc*/
#ifndef SCRATCH
    #define SCRATCH "/mnt/scratch/tmp_ack/sh/"
#endif
/*path in ocre*/
//#define SCRATCH "/ccc/home/cont001/ocre/billae/prototype_MDS/tmp/"


int distribution_init()
{
    /*getting id_srv_self*/
    char *id_srv;
    id_srv = malloc(max_id_size*sizeof(*id_srv));
    if (id_srv == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution:init: id_srv malloc error: %s\n",
            strerror(err));
        return 0;
    }

    FILE *fd = fopen(SRV_PATH, "r");
    if (fd == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution:init: open server.cfg file failed: %s\n",
            strerror(err));
        free(id_srv);
        return -1;
    }
    if (fgets(id_srv, max_id_size, fd) == NULL) {
        fprintf(stderr, "Distribution:init: read config file failed\n");
        free(id_srv);
        return -1;
    }
    char *positionEntree = strchr(id_srv, '\n');
    if (positionEntree != NULL)
        *positionEntree = '\0';

    char *value = strstr(id_srv, "ID");
    value = strchr(value, '=');
    value++;
    id_srv_self = atoi(value);

    free(id_srv);
    fclose(fd);

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
    asprintf(&file_name, "%svm%dUSR-0", SCRATCH, id_srv_self);
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
    asprintf(&file_name, "%svm%dUSR-0", SCRATCH, id_srv_self);
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
