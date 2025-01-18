#include "vm.h"
#include "stdio.h"

VM vm;

void initVM(){
	vm.chunk = NULL;
	vm.ip = NULL;
	resetStack();
}

InterpreterResult interpret(Chunk* chunk){

	#define READ_BYTE() *(vm.ip++)
	#define READ_CONSTANT() (vm.chunk->constants).values[READ_BYTE()]

	vm.chunk = chunk;
	vm.ip = (vm.chunk)->code;
	
	Value value;
	for (int instruction_index=0; instruction_index < chunk->count; instruction_index++){
		
		uint8_t byte = READ_BYTE();
		switch (byte){
			case OP_RETURN:
				break;
			case OP_CONSTANT:
				value = READ_CONSTANT();
				printf("%lf\n", value);
				instruction_index++;
				break;
		}
	}
	return NO_ERROR;

	#undef READ_BYTE
	#undef READ_CONSTANT
}

void freeVM(){
	initVM();
}


// stack based functions : TODO
void push(Value value){

}

Value pop(){
	return 0;
}

void resetStack(){
	vm.stackpointer = vm.stack;
}

