#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "argument_parser.h"

static const int PARAM_NUM = 1;
static const int PATH_INDEX = 1;

/*
*   Functions declarations area
*/

bool _isParamNumberCorrect(int argc);
bool _isPathValid(const char* path);

/*
*   Functions declarations area end
*/

void printHelp(){
    printf("Program call:\nprogram_name batch_processing_file_src\n");
    printf("Please be aware that batch commands must be in separate lines, only ole-level \"\" "
           "is allowed and no pipes are allowed.\n");
}

ProgramInput* parseArguments(int argc, const char* argv[]){
    if(
        _isParamNumberCorrect(argc) != true ||
        _isPathValid(argv[PATH_INDEX]) != true
    ) return NULL;
    ProgramInput* programInput = (ProgramInput*)calloc(1, sizeof(ProgramInput));
    if(programInput == NULL) return NULL;
    programInput->batchFilePath = argv[PATH_INDEX];
    return programInput;
}

bool _isParamNumberCorrect(int argc){
    return (argc == PARAM_NUM + 1);
}

bool _isPathValid(const char* path){
    return (strlen(path) > 0);
}