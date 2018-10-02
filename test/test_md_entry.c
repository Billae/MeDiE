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
    md_entry_init(&head1, 1, "name");
    md_entry_insert(&tab[0], &head1);

    struct md_entry other;
    md_entry_init(&other, 1, "another");
    md_entry_insert(&tab[0], &other);

    struct md_entry head2;
    md_entry_init(&head2, 2, "nameagain");
    md_entry_insert(&tab[1], &head2);


    for (i = 0; i < 5; i++) {
        struct md_entry *current = tab[i];
        while (current != NULL) {
            printf("list n%d = %d - %s\n", i, current->entry, current->md_name);
            current = current->next;
        }
    }

    printf("poping items:\n");
    struct md_entry *out = md_entry_pop(&tab[0]);
    printf("entry=%d, md_name=%s\n", out->entry, out->md_name);

    out = md_entry_pop(&tab[0]);
    printf("entry=%d, md_name=%s\n", out->entry, out->md_name);

    out = md_entry_pop(&tab[0]);
    if (out == NULL)
        printf("nothing to pop\n\n\n\n");

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
