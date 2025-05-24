#ifndef CHUNK_H
#define CHUNK_H

#include "../common.h"
#include "value.h"

// Enum
typedef enum {
	OP_RETURN=0,
	OP_CONSTANT,
	OP_NEGATE,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_TRUE,
	OP_FALSE,
	OP_NIL,
	OP_NOT,
	OP_EQUAL,
	OP_GT,
	OP_LT,
	OP_POP,
	OP_POP_UPVALUE,
	OP_PRINT,
	OP_DEFINE_GLOBAL,
	OP_GET_GLOBAL,
	OP_SET_GLOBAL,
	OP_GET_LOCAL,
	OP_SET_LOCAL,
	OP_GET_UPVALUE,
	OP_SET_UPVALUE,
	OP_JUMP_IF_FALSE,
	OP_JUMP_IF_TRUE,
	OP_JUMP,
	OP_LOOP,
	OP_CALL,
	OP_CLOSURE,
	OP_CLOSE_LOCAL,
	OP_CLOSE_UPVALUE,
	OP_CLASS,
	OP_GET_PROPERTY,
	OP_SET_PROPERTY,
	OP_METHOD,
	OP_FAST_METHOD_CALL,
	OP_INHERIT_SUPERCLASS,
	OP_STACK_SWAP,
	OP_GET_SUPER,
	OP_FAST_SUPER_METHOD_CALL,
} OPCode;

// Struct
typedef struct Chunk{
	int capacity;
	int count;
	uint8_t *code;		
	int *lines;		
	ValueArray constants;
} Chunk;

// function prototypes
void initChunk(Chunk*);
void freeChunk(Chunk*);
void addCode(Chunk*, uint8_t, int);
int addConstant(Chunk*, Value);

#endif
