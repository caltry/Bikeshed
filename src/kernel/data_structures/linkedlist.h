#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

/* The code for this data structure was taken and modified from the book
 *
 * Mastering Algorithms with C - Chapter 5 - Linked Lists
 *
 * ISBN-10: 1565924533
 * ISBN-13: 978-1565924536
 *
 */

#include "types.h"

typedef struct ListElement
{
	void* data;
	struct ListElement* next;
	struct ListElement* prev;
} list_element_t;

typedef struct LinkedList
{
	Int32 size;
	void  (*destroy) (void* data);

	list_element_t* head;
	list_element_t* tail;
} linked_list_t;

/* Initialize a linked_list_t structure
 *
 * This method should be called before using a linked_list_t structure. The 
 * destroy argument is used to free dynamically allocated data when list_destroy()
 * is called, for example it should be set to __kfree() if you are storing data
 * that has been allocated with __kmalloc()
 *
 * Destroy may be set to NULL if no function should be called when destroying the list
 *
 * Returns: Nothing
 */
void list_init(linked_list_t* list, void (*destroy)(void *data));

/* Destroys a linked list. list_init() shoudl be called before using the list again.
 *
 * All the nodes of the list will be deleted and destroy will be called on the node's
 * data provided destroy is not set to NULL.
 *
 * Returns: Nothing
 */
void list_destroy(linked_list_t* list);

/* Insert a new element into the list after an existing element.
 *
 * If element is NULL it inserts data at the head of the list.
 *
 * Returns: 0 if successful, -1 if there was a problem
 */
Int32 list_insert_next(linked_list_t* list, list_element_t* element, const void* data);

/* Removes an element from the list after the element passed in.
 * If element is NULL the head of the list is removed. Upon Return 
 * data is set to the data of the removed node.
 *
 * Returns: 0 if successful, -1 if there was a problem
 */
Int32 list_remove_next(linked_list_t* list, list_element_t* element, void** data);

/* Self explanatory macros */
#define list_size(list) ((list)->size)

#define list_head(list) ((list)->head)

#define list_tail(list) ((list)->tail)

#define list_is_head(list, element) ((element) == (list)->head ? 1 : 0)

#define list_is_tail(list, element) ((element)->next == NULL ? 1 : 0)

#define list_data(element) ((element)->data)

#define list_next(element) ((element)->next)

#define list_prev(element) ((element)->prev)

#endif
