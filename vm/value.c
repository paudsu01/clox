#include <stdlib.h>
#include <stdio.h>
#include "value.h"
#include "memory.h"


void initValueArray(ValueArray* array){
	array->count =0;
	array->capacity =0;
	array->values = NULL;
}

void appendValue(ValueArray* array, Value value){
	if (array->count == array->capacity){
		int old_capacity = array->capacity;
		array->capacity = GROW_CAPACITY(old_capacity);
		array->values = GROW_ARRAY(Value, array->values, old_capacity, array->capacity);
	}

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
			printf("%lf", AS_NUM(value));
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
		default:
			break;
	}
}

bool checkIfValuesEqual(Value val1, Value val2){
	if (val1.type != val2.type) return false;
	switch (val1.type){
		case TYPE_NIL: return true;
		case TYPE_NUM: return AS_NUM(val1) == AS_NUM(val2);
		case TYPE_BOOL: return AS_BOOL(val1) == AS_BOOL(val2);
		default: return false;
	}
}
