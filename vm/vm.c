#include "vm.h"
#include "memory.h"
#include "object.h"
#include "../compiler/compiler.h"
#include "../debug/disassembler.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

VM vm;

void initVM(bool end){
	vm.objects = NULL;
	vm.frameCount = 0;
	initTable(&vm.strings);
	initTable(&vm.globals);
	resetStack();
	resetOpenObjUpvalues();
	vm.gc = (GC) {.count=0, .capacity=0, .objectsQueue=NULL};
	vm.bytesAllocated = 0;
	vm.nextGCRun = INITIAL_GC_TRIGGER_VALUE;

	if (!end) declareNativeFunctions();
}

void initCallFrame(CallFrame* frame){
	frame->ip = NULL;
	frame->stackStart = vm.stackpointer;
	frame->closure = NULL;
}

void addClosureToCurrentCallFrame(CallFrame* frame, ObjectClosure* closure){
	frame->closure = closure;
	frame->ip = closure->function->chunk->code;
}

InterpreterResult interpret(const char* source){

	ObjectFunction* currentFunction = compile(source);
	ObjectClosure* currentClosure = makeNewFunctionClosureObject(currentFunction);

	if (currentFunction == NULL){
		return COMPILE_ERROR;
	}

	else {

		CallFrame* frame = &(vm.frames[vm.frameCount=0]);
		initCallFrame(frame);
		addClosureToCurrentCallFrame(frame, currentClosure);
		push(OBJECT(currentClosure));

		return runVM();
	}
}

