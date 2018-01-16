#include <stdio.h>
#include <stdlib.h>
#include "generic_storage.h"


int generic_put(const char* data, const char* key)
{
    printf("ecriture de la donnee %s à la clef %s \n", data, key);
    
    return 1;
}
