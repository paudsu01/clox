#include "object.h"
#include "memory.h"
#include "string.h"

Object * allocateObject(int size, ObjectType type){
	Object* object = (Object*) reallocate(NULL,0,size);
	object->objectType = type;
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
