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

typedef enum{
	PREC_NONE=0,
	PREC_ASSIGN,
	PREC_OR,
	PREC_AND,
	PREC_EQUALITY,
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY,
} Precedence;

// function pointer typdef
typedef void (*parseFn)(bool);

typedef struct{
	parseFn prefixFunction;	
	parseFn infixFunction;	
	Precedence level;
} ParseRow;

#endif
