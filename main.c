#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm/vm.h"

#define DEBUG_CHUNK

// function prototypes
static void runREPL();
static void runFile(char*);
static char* readFile(char*);

int main(int nargs, char * args[]){
	initVM(false);

	if (nargs == 1){
		// REPL
		runREPL();

	} else if (nargs == 2){
		// Run a file
		runFile(*(args+1));

	} else {
		printf("Usage: clox [path]\n");
		exit(49);
	}
	freeVM();
	return 0;
}


static void runREPL(){
	char line[1024];

	printf("Clox interpreter v2.2.0. Type `exit` to quit the interpreter.\n");
	while (true){
		short index=0;
		printf(">> ");
		while ((line[index]=getchar()) != '\n'){
			index++;
		}
		
		line[index+1] = '\0';

		if (strcmp("exit\n", line) == 0) {
			break;
		}

		interpret(line);
	}
}

static void runFile(char* fileName){
	char * fileSource;
	fileSource = readFile(fileName);
	InterpreterResult result = interpret(fileSource);
	free(fileSource);
	if (result == COMPILE_ERROR) exit(65);
	if (result == RUNTIME_ERROR) exit(70);
}


static char* readFile(char* fileName){
	FILE* pFile = fopen(fileName, "r");
	if (pFile == NULL) {
		fprintf(stderr, "Unable to open file : %s\n", fileName);
		exit(74);
	}

	fseek(pFile, 0, SEEK_END); 
	long size = ftell(pFile);
	fseek(pFile, 0, SEEK_SET); 

	char* filePointer = (char*) malloc(size+1);
	if (filePointer == NULL){
		fprintf(stderr, "Not enough memory to read from file : %s\n", fileName);
		exit(74);
	}

	for (int i=0; i<size;++i){
		*(filePointer + i) = fgetc(pFile);
	}
	filePointer[size] = '\0';
	
	fclose(pFile);
	return filePointer;
}
