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
	#define BYTES_LEFT_TO_EXECUTE() (vm.ip <= (vm.chunk->code + vm.chunk->count))

	vm.chunk = chunk;
	vm.ip = (vm.chunk)->code;
	
	Value value;
	while (BYTES_LEFT_TO_EXECUTE()){
		
		uint8_t byte = READ_BYTE();
		switch (byte){
			case OP_RETURN:
				break;
			case OP_CONSTANT:
				value = READ_CONSTANT();
				printf("%lf\n", value);
				break;
		}
	}
	return NO_ERROR;

	#undef READ_BYTE
	#undef READ_CONSTANT
	#undef BYTES_LEFT_TO_EXECUTE
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

