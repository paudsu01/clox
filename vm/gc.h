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

#endif
