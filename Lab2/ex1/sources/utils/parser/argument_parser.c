#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "argument_parser.h"

static const int GEN_ARG_NUM = 5;
static const int SORT_ARG_NUM = 4;
static const int COPY_ARG_NUM = 5;
static const int MODE_POS = 1;
static const int FT_POS = 2;

/*
*   "Private" function declaration
*/

bool _isFirstParametersCorr(int argc, const char* argv[]);
bool _isModeInGoodRange(const char* mode);
bool _isFuncTpeInGoodRange(const char* funcType);
ProgramInput* _dispatchArgs(int argc, const char* argv[]);
ProgramInput* _dispatchGenArgs(int argc, const char* argv[]);
bool _dispGenCheckPassed(int argc, const char* argv[]);
bool _isGenModeParamNumOK(int argc);
void _fulfillGenInput(ProgramInput* genInput, const char* argv[]);
ProgramInput* _dispatchSortArgs(int argc, const char* argv[]);
bool _dispSortCheckPassed(int argc, const char* argv[]);
bool _isSortModeParamNumOK(int argc);
void _fulfillSortInput(ProgramInput* sortInput, const char* argv[]);
ProgramInput* _dispatchCopyArgs(int argc, const char* argv[]);
bool _dispCopyCheckPassed(int argc, const char* argv[]);
bool _isCopyModeParamNumOK(int argc);
void _fulfillCopyInput(ProgramInput* copyInput, const char* argv[]);
bool _isStringLenOK(const char* name);
bool _isIntValueOK(const char* number);
void _fulfillSharedInputFields(ProgramInput* input, const char* argv[], int mode);

/*
*   End of declarations
*/

void printHelp(){
    printf("Available program modes:\n\nprogram_name g {s} file_name block_size record_numbers\n"
            "Program generates random record table into a file (name given by file_name)\nwith given "
            "block_size and record_numbers.\n\n"
            "program_name s {s or l} file_name block_size\n"
            "Program performs insertion sort at file given by file_name with given block_size.\n\n"
            "program_name c {s or l} source dest buffer_size\n"
            "Program performs copying from source to dest with given buffer_size.\n\n"
            "{s} / {s or l} - we can choose the (s)ystem (read and write) or \n(l)ibrary (fread and fwrite)"
            "function calls to be used\n");
}

ProgramInput* parseArguments(int argc, const char* argv[]){
    if(_isFirstParametersCorr(argc, argv)){
        return _dispatchArgs(argc, argv);
    } else {
        return NULL;
    }
}

bool _isFirstParametersCorr(int argc, const char* argv[]){
    return (
        argc >= 3 && 
        _isModeInGoodRange(argv[MODE_POS]) && 
        _isFuncTpeInGoodRange(argv[FT_POS])
    );
}

bool _isModeInGoodRange(const char* mode){
    return (
        strlen(mode) == 1 &&
        (mode[0] == 'g' || mode[0] == 's' || mode[0] == 'c')
    );
}

bool _isFuncTpeInGoodRange(const char* funcType){
    return (
        strlen(funcType) == 1 &&
        (funcType[0] == 's' || funcType[0] == 'l')
    );
}

ProgramInput* _dispatchArgs(int argc, const char* argv[]){
    ProgramInput* parsedArgs;
    switch(argv[MODE_POS][0]){
        case 'g':
            parsedArgs = _dispatchGenArgs(argc, argv);
            break;
        case 's':
            parsedArgs = _dispatchSortArgs(argc, argv);
            break;
        case 'c':
            parsedArgs = _dispatchCopyArgs(argc, argv);
            break;
        default:
            parsedArgs = NULL;
    }
    return parsedArgs;
}

ProgramInput* _dispatchGenArgs(int argc, const char* argv[]){
    if(!_dispGenCheckPassed(argc, argv)) 
        return NULL;
    ProgramInput* genInput = calloc(1, sizeof(ProgramInput));
    if(genInput != NULL)
        _fulfillGenInput(genInput, argv);
    return genInput;
}

bool _dispGenCheckPassed(int argc, const char* argv[]){
    return (
        _isGenModeParamNumOK(argc) &&
        _isStringLenOK(argv[3]) &&
        _isIntValueOK(argv[4]) &&
        _isIntValueOK(argv[5]) &&
        strlen(argv[2]) == 1 &&
        argv[2][0] == 's'
    );
}

bool _isGenModeParamNumOK(int argc){
    return argc == 1 + GEN_ARG_NUM;
}

void _fulfillGenInput(ProgramInput* genInput, const char* argv[]){
    _fulfillSharedInputFields(genInput, argv, GENERATE);
    genInput->blockSize = atoi(argv[4]);
    genInput->numOfRecords = atoi(argv[5]);
}

ProgramInput* _dispatchSortArgs(int argc, const char* argv[]){
    if(!_dispSortCheckPassed(argc, argv)) return NULL;
    ProgramInput* sortInput = calloc(1, sizeof(ProgramInput));
    if(sortInput != NULL) 
        _fulfillSortInput(sortInput, argv);
    return sortInput;
}

bool _dispSortCheckPassed(int argc, const char* argv[]){
    return (
        _isSortModeParamNumOK(argc) &&
        _isStringLenOK(argv[3]) &&
        _isIntValueOK(argv[4])
    );
}

bool _isSortModeParamNumOK(int argc){
    return argc == 1 + SORT_ARG_NUM;
}

void _fulfillSortInput(ProgramInput* sortInput, const char* argv[]){
    _fulfillSharedInputFields(sortInput, argv, SORT);
    sortInput->blockSize = atoi(argv[4]);
}


ProgramInput* _dispatchCopyArgs(int argc, const char* argv[]){
    if(!_dispCopyCheckPassed(argc, argv)) return NULL;
    ProgramInput* copyInput = calloc(1, sizeof(ProgramInput));
    if(copyInput != NULL) 
        _fulfillCopyInput(copyInput, argv);
    return copyInput;
}

bool _dispCopyCheckPassed(int argc, const char* argv[]){
    return (
        _isCopyModeParamNumOK(argc) &&
        _isStringLenOK(argv[3]) &&
        _isStringLenOK(argv[4]) &&
        _isIntValueOK(argv[5])
    );
}

bool _isCopyModeParamNumOK(int argc){
    return argc == 1 + COPY_ARG_NUM;
}

void _fulfillCopyInput(ProgramInput* copyInput, const char* argv[]){
    _fulfillSharedInputFields(copyInput, argv, COPY);
    copyInput->copyDestFile = argv[4];
    copyInput->blockSize = atoi(argv[5]);
}

bool _isStringLenOK(const char* name){
    return (strlen(name) > 0);
}

bool _isIntValueOK(const char* number){
    return (atoi(number) > 0);
}

void _fulfillSharedInputFields(ProgramInput* input, const char* argv[], int mode){
    if(argv[FT_POS][0] == 's') input->funcTypes = SYSTEM;
    else input->funcTypes = LIBRARY;
    input->mode = mode;
    input->baseFile = argv[3];
}
