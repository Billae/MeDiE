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
    if (!*head)
        return NULL;

    struct md_entry *temp = *head;
    *head = (*head)->next;
    return temp;
}

struct md_entry *md_entry_search_md_name(struct md_entry **head, char *name)
{
    struct md_entry *current = *head;
    struct md_entry *prev = NULL;
    while (current != NULL) {
        if (current->md_name == name) {
            if (prev == NULL) {
                *head = current->next;
                return current;
            } else {
            prev->next = current->next;
            return current;
            }
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}
