#ifndef TABLE_H
#define TABLE_H

#include "value.h"

#define MAX_TABLE_LOAD 0.75

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

void tableAdd(Table*, ObjectString*, Value);
Entry* tableFind(Entry*,int,ObjectString*);
bool tableHas(Table*, ObjectString*);
Value tableGet(Table*, ObjectString*);
bool tableDelete(Table*, ObjectString*);

void adjustHashTable(Table*, int);

#endif
