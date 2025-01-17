#ifndef CHUNK_H
#define CHUNK_H

#include "common.h"

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
} Chunk;

// function prototypes
void initChunk(Chunk*);
void freeChunk(Chunk*);
void addCode(Chunk*, uint8_t);

#endif
