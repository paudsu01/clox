#include "vm.h"
#include "memory.h"
#include "object.h"
#include "../compiler/compiler.h"
#include "../debug/disassembler.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

VM vm;

void initVM(){
	vm.objects = NULL;
	vm.frameCount = 0;
	initTable(&vm.strings);
	initTable(&vm.globals);
	resetStack();
	declareNativeFunctions();
}

void initCallFrame(CallFrame* frame){
	frame->ip = NULL;
	frame->stackStart = vm.stackpointer;
	frame->function = NULL;
}

void addFunctionToCurrentCallFrame(CallFrame* frame, ObjectFunction* function){
	frame->function = function;
	frame->ip = function->chunk->code;
}

InterpreterResult interpret(const char* source){

	ObjectFunction* currentFunction = compile(source);

	if (currentFunction == NULL){
		return COMPILE_ERROR;
	}

	else {

		CallFrame* frame = &(vm.frames[vm.frameCount]);
		initCallFrame(frame);
		addFunctionToCurrentCallFrame(frame, currentFunction);
		push(OBJECT(currentFunction));

		return runVM();
	}
}

InterpreterResult runVM(){
	
	CallFrame* frame = &vm.frames[vm.frameCount++];

	#define READ_BYTE() *(frame->ip++)
	#define READ_CONSTANT() (frame->function->chunk->constants).values[READ_BYTE()]
	#define READ_2BYTES() ((uint16_t) (*frame->ip << 8)) + *(frame->ip+1)
		

	#define BYTES_LEFT_TO_EXECUTE() (frame->ip < (frame->function->chunk->code + frame->function->chunk->count))

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
			disassembleInstruction(frame->function->chunk, (int) ((frame->ip) - (frame->function->chunk->code)));
		#endif

		uint8_t byte = READ_BYTE();
		switch (byte){
			case OP_RETURN:
				{
					Value returnValue = pop();
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

			case OP_CALL:
				{
					uint8_t nargs = READ_BYTE();
					Value funcVal = peek(nargs);

					if (!(IS_FUNCTION(funcVal))){
						runtimeError("Can only call functions and classes");
						return RUNTIME_ERROR;
					}

					ObjectFunction* funcObject = AS_FUNCTION_OBJ(funcVal);
					if (nargs != funcObject->arity){
						runtimeError("Expected %d arguments, got %d", funcObject->arity, nargs);
						return RUNTIME_ERROR;
					}

					frame = &(vm.frames[vm.frameCount++]);
					if (vm.frameCount == CALL_FRAMES_MAX) {
						runtimeError("Call stack overflow !!");
						return RUNTIME_ERROR;
					}

					initCallFrame(frame);
					addFunctionToCurrentCallFrame(frame, funcObject);
					frame->stackStart = vm.stackpointer - nargs - 1;
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
	freeObjects();
	freeTable(&vm.strings);
	freeTable(&vm.globals);
	initVM();
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
		int index = frame->ip - 1 - frame->function->chunk->code;
		int line = frame->function->chunk->lines[index];
		if (frame->function->name == NULL) fprintf(stderr, "line [%d] : in < script >\n", line);
		else fprintf(stderr, "line [%d] : in `%s()`\n", line, frame->function->name->string);
	}

	resetStack();
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

// Functions for native functions in Lox
void declareNativeFunctions(){
	ObjectString* clockLoxString = makeStringObject("clock", 5);
	ObjectNativeFunction* clockFn = makeNewNativeFunctionObject(clockLoxString, 0, clockNativeFunction);
	tableAdd(&vm.globals, clockLoxString, OBJECT(clockFn));
}

void clockNativeFunction(){
	push(NUMBER(2049));
}

