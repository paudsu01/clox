#ifndef OBJECT_H
#define OBJECT_H

typedef enum{
	OBJECT_STRING,
} ObjectType;

typedef struct{
	ObjectType objectType;	
} Object;

typedef struct{
	Object object;
	int length;
	char* string;
} ObjectString;

ObjectString* makeStringObject(const char*,int);
Object* allocateObject(int,ObjectType);

#endif
