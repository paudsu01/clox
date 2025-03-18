#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>
#include <math.h>
#include "object.h"

// enum declaration
typedef enum{
	TYPE_NUM,
	TYPE_BOOL,
	TYPE_NIL,
	TYPE_OBJ
} ValueType;

// struct declaration
typedef struct{
	ValueType type;
	union{
		double number;
		bool boolean;
		Object* object;
	} as;
} Value;

typedef struct{
	int count;
	int capacity;
	Value* values;
} ValueArray;

// function prototypes
void initValueArray(ValueArray*);
void appendValue(ValueArray*, Value);
void freeValueArray(ValueArray*);

void printValue(Value value);
void printObject(Object* object);
void printFunction(ObjectFunction*);
bool checkIfValuesEqual(Value,Value);
bool checkIfObjectsEqual(Object*,Object*);


// Useful macros

#define BOOLEAN(value) ((Value) {.type=TYPE_BOOL, .as.boolean=value})
#define NUMBER(value) ((Value) {.type=TYPE_NUM, .as.number=value})
#define NIL ((Value) {.type=TYPE_NIL, .as.number=0})
#define OBJECT(obj) (Value) {.type=TYPE_OBJ, .as.object = (Object*) obj}

#define AS_BOOL(value) value.as.boolean
#define AS_NUM(value) value.as.number
#define AS_OBJ(value) value.as.object
#define AS_STRING_OBJ(value) (ObjectString*) value.as.object
#define AS_FUNCTION_OBJ(value) (ObjectFunction*) value.as.object
#define AS_NATIVE_FUNCTION_OBJ(value) (ObjectNativeFunction*) value.as.object

#define IS_BOOL(value) value.type == TYPE_BOOL
#define IS_NIL(value) value.type == TYPE_NIL
#define IS_NUM(value) value.type == TYPE_NUM
#define IS_OBJ(value) value.type == TYPE_OBJ
#define IS_STRING(value) IS_OBJ(value) && (AS_OBJ(value)->objectType == OBJECT_STRING)
#define IS_FUNCTION(value) IS_OBJ(value) && (AS_OBJ(value)->objectType == OBJECT_FUNCTION)

#endif
