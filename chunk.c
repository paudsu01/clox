#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk){
	chunk->capacity=0;	
	chunk->count=0;	
	chunk->code = NULL;
}

void addCode(Chunk* chunk, uint8_t byte){
	//check if size is full
	if (chunk->count == chunk->capacity){
		// double size if full
		int old_capacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(chunk->capacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);

	}

	*((chunk->code) + chunk->count) = byte;
	(chunk->count)++;
}


void freeChunk(Chunk* chunk){

	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	initChunk(chunk);
}

