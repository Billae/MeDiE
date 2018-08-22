#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "generic_storage.h"

/*path in pcocc*/
#define PREFIX "/dev/shm/" //"/home/billae/prototype_MDS/dataStore/"
/*path in ocre*/
//#define PREFIX "/ccc/home/cont001/ocre/billae/prototype_MDS/dataStore/"

int generic_put(const char *data, const char *key)
{
    /*printf("ecriture de la donnee %s à la clef %s\n", data, key);*/


    char *path;
    if (asprintf(&path, "%s%s", PREFIX, key) == -1) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_put: path creation error:%s\n",
            strerror(err));
        return -1;
    }

    int fd = open(path, O_WRONLY | O_EXCL | O_CREAT, 0664);
    if (fd == -1) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_put: open error: %s\n", strerror(err));
        free(path);
        return -1;
    }

    if ((write(fd, data, strlen(data))) < 0) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_put: write error: %s\n", strerror(err));
        free(path);
        return -1;
    }


    close(fd);
    free(path);
    return 1;
}
