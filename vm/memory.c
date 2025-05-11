#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "vm.h"

extern VM vm;

void * reallocate(void* pointer, int oldsize, int newsize){

	vm.bytesAllocated += (newsize - oldsize);

	// this function is indirectly recursive since if the runGarbageCollector() is triggered, it can call the reallocate() function again while object from memory is being freed
	// In that case, we don't want to run the garbageCollector again which is why the newsize > oldsize requirement is also there
	// The vm.bytesAllocated >= vm.nextGCRun might not be enough because if a lot of bytes were told to be allocated, a small # of bytes being freed might still make the bytesAllocated >= nextGCRun which in turn triggers the runGarbageCollector again
	if (newsize > oldsize && vm.bytesAllocated >= vm.nextGCRun){
		runGarbageCollector();
	} else{
		#ifdef EXCESSIVE_GC_MODE
		if (newsize > oldsize) runGarbageCollector();
		#endif
	}


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
		case OBJECT_CLASS:
			{
				ObjectClass* objectClass = (ObjectClass*) object;
				reallocate(objectClass, sizeof(*objectClass), 0);
			}
			break;
		case OBJECT_INSTANCE:
			{
				ObjectInstance* objectInstance = (ObjectInstance*) object;
				freeTable(objectInstance->fields);
				reallocate(objectInstance->fields, sizeof(*objectInstance->fields), 0);
				reallocate(objectInstance, sizeof(*objectInstance), 0);
			}
			break;
		default:
			break;
	}

}


// Garbage collector functions
void runGarbageCollector(){
	initGC();

	int bytesBefore = vm.bytesAllocated;

	#ifdef DEBUG_LOG_GC
	printf("--- GC run --\n");
	printf("- Marking objects -\n");
	#endif

	markObjects();
	freeStringsFromVMHashTable();

	#ifdef DEBUG_LOG_GC
	printf("- Sweeping unreachable objects -\n");
	#endif
	sweepObjects();


	// Set the next GC run
	 vm.nextGCRun = (vm.bytesAllocated * 2 <= INITIAL_GC_TRIGGER_VALUE) ? INITIAL_GC_TRIGGER_VALUE : 2 * vm.bytesAllocated;
	
	resetGC();

	#ifdef DEBUG_LOG_GC
	int bytesAfter = vm.bytesAllocated;
	printf("- Bytes freed from memory: %d -\n", bytesBefore - bytesAfter);
	printf("--- GC end ---\n");
	#endif
}
