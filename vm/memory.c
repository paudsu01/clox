#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "../compiler/compiler.h"
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

void markObjects(){

	// The idea is that we do a breadth first search
	// First, we mark the roots and add their child objects to the queue
	markRoots();
	markCompilerRoots();
	// Then we remove an Object from the idea, mark it, add their child objects to the queue and keep repeating until there are no more objects in the queue
	int index=0;
	while (index < vm.gc.count){
		Object* obj = vm.gc.objectsQueue[index];	
		markObject(obj);
		index++;
	}
}

void markRoots(){
	// mark the global variables first
	markHashTable(&vm.globals);

	// mark the stack values
	markStack();

	//call frame mark
	markCallFrame();
}

void markCompilerRoots(){
	extern Compiler* currentCompiler;
	
	Compiler* current = currentCompiler;
	while (current != NULL){
		markObject((Object *) current->function);
		current = current->parentCompiler;
	}
}


void markValue(Value value){
	if (IS_OBJ(value)) markObject(AS_OBJ(value));
}

void markObject(Object* object){

	if (object == NULL) return;
	if (object->isMarked) return;

	#ifdef DEBUG_LOG_GC
	printf("mark type: %d\t", object->objectType);
	printValue(OBJECT(object));
	printf("\n");
	#endif

	object->isMarked = true;

	addChildObjectsToGCQueue(object);
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

void addChildObjectsToGCQueue(Object* object){
	
	switch(object->objectType){
		case OBJECT_NATIVE_FUNCTION:
			{
				ObjectNativeFunction* objNative = (ObjectNativeFunction*) object;
				addObject((Object*) objNative->name);
			}
			break;
		case OBJECT_FUNCTION:
			{
				ObjectFunction* objFunc = (ObjectFunction*) object;
				addObject((Object*) objFunc->name);

				ValueArray* objFuncConstArray = &(objFunc->chunk->constants);
				for (int i=0; i < objFuncConstArray->count; i++){
					Value value = objFuncConstArray->values[i];
					markValue(value);
				}
			}
			break;
		case OBJECT_CLOSURE:
			{
				ObjectClosure* objClosure = (ObjectClosure*) object;
				addObject((Object*) objClosure->function);
				for (int i=0; i< objClosure->upvaluesCount; i++){
					addObject((Object*) objClosure->objUpvalues[i]);
				}
			}
			break;
		case OBJECT_UPVALUE:
			{
				ObjectUpvalue* objUpvalue = (ObjectUpvalue*) object;
				Value value = *objUpvalue->value;
				addObject(AS_OBJ(value));
			}
			break;
		case OBJECT_STRING:
			// Nothing to do
			break;
	}

}

void freeStringsFromVMHashTable(){
	
	for (int i=0; i< vm.strings.capacity; i++){
		Entry entry = vm.strings.entries[i];
		if (entry.key != NULL && !(((Object*) entry.key)->isMarked))
			tableDelete(&vm.strings, entry.key);
	}
}

void sweepObjects(){

	Object* previous = NULL;
	Object* current = vm.objects;
	while (current != NULL){

		if (current->isMarked){
			current->isMarked = false;
			previous = current;	
			current = current->next;
		} else {
			if (previous == NULL) {
				vm.objects = current->next;
				freeObject(current);
				current = vm.objects;

			} else{
				current = current->next;
				freeObject(previous->next);
			        previous->next = current;
			}
		}
	}
}
