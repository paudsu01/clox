#ifndef GARBAGE_COLLECTOR_HEADER
#define GARBAGE_COLLECTOR_HEADER

#include "object.h"
#include "memory.h"

#include <stdlib.h>


void initGC();
void resetGC();

typedef struct{
	int capacity;
	int count;
	Object** objectsQueue;
} GC;


void addObject(Object*);

void markObjects();
void markRoots();
void markCompilerRoots();

void sweepObjects();

void markValue(Value value);
void markObject(Object* object);
void markHashTable(Table* table);
void markStack();
void markCallFrame();

void addChildObjectsToGCQueue(Object*);
void freeStringsFromVMHashTable();
#endif
