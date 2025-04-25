#include <stdio.h>

#include "object.h"
#include "memory.h"
#include "string.h"
#include "vm.h"

extern VM vm;

Object * allocateObject(int size, ObjectType type){
	extern VM vm;
	Object* object = (Object*) reallocate(NULL,0,size);
	object->objectType = type;

	object->next = vm.objects;
	object->isMarked = false;

	#ifdef DEBUG_LOG_GC
	printf("Allocate object of type %d\n", type);
	#endif

	vm.objects = object;

	return object;
}

ObjectString* makeStringObject(const char* start, int length){

	char* string = (char*) reallocate(NULL,0,length+1);
	memcpy(string, start, length);
	string[length] = '\0';
	return allocateStringObject(string, length);
}

ObjectString* allocateStringObject(char* string, int length){

	uint32_t hash = jenkinsHash(string, length);
	ObjectString* interned = tableFindString(&vm.strings, string, length, hash);

	if (interned == NULL){

		ObjectString* objString = (ObjectString *) allocateObject(sizeof(ObjectString), OBJECT_STRING);
		objString->string = string;
		objString->length = length;
		objString->hash = hash; 

		// Add to hash set
		tableAdd(&vm.strings, objString, NIL);

		return objString;
	}
	else {
		FREE_ARRAY(char, string, length + 1);
		return interned;
	}
}

ObjectFunction* makeNewFunctionObject(){

	ObjectFunction* objFunction = (ObjectFunction *) allocateObject(sizeof(ObjectFunction), OBJECT_FUNCTION);
	objFunction->name = NULL;
	objFunction->arity = 0;
	objFunction->upvaluesCount = 0;
	objFunction->chunk = (Chunk*) reallocate(NULL,0,sizeof(Chunk));
	initChunk(objFunction->chunk);

	return objFunction;
}

ObjectClosure* makeNewFunctionClosureObject(ObjectFunction* function){
	ObjectClosure* objFuncClosure = (ObjectClosure *) allocateObject(sizeof(ObjectClosure), OBJECT_CLOSURE);
	objFuncClosure->function = function;
	objFuncClosure->upvaluesCount = function->upvaluesCount;
	objFuncClosure->objUpvalues = reallocate(NULL, 0, (sizeof(ObjectUpvalue*) * function->upvaluesCount));
	
	for (int i = 0; i < function->upvaluesCount; i++){
		objFuncClosure->objUpvalues[i] = NULL;
	}

	return objFuncClosure;
}

ObjectNativeFunction* makeNewNativeFunctionObject(ObjectString* name, int arity, NativeFunction function){
	
	ObjectNativeFunction* objFunction = (ObjectNativeFunction *) allocateObject(sizeof(ObjectNativeFunction), OBJECT_NATIVE_FUNCTION);
	objFunction->name = name;
	objFunction->arity = arity;
	objFunction->nativeFunction = function;
	return objFunction;
}

ObjectUpvalue* makeNewUpvalueObject(int index){
	ObjectUpvalue* objUpvalue;

	// Reuse objUpvalue if an open upvalue already exists
	if ((objUpvalue = vm.openObjUpvalues[index]) == NULL) {
		objUpvalue = (ObjectUpvalue *) allocateObject(sizeof(ObjectUpvalue), OBJECT_UPVALUE);
		vm.openObjUpvalues[index] = objUpvalue;
	}

	return objUpvalue;
}

uint32_t jenkinsHash(const char* key, int length){
	// Jenkins hash function
	// Reference: https://en.wikipedia.org/wiki/Jenkins_hash_function

	size_t i = 0;
	uint32_t hash = 0;
	while (i != length) {
	  hash += key[i++];
	  hash += hash << 10;
	  hash ^= hash >> 6;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;

	return hash;
}
