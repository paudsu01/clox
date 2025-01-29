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
	//TODO

	// find entry to put value in
	Entry* entry = findEntry(table, key);
	if (entry->key != NULL) table->count++;

	// add new entry
	entry->key = key;
	entry->value= value;
}

Entry* findEntry(Table* table, ObjectString* key){
	int capacity = table->capacity;
	int index = key->hash % capacity;

	Entry* entry = NULL;
	while (true){
		entry = table->entries + index;
		if (entry->key == NULL || entry->key == key) return entry;
		index = (index + 1) % capacity;
	}
	return entry;
}

