#include <stdlib.h>

#include "memory.h"
#include "vm.h"


void * reallocate(void* pointer, int oldsize, int newsize){

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
				reallocate(objectClosure, sizeof(*objectClosure), 0);
			}
		default:
			break;
	}
}
