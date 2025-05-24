#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common.h"

#include "compiler.h"
#include "../scanner/scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "../debug/disassembler.h"
#endif

// Function prototypes

// Token-handling functions
static void consumeToken(TokenType, char*);
static bool matchToken(TokenType);
static bool checkToken(TokenType);
static void advanceToken();

// Helper functions
static int parseGlobalVariable();
static void handleLocalVariable();
static void addSuperAsLocalVariable();
static void beginScope();
static void endScope();
static Chunk* currentChunk();
static int getLocalDepth(Compiler*, Token);
static int getUpvalueDepth(Compiler*, Token);
static int addUpvalue(Compiler*, int, bool);
static bool noDuplicateVarInCurrentScope();
static bool identifiersEqual(Token*,Token*);
static void markInitialized();
static int getValueConstantIndex(Value);

// Bytecode emitting function prototypes

static void emitByte(uint8_t);
static void emitBytes(uint8_t, uint8_t);
static int emitJump(uint8_t);
static void patchJump(int, uint8_t);
static ObjectFunction* endCompiler();
static void emitReturn(bool,Token);
static void emitConstant(Value);
static uint8_t addConstantAndCheckLimit(Value value);

// Error handling function prototypes
static void errorAtCurrentToken(const char*);
static void errorAtPreviousToken(const char*);
static void error(Token, const char*);
static void synchronize();

// parsing functions
static void parseDeclaration();
static void parseVarDeclaration();
static void parseFuncDeclaration();
static void parseFunction(FunctionType);
static void parseClassDeclaration();
static int parseParameters();
static void parseStatement();
static void parsePrintStatement();
static void parseIfStatement();
static void parseWhileStatement();
static void parseForStatement();
static void parseReturnStatement();
static void parseExpressionStatement();
static void parseBlockStatement();

// pratt parsing functions
static ParseRow* getParseRow(TokenType);
static void parseExpression();
static void parsePrecedence(Precedence);
static void parseBinary(bool);
static void parseDot(bool);
static void parseAnd(bool);
static void parseOr(bool);
static void parseUnary(bool);
static void parseNumber(bool);
static void parseString(bool);
static void parseLiteral(bool);
static void parseSuper(bool);
static void parseThis(bool);
static void parseIdentifier(bool);
static void parseGrouping(bool);
static int parseArguments();
static void parseFuncCall(bool);

ParseRow rules[] = {
  [TOKEN_LEFT_PAREN]    = {parseGrouping,    parseFuncCall,PREC_CALL},	
  [TOKEN_RIGHT_PAREN]   = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_LEFT_BRACE]    = {NULL,	     NULL,	   PREC_NONE},	 
  [TOKEN_RIGHT_BRACE]   = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_COMMA]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_DOT]           = {NULL,	     parseDot,	   PREC_CALL},	
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
  [TOKEN_IDENTIFIER]    = {parseIdentifier,  NULL,	   PREC_NONE},	
  [TOKEN_STRING]        = {parseString,	     NULL,	   PREC_NONE},	
  [TOKEN_NUMBER]        = {parseNumber,	     NULL,	   PREC_NONE},	
  [TOKEN_AND]           = {NULL,	     parseAnd,	   PREC_AND},	
  [TOKEN_CLASS]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_ELSE]          = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_FALSE]         = {parseLiteral,     NULL,	   PREC_NONE},	
  [TOKEN_FOR]           = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_FUN]           = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_IF]            = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_NIL]           = {parseLiteral,     NULL,	   PREC_NONE},	
  [TOKEN_OR]            = {NULL,	     parseOr,	   PREC_OR},	
  [TOKEN_PRINT]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_RETURN]        = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_SUPER]         = {parseSuper,	     NULL,	   PREC_NONE},	
  [TOKEN_THIS]          = {parseThis,	     NULL,	   PREC_NONE},	
  [TOKEN_TRUE]          = {parseLiteral,     NULL,	   PREC_NONE},	
  [TOKEN_VAR]           = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_WHILE]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_ERROR]         = {NULL,	     NULL,	   PREC_NONE},	
  [TOKEN_EOF]           = {NULL,	     NULL,	   PREC_NONE},	
};

