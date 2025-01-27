#ifndef OBJECT_H
#define OBJECT_H

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
} ObjectString;

ObjectString* makeStringObject(const char*,int);
Object* allocateObject(int,ObjectType);

#endif
