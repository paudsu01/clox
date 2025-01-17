#ifndef VALUE_H
#define VALUE_H

typedef double Value;

// struct declaration
typedef struct{
	int count;
	int capacity;
	Value* values;
} ValueArray;

// function prototypes
//
void initValueArray(ValueArray*);
void appendValue(ValueArray*, Value);
void freeValueArray(ValueArray*);

#endif
