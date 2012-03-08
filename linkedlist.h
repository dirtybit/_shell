#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef struct list_node list_node;
typedef struct list list;

list * list_init();
void list_add(list *, void *);
void * list_get(list *, size_t);
void list_free(list *);
size_t list_size(list *);

#endif
