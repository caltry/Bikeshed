#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include "headers.h"

#include "linkedlist.h"

typedef struct HashTable
{
	Int32 buckets;
	Int32 (*hash_func) (const void* key);
	Int32 (*match) (const void* key1, const void* key2);
	void  (*destroy) (void* data);

	Int32 size;
	linked_list_t* table;
} hash_table_t;

typedef struct KeyValue
{
	const void* key;
	const void* data;
} key_val_t;

typedef Int32 (*hash_func)(const void* key1, const void* key2);
typedef Int32 (*match_func)(const void* key1, const void* key2);
typedef Int32 (*destroy_func)(void* data);

// TODO this data structure isn't fully implemented yet!

/* Decent string hashing function, adapted from
 *
 * Mastering Algorithms with C
 */
Int32 hash_string(const void* key);

/* Initialize a new hash_table_t structure. This method should be called before
 * the hash_table_t structure is used, Initializes to default values
 *
 * Destroy is used to free the data stored in the hash table when hash_table_destroy is
 * called. If it should be set to whatever can free the data (like __kfree()), if it's NULL
 * destroy() won't be called on the data
 *
 * hash_func is a function that should be able to turn keys into a number which can be used
 * to index the table
 *
 * match is a function that can test if two keys are equal
 *
 * Returns 0 if successful, -1 otherwise
 */
Int32 hash_table_init(hash_table_t* hash_tbl, Int32 buckets, Int32 (*hash_func)(const void* key), Int32 (*match)(const void* key1, const void* key2), void (*destroy)(void* data));

/* Should be called when you're done using the hash_table_t structure. It will free
 * all of the internal hash table data structures and if destroy is not NULL it will
 * free all of the data stored in the hash table
 *
 * Returns: Nothing
 */
void hash_table_destroy(hash_table_t* hash_tbl);

/* Inserts a new value into the hash table. If the key already exists
 * the data is NOT inserted into the table.
 *
 * Returns: 0 on success, -1 otherwise
 */
Int32 hash_table_insert(hash_table_t* hash_tbl, const void* key, const void* data);

/* Removes and item from the hash table. If the value is found
 * data will be set to the removed items data
 *
 * Returns: 0 if successful, -1 otherwise
 */
Int32 hash_table_remove(hash_table_t* hash_tbl, const void* key, void** data);

/* Checks to see if a value exists in the table.
 *
 * Data should initialy point to the key of the element to look up. 
 * If the value is found in the table 
Int32 hash_table_lookup(const hash_table_t* hash_tbl, void** data);

#define hash_table_size(hash_tbl) ((hash_tbl)->size)

#endif
