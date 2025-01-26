#include "vm.h"
#include "../compiler/compiler.h"
#include "../debug/disassembler.h"

#include <stdio.h>
#include <stdarg.h>

VM vm;

void initVM(){
	vm.chunk = NULL;
	vm.ip = NULL;
	resetStack();
}

InterpreterResult interpret(const char* source){

	Chunk chunk;
	initChunk(&chunk);

	if (!compile(source, &chunk)){
		freeChunk(&chunk);
		return COMPILE_ERROR;
	}

	else {
		vm.chunk = &chunk;
		vm.ip = vm.chunk->code;

		InterpreterResult result = runVM();

		freeChunk(&chunk);
		return result;
	}
}

InterpreterResult runVM(){
	
	#define READ_BYTE() *(vm.ip++)
	#define READ_CONSTANT() (vm.chunk->constants).values[READ_BYTE()]

	#define BYTES_LEFT_TO_EXECUTE() (vm.ip < (vm.chunk->code + vm.chunk->count))

	#define BINARY_OP(resultValue, op) \
			do { 	Value b = peek(0); Value a=peek(1); \
				if (IS_NUM(b) && IS_NUM(a)){ \
			 		double d = AS_NUM(pop()); double c=AS_NUM(pop()); \
			  		push(resultValue(c op d)); \
				} \
				else{ \
					runtimeError("Operands must be numbers");\
					return RUNTIME_ERROR;\
				} \
			} while (false) \
		

	Value value;
	while (BYTES_LEFT_TO_EXECUTE()){
		
		#ifdef DEBUG_TRACE_EXECUTION
			disassembleVMStack();
			disassembleInstruction(vm.chunk, (int) ((vm.ip) - (vm.chunk->code)));
		#endif

		uint8_t byte = READ_BYTE();
		switch (byte){
			case OP_RETURN:
				printValue(pop());
				printf("\n");
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
				BINARY_OP(NUMBER, +);
				break;
			
			case OP_SUBTRACT:
				BINARY_OP(NUMBER, -);
				break;

			case OP_MULTIPLY:
				BINARY_OP(NUMBER, *);
				break;

			case OP_DIVIDE:
				BINARY_OP(NUMBER, /);
				break;

			case OP_GT:
				BINARY_OP(BOOLEAN, >);
				break;

			case OP_LT:
				BINARY_OP(BOOLEAN, <);
				break;

			case OP_EQUAL:
				push(BOOLEAN(checkIfValuesEqual(pop(), pop())));
				break;

			default:
				return COMPILE_ERROR;
		}
	}
	return NO_ERROR;

	#undef BINARY_OPERATION
	#undef READ_BYTE
	#undef READ_CONSTANT
	#undef BYTES_LEFT_TO_EXECUTE
}

void freeVM(){
	initVM();
}

// Helper functions
bool trueOrFalse(Value val){
	if (val.type == TYPE_NUM) return true;
	else if (val.type == TYPE_BOOL) return AS_BOOL(val);
	return false;
}

//Error handling functions
void runtimeError(char* format, ...){
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fprintf(stderr, "\n");

	int index = vm.ip - 1 - vm.chunk->code;
	int line = vm.chunk->lines[index];
	fprintf(stderr, "line [%d] in script\n", line);
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

