#include <stdio.h>
#include <stdlib.h>
#include "../common.h"

#include "compiler.h"
#include "../scanner/scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "../debug/disassembler.h"
#endif

// Variables
Chunk* compilingChunk;

// Function prototypes

static void consumeToken(TokenType, char*);
static void advanceToken();

// Bytecode emitting function prototypes

static void emitByte(uint8_t);
static void emitBytes(uint8_t, uint8_t);
static void endCompiler();
static void emitReturn();
static void emitConstant(Value);
static Chunk* currentChunk();

// Error handling function prototypes
static void errorAtCurrentToken(const char*);
static void errorAtPreviousToken(const char*);
static void error(Token, const char*);

// pratt parsing functions
static ParseRow* getParseRow(TokenType);
static void parseExpression();
static void parsePrecedence(Precedence);
static void parseBinary();
static void parseUnary();
static void parseNumber();
static void parseString();
static void parseLiteral();
static void parseGrouping();

ParseRow rules[] = {
  [TOKEN_LEFT_PAREN]    = {parseGrouping,    NULL,	   PREC_NONE},	
  [TOKEN_RIGHT_PAREN]   = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_LEFT_BRACE]    = {NULL,	     NULL,	   PREC_NONE},	 
  [TOKEN_RIGHT_BRACE]   = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_COMMA]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_DOT]           = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_MINUS]         = {parseUnary,	     parseBinary,  PREC_TERM},	
  [TOKEN_PLUS]          = {NULL,	     parseBinary,  PREC_TERM},	
  [TOKEN_SEMICOLON]     = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_SLASH]         = {NULL,	     parseBinary,  PREC_FACTOR},	
  [TOKEN_STAR]          = {NULL,	     parseBinary,  PREC_FACTOR},	
  [TOKEN_BANG]          = {parseUnary,	     NULL,	   PREC_NONE},	
  [TOKEN_BANG_EQUAL]    = {NULL,	     parseBinary,  PREC_EQUALITY},	
  [TOKEN_EQUAL]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_EQUAL_EQUAL]   = {NULL,	     parseBinary,  PREC_EQUALITY},	
  [TOKEN_GREATER]       = {NULL,	     parseBinary,  PREC_COMPARISON},	
  [TOKEN_GREATER_EQUAL] = {NULL,	     parseBinary,  PREC_COMPARISON},	
  [TOKEN_LESS]          = {NULL,	     parseBinary,  PREC_COMPARISON},	
  [TOKEN_LESS_EQUAL]    = {NULL,	     parseBinary,  PREC_COMPARISON},	
  [TOKEN_IDENTIFIER]    = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_STRING]        = {parseString,	     NULL,	   PREC_NONE},	
  [TOKEN_NUMBER]        = {parseNumber,	     NULL,	   PREC_NONE},	
  [TOKEN_AND]           = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_CLASS]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_ELSE]          = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_FALSE]         = {parseLiteral,     NULL,	   PREC_NONE},	
  [TOKEN_FOR]           = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_FUN]           = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_IF]            = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_NIL]           = {parseLiteral,     NULL,	   PREC_NONE},	
  [TOKEN_OR]            = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_PRINT]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_RETURN]        = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_SUPER]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_THIS]          = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_TRUE]          = {parseLiteral,     NULL,	   PREC_NONE},	
  [TOKEN_VAR]           = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_WHILE]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_ERROR]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_EOF]           = {NULL,	     NULL,	   PREC_NONE},	
};

bool compile(const char* source, Chunk* chunk){
	parser.hadError = false;
	parser.panicMode = false;
	compilingChunk = chunk;

	initScanner(source);
	advanceToken();
	parseExpression();

	consumeToken(TOKEN_EOF, "Expect end of expression");
	endCompiler();
	return !parser.hadError;
}

// PrattParsing functions

static void parseExpression(){
	parsePrecedence(PREC_ASSIGN);
}

static void parseGrouping(){
	parseExpression();
	consumeToken(TOKEN_RIGHT_PAREN, "'(' expected");
}

static void parseNumber(){
	double value = strtod(parser.previousToken.start, NULL);
	emitConstant(NUMBER(value));
}

static void parseString(){
	emitConstant(OBJECT(makeStringObject(parser.previousToken.start+1, parser.previousToken.length-2)));
}

static void parseLiteral(){
	switch (parser.previousToken.type){
		case TOKEN_TRUE:
			emitByte(OP_TRUE);
			break;
		case TOKEN_FALSE:
			emitByte(OP_FALSE);
			break;
		case TOKEN_NIL:
			emitByte(OP_NIL);
			break;
		default:
			// Unreachable
			break;
	}
}

static void parseUnary(){
	TokenType tokenType = parser.previousToken.type;
	parsePrecedence(PREC_UNARY);

	switch(tokenType){
		case TOKEN_MINUS:
			emitByte(OP_NEGATE);
			break;
		case TOKEN_BANG:
			emitByte(OP_NOT);
			break;
		case TOKEN_PLUS:
			// do nothing
			break;
		// unreachable	
		default:
			break;
	}
}

static ParseRow* getParseRow(TokenType type){
	return &(rules[type]);
}

static void parseBinary(){
	TokenType type = parser.previousToken.type;
	ParseRow* parseRow = getParseRow(type);	
	parsePrecedence((Precedence) (parseRow->level+1));

	switch (type){

		case TOKEN_PLUS:
			emitByte(OP_ADD); break;	

		case TOKEN_MINUS:
			emitByte(OP_SUBTRACT); break;	

		case TOKEN_STAR:
			emitByte(OP_MULTIPLY); break;	

		case TOKEN_SLASH:
			emitByte(OP_DIVIDE); break;	

		case TOKEN_EQUAL_EQUAL:
			emitByte(OP_EQUAL); break;	

		case TOKEN_BANG_EQUAL:
			emitBytes(OP_EQUAL, OP_NOT); break;	

		case TOKEN_LESS:
			emitByte(OP_LT); break;	

		case TOKEN_LESS_EQUAL:
			emitBytes(OP_GT, OP_NOT); break;	

		case TOKEN_GREATER:
			emitByte(OP_GT); break;	

		case TOKEN_GREATER_EQUAL:
			emitBytes(OP_LT, OP_NOT); break;	

		default:
			break;
	};
}

static void parsePrecedence(Precedence precedence){
	advanceToken();
	Token token = parser.previousToken;
	parseFn prefix = (getParseRow(token.type))->prefixFunction;
	(*prefix)();

	while (getParseRow(parser.currentToken.type)->level >= precedence){
		advanceToken();
		parseFn infix = (getParseRow(parser.previousToken.type))->infixFunction;
		(*infix)();
	}

}
// Token handling functions

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

static void emitConstant(Value value){
	emitByte(OP_CONSTANT);
	int index = addConstant(currentChunk(), value);
	if (index > UINT8_MAX){
		// error
		errorAtPreviousToken("Too many constants in one chunk");
		index=0;
	}
	emitByte((uint8_t) index);
}

static void endCompiler(){
	emitReturn();

	#ifdef DEBUG_PRINT_CODE
	if (!parser.hadError) disassembleChunk(currentChunk(), "Compiled code");
	#endif
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
