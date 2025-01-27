#include "object.h"
#include "memory.h"
#include "string.h"
#include "vm.h"

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

	ObjectString* objString = (ObjectString *) allocateObject(sizeof(ObjectString), OBJECT_STRING);
	objString->string = string;
	objString->length = length;

	return objString;
}
