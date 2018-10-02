#ifndef __MD_ENTRY_H__
#define __MD_ENTRY_H__

/**
 * @file md_entry.h
 * @author E. Billa
 * @brief linked list to know which md is associated to wich entry.
 * One linked list for one entry.
 * **/


/*each element is a md associated to the entry*/
struct md_entry {
    int entry;
    char *md_name;

    struct md_entry *next;
};

/** Init an element **/
void md_entry_init(struct md_entry *elem, int entry, char *name);

/** Insert an element a the top of the linked list **/
void md_entry_insert(struct md_entry **head, struct md_entry *elem);

/** Retrieve the first element of the linked list **/
struct md_entry *md_entry_pop(struct md_entry **head);

#endif
