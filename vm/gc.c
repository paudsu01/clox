#include <stdio.h>

#include "gc.h"
#include "vm.h"
#include "../compiler/compiler.h"

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
	if (object == NULL) return;
	if (vm.gc.count == vm.gc.capacity){
		vm.gc.capacity = GROW_CAPACITY(vm.gc.capacity);
		vm.gc.objectsQueue = (Object**) realloc(vm.gc.objectsQueue, sizeof(Object*) *  vm.gc.capacity);
		if (vm.gc.objectsQueue == NULL) exit(1);
	}

	vm.gc.objectsQueue[vm.gc.count++] = object;
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
		case OBJECT_CLASS:
			{
				ObjectClass* objClass = (ObjectClass*) object;
				addObject((Object*) objClass->name);
			}
			break;
		case OBJECT_INSTANCE:
			{
				ObjectInstance* objInstance = (ObjectInstance*) object;
				addObject((Object*) objInstance->Class);
				for (int i=0; i< objInstance->fields->capacity; i++){
					Entry entry = objInstance->fields->entries[i];
					addObject((Object*) entry.key);
					addObject(AS_OBJ(entry.value));
				}
				
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

