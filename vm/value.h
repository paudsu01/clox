#ifndef VALUE_H
#define VALUE_H

// enum declaration
typedef enum{
	TYPE_NUM,
	TYPE_BOOL,
	TYPE_NIL
} ValueType;

// struct declaration
typedef struct{
	ValueType type;
	union{
		double number;
		bool boolean;
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


// Useful macros

#define BOOLEAN(value) ((Value) {.type=TYPE_BOOL, .as.boolean=value})
#define NUMBER(value) ((Value) {.type=TYPE_NUM, .as.number=value})
#define NIL ((Value) {.type=TYPE_NIL, .as.number=0})

#define AS_BOOL(value) value.as.boolean
#define AS_NUM(value) value.as.number

#define IS_BOOL(value) value.type == TYPE_BOOL
#define IS_NIL(value) value.type == TYPE_NIL
#define IS_NUM(value) value.type == TYPE_NUM

#endif
