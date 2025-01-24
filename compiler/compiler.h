#ifndef COMPILER_H
#define COMPILER_H

#include "../vm/vm.h"
#include "../scanner/token.h"

bool compile(const char*, Chunk*);

typedef struct{
	Token currentToken;
	Token previousToken;
	bool hadError;
	bool panicMode;
} Parser;

Parser parser;

#endif