InterpreterResult runVM(){
	
	CallFrame* frame = &vm.frames[vm.frameCount++];

	#define READ_BYTE() *(frame->ip++)
	#define READ_CONSTANT() (frame->closure->function->chunk->constants).values[READ_BYTE()]
	#define READ_2BYTES() ((uint16_t) (*frame->ip << 8)) + *(frame->ip+1)
		

	#define BYTES_LEFT_TO_EXECUTE() (frame->ip < (frame->closure->function->chunk->code + frame->closure->function->chunk->count))

	#define BINARY_OP(resultValue, op, type) \
			do { 	Value b = peek(0); Value a=peek(1); \
				if (IS_NUM(b) && IS_NUM(a)){ \
			 		double d = AS_NUM(pop()); double c=AS_NUM(pop()); \
			  		push(resultValue(c op d)); \
				} \
				else if (type == OP_ADD && IS_STRING(b) && IS_STRING(a)){ \
					push(OBJECT(concatenate())); \
				} else if (type == OP_ADD){\
					runtimeError("Operands must be two numbers or two strings");\
					return RUNTIME_ERROR;\
				} else{ \
					runtimeError("Operands must be numbers");\
					return RUNTIME_ERROR;\
				} \
			} while (false) \
		

	Value value;
	while (BYTES_LEFT_TO_EXECUTE()){
		
		#ifdef DEBUG_TRACE_EXECUTION
			disassembleVMStack();
			disassembleInstruction(frame->closure->function->chunk, (int) ((frame->ip) - (frame->closure->function->chunk->code)));
		#endif

		uint8_t byte = READ_BYTE();
		switch (byte){
			case OP_RETURN:
				{
					Value returnValue = pop();

					// Handle any open upvalues that need to be closed
					closeObjUpvalues();

					vm.stackpointer = frame->stackStart;
					// no need to push the `nil` value for the main function
					if (vm.frameCount > 1){
						push(returnValue);
						frame = &vm.frames[(--vm.frameCount) - 1];
					} else return NO_ERROR;
				}
				break;

			case OP_PRINT:
				printValue(pop());
				printf("\n");
				break;

			case OP_POP:
				pop();
				break;

			case OP_POP_UPVALUE:
				closeObjUpvalue((vm.stackpointer - 1) - vm.stack);
				pop();
				break;

			case OP_CONSTANT:
				value = READ_CONSTANT();
				push(value);
				break;

			case OP_TRUE:
				push(BOOLEAN(true)); break;

			case OP_FALSE:
				push(BOOLEAN(false)); break;

			case OP_NIL:
				push(NIL); break;

			case OP_NEGATE:
				if (!(IS_NUM(peek(0)))){
					runtimeError("Operand must be a number");
					return RUNTIME_ERROR;
				} else{
					push(NUMBER(-AS_NUM(pop())));
				}
				break;

			case OP_NOT:
				push(BOOLEAN(!trueOrFalse(pop())));
				break;

			case OP_ADD:
				BINARY_OP(NUMBER, +, OP_ADD);
				break;
			
			case OP_SUBTRACT:
				BINARY_OP(NUMBER, -, OP_SUBTRACT);
				break;

			case OP_MULTIPLY:
				BINARY_OP(NUMBER, *, OP_MULTIPLY);
				break;

			case OP_DIVIDE:
				BINARY_OP(NUMBER, /, OP_DIVIDE);
				break;

			case OP_GT:
				BINARY_OP(BOOLEAN, >, OP_GT);
				break;

			case OP_LT:
				BINARY_OP(BOOLEAN, <, OP_LT);
				break;

			case OP_EQUAL:
				push(BOOLEAN(checkIfValuesEqual(pop(), pop())));
				break;

			case OP_DEFINE_GLOBAL:
				{
					value = READ_CONSTANT();
					ObjectString* objString = AS_STRING_OBJ(value);
					tableAdd(&vm.globals, objString, pop());
				}
				break;

			case OP_GET_GLOBAL:
				{
					value = READ_CONSTANT();
					ObjectString* objString = AS_STRING_OBJ(value);
					if (tableHas(&vm.globals, objString)){
						push(tableGet(&vm.globals, objString));

					} else {
						runtimeError("Undefined variable '%s'", objString->string) ;
						return RUNTIME_ERROR;
					}
				}
				break;

			case OP_GET_LOCAL:
				{
					uint8_t index = READ_BYTE();
					push(*(frame->stackStart + index));
				}
				break;

			case OP_SET_LOCAL:
				{
					uint8_t index = READ_BYTE();
					*(frame->stackStart + index) = peek(0);
				}
				break;

			case OP_SET_GLOBAL:
				{
					value = READ_CONSTANT();
					ObjectString* objString = AS_STRING_OBJ(value);
					if (tableHas(&vm.globals, objString)){
						tableAdd(&vm.globals, objString, peek(0));
					} else {
						runtimeError("Undefined variable '%s'", objString->string) ;
						return RUNTIME_ERROR;
					}
				}
				break;

			case OP_JUMP_IF_FALSE:
				{
					uint16_t offset = READ_2BYTES();
					mutate_vm_ip(OP_JUMP_IF_FALSE, offset);
				}
				break;

			case OP_JUMP_IF_TRUE:
				{
					uint16_t offset = READ_2BYTES();
					mutate_vm_ip(OP_JUMP_IF_TRUE, offset);
				}
				break;

			case OP_JUMP:
				{
					uint16_t offset = READ_2BYTES();
					mutate_vm_ip(OP_JUMP, offset);
				}
				break;

			case OP_LOOP:
				{
					uint16_t offset = READ_2BYTES();
					mutate_vm_ip(OP_LOOP, offset);
				}
				break;

			case OP_GET_UPVALUE:
				{
					int index = READ_BYTE();
					ObjectUpvalue* objUpvalue = *(frame->closure->objUpvalues + index);
					push(*(objUpvalue->value));
				}
				break;

			case OP_SET_UPVALUE:
				{
					int index = READ_BYTE();
					ObjectUpvalue* objUpvalue = *(frame->closure->objUpvalues + index);
					*(objUpvalue->value) = peek(0);
				}
				break;

			case OP_CLOSURE:
				{
					ObjectFunction* function = AS_FUNCTION_OBJ(READ_CONSTANT());
					ObjectClosure* closure = makeNewFunctionClosureObject(function);
					push(OBJECT(closure));

					for (int i=0; i<function->upvaluesCount; i++){
						uint8_t instruction = READ_BYTE();
						uint8_t index = READ_BYTE();
						if (instruction == OP_CLOSE_LOCAL){
							closure->objUpvalues[i] = makeNewUpvalueObject((frame->stackStart+index) - vm.stack);
							closure->objUpvalues[i]->value = (frame->stackStart + index);
						} else {
							closure->objUpvalues[i] = frame->closure->objUpvalues[index];
						}

					}
				}
				break;

			case OP_CALL:
				{
					uint8_t nargs = READ_BYTE();
					Value funcVal = peek(nargs);

					if (callNoErrors(nargs, funcVal)){
						switch (AS_OBJ(funcVal)->objectType){
							case OBJECT_NATIVE_FUNCTION:
							{
								ObjectNativeFunction* nativeFn = AS_NATIVE_FUNCTION_OBJ(peek(nargs));
								bool success = nativeFn->nativeFunction();
								if (!success) {
									return RUNTIME_ERROR;
								} 
								Value nativeValue = pop();
								vm.stackpointer -= (nargs +1);
								push(nativeValue);
							}
								break;
							case OBJECT_CLOSURE:
							{
								frame = &(vm.frames[vm.frameCount++]);
								initCallFrame(frame);
								addClosureToCurrentCallFrame(frame, AS_CLOSURE_OBJ(funcVal));
								frame->stackStart = vm.stackpointer - nargs - 1;
							}
								break;
							//Unreachable since every ObjectFunction object is wrapped around ObjectClosure
							case OBJECT_FUNCTION:
								break;
							default:
								break;
						}
					} else return RUNTIME_ERROR;

				}
				break;

			default:
				return COMPILE_ERROR;
		}
	}
	return NO_ERROR;

	#undef BINARY_OPERATION
	#undef READ_BYTE
	#undef READ_2BYTES
	#undef READ_CONSTANT
	#undef BYTES_LEFT_TO_EXECUTE
}

