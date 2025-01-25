#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk){
	chunk->capacity=0;	
	chunk->count=0;	
	chunk->code = NULL;
	chunk->lines = NULL;
	initValueArray(&(chunk->constants));
}

void addCode(Chunk* chunk, uint8_t byte, int line){
	//check if size is full
	if (chunk->count == chunk->capacity){
		// double size if full
		int old_capacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(chunk->capacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
		chunk->lines = GROW_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);

	}

	*((chunk->code) + chunk->count) = byte;
	*((chunk->lines) + chunk->count) = line;
	(chunk->count)++;
}

int addConstant(Chunk* chunk, Value constant){
	appendValue(&(chunk->constants), constant);
	return (chunk->constants).count - 1;
}

void freeChunk(Chunk* chunk){

	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	FREE_ARRAY(int, chunk->lines, chunk->capacity);
	freeValueArray(&(chunk->constants));
	initChunk(chunk);
}

