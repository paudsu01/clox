#include "scanner.h"
#include <string.h>
#include <stdbool.h>


extern Scanner scanner;

// static function prototypes
static char peekChar();
static char consumeChar();
static void consumeLine();
static bool isAtEnd();
static Token makeToken(TokenType);
static Token makeErrorToken(char*);


void initScanner(char* source){

	scanner.line = 1;
	scanner.start = source;
	scanner.current = source;

}

Token scanToken(){
	
	// Readjust pointers
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
				break;
			case '<':
				break;
			case '=':
				break;
			case '!':
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

static void consumeLine(){
	while (consumeChar() != '\n') {};
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

static Token makeErrorToken(char* message){

	Token token;
	token.type = TOKEN_ERROR;
	token.start = message;
	token.line = scanner.line;
	token.length = (int) strlen(message);
	return token;

}
