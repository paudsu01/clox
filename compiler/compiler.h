#ifndef COMPILER_H
#define COMPILER_H

#include "../vm/vm.h"
#include "../scanner/token.h"


ObjectFunction* compile(const char*);

typedef struct{
	Token currentToken;
	Token previousToken;
	bool hadError;
	bool panicMode;
} Parser;

typedef struct{
	Token name;
	int depth;
} Local;

typedef struct{
	Local locals[UINT8_T_LIMIT+1];
	int currentScopeDepth;
	int currentLocalsCount;
	ObjectFunction* function;
	FunctionType type;
} Compiler;

void initCompiler(Compiler*, FunctionType);

Parser parser;
Compiler* currentCompiler;

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
