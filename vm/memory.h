#ifndef MEMORY_H
#define MEMORY_H

#include "../common.h"
#include "object.h"
#include "value.h"
#include "table.h"

#define GROW_CAPACITY(capacity) \
	((capacity < 8) ? 8: capacity*2)

#define GROW_ARRAY(type, pointer, oldsize, newsize) \
       	((type *)reallocate(pointer, sizeof(type) * oldsize, sizeof(type) * newsize))

#define FREE_ARRAY(type, pointer, oldsize) \
       	reallocate(pointer, sizeof(type) * oldsize, 0)

void* reallocate(void*, int, int);

void freeObjects();
void freeObject(Object* object);

// Garbage collector functions
void runGarbageCollector();
void markObjects();
void markRoots();

void markValue(Value value);
void markObject(Object* object);
void markHashTable(Table* table);
void markStack();

#endif
