
#include <stdio.h>
#include <stdlib.h>

/*Sort servers in a subset by ungrowing order of the abs(load)
 * @param[in, out] subset the subset of servers to sort
 * @param[in] srvload the load of all servers
 * @param[in] size the subset size
 * @return 0 on success and -1 on failure*/
void manager_sort_subset(int *subset, int *srvload, int size)
{
    int i, j;
    int sorted;
    for (i = size-1; i >= 0; i--) {
        sorted = 0;
        for (j = 0; j < i; j++) {
            if (abs(srvload[subset[j]]) < abs(srvload[subset[j+1]])) {
                int temp = subset[j];
                subset[j] = subset[j+1];
                subset[j+1] = temp;
                sorted = 1;
            }
        }
        if (sorted == 0)
            return;
    }
    return;
}


int main()
{
    int n_srv = 10;
    /*load[i] is the relative load of each server*/
    int *load = malloc(sizeof(int) * n_srv);
    if (load == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc load failed\n");
        return -1;
    }

    load[0] = 8;
    load[1] = -2;
    load[2] = -6;
    load[3] = -2;
    load[4] = -4;
    load[5] = -3;
    load[6] = 4;
    load[7] = -7;
    load[8] = 0;
    load[9] = -1;


    int *subset = calloc(n_srv, sizeof(int));
    if (subset == NULL) {
        fprintf(stderr, "Manager:calculate_relab: malloc subset_s failed\n");
        return -1;
    }
    int size_subset = 0;


    int i;
    for (i = 0; i < n_srv; i++) {
        printf("load of srv %d = %d\n", i, load[i]);
        if (load[i] < 0) {
            subset[size_subset] = i;
            size_subset++;
        }
    }
    for (i = 0; i < size_subset; i++)
        printf("subset load of srv %d = %d\n", subset[i], load[subset[i]]);

    manager_sort_subset(subset, load, size_subset);

    printf("Sort subset...   Subset now:\n");
    for (i = 0; i < size_subset; i++)
        printf("load of srv %d = %d\n", subset[i], load[subset[i]]);

    return 0;
}
