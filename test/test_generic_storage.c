#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generic_storage.h"

int main(int argc, char *argv[])
{
    char *key = "a_key";
    char *data = "trololo";
    int rc;
    rc = generic_put(key, data);
    if (rc == -1)
        fprintf(stderr, "put error\n");

    printf("key %s created\n", key);

    char *value = generic_get(key);
    if (value == NULL)
        fprintf(stderr, "get error\n");

    printf("data associated to key: %s\n", value);
    free(value);

    rc = generic_update(key, "another");
    if (rc == -1)
        fprintf(stderr, "update error\n");

    value = generic_get(key);
    if (value == NULL)
        fprintf(stderr, "get error\n");

    printf("data associated to key: %s\n", value);
    free(value);

    rc = generic_del(key);
    if (rc == -1)
        fprintf(stderr, "delete error\n");

    printf("key %s deleted\n", key);

    value = generic_get(key);
    if (value == NULL)
        fprintf(stderr, "get error\n");

    return 0;
}
