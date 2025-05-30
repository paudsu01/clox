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

ObjectFunction* makeNewFunctionObject(FunctionType type){

	ObjectFunction* objFunction = (ObjectFunction *) allocateObject(sizeof(ObjectFunction), OBJECT_FUNCTION);
	objFunction->name = NULL;
	objFunction->arity = 0;
	objFunction->upvaluesCount = 0;
	objFunction->type = type;

	// Push beforehand in the off chance the gc runs and we lose the function object
	push(OBJECT(objFunction));
	objFunction->chunk = (Chunk*) reallocate(NULL,0,sizeof(Chunk));
	// pop afterwards
	pop();

	initChunk(objFunction->chunk);

	return objFunction;
}

ObjectClosure* makeNewFunctionClosureObject(ObjectFunction* function){
	// Push beforehand in the off chance the gc runs and we lose the function object
	push(OBJECT(function));
	ObjectClosure* objFuncClosure = (ObjectClosure *) allocateObject(sizeof(ObjectClosure), OBJECT_CLOSURE);


	objFuncClosure->function = function;
	objFuncClosure->upvaluesCount = function->upvaluesCount;

	// Push beforehand in the off chance the gc runs and we lose the function closure object
	push(OBJECT(objFuncClosure));
	objFuncClosure->objUpvalues = reallocate(NULL, 0, (sizeof(ObjectUpvalue*) * function->upvaluesCount));
	// pop afterwards
	pop();
	pop();
	
	for (int i = 0; i < function->upvaluesCount; i++){
		objFuncClosure->objUpvalues[i] = NULL;
	}

	return objFuncClosure;
}

ObjectNativeFunction* makeNewNativeFunctionObject(ObjectString* name, int arity, NativeFunction function){
	
	// Push beforehand in the off chance the gc runs and we lose the LoxString object
	push(OBJECT(name));
	ObjectNativeFunction* objFunction = (ObjectNativeFunction *) allocateObject(sizeof(ObjectNativeFunction), OBJECT_NATIVE_FUNCTION);
	// pop afterwards
	pop();

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

ObjectClass* makeClassObject(ObjectString* name){
	ObjectClass* class =(ObjectClass*) allocateObject(sizeof(ObjectClass), OBJECT_CLASS);
	class->name = name;

	Table* methods = reallocate(NULL, 0, sizeof(Table));
	initTable(methods);
	class->methods = methods;
	class->superclass = NULL;

	return class;
}

ObjectInstance* makeInstanceObject(ObjectClass* Class){
	ObjectInstance* instance =(ObjectInstance*) allocateObject(sizeof(ObjectInstance), OBJECT_INSTANCE);
	instance->Class = Class;

	Table* fields = reallocate(NULL, 0, sizeof(Table));
	initTable(fields);
	instance->fields = fields;

	return instance;
}

ObjectBoundMethod* makeBoundMethodObject(ObjectClosure* closure, ObjectInstance* instance){
	ObjectBoundMethod* boundMethod =(ObjectBoundMethod*) allocateObject(sizeof(ObjectBoundMethod), OBJECT_BOUND_METHOD);
	boundMethod->closure = closure;
	boundMethod->instance = instance;
	return boundMethod;
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