void initCompiler(Compiler* compiler, FunctionType type){
	compiler->parentCompiler = currentCompiler;
	compiler->currentScopeDepth = 0;
	compiler->currentLocalsCount = 0;
	compiler->currentUpvaluesCount = 0;

	compiler->type = type;
	compiler->function = makeNewFunctionObject(type);

	// Assign first slot to the current function
	Local* local = &compiler->locals[compiler->currentLocalsCount++];
	local->depth = 0;
	local->isCaptured = false;
	local->name.start = "";
	local->name.length = 0;

	if (type != FUNCTION_MAIN){
		compiler->function->name = makeStringObject(parser.previousToken.start, parser.previousToken.length);
		if (type == METHOD || type == METHOD_INIT){
			// Store `this` as the first "hidden" local variable for methods
			local->name.start = "this";
			local->name.length = 4;
		} else{
			local->name.start = compiler->function->name->string;
			local->name.length = compiler->function->name->length;
		}
	} else{
		currentCompilingClass = NULL;
	}

	currentCompiler = compiler;
}

ObjectFunction* compile(const char* source){
	parser.hadError = false;
	parser.panicMode = false;
	Compiler compiler;
	compiler.parentCompiler = NULL;

	initScanner(source);
	initCompiler(&compiler, FUNCTION_MAIN);

	advanceToken();

	while (!matchToken(TOKEN_EOF)){
		parseDeclaration();
	}

	ObjectFunction* objFunction = endCompiler();
	return (parser.hadError) ? NULL : objFunction;
}

// parsing declaration/statements

// declaration -> classDecl | varDecl | statement | funcDecl ;
static void parseDeclaration(){
	if (matchToken(TOKEN_VAR)) parseVarDeclaration();
	else if (matchToken(TOKEN_FUN)) parseFuncDeclaration();
	else if (matchToken(TOKEN_CLASS)) parseClassDeclaration();
	else parseStatement();

	if (parser.panicMode){
		synchronize();
	}
}

// varDecl -> "var" IDENTIFIER ("=" expression)? ";"
static void parseVarDeclaration(){
	// make Lox string with current token and add it to chunk's constant table
	consumeToken(TOKEN_IDENTIFIER, "Expect variable name");

	uint8_t index;
	if (currentCompiler->currentScopeDepth == 0){
		// Global variable
		index = parseGlobalVariable();

	} else {
		// Local variable handling
		handleLocalVariable();
	}
	
	if (matchToken(TOKEN_EQUAL)){
		parseExpression();
	} else{
		emitByte(OP_NIL);
	}

	consumeToken(TOKEN_SEMICOLON, "Expected ';' after end of var declaration");

	// emit byte to add it to global hash table
	if (currentCompiler->currentScopeDepth == 0) emitBytes(OP_DEFINE_GLOBAL, index);
	else markInitialized();
}

// Parses the function's body and emits OP_CLOSURE instructions to create the function closure at runtime
static void parseFunction(FunctionType type){
	Compiler newCompiler;
	initCompiler(&newCompiler, type);

	// There is only beginScope() called and no endScope() because the return instruction takes care of "popping" the local variables from the stack as well as the upvalues
	beginScope();
	//handleArguments
	int nargs = parseParameters();
	consumeToken(TOKEN_LEFT_BRACE, "Expected block body");

	beginScope();
	parseBlockStatement();

	// push the function onto the stack
	ObjectFunction* function = endCompiler();
	function->arity = nargs;
	function->upvaluesCount = newCompiler.currentUpvaluesCount;

	emitByte(OP_CLOSURE);
	int funcIndex = addConstantAndCheckLimit(OBJECT(function));
	emitByte(funcIndex);

	for (int i=0; i< newCompiler.currentUpvaluesCount; i++){
		Upvalue upvalue = newCompiler.upvalues[i];
		emitBytes((upvalue.isLocal) ? OP_CLOSE_LOCAL: OP_CLOSE_UPVALUE, upvalue.index);
	}
}


