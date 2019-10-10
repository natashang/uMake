#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "list.h"

/* List Append
 *
 */
void list_append(char* str, list_t *lst) {
    assert(lst != EMPTY);

    while (*lst != EMPTY)
      lst = &((*lst)->next);

    list_node* n = malloc(sizeof(list_node));

    if (!n) {
      perror("malloc: list");
      exit(EXIT_FAILURE);
    }

    if (n != EMPTY) {
      n->string = str;
      n->next = EMPTY;
      *lst = n;
    }
    else
      errno = ENOMEM;
}

/* List For Each
 *
 */
void list_for_each(list_t* lst, void (*act)(char*)) {
    while (*lst != EMPTY) {
      char* copy = strdup((*lst)->string);
      act(copy);
      lst = &((*lst)->next);
      free(copy);
    }
}
