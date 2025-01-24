#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "../vm/chunk.h"

// function prototypes
void disassembleChunk(Chunk*, char[]);
int disassembleInstruction(Chunk*, int);
void disassembleVMStack();

#endif
