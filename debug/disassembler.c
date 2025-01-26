#include <stdio.h>
#include "disassembler.h"
#include "../vm/vm.h"

static void handleConstantInstruction(Chunk*,int);

void disassembleChunk(Chunk* chunk, char name[]){
	printf("=== %s ===\n", name);
	int index=0;

	while (index < chunk->count){
		index = disassembleInstruction(chunk, index);
	}
	printf("=== END %s ===\n", name);

}


int disassembleInstruction(Chunk* chunk, int index){
	uint8_t instruction_byte = *((chunk->code)+index);
	int linenumber = *((chunk->lines)+index);

	printf("%04d\tLine:%04d\t", index, linenumber);
	switch (instruction_byte){
		case OP_RETURN:
			printf("OP_RETURN\n");
			break;

		case OP_CONSTANT:
			printf("OP_CONSTANT\t");
			index++;
			handleConstantInstruction(chunk, index);
			break;

		case OP_TRUE:
			printf("OP_TRUE\n");
			break;

		case OP_FALSE:
			printf("OP_FALSE\n");
			break;

		case OP_NIL:
			printf("OP_NIL\n");
			break;

		case OP_NEGATE:
			printf("OP_NEGATE\n");
			break;
			
		case OP_ADD:
			printf("OP_ADD\n");
			break;
		
		case OP_SUBTRACT:
			printf("OP_SUBTRACT\n");
			break;

		case OP_MULTIPLY:
			printf("OP_MULTIPLY\n");
			break;

		case OP_DIVIDE:
			printf("OP_DIVIDE\n");
			break;
		default:
			printf("UNKNOWN_OP_CODE\n");
			break;

	}
	return index+1;
}

void disassembleVMStack(){
	extern VM vm;
	printf("VM Stack: [\t");
	int index = 0;
	while ((vm.stack + index) < vm.stackpointer){
		printValue(vm.stack[index++]);
	}
	printf("\t]\n");
}

static void handleConstantInstruction(Chunk* chunk, int index){
	uint8_t offset = *((chunk->code)+index);
	Value value = *(((chunk->constants).values) + offset);
	printValue(value);
}

