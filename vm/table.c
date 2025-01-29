#include "../common.h"
#include "table.h"
#include "memory.h"


void initTable(Table* table){
	table->count=0;
	table->capacity=0;
	table->entries=NULL;
}


void freeTable(Table* table){
	FREE_ARRAY(Entry, table->entries, table->capacity);
	initTable(table);
}

void addEntry(Table* table, ObjectString* key, Value value){
	// allocate and resize if necessary
	if (table->count + 1 > table->capacity * MAX_TABLE_LOAD) {
		int capacity = GROW_CAPACITY(table->capacity);
		adjustHashTable(table, capacity);
	}

	// find entry to put value in
	Entry* entry = findEntry(table->entries, table->capacity, key);
	if (entry->key != NULL) table->count++;

	// add new entry
	entry->key = key;
	entry->value= value;
}

Entry* findEntry(Entry* initialEntry, int capacity, ObjectString* key){
	int index = key->hash % capacity;

	Entry* entry = NULL;
	while (true){
		entry = initialEntry + index;
		if (entry->key == NULL || entry->key == key) return entry;
		index = (index + 1) % capacity;
	}
	return entry;
}

void adjustHashTable(Table* table, int capacity){
	// Allocate memory
	Entry* entries = (Entry*) reallocate(NULL, sizeof(Entry) * table->capacity, sizeof(Entry) * capacity);
	for (int i=0; i < capacity; i++){
		entries[i].key = NULL;	
		entries[i].value = NIL;	
	}

	// Re-insert all elements from our heap array
	for (int i=0; i< table->capacity;i++){
		Entry* source= table->entries + i;
		if (source->key != NULL) {
			//re-insert	
			Entry* dest= findEntry(entries, capacity, source->key);
			dest->key = source->key;
			dest->value = source->value;
		}
	}
	FREE_ARRAY(Entry, table->entries, table->capacity);
	table->entries = entries;
	table->capacity = capacity;
}
