#include <stdlib.h>
#include "value.h"
#include "memory.h"


void initValueArray(ValueArray* array){
	array->count =0;
	array->capacity =0;
	array->constants = NULL;
}

int appendValue(ValueArray* array, Value value){
	if (array->count == array->capacity){
		int old_capacity = array->capacity;
		array->capacity = GROW_CAPACITY(old_capacity);
		array->constants = GROW_ARRAY(Value, array->constants, old_capacity, array->capacity);
	}

	*((array->constants) + array->count) = value;
	(array->count)++;

	return 0;
}

void freeValueArray(ValueArray* array){
	FREE_ARRAY(Value, array->constants, array->capacity);
	initValueArray(array);
}
