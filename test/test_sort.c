
#include <stdio.h>
#include <stdlib.h>

struct transfert_load_args {
    int *entries;
    int *servers;
    int size;
};

/*Sort in growing order the servers and adapt the change to entries*/
void load_args_sort(struct transfert_load_args *list)
{   
    int i, change;
    change = 1;
    while (change) {
        change = 0;
        for (i = 0; i < list->size; i++) {
            if (list->servers[i] > list->servers[i+1]) {
                change = 1;
                int tmp_srv = list->servers[i];
                int tmp_entry = list->entries[i];
                list->servers[i] = list->servers[i+1];
                list->entries[i] = list->entries[i+1];
                list->servers[i+1] = tmp_srv;
                list->entries[i+1] = tmp_entry;
            }
        }
    }
}

int main()
{

    struct transfert_load_args arg;
    arg.size = 10;
    arg.servers = malloc(10 * sizeof(int));
    arg.entries = malloc(10 * sizeof(int));

    arg.servers[0] = 4;
    arg.entries[0] = 4;
    arg.servers[1] = 5;
    arg.entries[1] = 5;
    arg.servers[2] = 7;
    arg.entries[2] = 7;
    arg.servers[3] = 1;
    arg.entries[3] = 1;
    arg.servers[4] = 9;
    arg.entries[4] = 9;
    arg.servers[5] = 7;
    arg.entries[5] = 7;
    arg.servers[6] = 11;
    arg.entries[6] = 11;
    arg.servers[7] = 2;
    arg.entries[7] = 2;
    arg.servers[8] = 10;
    arg.entries[8] = 10;
    arg.servers[9] = 4;
    arg.entries[9] = 4;
    load_args_sort(&arg);

    int i;
    for (i = 0; i < arg.size; i++)
        printf("arg[%d] srv:%d, entry:%d\n", i, arg.servers[i], arg.entries[i]);
    return 0;
}
