#include <stdlib.h>

#include "memory.h"


void * reallocate(void* pointer, int oldsize, int newsize){

	if (newsize == 0){
		free(pointer);
		return NULL;
	}
	pointer = realloc(pointer, newsize);
	if (pointer == NULL) exit(1);
	return pointer;
}
