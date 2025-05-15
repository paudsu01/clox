#include <stdio.h>
#include "disassembler.h"
#include "../vm/vm.h"

static void handleConstantInstruction(Chunk*,int);
static void handleByteInstruction(Chunk*,int);
static void handleJumpInstruction(uint8_t, Chunk*,int);
static int handleClosureUpvalues(Chunk*,int,int);

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

		case OP_POP_UPVALUE:
			printf("OP_POP_UPVALUE\n");
			break;

		// OP_METHOD will take the OP_CLOSURE from the top of the stack and add it to the class which can be retrieved by peek(1)
		case OP_METHOD:
			printf("OP_METHOD\n");
			break;

		case OP_JUMP:
			printf("OP_JUMP\t");
			handleJumpInstruction(OP_JUMP, chunk, index = index + 2);
			break;

		case OP_JUMP_IF_FALSE:
			printf("OP_JUMP_IF_FALSE\t");
			handleJumpInstruction(OP_JUMP_IF_FALSE, chunk, index = index + 2);
			break;

		case OP_JUMP_IF_TRUE:
			printf("OP_JUMP_IF_TRUE\t");
			handleJumpInstruction(OP_JUMP_IF_TRUE, chunk, index = index + 2);
		
		case OP_LOOP:
			printf("OP_LOOP\t");
			handleJumpInstruction(OP_LOOP, chunk, index = index + 2);
			break;

		case OP_CALL:
			printf("OP_CALL: %d args\n", *((chunk->code)+(++index)));
			break;

		case OP_CLOSURE:
			{
				printf("OP_CLOSURE\n|\t");
				handleConstantInstruction(chunk, ++index);
				Value value = *(((chunk->constants).values) + *((chunk->code)+index));
				index = handleClosureUpvalues(chunk, ++index, (AS_FUNCTION_OBJ(value))->upvaluesCount);
			}
			break;

		case OP_GET_UPVALUE:
			printf("OP_GET_UPVALUE\t");
			handleByteInstruction(chunk, ++index);
			break;
		case OP_SET_UPVALUE:
			printf("OP_SET_UPVALUE\t");
			handleByteInstruction(chunk, ++index);
			break;

		case OP_CLASS:
			printf("OP_CLASS\t");
			handleConstantInstruction(chunk, ++index);
			break;

		case OP_GET_PROPERTY:
			printf("OP_GET_PROPERTY\t");
			handleConstantInstruction(chunk, ++index);
			break;

		case OP_SET_PROPERTY:
			printf("OP_SET_PROPERTY\t");
			handleConstantInstruction(chunk, ++index);
			break;

		case OP_FAST_METHOD_CALL:
			printf("OP_FAST_METHOD_CALL\t");
			handleConstantInstruction(chunk, ++index);
			handleByteInstruction(chunk, ++index);
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

static void handleJumpInstruction(uint8_t opcode, Chunk* chunk, int index){
	uint16_t jump = (uint16_t) *(chunk->code + (index-1));
	jump = (jump << 8) + *(chunk->code + (index));
	printf("%5d -->\t", jump);

	index = (opcode == OP_LOOP) ? index-jump-1 : index+jump-1;
	disassembleInstruction(chunk, index);
}

static int handleClosureUpvalues(Chunk* chunk, int currentIndex, int upvalueCount){
	for (int i=0; i< upvalueCount; i++){

		uint8_t localOrUpvalue = *(chunk->code + currentIndex++);
		uint8_t index = *(chunk->code + currentIndex++);

		if (localOrUpvalue == OP_CLOSE_LOCAL){
			printf("|\tlocal\t%d\n", index);
		} else if (localOrUpvalue == OP_CLOSE_UPVALUE){
			printf("|\tupvalue\t%d\n", index);
		} 
	}
	return --currentIndex;
}
