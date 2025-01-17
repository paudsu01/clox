#ifndef CHUNK_H
#define CHUNK_H

#include "common.h"
#include "value.h"

// Enum
typedef enum {
	OP_RETURN=0,
	OP_CONSTANT
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
