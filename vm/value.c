#include <stdlib.h>
#include <stdio.h>
#include "value.h"
#include "memory.h"
#include "string.h"
#include "vm.h"


void initValueArray(ValueArray* array){
	array->count =0;
	array->capacity =0;
	array->values = NULL;
}

void appendValue(ValueArray* array, Value value){

	// push in the off chance the gc gets triggered and the LoxValue is an object
	push(value);

	if (array->count == array->capacity){
		int old_capacity = array->capacity;
		array->capacity = GROW_CAPACITY(old_capacity);
		array->values = GROW_ARRAY(Value, array->values, old_capacity, array->capacity);
	}

	// pop afterwards
	pop();

	*((array->values) + array->count) = value;
	(array->count)++;
}

void freeValueArray(ValueArray* array){
	FREE_ARRAY(Value, array->values, array->capacity);
	initValueArray(array);
}

void printValue(Value value){
	switch(value.type){
		case TYPE_NUM:
			{
				double val = AS_NUM(value);
				if (fmod(val, 1) == 0) printf("%d", (int) AS_NUM(value));
				else printf("%lf", AS_NUM(value));
			}
			break;
		case TYPE_BOOL:
			printf("%s", (AS_BOOL(value) == true) ? "true" : "false");
			break;
		case TYPE_NIL:
			printf("nil");
			break;
		case TYPE_OBJ:
			printObject(AS_OBJ(value));
			break;
		default:
			break;
	}
}

void printObject(Object* object){
	switch(object->objectType){
		case OBJECT_STRING:
			printf("\"%s\"", ((ObjectString*)object)->string);
			break;
		case OBJECT_FUNCTION:
			printFunction((ObjectFunction *)object);
			break;
		case OBJECT_NATIVE_FUNCTION:
			printf("< native fn: %s >", ((ObjectNativeFunction*) (object))->name->string);
			break;
		case OBJECT_CLOSURE:
			printFunction(((ObjectClosure *)object)->function);
			break;
		case OBJECT_UPVALUE:
			printf("< builtin obj: UPVALUE >");
			break;
		default:
			break;
	}
}

void printFunction(ObjectFunction* function) {
	if (function->name != NULL)
		printf("< function: %.*s >", function->name->length, function->name->string);
	else
		printf("< script >");
}

bool checkIfValuesEqual(Value val1, Value val2){
	if (val1.type != val2.type) return false;
	switch (val1.type){
		case TYPE_NIL: return true;
		case TYPE_NUM: return AS_NUM(val1) == AS_NUM(val2);
		case TYPE_BOOL: return AS_BOOL(val1) == AS_BOOL(val2);
		case TYPE_OBJ: return checkIfObjectsEqual(AS_OBJ(val1), AS_OBJ(val2));
		default: return false;
	}
}

bool checkIfObjectsEqual(Object* obj1, Object* obj2){
	switch(obj1->objectType){
		case OBJECT_STRING:
			{
				if (obj2->objectType != OBJECT_STRING) return false;
				return obj1 == obj2;
			}
		case OBJECT_FUNCTION:
			{
				if (obj2->objectType != OBJECT_FUNCTION) return false;
				ObjectFunction* objFunc1 = ((ObjectFunction*)obj1);
				ObjectFunction* objFunc2 = ((ObjectFunction*)obj2);
				return (objFunc1->arity == objFunc2->arity && objFunc1->chunk == objFunc2->chunk) ? true : false;
			}
		case OBJECT_NATIVE_FUNCTION:
			{
				if (obj2->objectType != OBJECT_NATIVE_FUNCTION) return false;
				ObjectNativeFunction* objFunc1 = ((ObjectNativeFunction*)obj1);
				ObjectNativeFunction* objFunc2 = ((ObjectNativeFunction*)obj2);
				return objFunc1->name == objFunc2->name;
			}
		default:
			return false;
	}
}
