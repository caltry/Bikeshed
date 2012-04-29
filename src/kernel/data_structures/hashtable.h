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

typedef Int32 (*hash_func)(const void* key1, const void* key2);
typedef Int32 (*match_func)(const void* key1, const void* key2);
typedef Int32 (*destroy_func)(void* data);

Int32 hash_table_init(hash_table_t* hash_tbl, Int32 buckets, Int32 (*hash_func)(const void* key), Int32 (*match)(const void* key1, const void* key2), void (*destroy)(void* data));

void hash_table_destroy(hash_table_t* hash_tbl);

Int32 hash_table_insert(hash_table_t* hash_tbl, const void* data);

Int32 hash_table_remove(hash_table_t* hash_tbl, void** data);

Int32 hash_table_lookup(const hash_table_t* hash_tbl, void** data);

#define hash_table_size(hash_tbl) ((hash_tbl)->size)

#endif
