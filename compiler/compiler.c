#include <stdio.h>
#include "../common.h"

#include "compiler.h"
#include "../vm/vm.h"
#include "../scanner/scanner.h"

// Variables
Chunk* compilingChunk;

// Function prototypes

static void parseExpression();
static void consumeToken(TokenType, char*);
static void advanceToken();

// Bytecode emitting function prototypes

static void emitByte(uint8_t);
static void emitBytes(uint8_t, uint8_t);
static void endCompiler();
static void emitReturn();
static Chunk* currentChunk();

// Error handling function prototypes
static void errorAtCurrentToken(const char*);
static void errorAtPreviousToken(const char*);
static void error(Token, const char*);

// pratt parsing functions
static void binary();
static void unary();
static void number();
static void grouping();

ParseRow rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

bool compile(const char* source, Chunk* chunk){
	parser.hadError = false;
	parser.panicMode = false;
	compilingChunk = chunk;

	initScanner(source);
	advanceToken();
	parseExpression();

	consumeToken(TOKEN_EOF, "Expect end of expression");
	return !parser.hadError;
}

static void parseExpression(){

}

static void advanceToken(){
	parser.previousToken = parser.currentToken;
	while (true){
		parser.currentToken = scanToken();
		if (parser.currentToken.type != TOKEN_ERROR) break;
		errorAtCurrentToken(parser.currentToken.start);
	}
	
}

static void consumeToken(TokenType type, char * message){
	if (parser.currentToken.type != type) {
		errorAtCurrentToken(message);
	} else{
		advanceToken();
	}
}


// Bytecode emitting function prototypes

static void emitByte(uint8_t byte){
	addCode(currentChunk(), byte, parser.previousToken.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2){
	emitByte(byte1);
	emitByte(byte2);

}

static void endCompiler(){
	emitReturn();
}

static void emitReturn(){
	emitByte(OP_RETURN);

}

static Chunk* currentChunk(){
	return compilingChunk;
}
// Error handling functions

static void errorAtCurrentToken(const char* message){
	error(parser.currentToken, message);
}

static void errorAtPreviousToken(const char*message){
	error(parser.previousToken, message);
}


static void error(Token token, const char* message){
	if (parser.panicMode) return;

	parser.hadError = true;
	parser.panicMode = true;

	fprintf(stderr, "Line [%d]: ", token.line);
	if (token.type == TOKEN_EOF) {
		fprintf(stderr, " At end");
	}
	else if (token.type != TOKEN_ERROR){
		fprintf(stderr, " At '%.*s'", token.length, token.start);
	}

	fprintf(stderr, ": %s\n", message);

}