// funDec -> "fun" IDENTIFIER "(" IDENTIFIER ? ("," IDENTIFIER)* ")" block
static void parseFuncDeclaration(){
	consumeToken(TOKEN_IDENTIFIER, "Expect variable name");
	uint8_t index;

	if (currentCompiler->currentScopeDepth == 0){
		// Global variable
		index = parseGlobalVariable();

	} else {
		// Local variable handling
		handleLocalVariable();
		markInitialized();
	}

	parseFunction(FUNCTION);	
	// emit byte to add it to global hash table if it is a global variable
	if (currentCompiler->currentScopeDepth == 0) emitBytes(OP_DEFINE_GLOBAL, index);
}


static void parseClassDeclaration(){
	consumeToken(TOKEN_IDENTIFIER, "Expect class name");
	Token classToken = parser.previousToken;
	uint8_t index;

	// Add name LoxString object to the constant table no matter what( even if it is in local context )
	index = parseGlobalVariable();
	if (currentCompiler->currentScopeDepth != 0){
		// Local variable handling
		handleLocalVariable();
		markInitialized();
	}

	emitBytes(OP_CLASS, index);

	// Define a new compiling class
	CompilingClass newCompilingClass;
	newCompilingClass.parent = currentCompilingClass;
	newCompilingClass.hasSuperClass = false;
	currentCompilingClass = &newCompilingClass;

	if (currentCompiler->currentScopeDepth == 0) emitBytes(OP_DEFINE_GLOBAL, index);

	// push the class object on the top again since we will need it to bind the methods to the class
	parseIdentifier(false);
	if (matchToken(TOKEN_LESS)){

		newCompilingClass.hasSuperClass = true;
		consumeToken(TOKEN_IDENTIFIER, "Expect superclass name");

		// begin scope to include `super` as a hidden local variable so that class methods can capture `super` as a upvalue
		beginScope();
		addSuperAsLocalVariable();

		// push the superclass object on the top of the stack
		parseIdentifier(false);

		if (parser.previousToken.length == classToken.length 
				&& strncmp(parser.previousToken.start, classToken.start, parser.previousToken.length) == 0)
			errorAtPreviousToken("A class cannot inherit from itself");

		emitByte(OP_INHERIT_SUPERCLASS);

		//swap the superclass and the class values so that class value stays on top
		emitByte(OP_STACK_SWAP);
	}
	

	consumeToken(TOKEN_LEFT_BRACE, "Expect class body");
	// parse the method declarations
	while (!checkToken(TOKEN_EOF) && checkToken(TOKEN_IDENTIFIER)){
		consumeToken(TOKEN_IDENTIFIER, "Expected method name");

		if(parser.previousToken.length == 4 && (strncmp(parser.previousToken.start, "init", 4) == 0))
		{
			parseFunction(METHOD_INIT);
		}
		else
			parseFunction(METHOD);

		emitByte(OP_METHOD);
	}

	// pop class object from top once method binding is done
	emitByte(OP_POP);
	// remove upvalue if class has a superclass
	if (newCompilingClass.hasSuperClass){
		endScope();
	}

	// Change the current compiling class back to the parent
	currentCompilingClass = newCompilingClass.parent;
	consumeToken(TOKEN_RIGHT_BRACE, "'}' expected at end of class body");
}

