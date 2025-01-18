#ifndef VM_H
#define VM_H

#include "chunk.h"

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
} VM;

// function prototypes
void initVM();
void freeVM();

InterpreterResult interpret(Chunk*);

void push(Value value);
Value pop();
void resetStack();

#endif
