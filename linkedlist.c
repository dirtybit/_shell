#include <stdlib.h>

struct list_node {
	void *value;
	struct list_node *next;
};
typedef struct list_node list_node;

struct list {
	list_node *begin;
	list_node *end;
	size_t size;
};
typedef struct list list;

list *list_init()
{
	list *list_head = (list *) malloc(sizeof(list));
	list_head->begin = NULL;
	list_head->end = NULL;
	list_head->size = 0;

	return list_head;
}

void list_add(list *lst, void *node_value)
{
	if (lst->end) {
		lst->end->next = (list_node *) malloc(sizeof(list_node));
		lst->end = lst->end->next;
	} else {
		lst->end = (list_node *) malloc(sizeof(list_node));
		lst->begin = lst->end;
	}

	lst->end->value = node_value;
	lst->end->next = NULL;
	lst->size++;
}

void *list_get(list *lst, size_t index)
{
	list_node *node = NULL;
	if (index < lst->size) {
		node = lst->begin;
		while (index)
			node = node->next;
	}

	return node;
}

void list_empty(list *lst)
{
	list_node *node;

	while (lst->begin) {
		node = lst->begin;
		lst->begin = lst->begin->next;
		free(node);
	}

	lst->begin = lst->end = NULL;
	lst->size = 0;
}


void list_free(list *lst)
{
	list_empty(lst);
	free(lst);
}

