#include "vm.h"
#include "../compiler/compiler.h"
#include "../debug/disassembler.h"

#include <stdio.h>

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

	#define BINARY_OP(op) \
			do { Value b = pop(); Value a=pop(); \
			  push(a op b); \
			} while (false)
		

	Value value;
	while (BYTES_LEFT_TO_EXECUTE()){
		
		#ifdef DEBUG_TRACE_EXECUTION
			disassembleVMStack();
			disassembleInstruction(vm.chunk, (int) ((vm.ip) - (vm.chunk->code)));
		#endif

		uint8_t byte = READ_BYTE();
		switch (byte){
			case OP_RETURN:
				printf("%lf\n", pop());
				break;

			case OP_CONSTANT:
				value = READ_CONSTANT();
				push(value);
				break;

			case OP_NEGATE:
				push(-pop());
				break;

			case OP_ADD:
				BINARY_OP(+);
				break;
			
			case OP_SUBTRACT:
				BINARY_OP(-);
				break;

			case OP_MULTIPLY:
				BINARY_OP(*);
				break;

			case OP_DIVIDE:
				BINARY_OP(/);
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


// stack based functions 
void push(Value value){
	*(vm.stackpointer++) = value;
}

Value pop(){
	return *(--vm.stackpointer);
}

void resetStack(){
	vm.stackpointer = vm.stack;
}