void freeVM(){

	#ifdef RUN_GC_AT_END
	runGarbageCollector();
	#endif

	freeObjects();
	freeTable(&vm.strings);
	freeTable(&vm.globals);
	initVM(true);
}

// Helper functions
bool trueOrFalse(Value val){
	if (val.type == TYPE_NUM) return true;
	else if (val.type == TYPE_OBJ) return true;
	else if (val.type == TYPE_BOOL) return AS_BOOL(val);
	return false;
}

Object* concatenate(){
	ObjectString* b = AS_STRING_OBJ(pop());
	ObjectString* a = AS_STRING_OBJ(pop());

	int length = b->length+a->length;
	char* string = (char*) reallocate(NULL, 0, length);
	memcpy(string, a->string, a->length);
	memcpy(string+a->length, b->string, b->length);

	ObjectString* ptr = makeStringObject(string, length);

	reallocate(string, length, 0);
	return (Object*) ptr;
}

void mutate_vm_ip(uint8_t opcode, uint16_t offset){
	CallFrame* frame = &vm.frames[vm.frameCount-1];
	bool mutate = (opcode == OP_JUMP_IF_FALSE && (trueOrFalse(peek(0)) == false)) ||
			(opcode == OP_JUMP_IF_TRUE && (trueOrFalse(peek(0)) == true));

	switch(opcode){
		case OP_JUMP_IF_TRUE:
		case OP_JUMP_IF_FALSE:
			if (!mutate){
				frame->ip += 2;
				return;
			}
		case OP_JUMP:
			frame->ip += offset;
			break;
		case OP_LOOP:
			frame->ip -= offset;
			break;
	}

}

void closeObjUpvalue(int index){
	if (vm.openObjUpvalues[index] != NULL){
		ObjectUpvalue* upvalue = vm.openObjUpvalues[index];

		upvalue->closedValue = *(upvalue->value);
		upvalue->value = &upvalue->closedValue;

		vm.openObjUpvalues[index] = NULL;
	}
}

void closeObjUpvalues(){
	// This function is called during OP_RETURN execution
	// since the OP_POP_UPVALUE instruction won't execute during function returns since
	// the stackpointer is mutated instead, we will need to emulate as if we are running OP_POP_UPVALUE instructions
	// The idea is just to go through the vm.openObjUpalues array from the frame's starting stack index till the current stackpointer and close the open upvalue if its applicable
	
	Value* currentStackSlot = vm.stackpointer - 1;
	Value* stackStartForFrame = (vm.frames[vm.frameCount-1]).stackStart;
	while (currentStackSlot >= stackStartForFrame){

		closeObjUpvalue(currentStackSlot - stackStartForFrame);
		currentStackSlot--;
	}
}

