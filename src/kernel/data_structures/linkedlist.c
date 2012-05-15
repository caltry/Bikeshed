#include "linkedlist.h"
#include "defs.h"
#include "memory/kmalloc.h"

void list_init(linked_list_t* list, void (*destroy)(void *data))
{
	list->size = 0;
	list->destroy = destroy;
	list->head = NULL;
	list->tail = NULL;
}

void list_destroy(linked_list_t* list)
{
	void* data;

	while (list_size(list) > 0)
	{
		if (list_remove_next(list, NULL, (void **)&data) == 0 && list->destroy != NULL)
		{
			// Free the data (if it needs to be freed)
			list->destroy(data);
		}
	}

	// Clear the list just in case
	list->size = 0;
	list->destroy = NULL;
	list->head = NULL;
	list->tail = NULL;
}

Int32 list_insert_next(linked_list_t* list, list_element_t* element, const void* data)
{
	list_element_t* new_element;

	if ((new_element = (list_element_t *)__kmalloc(sizeof(list_element_t))) == NULL)
	{
		return -1;
	}

	new_element->data = (void *)data;

	if (element == NULL)
	{
		if (list_size(list) == 0)
		{
			list->tail = new_element;
		}

		new_element->next = list->head;
		list->head = new_element;
	} else {
		if (element->next == NULL)
		{
			list->tail = new_element;
		}

		new_element->next = element->next;
		element->next = new_element;
	}

	list->size++;

	return 0;
}

Int32 list_remove_next(linked_list_t* list, list_element_t* element, void** data)
{
	list_element_t* old_element;

	if (list_size(list) == 0)
	{
		return -1;
	}

	if (element == NULL)
	{
		*data = list->head->data;
		old_element = list->head;
		list->head = list->head->next;

		if (list_size(list) == 1)
		{
			list->tail = NULL;
		}
	} else {
		if (element->next == NULL)
		{
			return -1;
		}

		*data = element->next->data;
		old_element = element->next;
		element->next = element->next->next;

		if (element->next == NULL)
		{
			list->tail = element;
		}
	}

	__kfree(old_element);

	list->size--;

	return 0;
}
