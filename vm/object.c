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
	objFunction->chunk = (Chunk*) reallocate(NULL,0,sizeof(Chunk));
	initChunk(objFunction->chunk);

	return objFunction;
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
