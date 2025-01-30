#ifndef OBJECT_H
#define OBJECT_H

#include "../common.h"

typedef enum{
	OBJECT_STRING,
} ObjectType;

struct Object{
	ObjectType objectType;	
	struct Object* next;
};
typedef struct Object Object;

typedef struct{
	Object object;
	int length;
	char* string;
	uint32_t hash;
} ObjectString;

ObjectString* makeStringObject(const char*,int);
ObjectString* allocateStringObject(char*, int);
Object* allocateObject(int,ObjectType);
uint32_t jenkinsHash(const char*,int);

#endif
