#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "generic_storage.h"

#ifndef STORE
    #define STORE "/dev/shm/"
#endif

int generic_put(const char *key, const char *value)
{
    /*printf("ecriture de la donnee %s à la clef %s\n", value, key);*/

    char *path;
    if (asprintf(&path, "%s%s", STORE, key) == -1) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_put: ");
        fprintf(stderr, "path \"%s\" creation error:%s\n", key, strerror(err));
        return -1;
    }

    int fd = open(path, O_WRONLY | O_EXCL | O_CREAT, 0664);
    if (fd == -1) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_put: ");
        fprintf(stderr, "open %s error: %s\n", path, strerror(err));
        free(path);
        return -1;
    }

    if ((write(fd, value, strlen(value))) < 0) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_put: ");
        fprintf(stderr, "write in %s error: %s\n", path, strerror(err));
        free(path);
        return -1;
    }


    close(fd);
    free(path);
    return 0;
}

/*size of data associated to key is under 10bits*/
char *generic_get(const char *key)
{
    char *path;
    if (asprintf(&path, "%s%s", STORE, key) == -1) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_get: ");
        fprintf(stderr, "path \"%s\" creation error:%s\n", key,  strerror(err));
        return NULL;
    }

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_get: ");
        fprintf(stderr, "open %s error: %s\n", path, strerror(err));
        free(path);
        return NULL;
    }

    char *data = malloc(10);
    int rc;
    rc = read(fd, data, 10);
    if (rc < 0) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_get: ");
        fprintf(stderr, "read in %s error: %s\n", path, strerror(err));
        return NULL;
    }
    data[rc] = '\0';

    close(fd);
    free(path);
    return data;
}


int generic_update(const char *key, const char *value)
{
    char *path;
    if (asprintf(&path, "%s%s", STORE, key) == -1) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_update: ");
        fprintf(stderr, "path \"%s\" creation error: %s\n", key, strerror(err));
        return -1;
    }

    int fd = open(path, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_update: ");
        fprintf(stderr, "open %s error: %s\n", path, strerror(err));
        free(path);
        return -1;
    }

    if ((write(fd, value, strlen(value))) < 0) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_update: ");
        fprintf(stderr, "write in %s error: %s\n", path, strerror(err));
        free(path);
        return -1;
    }

    close(fd);
    free(path);
    return 0;
}


int generic_del(const char *key)
{
    char *path;
    if (asprintf(&path, "%s%s", STORE, key) == -1) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_del: ");
        fprintf(stderr, "path \"%s\" creation error: %s\n", key, strerror(err));
        return -1;
    }

    int rc = remove(path);
    if (rc != 0) {
        int err = errno;
        fprintf(stderr, "Generic storage:generic_del: ");
        fprintf(stderr, "delete file %s error: %s\n", path, strerror(err));
        return -1;
    }
    free(path);

    return 0;
}
