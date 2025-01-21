#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"

typedef struct {
	const char* start;
	const char* current;
	int line;
} Scanner;

Scanner scanner;

// prototypes
void initScanner(const char*);
Token scanToken();

#endif
