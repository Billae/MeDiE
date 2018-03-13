#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "distribution.h"

#define PATH "hosts.conf"
#define max_id_size 21

static char **servers;
static int servers_cpt;


int DistributionInit()
{
    servers = malloc(sizeof(**servers));
    if (servers == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution: servers init alloc error: %s\n",
                strerror(err));
        return -1;
    }

    servers_cpt = 0;


    FILE *fd = fopen(PATH, "r");
    if (fd == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution: open hosts file error: %s\n",
                strerror(err));
        free(servers);
    }


    char *id_srv;
    id_srv = malloc(max_id_size*sizeof(*id_srv));
    if (id_srv == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution: id_srv malloc error: %s\n",
                strerror(err));
        return -1;
    }

    while (fgets(id_srv, max_id_size, fd) != NULL) {
        int rc;
        rc = AddServerToList(id_srv);
        if (rc != 1) {
            fprintf(stderr, "Distribution: AddServerToList error\n");
            free(id_srv);
            return -1;
        }
    }

    if (!feof(fd)) {
        int err = errno;
        fprintf(stderr, "Distribution: read hosts file error: %s\n",
                strerror(err));
        free(id_srv);
        return -1;
    }

    /*int i;
    for (i = 0; i < servers_cpt; i++)
        printf("%s\n", servers[i]);
*/
    free(id_srv);
    fclose(fd);
    return 1;
}


void DistributionFinalize()
{
    int i;
    for (i = 0; i < servers_cpt; i++)
        free(servers[i]);
    free(servers);
}


int AddServerToList(char *name)
{
    servers_cpt++;
    servers = realloc(servers, servers_cpt*sizeof(char *));
    if (servers == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution: servers realloc error: %s\n",
                strerror(err));
        return -1;
    }

    servers[servers_cpt-1] = strdup(name);
    if (servers[servers_cpt-1] == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution: servers strdup error:%s\n",
                strerror(err));
        return -1;
    }

    return 1;
}


const char *AssignSrvByKey(const char *key)
{
    char *id_srv;

    id_srv = servers[0];
    char *socket = malloc((strlen(id_srv)+7)*sizeof(*socket));
    if (socket == NULL) {
        int err = errno;
        fprintf(stderr, "Distribution: socket malloc error: %s\n",
                strerror(err));
        return NULL;
    }

    strncpy(socket, "tcp://", 7);
    strncat(socket, id_srv, (strlen(id_srv)-1));

    const char *const_socket = strdup(socket);
    //printf("%s\n",const_socket);
    free(socket);
    return const_socket;
}
