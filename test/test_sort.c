
#include <stdio.h>
#include <stdlib.h>

struct transfert_load_args {
    int *entries;
    int *servers;
    int size;
};


int cut(struct transfert_load_args *list, int p, int r)
{
    int pivot = list->servers[p], i = p-1, j = r+1;
    while (1) {
        do
            j--;
        while (list->servers[j] > pivot);
        do
            i++;
        while (list->servers[i] < pivot);
        if (i < j) {
            int tmp_srv = list->servers[i];
            int tmp_entry = list->entries[i];
            list->servers[i] = list->servers[j];
            list->entries[i] = list->entries[j];
            list->servers[j] = tmp_srv;
            list->entries[j] = tmp_entry;
        } else
            return j;
    }
}

/*Sort in growing order the servers and adapt the change to entries*/
/*p = inf and r = size-1*/
void load_args_sort(struct transfert_load_args *list, int p, int r)
{
    int q;
    if (p < r) {
        q = cut(list, p, r);
        load_args_sort(list, p, q);
        load_args_sort(list, q+1, r);
                                            }
}


int main()
{

    struct transfert_load_args arg;
    arg.size = 28;
    arg.servers = malloc(28 * sizeof(int));
    arg.entries = malloc(28 * sizeof(int));

    arg.servers[0] = 4;
    arg.entries[0] = 4;
    arg.servers[1] = 6;
    arg.entries[1] = 6;
    arg.servers[2] = 10;
    arg.entries[2] = 10;
    arg.servers[3] = 16;
    arg.entries[3] = 16;
    arg.servers[4] = 18;
    arg.entries[4] = 18;
    arg.servers[5] = 20;
    arg.entries[5] = 20;
    arg.servers[6] = 24;
    arg.entries[6] = 24;
    arg.servers[7] = 26;
    arg.entries[7] = 26;
    arg.servers[8] = 28;
    arg.entries[8] = 28;
    arg.servers[9] = 32;
    arg.entries[9] = 32;
    arg.servers[10] = 40;
    arg.entries[10] = 40;
    arg.servers[11] = 44;
    arg.entries[11] = 44;
    arg.servers[12] = 48;
    arg.entries[12] = 48;
    arg.servers[13] = 52;
    arg.entries[13] = 52;
    arg.servers[14] = 56;
    arg.entries[14] = 56;
    arg.servers[15] = 58;
    arg.entries[15] = 58;
    arg.servers[16] = 62;
    arg.entries[16] = 62;
    arg.servers[17] = 64;
    arg.entries[17] = 64;
    arg.servers[18] = 66;
    arg.entries[18] = 66;
    arg.servers[19] = 76;
    arg.entries[19] = 76;
    arg.servers[20] = 78;
    arg.entries[20] = 78;
    arg.servers[21] = 82;
    arg.entries[21] = 82;
    arg.servers[22] = 84;
    arg.entries[22] = 84;
    arg.servers[23] = 90;
    arg.entries[23] = 90;
    arg.servers[24] = 92;
    arg.entries[24] = 92;
    arg.servers[25] = 94;
    arg.entries[25] = 94;
    arg.servers[26] = 68;
    arg.entries[26] = 68;
    arg.servers[27] = 42;
    arg.entries[27] = 42;
    load_args_sort(&arg, 0, arg.size-1);

    int i;
    for (i = 0; i < arg.size; i++)
        printf("arg[%d] srv:%d, entry:%d\n", i, arg.servers[i], arg.entries[i]);
    return 0;
}
