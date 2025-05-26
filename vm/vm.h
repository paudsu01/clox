#ifndef VM_H
#define VM_H

#include "chunk.h"
#include "table.h"
#include "gc.h"

#define STACK_MAX_SIZE 256

typedef enum{
	COMPILE_ERROR,
	RUNTIME_ERROR,
	NO_ERROR
} InterpreterResult;

typedef struct{
	ObjectClosure* closure;
	uint8_t* ip;
	Value* stackStart;
} CallFrame;

typedef struct{
	CallFrame frames[CALL_FRAMES_MAX];
	int frameCount;

	Value* stackpointer;
	Value stack[STACK_MAX_SIZE];
	ObjectUpvalue* openObjUpvalues[STACK_MAX_SIZE];

	Object* objects;
	Table strings;
	Table globals;

	ObjectString* init;

	int bytesAllocated;
	int nextGCRun;

	GC gc;
} VM;

// function prototypes
void initVM(bool end);
void freeVM();

// Call frame function prototypes
void initCallFrame(CallFrame*);
void addClosureToCurrentCallFrame(CallFrame*, ObjectClosure*);

InterpreterResult interpret(const char* source);
InterpreterResult runVM();

bool trueOrFalse(Value);
Object* concatenate();
void mutate_vm_ip(uint8_t, uint16_t);
void closeObjUpvalue(int);
void closeObjUpvalues();
int findAndBindMethod(ObjectInstance*, ObjectClass*, ObjectString*);


void declareNativeFunctions();
void declareNativeFunction(char[], int, NativeFunction);
bool clockNativeFunction();
bool inputNativeFunction();
bool numberNativeFunction();

void runtimeError(char*,...);
bool callNoErrors(int, Value);
bool call(Value, int, CallFrame**);

void push(Value);
Value pop();
Value peek(int);
void resetStack();
void resetOpenObjUpvalues();

#endif
