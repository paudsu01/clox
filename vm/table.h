#ifndef TABLE_H
#define TABLE_H

#include "value.h"

typedef struct{
	ObjectString* key;
	Value value;
} Entry;

typedef struct{
	int count;
	int capacity;
	Entry* entries;
} Table;

// function prototypes
void initTable(Table*);
void freeTable(Table*);

void addEntry(Table*, ObjectString*, Value);
Entry* findEntry(Table*, ObjectString*);

#endif