// param -> "(" IDENTIFIER ? ("," IDENTIFIER )* ")"
static int parseParameters(){

	consumeToken(TOKEN_LEFT_PAREN, "'(' expected for function declaration");
	if (matchToken(TOKEN_RIGHT_PAREN)) return 0;

	int nargs = 0;
	do{
		consumeToken(TOKEN_IDENTIFIER, "Parameter name expected");
		nargs++;
		currentCompiler->locals[currentCompiler->currentLocalsCount++] = (Local) {.depth = currentCompiler->currentScopeDepth, .name=parser.previousToken};

		if (nargs > UINT8_T_LIMIT) errorAtPreviousToken("Cannot have more than 255 arguments");
	}
	while (matchToken(TOKEN_COMMA));
	
	consumeToken(TOKEN_RIGHT_PAREN, "')' expected at end of function parameters");
	return nargs;
}

// statement -> printStatement | block | exprStatement | ifStatement | whileStatement | forStatement | returnStatement
static void parseStatement(){
	if (matchToken(TOKEN_PRINT)){
		parsePrintStatement();
	} else if (matchToken(TOKEN_LEFT_BRACE)){
		beginScope();
		parseBlockStatement();
		endScope();
	} else if (matchToken(TOKEN_IF)){
		parseIfStatement();
	} else if (matchToken(TOKEN_WHILE)){
		parseWhileStatement();
	} else if (matchToken(TOKEN_FOR)){
		parseForStatement();
	} else if (matchToken(TOKEN_RETURN)){
		parseReturnStatement();
	} else{
		parseExpressionStatement();
	}
}

// block -> "{" (declaration)* "}"
static void parseBlockStatement(){
	while (!checkToken(TOKEN_EOF) && !checkToken(TOKEN_RIGHT_BRACE)){
		parseDeclaration();
	}

	consumeToken(TOKEN_RIGHT_BRACE, "Expect '}' at end of block statement");
}

// exprStatement -> expr ";"
static void parseExpressionStatement(){
	parseExpression();
	consumeToken(TOKEN_SEMICOLON, "Expected ';' after end of expression");
	emitByte(OP_POP);
}

// printStatement -> "print" expression ";"
static void parsePrintStatement(){
	parseExpression();
	consumeToken(TOKEN_SEMICOLON, "Expected ';' after end of expression");
	emitByte(OP_PRINT);
}

// ifStatement -> "if" "(" expression ")" statement ("else" statement)?
static void parseIfStatement(){
	consumeToken(TOKEN_LEFT_PAREN, "Expect '(' after if");
	parseExpression();
	consumeToken(TOKEN_RIGHT_PAREN, "Expect ')' after if");
	
	int index = emitJump(OP_JUMP_IF_FALSE);
	emitByte(OP_POP);

	parseStatement();
	int endIndex = emitJump(OP_JUMP);

	// Use backpatching	
	patchJump(index, OP_JUMP_IF_FALSE);
	emitByte(OP_POP);
	if (matchToken(TOKEN_ELSE)){
		parseStatement();
	}

	patchJump(endIndex, OP_JUMP);
}

// forStatement -> "for" "(" ";" ";" ")" statement;
static void parseForStatement(){
	int endOfFor = -1, bodyIndex = -1;
	int conditionalIndex = -1, incrementIndex = -1;

	beginScope();
	consumeToken(TOKEN_LEFT_PAREN, "Expect '(' after for");
	
	if (!matchToken(TOKEN_SEMICOLON)){
		// initializer optional
		if (matchToken(TOKEN_VAR)) parseVarDeclaration();
		else parseExpressionStatement();
	}

	conditionalIndex = currentChunk()->count;
	if (!checkToken(TOKEN_SEMICOLON)){
		// condition clause
		parseExpression();
		endOfFor = emitJump(OP_JUMP_IF_FALSE);
		emitByte(OP_POP);
	} 

	bodyIndex = emitJump(OP_JUMP);
	incrementIndex = currentChunk()->count;

	consumeToken(TOKEN_SEMICOLON, "Expect ';' after 'for' loop's condition clause");

	if (!checkToken(TOKEN_RIGHT_PAREN)){
		// increment clause
		parseExpression();
		emitByte(OP_POP);
	}

	emitByte(OP_LOOP);
	patchJump(conditionalIndex, OP_LOOP);

	consumeToken(TOKEN_RIGHT_PAREN, "Expect ')' after for");

	patchJump(bodyIndex, OP_JUMP);
	parseStatement();

	emitByte(OP_LOOP);
	patchJump(incrementIndex, OP_LOOP);

	if (endOfFor != -1) {
		patchJump(endOfFor, OP_JUMP_IF_FALSE);
		emitByte(OP_POP);
	}

	endScope();
}

