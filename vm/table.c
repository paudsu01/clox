#include "../common.h"
#include "table.h"
#include "memory.h"
#include "string.h"

void initTable(Table* table){
	table->count=0;
	table->capacity=0;
	table->entries=NULL;
}


void freeTable(Table* table){
	FREE_ARRAY(Entry, table->entries, table->capacity);
	initTable(table);
}

void tableAdd(Table* table, ObjectString* key, Value value){
	// allocate and resize if necessary
	if (table->count + 1 > table->capacity * MAX_TABLE_LOAD) {
		int capacity = GROW_CAPACITY(table->capacity);
		adjustHashTable(table, capacity);
	}

	// find entry to put value in
	Entry* entry = tableFind(table->entries, table->capacity, key);
	if (entry->key == NULL && IS_NIL(entry->value)) table->count++;

	// add new entry
	entry->key = key;
	entry->value= value;
}

Entry* tableFind(Entry* initialEntry, int capacity, ObjectString* key){
	int index = key->hash % capacity;

	Entry* tombstone = NULL;
	Entry* entry = NULL;

	while (true){
		entry = initialEntry + index;
		if (entry->key == NULL){
		      if (IS_NIL(entry->value)){
				return (tombstone != NULL) ? tombstone : entry;
		      } else {
				if (tombstone == NULL) tombstone = entry;
		      }

		} else if (entry->key == key) return entry;

		index = (index + 1) % capacity;
	}
	return entry;
}

bool tableHas(Table* table, ObjectString* key){
	if (table->capacity == 0) return false;

	Entry* entry = tableFind(table->entries, table->capacity, key);
	return (entry->key == NULL) ? false : true;
}

Value tableGet(Table* table, ObjectString* key){
	Entry* entry = tableFind(table->entries, table->capacity, key);
	if (entry->key == NULL) return NIL;
	return entry->value;
}

bool tableDelete(Table* table, ObjectString* key){
	if (table->count == 0) return false;

	Entry* entry = tableFind(table->entries, table->capacity, key);
	if (entry->key == NULL) return false;

	entry->key = NULL;
	entry->value = BOOLEAN(true);
	return true;
}

void adjustHashTable(Table* table, int capacity){
	// Allocate memory
	Entry* entries = (Entry*) reallocate(NULL, sizeof(Entry) * table->capacity, sizeof(Entry) * capacity);
	for (int i=0; i < capacity; i++){
		entries[i].key = NULL;	
		entries[i].value = NIL;	
	}

	table->count = 0;
	// Re-insert all elements from our heap array
	for (int i=0; i< table->capacity;i++){
		Entry* source= table->entries + i;

		if (source->key != NULL) {
			//re-insert	
			Entry* dest= tableFind(entries, capacity, source->key);
			dest->key = source->key;
			dest->value = source->value;

			table->count++;
		}
	}

	FREE_ARRAY(Entry, table->entries, table->capacity);
	table->entries = entries;
	table->capacity = capacity;
}

ObjectString* tableFindString(Table* table, const char* string, int length, uint32_t hash){
	if (table->capacity == 0) return NULL;

	int capacity = table->capacity;
	int index = hash % capacity;

	Entry* entry = NULL;

	while (true){
		entry = table->entries + index;
		if (entry->key == NULL){
		      if (IS_NIL(entry->value)) return NULL;

		} else if (entry->key->hash == hash && length == entry->key->length && memcmp(entry->key->string, string, length) == 0){
			   return entry->key;
		}

		index = (index + 1) % capacity;
	}
}
