#include "gc.h"
#include "vm.h"

extern VM vm;

void initGC(){
	vm.gc.count = 0;
	vm.gc.capacity = 0;
	vm.gc.objectsQueue = NULL;
}

void resetGC(){
	free(vm.gc.objectsQueue);	
	initGC();
}

void addObject(Object* object){
	if (vm.gc.count == vm.gc.capacity){
		vm.gc.capacity = GROW_CAPACITY(vm.gc.capacity);
		vm.gc.objectsQueue = (Object**) realloc(vm.gc.objectsQueue, sizeof(Object*) *  vm.gc.capacity);
		if (vm.gc.objectsQueue == NULL) exit(1);
	}

	vm.gc.objectsQueue[vm.gc.count++] = object;
}
