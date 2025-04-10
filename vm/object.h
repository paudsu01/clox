#ifndef OBJECT_H
#define OBJECT_H

#include "../common.h"

// Rely on forward declaration for Chunk
// since "chunk.h" uses "object.h", so cannot include "chunk.h"
typedef struct Chunk Chunk;

typedef enum{
	OBJECT_STRING,
	OBJECT_FUNCTION,
	OBJECT_NATIVE_FUNCTION,
	OBJECT_CLOSURE,
} ObjectType;

typedef enum{
	FUNCTION_MAIN,
	FUNCTION,
} FunctionType;

typedef struct Object{
	ObjectType objectType;	
	struct Object* next;
} Object;

typedef struct{
	Object object;
	int length;
	char* string;
	uint32_t hash;
} ObjectString;

typedef struct{
	Object object;
	int arity;
	ObjectString* name;
	Chunk* chunk;
} ObjectFunction;

typedef struct{
	Object object;
	ObjectFunction* function;
} ObjectClosure;

typedef bool (*NativeFunction) ();

typedef struct{
	Object object;
	int arity;
	ObjectString* name;
	NativeFunction nativeFunction;
} ObjectNativeFunction;

ObjectString* makeStringObject(const char*,int);
ObjectString* allocateStringObject(char*, int);
ObjectFunction* makeNewFunctionObject();
ObjectClosure* makeNewFunctionClosureObject(ObjectFunction*);
ObjectNativeFunction* makeNewNativeFunctionObject(ObjectString*, int, NativeFunction);
Object* allocateObject(int,ObjectType);
uint32_t jenkinsHash(const char*,int);

#endif
