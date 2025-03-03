#include <stdio.h>
#include "disassembler.h"
#include "../vm/vm.h"

static void handleConstantInstruction(Chunk*,int);
static void handleByteInstruction(Chunk*,int);

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
			handleConstantInstruction(chunk, ++index);
			break;

		case OP_DEFINE_GLOBAL:
			printf("OP_DEFINE_GLOBAL\t");
			handleConstantInstruction(chunk, ++index);
			break;

		case OP_GET_GLOBAL:
			printf("OP_GET_GLOBAL\t");
			handleConstantInstruction(chunk, ++index);
			break;

		case OP_SET_GLOBAL:
			printf("OP_SET_GLOBAL\t");
			handleConstantInstruction(chunk, ++index);
			break;

		case OP_GET_LOCAL:
			printf("OP_GET_LOCAL\t");
			handleByteInstruction(chunk, ++index);
			break;

		case OP_SET_LOCAL:
			printf("OP_SET_LOCAL\t");
			handleByteInstruction(chunk, ++index);
			break;

		case OP_NOT:
			printf("OP_NOT\n");
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

		case OP_GT:
			printf("OP_GT\n");
			break;

		case OP_LT:
			printf("OP_LT\n");
			break;

		case OP_EQUAL:
			printf("OP_EQUAL\n");
			break;

		case OP_PRINT:
			printf("OP_PRINT\n");
			break;

		case OP_POP:
			printf("OP_POP\n");
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
		printf("\t");
	}
	printf("\t]\n");
}

static void handleConstantInstruction(Chunk* chunk, int index){
	uint8_t offset = *((chunk->code)+index);
	Value value = *(((chunk->constants).values) + offset);
	printValue(value);
	printf("\n");
}

static void handleByteInstruction(Chunk* chunk, int index){
	uint8_t slot = *((chunk->code)+index);
	printf("%4d\n", slot);
}
