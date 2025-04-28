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
	printf("free object of type: %d\n", object->objectType);
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
	#ifdef DEBUG_LOG_GC
	printf("-- GC run --\n");
	#endif
	
	//TODO
	markObjects();

	#ifdef DEBUG_LOG_GC
	printf("-- GC end --\n");
	#endif
}

void markObjects(){

	markRoots();
	//TODO
}

void markRoots(){
	// mark the global variables first
	markHashTable(&vm.globals);

	// mark the stack values
	markStack();

	//call frame mark
	markCallFrame();
}

void markValue(Value value){
	if (IS_OBJ(value)) markObject(AS_OBJ(value));
}

void markObject(Object* object){

	if (object->isMarked) return;

	#ifdef DEBUG_LOG_GC
	printf("mark %d\n", object->objectType);
	#endif

	object->isMarked = true;
}

void markHashTable(Table* table){
	for (int i=0; i < table->capacity; i++){
		if ((table->entries[i].key != NULL) || (table->entries[i].value.type != TYPE_NIL)){
			markObject((Object *) table->entries[i].key);
			markValue(table->entries[i].value);
		}
	}
}

void markStack(){
	for (Value* i=vm.stack; i < vm.stackpointer; i++){
		markValue(*i);
	}
}

void markCallFrame(){
	for (int i=0; i < vm.frameCount; i++){
		CallFrame frame = vm.frames[i];
		markObject((Object*) frame.closure);
	}
}