//Error handling functions
void runtimeError(char* format, ...){

	fprintf(stderr, "Runtime Error:\t");
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	
	for (int frameIndex=vm.frameCount; frameIndex>0; frameIndex--){
		CallFrame* frame = &vm.frames[frameIndex - 1];
		int index = frame->ip - 1 - frame->closure->function->chunk->code;
		int line = frame->closure->function->chunk->lines[index];
		if (frame->closure->function->name == NULL) fprintf(stderr, "line [%d] : in < script >\n", line);
		else fprintf(stderr, "line [%d] : in `%s()`\n", line, frame->closure->function->name->string);
	}

	resetStack();
}

bool callNoErrors(int nargs, Value funcVal){
	int arity;
	switch (funcVal.type){
		case TYPE_OBJ:
		{
			switch (AS_OBJ(funcVal)->objectType){
				case OBJECT_NATIVE_FUNCTION:
					arity = (AS_NATIVE_FUNCTION_OBJ(funcVal))->arity;
				case OBJECT_CLOSURE:{
					arity = (AS_CLOSURE_OBJ(funcVal))->function->arity;
					if (nargs != arity){
						runtimeError("Expected %d arguments, got %d", arity, nargs);
						return false;
					}
					if (vm.frameCount == CALL_FRAMES_MAX) {
						runtimeError("Call stack overflow !!");
						return false;
					}}
				return true;
				default:
					break;
			}}
			break;
		default:
			break;
	}
	runtimeError("Can only call functions and classes");
	return false;
}

// stack based functions 
void push(Value value){
	*(vm.stackpointer++) = value;
}

Value pop(){
	return *(--vm.stackpointer);
}

Value peek(int depth){
	return *(vm.stackpointer - depth - 1);
}

void resetStack(){
	vm.stackpointer = vm.stack;
}

void resetOpenObjUpvalues(){
	for (int i=0; i < STACK_MAX_SIZE; i++){
		vm.openObjUpvalues[i] = NULL;
	}
}

// Functions for native functions in Lox
// DESIGN NOTE: Native functions can assume their 'n' arguments on top of the stack with last argument being at the top
// Native functions musn't pop any values, they should instead use the peek function.
// And, these functions MUST push some final value of top of the stack (if the native function doesn't need to return anything,
// it must still return `nil` LoxValue.
void declareNativeFunctions(){
	declareNativeFunction("clock", 0, clockNativeFunction);
	declareNativeFunction("input", 0, inputNativeFunction);
	declareNativeFunction("number", 1, numberNativeFunction);
}

void declareNativeFunction(char name[], int arity, NativeFunction functionToExecute){
	ObjectString* loxString = makeStringObject(name, strlen(name));
	ObjectNativeFunction* nativeFn = makeNewNativeFunctionObject(loxString, arity, functionToExecute);
	tableAdd(&vm.globals, loxString, OBJECT(nativeFn));
}
bool clockNativeFunction(){
	push(NUMBER(clock()));
	return true;
}

bool inputNativeFunction(){
	// takes user input as string
	int length = 0;
	char input[1024];	

	int c = getchar();
	input[length] = c;
	while (c != '\n'){
		length++;
		input[length] = c = getchar();
	}

	push(OBJECT(makeStringObject(input, length)));
	return true;
}

bool numberNativeFunction(){
	switch(peek(0).type){
		case TYPE_NUM:
			push(peek(0));
			break;
		case TYPE_BOOL:
			{
				double val = 0;
				if (peek(0).as.boolean == true) val = 1;
				push(NUMBER(val));
			}
			break;
		case TYPE_NIL:
			push(NUMBER(0));
			break;
		case TYPE_OBJ:
			{
				double num;
				char* ptr;
				if ((AS_OBJ(peek(0)))->objectType != OBJECT_STRING){
					push(NUMBER(0));
					runtimeError("Cannot convert provided value type to number");
					return false;
				}
				ObjectString* str = AS_STRING_OBJ(peek(0));
				num = strtod(str->string, &ptr);
				push(NUMBER(num));
			}
			break;
	}
	return true;
}

