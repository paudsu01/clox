#include "scanner.h"
#include <string.h>
#include <stdbool.h>


extern Scanner scanner;

// static function prototypes
static char peekChar();
static char peekNextChar();
static char consumeChar();
static void consumeLine();
static bool matchChar(char);

static bool isDigit(char);
static bool isAlpha(char);
static bool isAtEnd();

static Token makeToken(TokenType);
static Token makeNumberToken();
static Token makeIdentifierToken();
static Token makeStringToken();
static Token makeErrorToken(char*);


void initScanner(char* source){

	scanner.line = 1;
	scanner.start = source;
	scanner.current = source;

}

Token scanToken(){
	
	// Re-adjust pointers
	scanner.start = scanner.current;

	while (true){
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
				return (matchChar('=')) ? makeToken(TOKEN_LESS_EQUAL) : makeToken(TOKEN_LESS);
			case '<':
				return (matchChar('=')) ? makeToken(TOKEN_GREATER_EQUAL) : makeToken(TOKEN_GREATER);
			case '=':
				return (matchChar('=')) ? makeToken(TOKEN_EQUAL_EQUAL) : makeToken(TOKEN_EQUAL);
			case '!':
				return (matchChar('=')) ? makeToken(TOKEN_BANG_EQUAL) : makeToken(TOKEN_BANG);

			case '"':
				return makeStringToken();// string constant
			default:
				if (isDigit(c)) return makeNumberToken();// number constant
				else if (isAlpha(c)) return makeIdentifierToken();
				break;

		}
	}
		
	return makeErrorToken("Unrecognized character");
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

// Helper static functions with tokens

static Token makeToken(TokenType type){

	Token token;
	token.type = type;
	token.start = scanner.start;
	token.line = scanner.line;
	token.length = scanner.current - scanner.start;
	return token;
}


static Token makeIdentifierToken(){
	while (isAlpha(peekChar()) || isDigit(peekChar())) consumeChar();
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
