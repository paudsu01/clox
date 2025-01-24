#include <stdio.h>
#include "../common.h"

#include "compiler.h"
#include "../vm/vm.h"
#include "../scanner/scanner.h"

void compile(const char* source){
	initScanner(source);
	while (true){
		Token token = scanToken();	
		printf("Line :%d\tTokenID: %d\tlen: %d\t'%.*s'\n", token.line, token.type, token.length, token.length, token.start);
		if (token.type == TOKEN_EOF) break;
	}
}
