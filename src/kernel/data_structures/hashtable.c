#include "hashtable.h"

#include "../lib/klib.h"
#include "../memory/kmalloc.h"

Int32 hash_string(const void* key)
{
	const char* ptr = (const char *)key;
	Uint32 val  = 0;
	Uint32 temp = 0;

	while (*ptr != '\0')
	{
		val = (val << 4) + (*ptr);

		if (temp = (val & 0xF0000000))
		{
			val = val ^ (tmp >> 24);
			val = val ^ tmp;
		}

		++ptr;
	}

	/* The 100 should be the size of your hash table */
	//return val % 100;
	return val;
}

static void hash_table_destroy_key_val(hash_table_t* hash_tbl, const void* data)
{
	key_val_t* key_val = (key_val_t *)data;


}

Int32 hash_table_init(hash_table_t* hash_tbl, Int32 buckets, Int32 (*hash_func)(const void* key), Int32 (*match_func)(const void* key1, const void* key2), void (*destroy)(void* data))
{
	if ((hash_tbl->table = (linked_list_t *)__kmalloc(buckets * sizeof(linked_list_t))) == NULL)
	{
		return -1;
	}

	hash_tbl->buckets = buckets;
	for (Int32 i = 0; i < hash_tbl->buckets; ++i)
	{
		//list_init(&hash_tbl->table[i], hash_table_destroy_key_val);
	}

	hash_tbl->hash_func = hash_func;
	hash_tbl->match = match_func;
	hash_tbl->destroy = destroy;	
	hash_tbl->size = 0;

	return 0;
}

void hash_table_destroy(hash_table_t* hash_tbl)
{
	for (Int32 i = 0; i < hash_tbl->buckets; ++i)
	{
		list_destroy(&hash_tbl->table[i]);
	}

	__kfree(hash_tbl->table);

	_kmemclr(hash_tbl, sizeof(hash_table_t));
}

Int32 hash_table_insert(hash_table_t* hash_tbl, const void* data)
{
	void* temp;

	Int32 bucket, retval;

	temp = (void *)data;

	if (hash_table_lookup(hash_tbl, &temp) == 0)
	{
		return 1;
	}

	bucket = hash_tbl->hash_func(data) % hash_tbl->buckets;

	if ((retval = list_insert_next(&hash_tbl->table[bucket], NULL, data)) == 0)
	{
		hash_tbl->size++;
	}

	return retval;
}

Int32 hash_table_remove(hash_table_t* hash_tbl, const void* key, void** data)
{
	list_element_t *element, *previous;

	Int32 bucket = hash_tbl->hash_func(key) % hash_tbl->buckets;

	previous = NULL;

	for (element = list_head(&hash_tbl->table[bucket]); element != NULL; element = list_next(element))
	{
		if (hash_tbl->match(key, list_data(element)))
		{
			// We found a match, remove it
			if (list_remove_next(&hash_tbl->table[bucket], previous, data) == 0)
			{
				hash_tbl->size--;
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

	Int32 bucket = hash_tbl->hash_func(key) % hash_tbl->buckets;

	for (element = list_head(&hash_tbl->table[bucket]); element != NULL; element = list_next(element))
	{
		if (hash_tbl->match(key, list_data(element)))
		{
			*data = list_data(element);
			return 0;
		}
	}

	return -1;
}
