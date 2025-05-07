#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "vm.h"

extern VM vm;

void * reallocate(void* pointer, int oldsize, int newsize){

	#ifdef EXCESSIVE_GC_MODE
	if (newsize > oldsize) runGarbageCollector();
	#endif

	if (newsize == 0){
		free(pointer);
		return NULL;
	}
	pointer = realloc(pointer, newsize);
	if (pointer == NULL) exit(1);

	return pointer;
}

void freeObjects(){
	extern VM vm;
	Object* current = vm.objects;
	while (current != NULL){
		Object* next = current->next;
		freeObject(current);
		current = next;
	}
}

void freeObject(Object* object){
	
	#ifdef DEBUG_LOG_GC
	printf("free object of type: %d\t", object->objectType);
	printValue(OBJECT(object));
	printf("\n");
	#endif

	switch(object->objectType){
		case OBJECT_STRING:
			{
				ObjectString* objectString = (ObjectString*)object;
				FREE_ARRAY(char, objectString->string, objectString->length+1);
				reallocate(objectString, sizeof(*objectString), 0);
			}
			break;
		case OBJECT_FUNCTION:
			{
				ObjectFunction* objectFunction = (ObjectFunction*)object;
				freeChunk(objectFunction->chunk);
				reallocate(objectFunction, sizeof(*objectFunction), 0);
			}
			break;
		case OBJECT_NATIVE_FUNCTION:
			{
				ObjectNativeFunction* objectNativeFunction = (ObjectNativeFunction*)object;
				reallocate(objectNativeFunction, sizeof(*objectNativeFunction), 0);
			}
			break;
		case OBJECT_CLOSURE:
			{
				ObjectClosure* objectClosure = (ObjectClosure*) object;
				FREE_ARRAY(ObjectUpvalue*, objectClosure->objUpvalues, objectClosure->upvaluesCount);
				reallocate(objectClosure, sizeof(*objectClosure), 0);
			}
			break;
		case OBJECT_UPVALUE:
			{
				ObjectUpvalue* objectUpvalue = (ObjectUpvalue*) object;
				reallocate(objectUpvalue, sizeof(*objectUpvalue), 0);
				
			}
			break;
		default:
			break;
	}

}


// Garbage collector functions
void runGarbageCollector(){
	initGC();

	#ifdef DEBUG_LOG_GC
	printf("-- GC run --\n");
	printf("Marking objects--\n");
	#endif

	markObjects();
	freeStringsFromVMHashTable();

	#ifdef DEBUG_LOG_GC
	printf("Sweeping unreachable objects--\n");
	#endif
	sweepObjects();

	resetGC();
	#ifdef DEBUG_LOG_GC
	printf("-- GC end --\n");
	#endif
}
