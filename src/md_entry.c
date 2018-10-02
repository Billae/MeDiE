#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "md_entry.h"


void md_entry_init(struct md_entry *elem, int entry, char *name)
{
    elem->entry = entry;
    elem->md_name = name;
    elem->next = NULL;
}

void md_entry_insert(struct md_entry **head, struct md_entry *elem)
{
    if (head == NULL)
        *head = elem;
    else {
        elem->next = *head;
        *head = elem;
    }
}

struct md_entry *md_entry_pop(struct md_entry **head)
{
    struct md_entry *temp = *head;
    *head = (*head)->next;
    return temp;
}
