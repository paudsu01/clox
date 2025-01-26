#include "scanner.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


extern Scanner scanner;

// static function prototypes
static char peekChar();
static char peekNextChar();
static char consumeChar();
static void consumeLine();
static bool matchChar(char);

static Token checkKeyword(char*, int, TokenType);

static bool isDigit(char);
static bool isAlpha(char);
static bool isAtEnd();

static Token makeToken(TokenType);
static Token makeNumberToken();
static Token makeIdentifierToken();
static Token makeStringToken();
static Token makeErrorToken(char*);


void initScanner(const char* source){

	scanner.line = 1;
	scanner.start = source;
	scanner.current = source;

}

Token scanToken(){

	while (true){
		// Re-adjust pointers
		scanner.start = scanner.current;

		// Check if EOF
		if (isAtEnd()) return makeToken(TOKEN_EOF);
		char c = consumeChar();

		switch(c){
			case '\n':
				scanner.line++;
			case '\t':
			case ' ':
				break;

			case '(': return makeToken(TOKEN_LEFT_PAREN);
			case ')': return makeToken(TOKEN_RIGHT_PAREN);
			case '{': return makeToken(TOKEN_LEFT_BRACE);
			case '}': return makeToken(TOKEN_RIGHT_BRACE);
			case ',': return makeToken(TOKEN_COMMA);
			case '.': return makeToken(TOKEN_DOT);
			case '+': return makeToken(TOKEN_PLUS);
			case '-': return makeToken(TOKEN_MINUS);
			case ';': return makeToken(TOKEN_SEMICOLON);
			case '*': return makeToken(TOKEN_STAR);
			case '/':
				if (peekChar() == '/') {
					consumeLine();
					break;
				}
				else return makeToken(TOKEN_SLASH);
			
			case '>':
				return (matchChar('=')) ? makeToken(TOKEN_GREATER_EQUAL) : makeToken(TOKEN_GREATER);
			case '<':
				return (matchChar('=')) ? makeToken(TOKEN_LESS_EQUAL) : makeToken(TOKEN_LESS);
			case '=':
				return (matchChar('=')) ? makeToken(TOKEN_EQUAL_EQUAL) : makeToken(TOKEN_EQUAL);
			case '!':
				return (matchChar('=')) ? makeToken(TOKEN_BANG_EQUAL) : makeToken(TOKEN_BANG);

			case '"':
				return makeStringToken();// string constant
			default:
				if (isDigit(c)) return makeNumberToken();// number constant
				else if (isAlpha(c)) return makeIdentifierToken();
				else return makeErrorToken("Unrecognized character");

		}
	}
		
}

// Helper static functions

static char consumeChar(){
	return *(scanner.current++);
}

static char peekChar(){
	return *(scanner.current);
}

static char peekNextChar(){
	return (isAtEnd()) ? '\0' : *(scanner.current+1);
}

static void consumeLine(){
	while (consumeChar() != '\n') {};
}

static bool matchChar(char c){
	if (peekChar() == c){
		consumeChar();
		return true;
	}
	return false;
}

static bool isDigit(char c){
	return (c >= '0') && (c <= '9');
}

static bool isAlpha(char c){
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		c == '_';
}

static bool isAtEnd(){
	return *(scanner.current) == '\0';
}

static Token checkKeyword(char* remainingString, int startIndex, TokenType type){
	if ((scanner.current - scanner.start) == (strlen(remainingString) + startIndex) && (memcmp(remainingString, scanner.start + startIndex, strlen(remainingString)) == 0))
		return makeToken(type);
	return makeToken(TOKEN_IDENTIFIER);
}

// Helper static functions with tokens

static Token makeToken(TokenType type){

	Token token;
	token.type = type;
	token.start = scanner.start;
	token.line = scanner.line;
	token.length = (type == TOKEN_EOF) ? 0 : scanner.current - scanner.start;
	return token;
}


static Token makeIdentifierToken(){
	while (isAlpha(peekChar()) || isDigit(peekChar())) consumeChar();
	switch (*(scanner.start)){
		case 'a': return checkKeyword("nd", 1, TOKEN_AND);
		case 'c': return checkKeyword("lass", 1, TOKEN_CLASS);
		case 'e': return checkKeyword("lse", 1, TOKEN_ELSE);
		case 'f':
			  if ((scanner.current - scanner.start) > 1){
				  switch (scanner.start[1]){
					  case 'a': return checkKeyword("lse", 2, TOKEN_FALSE);
					  case 'o': return checkKeyword("r", 2, TOKEN_FOR);
					  case 'u': return checkKeyword("n", 2, TOKEN_FUN);
				  }
			  }
			  break;

		case 'i': return checkKeyword("f", 1, TOKEN_IF);
		case 'n': return checkKeyword("il", 1, TOKEN_NIL);
		case 'o': return checkKeyword("r", 1, TOKEN_OR);
		case 'p': return checkKeyword("rint", 1, TOKEN_PRINT);
		case 'r': return checkKeyword("eturn", 1, TOKEN_RETURN);
		case 's': return checkKeyword("uper", 1, TOKEN_SUPER);
		case 't':
			  if ((scanner.current - scanner.start) > 1){
				  switch (scanner.start[1]){
					  case 'h': return checkKeyword("is", 2, TOKEN_THIS);
					  case 'r': return checkKeyword("ue", 2, TOKEN_TRUE);
				  }
			  }
			  break;
		case 'v': return checkKeyword("ar", 1, TOKEN_VAR);
		case 'w': return checkKeyword("hile", 1, TOKEN_WHILE);
	}
	return makeToken(TOKEN_IDENTIFIER);
}

static Token makeStringToken(){
	while (!isAtEnd() && peekChar() != '"'){
		if (peekChar() == '\n') scanner.line++;
		consumeChar();
	}
	if (isAtEnd()) return makeErrorToken("Unterminated String");
	// consume the '"' char
	consumeChar();
	return makeToken(TOKEN_STRING);
}

static Token makeNumberToken(){
	
	while (isDigit(peekChar())) {
		consumeChar();	
	}

	if (peekChar() == '.' && isDigit(peekNextChar())){
		
		// consume the '.'
		consumeChar();	
		while (isDigit(peekChar())) {
			consumeChar();	
		}
	}

	return makeToken(TOKEN_NUMBER);
}

static Token makeErrorToken(char* message){

	Token token;
	token.type = TOKEN_ERROR;
	token.start = message;
	token.line = scanner.line;
	token.length = (int) strlen(message);
	return token;

}