// whileStatement -> "while" "(" expression ")" statement
static void parseWhileStatement(){
	consumeToken(TOKEN_LEFT_PAREN, "Expect '(' after while");
	int conditionalIndex = currentChunk()->count;

	parseExpression();
	consumeToken(TOKEN_RIGHT_PAREN, "Expect ')' after while");

	int endJumpIndex = emitJump(OP_JUMP_IF_FALSE);
	emitByte(OP_POP);

	parseStatement();
	emitByte(OP_LOOP);
	patchJump(conditionalIndex, OP_LOOP);

	// jump back
	patchJump(endJumpIndex, OP_JUMP_IF_FALSE);
	emitByte(OP_POP);
}

static void parseReturnStatement(){
	if (currentCompiler->type == FUNCTION_MAIN) errorAtPreviousToken("Cannot return when not inside a function");

	Token previousToken = parser.previousToken;
	if (!matchToken(TOKEN_SEMICOLON)){
		parseExpression();
		previousToken = parser.previousToken;
		consumeToken(TOKEN_SEMICOLON, "Expected ';' at end of return statement");
	} else{
		emitByte(OP_NIL);
	}
	bool noExpression = (previousToken.length == 6) && (strncmp(previousToken.start, "return", 6) == 0);
	emitReturn(noExpression, previousToken);
}

// PrattParsing functions
static void parseExpression(){
	parsePrecedence(PREC_ASSIGN);
}


static void parseGrouping(bool canAssign){
	parseExpression();
	consumeToken(TOKEN_RIGHT_PAREN, "'(' expected");
}


static int parseArguments(){
	int nargs = 0;
	do{
		parseExpression();
		nargs++;
	} while(matchToken(TOKEN_COMMA));
	return nargs;
}


static void parseFuncCall(bool canAssign){
	int nargs=0;
	if (!matchToken(TOKEN_RIGHT_PAREN)){
		nargs = parseArguments();
		consumeToken(TOKEN_RIGHT_PAREN, "')' expected at end of function call");
	}
	emitBytes(OP_CALL, nargs);
}

static void parseNumber(bool canAssign){
	double value = strtod(parser.previousToken.start, NULL);
	emitConstant(NUMBER(value));
}

static void parseString(bool canAssign){
	emitConstant(OBJECT(makeStringObject(parser.previousToken.start+1, parser.previousToken.length-2)));
}

