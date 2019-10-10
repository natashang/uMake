#ifndef LIST_H_
#define LIST_H_
#define EMPTY NULL

typedef
struct list_node_st {
    char* string;
    struct list_node_st * next;
} list_node;

typedef list_node* list_t;

/* List Append
 * str The rule or dependency to be considered
 * lst The target list to be considered
 *
 * Adds rules or dependencies to their respective list
 */
void list_append(char* str, list_t * lst);

/* List For Each
 * lst The rules or dependencies list to be considered
 * act The action to be performed on list
 *
 * Iterates through a list performing the requested action
 */
void list_for_each(list_t* lst, void (*act)(char*));

#endif
