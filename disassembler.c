#include <stdio.h>
#include "disassembler.h"

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

		default:
			printf("UNKNOWN_OP_CODE\n");
			break;

	}
	return index+1;
}

static void handleConstantInstruction(Chunk* chunk, int index){
	uint8_t offset = *((chunk->code)+index);
	Value value = *(((chunk->constants).values) + offset);
	printf("%lf\n", value);
}
