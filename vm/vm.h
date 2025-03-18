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
	ObjectFunction* function;
	uint8_t* ip;
	Value* stackStart;
} CallFrame;

typedef struct{
	CallFrame frames[CALL_FRAMES_MAX];
	int frameCount;

	Value* stackpointer;
	Value stack[STACK_MAX_SIZE];

	Object* objects;
	Table strings;
	Table globals;
} VM;

// function prototypes
void initVM();
void freeVM();

// Call frame function prototypes
void initCallFrame(CallFrame*);
void addFunctionToCurrentCallFrame(CallFrame*, ObjectFunction*);

InterpreterResult interpret(const char* source);
InterpreterResult runVM();

bool trueOrFalse(Value);
Object* concatenate();
void mutate_vm_ip(uint8_t, uint16_t);

void declareNativeFunctions();
void clockNativeFunction();

void runtimeError(char*,...);

void push(Value);
Value pop();
Value peek(int);
void resetStack();

#endif
