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
