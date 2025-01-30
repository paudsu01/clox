#ifndef VM_H
#define VM_H

#include "chunk.h"
#include "table.h"

#define STACK_MAX_SIZE 256

typedef enum{
	COMPILE_ERROR,
	RUNTIME_ERROR,
	NO_ERROR
} InterpreterResult;

typedef struct{
	Chunk* chunk;
	uint8_t* ip;
	Value* stackpointer;
	Value stack[STACK_MAX_SIZE];
	Object* objects;
	Table strings;
} VM;

// function prototypes
void initVM();
void freeVM();

InterpreterResult interpret(const char* source);
InterpreterResult runVM();

bool trueOrFalse(Value);
Object* concatenate();

void runtimeError(char*,...);

void push(Value);
Value pop();
Value peek(int);
void resetStack();

#endif
