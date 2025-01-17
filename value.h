#ifndef VALUE_H
#define VALUE_H

typedef double Value;

// struct declaration
typedef struct{
	int count;
	int capacity;
	Value* constants;
} ValueArray;

// function prototypes
//
void initValueArray(ValueArray*);
int appendValue(ValueArray*, Value);
void freeValueArray(ValueArray*);

#endif