static void parseLiteral(bool canAssign){
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

static void parseThis(bool canAssign){
	if (currentCompilingClass == NULL)
		errorAtPreviousToken("Cannot use `this` keyword outside of a class");

	emitBytes(OP_GET_LOCAL, 0);
}

static void parseSuper(bool canAssign){
	if (currentCompilingClass == NULL)
		errorAtPreviousToken("Cannot use `super` keyword outside of a class");
	// push superclass on the top
	parseIdentifier(false);

	consumeToken(TOKEN_DOT, "'.' expected after `super` keyword");
	consumeToken(TOKEN_IDENTIFIER, "identifier expected after '.' with super keyword");

	ObjectString* string = makeStringObject(parser.previousToken.start, parser.previousToken.length);
	int index = getValueConstantIndex(OBJECT(string));
	if (index == -1) index = addConstantAndCheckLimit(OBJECT(string));
	emitBytes(OP_GET_SUPER, index);
}

static void parseIdentifier(bool canAssign){

	int index = getLocalDepth(currentCompiler, parser.previousToken);
	uint8_t set_op, get_op;
	if (index == -1){
		int upvalueIndex;
		if ((upvalueIndex = getUpvalueDepth(currentCompiler, parser.previousToken)) == -1){
			// Global variable
			set_op = OP_SET_GLOBAL; 
			get_op = OP_GET_GLOBAL; 
			Value value = OBJECT(makeStringObject(parser.previousToken.start, parser.previousToken.length));
			index = addConstantAndCheckLimit(value);
		} else {
			// Upvalue 
			set_op = OP_SET_UPVALUE; 
			get_op = OP_GET_UPVALUE; 
			index = upvalueIndex;
		}
	} else{
			// Local variable
			set_op = OP_SET_LOCAL; 
			get_op = OP_GET_LOCAL; 
	}

	if (canAssign && matchToken(TOKEN_EQUAL)){
		parseExpression();
		emitBytes(set_op, index);
	} else {
		emitBytes(get_op, index);
	}

}

static void parseUnary(bool canAssign){
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

static void parseAnd(bool canAssign){
	int jumpIndex = emitJump(OP_JUMP_IF_FALSE);
	emitByte(OP_POP);

	parsePrecedence(PREC_AND);
	patchJump(jumpIndex, OP_JUMP_IF_FALSE);
}

static void parseOr(bool canAssign){
	int jumpIndex = emitJump(OP_JUMP_IF_TRUE);
	emitByte(OP_POP);

	parsePrecedence(PREC_OR);
	patchJump(jumpIndex, OP_JUMP_IF_TRUE);
}

static ParseRow* getParseRow(TokenType type){
	return &(rules[type]);
}

static void parseBinary(bool canAssign){
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

static void parseDot(bool canAssign){
	consumeToken(TOKEN_IDENTIFIER, "Instance field or method name expected");
	ObjectString* string = makeStringObject(parser.previousToken.start, parser.previousToken.length);
	int index = getValueConstantIndex(OBJECT(string));
	if (index == -1) index = addConstantAndCheckLimit(OBJECT(string));

	if (canAssign && matchToken(TOKEN_EQUAL)){
		parseExpression();
		emitBytes(OP_SET_PROPERTY, index);
	} else if (matchToken(TOKEN_LEFT_PAREN)){
		int nargs = 0;
		if (!checkToken(TOKEN_RIGHT_PAREN))
			nargs = parseArguments();
		consumeToken(TOKEN_RIGHT_PAREN, "')' expected at end of function call");
		// OP_FAST_METHOD_CALL methodNameIndex args
		// using this instruction, we don't need to create boundMethod during runtime since it is slower and not necessary for simple cases like this as we know where the instance wil be on the stack
		emitBytes(OP_FAST_METHOD_CALL, index);
		emitByte(nargs);

	} else{
		emitBytes(OP_GET_PROPERTY, index);
	}
}

static void parsePrecedence(Precedence precedence){
	advanceToken();
	Token token = parser.previousToken;
	parseFn prefix = (getParseRow(token.type))->prefixFunction;

	bool canAssign = precedence <= PREC_ASSIGN;

	if (prefix != NULL) (*prefix)(canAssign);
	else {
		errorAtPreviousToken("Invalid target");
		return;
	}

	while (getParseRow(parser.currentToken.type)->level >= precedence){
		advanceToken();
		parseFn infix = (getParseRow(parser.previousToken.type))->infixFunction;
		if (infix != NULL) (*infix)(canAssign);
		else {
			errorAtPreviousToken("Invalid target");
			return;
		}
	}

	if (canAssign && matchToken(TOKEN_EQUAL)) errorAtPreviousToken("Invalid assignment target.");

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

static bool checkToken(TokenType type){
	if (parser.currentToken.type == type) return true;
	return false;
}

static bool matchToken(TokenType type){
	if (parser.currentToken.type == type){
		advanceToken();
		return true;
	}
	return false;
}

// Bytecode emitting function prototypes

static void emitByte(uint8_t byte){
	addCode(currentChunk(), byte, parser.previousToken.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2){
	emitByte(byte1);
	emitByte(byte2);

}

static int emitJump(uint8_t opcode){
	emitByte(opcode);
	emitBytes(0xff, 0xff);
	return currentChunk()->count - 2;
}

static void patchJump(int index, uint8_t opcode){
	
	int currentIndex = currentChunk()->count;
	int difference = currentIndex - index;
	if (difference > UINT16_T_LIMIT){
		// raise error if index is higher than UINT16_MAX
		errorAtPreviousToken("Too much code to jump over");
	} else{
		// Patch the jump instruction to jump to the current chunk's count index
		uint8_t diff1 = (uint8_t) difference & 255;
		uint8_t diff2 = difference >> 8;

		if (opcode == OP_LOOP){
			emitByte(diff2);
			emitByte(diff1);
		} else{
			*(currentChunk()->code + index) = diff2;
			*(currentChunk()->code + index + 1) = diff1;
		}
	}
}

static void emitConstant(Value value){
	emitByte(OP_CONSTANT);
	int index = addConstantAndCheckLimit(value);
	emitByte((uint8_t) index);
}

static uint8_t addConstantAndCheckLimit(Value value){
	int index = addConstant(currentChunk(), value);

	if (index > UINT8_MAX){
		// error
		errorAtPreviousToken("Too many values in one chunk");
		index=0;
	}
	return index;
}

static ObjectFunction* endCompiler(){
	emitReturn(true, parser.previousToken);
	#ifdef DEBUG_PRINT_CODE
	if (!parser.hadError) disassembleChunk(currentChunk(), currentCompiler->type == FUNCTION_MAIN ? "<script>" : currentCompiler->function->name->string);
	#endif

	ObjectFunction* function = currentCompiler->function;
	currentCompiler = currentCompiler->parentCompiler;
	return function;
}

static void emitReturn(bool noExpression, Token previousToken){
	ObjectFunction* function = currentCompiler->function;
	if (function->type == METHOD_INIT){
		if (noExpression){
			emitBytes(OP_GET_LOCAL, 0);
		} else{
			if(!(previousToken.length == 4 && (strncmp(previousToken.start, "this", 4) == 0)))
				 errorAtPreviousToken("Cannot return anything except for `this` inside the init method");
		}
	} else{
		if (noExpression){
			emitByte(OP_NIL);
		}
	}
	emitByte(OP_RETURN);
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

static void synchronize(){

	parser.panicMode = false;

	while (parser.currentToken.type != TOKEN_EOF){

		if (parser.previousToken.type == TOKEN_SEMICOLON) break;
		switch (parser.currentToken.type){
			
			case TOKEN_CLASS:
			case TOKEN_FUN:
			case TOKEN_IF:
			case TOKEN_WHILE:
			case TOKEN_FOR:
			case TOKEN_VAR:
			case TOKEN_PRINT:
			case TOKEN_RETURN:
				return;

			default:
				break;
		}
		advanceToken();

	}
}

// Helper functions

static int parseGlobalVariable(){
	Value value = OBJECT(makeStringObject(parser.previousToken.start, parser.previousToken.length));
	return addConstantAndCheckLimit(value);
}

static void addSuperAsLocalVariable(){
	Token superToken = (Token) {.type = TOKEN_IDENTIFIER, .length = 5, .line=-1, .start="super"};
	currentCompiler->locals[currentCompiler->currentLocalsCount++] = (Local) {.depth = currentCompiler->currentScopeDepth, .name=superToken, .isCaptured = false};
}

static void handleLocalVariable(){
	if (currentCompiler->currentLocalsCount > UINT8_T_LIMIT){
		errorAtPreviousToken("Too many locals variables!");

	} else{
		if (noDuplicateVarInCurrentScope()) {
			currentCompiler->locals[currentCompiler->currentLocalsCount++] = (Local) {.depth = -1, .name=parser.previousToken, .isCaptured = false};
		} else{
			errorAtPreviousToken("Local variable cannot be re-initialized!");
		}
	}
}

static int getLocalDepth(Compiler* compiler, Token token){
	// search if any such token in compiler locals array
	for (int i=(compiler->currentLocalsCount -1); i>=0; i--){
		Local* pLocal = &(compiler->locals[i]);
		Token* secondToken = &(pLocal->name);
		if (identifiersEqual(&token, secondToken)){
			if (pLocal->depth == -1) errorAtPreviousToken("Can't read local variable in its own initializer");
			return i;	
		}
	}
	return -1;
}

static int addUpvalue(Compiler* compiler, int index, bool isLocal){
	Upvalue upvalue;
	for (int upvalueIndex=0; upvalueIndex < compiler->currentUpvaluesCount ;upvalueIndex++){
		upvalue = compiler->upvalues[upvalueIndex];
		if (upvalue.index == index && upvalue.isLocal == isLocal){
			return upvalueIndex;
		}
	}

	if (compiler->currentUpvaluesCount == UINT8_T_LIMIT) errorAtPreviousToken("Cannot add more closure variables in function");
	
	compiler->upvalues[compiler->currentUpvaluesCount] = (Upvalue) {.index=index, .isLocal=isLocal};	

	if (isLocal) compiler->parentCompiler->locals[index].isCaptured = true;
	
	index = compiler->currentUpvaluesCount++;
	return index;
}

static int getUpvalueDepth(Compiler* compiler, Token token){
	if (compiler->parentCompiler == NULL) return -1;
	bool isLocal = true;

	int index = getLocalDepth(compiler->parentCompiler, token);
	if (index == -1){
		index = getUpvalueDepth(compiler->parentCompiler, token);
		isLocal = false;
	}

	return (index == -1) ? -1 : addUpvalue(compiler, index, isLocal);
}

static Chunk* currentChunk(){
	return currentCompiler->function->chunk;
}

static void beginScope(){
	currentCompiler->currentScopeDepth++;
}

static void endScope(){
	currentCompiler->currentScopeDepth--;
       	// remove all local variables from the stack that were declared in the scope that ended
       	int i = currentCompiler->currentLocalsCount - 1;
       	while (i >= 0 && currentCompiler->locals[i].depth > currentCompiler->currentScopeDepth){

	       if (currentCompiler->locals[i].isCaptured) emitByte(OP_POP_UPVALUE);
	       else emitByte(OP_POP);

	       currentCompiler->currentLocalsCount--;
	       i--;

	}
}

static bool noDuplicateVarInCurrentScope(){
	Token* token = &parser.previousToken;

	for (int i=(currentCompiler->currentLocalsCount -1); i>=0; i--){
		Local* pLocal = &(currentCompiler->locals[i]);
		Token* secondToken = &(pLocal->name);

		if (currentCompiler->currentScopeDepth == pLocal->depth && identifiersEqual(token, secondToken)){
			return false;	
		}
	}
	return true;
}

static bool identifiersEqual(Token* token1, Token* token2){
		if (token1->length == token2->length && memcmp(token1->start, token2->start, token1->length) == 0) return true;
		return false;
}

static void markInitialized(){
	currentCompiler->locals[currentCompiler->currentLocalsCount-1].depth = currentCompiler->currentScopeDepth;
}

static int getValueConstantIndex(Value value){
	Chunk* chunk = currentChunk();
	for (int i=0; i<chunk->constants.count; i++){
		Value val = chunk->constants.values[i];
		if (checkIfValuesEqual(val, value)) return i;
	}
	return -1;
}
