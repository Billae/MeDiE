#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "generic_storage.h"

/*path in pcocc*/
//#define PREFIX "/home/billae/prototype_MDS/dataStore/"
/*path in ocre*/
#define PREFIX "/ccc/home/cont001/ocre/billae/prototype_MDS/dataStore/"

int generic_put(const char *data, const char *key)
{
    printf("ecriture de la donnee %s à la clef %s\n", data, key);


    char *path;
    if (asprintf(&path, "%s%s", PREFIX, key) == -1) {
        int err = errno;
        fprintf(stderr, "Generic storage: path creation error:%s\n",
                strerror(err));
        return -1;
    }

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
