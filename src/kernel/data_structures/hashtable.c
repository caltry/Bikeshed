#include "hashtable.h"

#include "../lib/klib.h"
#include "../memory/kmalloc.h"

typedef struct KeyValue
{
	Int32 hashed_key;
	void* data;
} key_val_t;

Int32 hash_string(const void* key)
{
	const char* ptr = (const char *)key;
	Uint32 val  = 0;
	Uint32 temp = 0;

	while (*ptr != '\0')
	{
		val = (val << 4) + (*ptr);

		if ((temp = (val & 0xF0000000)) != 0)
		{
			val = val ^ (temp >> 24);
			val = val ^ temp;
		}

		++ptr;
	}

	/* The 100 should be the size of your hash table */
	//return val % 100;
	return val;
}

Int32 hash_table_init(hash_table_t* hash_tbl, Int32 buckets, Int32 (*hash_func)(const void* key), void (*destroy)(void* data))
{
	if ((hash_tbl->table = (linked_list_t *)__kmalloc(buckets * sizeof(linked_list_t))) == NULL)
	{
		return -1;
	}

	hash_tbl->buckets = buckets;
	for (Int32 i = 0; i < hash_tbl->buckets; ++i)
	{
		// This is a hack, we need to clean up the list ourselves
		// We have to cleanup the data inside the key_val_t structure
		list_init(&hash_tbl->table[i], __kfree);
	}

	hash_tbl->hash_func = hash_func;
	hash_tbl->destroy = destroy;	
	hash_tbl->size = 0;

	return 0;
}

void hash_table_destroy(hash_table_t* hash_tbl)
{
	for (Int32 i = 0; i < hash_tbl->buckets; ++i)
	{
		linked_list_t* lst_current = &hash_tbl->table[i];
		// This is a hack because we need to free the data
		// inside of the key_val_t structure, but we have no 
		// way of getting the correct destroy() function if
		// we use the regular list_destroy() function as We
		// can't pass the current hash_table_t object. So we'll
		// loop through the list and free the data elements
		// (iff destroy is not null) and then call list destroy
		// which will cleanup the linked list nodes and free
		// our key_val_t structures. We have to loop through
		// the list twice anyway, so it doesn't matter that
		// the list is freeing as opposed to us.

		// Loop through the list and free the data nodes
		if (hash_tbl->destroy != NULL)
		{
			list_element_t* node;
			for (node = list_head(lst_current); node != NULL; node = list_next(node))
			{
				key_val_t* key_val = (key_val_t *)list_data(node);
				hash_tbl->destroy(key_val->data);
			}
		}

		list_destroy(lst_current);
	}

	__kfree(hash_tbl->table);

	_kmemclr(hash_tbl, sizeof(hash_table_t));
}

Int32 hash_table_insert(hash_table_t* hash_tbl, const void* key, const void* data)
{
	void* temp; // Not used, needed for hash_table_lookup()

	if (hash_table_lookup(hash_tbl, key, &temp) == 0)
	{
		return 1;
	}

	Int32 hashed_key = hash_tbl->hash_func(key);
	Int32 bucket = hashed_key % hash_tbl->buckets;

	key_val_t* key_val = (key_val_t *)__kmalloc(sizeof(key_val_t));
	key_val->hashed_key = hashed_key;
	key_val->data = (void *)data; // Yeah..., we're removing the const :/

	Int32 retval;
	if ((retval = list_insert_next(&hash_tbl->table[bucket], NULL, key_val)) == 0)
	{
		hash_tbl->size++;
	}

	return retval;
}

Int32 hash_table_remove(hash_table_t* hash_tbl, const void* key, void** data)
{
	Int32 hashed_key = hash_tbl->hash_func(key);
	Int32 bucket = hashed_key % hash_tbl->buckets;

	list_element_t* element  = NULL;
	list_element_t* previous = NULL;

	for (element = list_head(&hash_tbl->table[bucket]); element != NULL; element = list_next(element))
	{
		key_val_t* key_val = (key_val_t *)list_data(element);
		if (hashed_key == key_val->hashed_key)
		{
			// We found a match, remove it
			if (list_remove_next(&hash_tbl->table[bucket], previous, data) == 0)
			{
				hash_tbl->size--;

				// The internal data we're storing is a key_val_t structure,
				// grab the data the user really wants
				key_val = (key_val_t *)(*data);
				*data = key_val->data;
				return 0;
			} else {
				return -1;
			}
		}

		previous = element;
	}

	// Element not found
	return -1;
}

Int32 hash_table_lookup(const hash_table_t* hash_tbl, const void* key, void** data)
{
	list_element_t* element;

	Int32 hashed_key = hash_tbl->hash_func(key);
	Int32 bucket = hashed_key % hash_tbl->buckets;

	for (element = list_head(&hash_tbl->table[bucket]); element != NULL; element = list_next(element))
	{
		key_val_t* key_val = (key_val_t *)list_data(element);
		if (hashed_key == key_val->hashed_key)
		{
			*data = key_val->data;
			return 0;
		}
	}

	return -1;
}
