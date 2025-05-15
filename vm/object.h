#ifndef OBJECT_H
#define OBJECT_H

#include "../common.h"

// ObjectUpvalue defined in "value.h" because of circular dependency problems

struct Table;

// Rely on forward declaration for Chunk
// since "chunk.h" uses "object.h", so cannot include "chunk.h"
typedef struct Chunk Chunk;
typedef struct ObjectUpvalue ObjectUpvalue;

typedef enum{
	OBJECT_STRING,
	OBJECT_FUNCTION,
	OBJECT_NATIVE_FUNCTION,
	OBJECT_CLOSURE,
	OBJECT_UPVALUE,
	OBJECT_CLASS,
	OBJECT_INSTANCE
} ObjectType;

typedef enum{
	FUNCTION_MAIN,
	FUNCTION,
	METHOD,
	METHOD_INIT
} FunctionType;

typedef struct Object{
	ObjectType objectType;	
	struct Object* next;
	bool isMarked;
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
	int upvaluesCount;
	ObjectString* name;
	Chunk* chunk;
} ObjectFunction;

typedef struct{
	Object object;
	ObjectFunction* function;
	int upvaluesCount;
	ObjectUpvalue** objUpvalues;
} ObjectClosure;

typedef bool (*NativeFunction) ();

typedef struct{
	Object object;
	int arity;
	ObjectString* name;
	NativeFunction nativeFunction;
} ObjectNativeFunction;

typedef struct{
	Object object;
	ObjectString* name;
} ObjectClass;

typedef struct{
	Object object;
	ObjectClass* Class;
	struct Table* fields;
} ObjectInstance;

ObjectString* makeStringObject(const char*,int);
ObjectString* allocateStringObject(char*, int);
ObjectFunction* makeNewFunctionObject();
ObjectClosure* makeNewFunctionClosureObject(ObjectFunction*);
ObjectNativeFunction* makeNewNativeFunctionObject(ObjectString*, int, NativeFunction);
ObjectUpvalue* makeNewUpvalueObject(int);
ObjectClass* makeClassObject(ObjectString*);
ObjectInstance* makeInstanceObject(ObjectClass*);

Object* allocateObject(int,ObjectType);
uint32_t jenkinsHash(const char*,int);

#endif
