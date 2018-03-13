#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "generic_storage.h"


#define PREFIX "dataStore/" // need to be in the "prototype_MDS" folder


int generic_put(const char *data, const char *key)
{
    printf("ecriture de la donnee %s à la clef %s\n", data, key);


    char *path;
    path = malloc((strlen(PREFIX)+strlen(key)+1)*sizeof(*path));
    if (path == NULL) {
        int err = errno;
        fprintf(stderr, "Generic storage: path malloc error: %s\n",
                strerror(err));
        return -1;
    }

    memset(path, 0, strlen(PREFIX)+strlen(key));
    strncpy(path, PREFIX, strlen(PREFIX)*sizeof(*PREFIX));
    strncat(path, key, strlen(key)*sizeof(*key));

    FILE *fd = fopen(path, "wx");
    if (fd == NULL) {
        int err = errno;
        fprintf(stderr, "Generic storage: fopen error: %s\n", strerror(err));
        free(path);
        return -1;
    }

    if ((fprintf(fd, "%s", data)) < 0) {
        int err = errno;
        fprintf(stderr, "Generic storage: fprintf error: %s\n", strerror(err));
        free(path);
        return -1;
    }


    fclose(fd);
    free(path);
    return 1;
}
