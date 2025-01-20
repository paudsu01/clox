#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"

typedef struct {
	char* start;
	char* current;
	int line;
} Scanner;

Scanner scanner;

// prototypes
void initScanner(char*);
Token scanToken();

#endif
