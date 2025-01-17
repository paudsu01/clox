#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "chunk.h"

// function prototypes
void disassembleChunk(Chunk*, char[]);
int disassembleInstruction(Chunk*, int);
static void handleConstantInstruction(Chunk*,int);


#endif
