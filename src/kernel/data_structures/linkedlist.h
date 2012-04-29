#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include "headers.h"

typedef struct ListElement
{
	void* data;
	struct ListElement* next;
} list_element_t;

typedef struct LinkedList
{
	Int32 size;
	Int32 (*match) (const void* key1, const void* key2);
	void  (*destroy) (void* data);

	list_element_t* head;
	list_element_t* tail;
} linked_list_t;

void list_init(linked_list_t* list, void (*destroy)(void *data));

void list_destroy(linked_list_t* list);

Int32 list_insert_next(linked_list_t* list, list_element_t* element, const void* data);

Int32 list_remove_next(linked_list_t* list, list_element_t* element, void** data);

#define list_size(list) ((list)->size)

#define list_head(list) ((list)->head)

#define list_tail(list) ((list)->tail)

#define list_is_head(list, element) ((element) == (list)->head ? 1 : 0)

#define list_is_tail(list, element) ((element)->next == NULL ? 1 : 0)

#define list_data(element) ((element)->data)

#define list_next(element) ((element)->next)

#endif
