#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md_entry.h"

int main(void)
{
    int i;
    struct md_entry **tab = malloc(5 * sizeof(struct md_entry *));

    for (i = 0; i < 5; i++)
        tab[i] = NULL;

    struct md_entry head1;
    md_entry_init(&head1, 1, "name1");
    md_entry_insert(&tab[0], &head1);

    struct md_entry other1;
    md_entry_init(&other1, 1, "name1again");
    md_entry_insert(&tab[0], &other1);


    struct md_entry other;
    md_entry_init(&other, 2, "name2");
    md_entry_insert(&tab[1], &other);

    struct md_entry head2;
    md_entry_init(&head2, 2, "name2again");
    md_entry_insert(&tab[1], &head2);


    struct md_entry head3;
    md_entry_init(&head3, 3, "name3");
    md_entry_insert(&tab[2], &head3);


    for (i = 0; i < 5; i++) {
        struct md_entry *current = tab[i];
        while (current != NULL) {
            printf("list n%d = %d - %s\n", i, current->entry, current->md_name);
            current = current->next;
        }
    }

    struct md_entry *test = md_entry_search_md_name(&tab[1], "name2");
    printf("searching another: %s\n\n", test->md_name);

    printf("poping items:\n");
    struct md_entry *out = md_entry_pop(&tab[1]);
    if (out == NULL)
        printf("nothing to pop\n\n\n\n");
    else
        printf("pop:\tentry=%d, md_name=%s\n\n", out->entry, out->md_name);

    out = md_entry_pop(&tab[4]);
    if (out == NULL)
        printf("nothing to pop\n\n\n\n");
    else
        printf("pop:\tentry=%d, md_name=%s\n\n", out->entry, out->md_name);

    for (i = 0; i < 5; i++) {
        struct md_entry *current = tab[i];
        while (current != NULL) {
            printf("list n%d = %d - %s\n", i, current->entry, current->md_name);
            current = current->next;
        }
    }

    free(tab);
    return 0;
}
