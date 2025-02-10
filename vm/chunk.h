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
	OP_PRINT,
	OP_DEFINE_GLOBAL,
	OP_GET_GLOBAL,
	OP_SET_GLOBAL,
} OPCode;

// Struct
typedef struct{
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
