#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "interpreter.h"
#include "file_reader.h"
static const int OP_OK = 0;
static const int OP_ERROR = -1;

/*
* Declarations of functions
*/

int _proceedInterpretationLoop(FILE* fileStream, ProgramInput* input);
int _invokeProcess(char** args, ProgramInput* input, FILE* file);
void _freeArgs(char** args);
void _printError(int line);

/*
* Declarations of functions END
*/

int proceedBatchInterpretation(ProgramInput* input){
    FILE* fileStream = fopen(input->batchFilePath, "r");
    if(fileStream == NULL){
        perror("Could not open file.");
        return OP_ERROR;
    }
    int opStatus = _proceedInterpretationLoop(fileStream, input);
    fclose(fileStream);
    return opStatus;
}

int _proceedInterpretationLoop(FILE* fileStream, ProgramInput* input){
    char ** args;
    int line = 1, opStatus = OP_OK;
    while((args = preprocessLineOfFile(fileStream)) != NULL){        
        opStatus = _invokeProcess(args, input, fileStream);
        _freeArgs(args);
        if(opStatus == OP_ERROR) break;
        line ++;
    }
    if(opStatus == OP_ERROR) _printError(line);
    return opStatus;
}

int _invokeProcess(char** args, ProgramInput* input, FILE* file){
    int opStatus = OP_OK;
    pid_t child_pid = fork();
    if(child_pid != 0){
        // parent
        wait(&opStatus);
    } else {
        // new process
        int output = execvp(args[0], args);
        _freeArgs(args);
        free(input);
        fclose(file);
	    exit(output);
    }
    if(opStatus != OP_OK) return OP_ERROR;
    else return OP_OK;
}

void _freeArgs(char** args){
    int i = 0;
    while(args[i] != NULL){
        free(args[i]);
        i++;
    }
    free(args);
}

void _printError(int line){
    printf("There was and error while processing batch command in line %d\n", line);
}
