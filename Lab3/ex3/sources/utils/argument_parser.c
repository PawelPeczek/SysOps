#define _GNU_SOURCE
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "argument_parser.h"

static const int PARAM_NUM = 3;
static const int PATH_INDEX = 1;
static const int CPU_TIME_INDEX = 2;
static const int VIRT_MEM_INDEX = 3;

/*
*   Functions declarations area
*/

bool _isParamNumberCorrect(int argc);
bool _isPathValid(const char* path);
bool _isIntValueCorrect(const char* value);

/*
*   Functions declarations area end
*/

void printHelp(){
    printf("Program call:\nprogram_name batch_processing_file_src MAX_CPU_time_S MAX_VIRT_MEM_MB\n");
    printf("Please be aware that batch commands must be in separate lines, only ole-level \"\" "
           "is allowed and no pipes are allowed. Point out also that limits values must be greater than zero.\n");
}

ProgramInput* parseArguments(int argc, const char* argv[]){
    if(
        _isParamNumberCorrect(argc) != true ||
        _isPathValid(argv[PATH_INDEX]) != true ||
        _isIntValueCorrect(argv[CPU_TIME_INDEX]) != true ||
        _isIntValueCorrect(argv[VIRT_MEM_INDEX]) != true
    ) return NULL;
    ProgramInput* programInput = (ProgramInput*)calloc(1, sizeof(ProgramInput));
    if(programInput == NULL) return NULL;
    programInput->batchFilePath = argv[PATH_INDEX];
    programInput->maxCPUTime = atoi(argv[CPU_TIME_INDEX]);
    programInput->maxVirtMemSizeMB = atoi(argv[VIRT_MEM_INDEX]);
    return programInput;
}

bool _isParamNumberCorrect(int argc){
    return (argc == PARAM_NUM + 1);
}

bool _isPathValid(const char* path){
    return (strlen(path) > 0);
}

bool _isIntValueCorrect(const char* value){
    int number = atoi(value);
    return (number > 0);
}