#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include "file_reader.h"

static const int NULL_MARGIN = 1;
static const int OP_OK = 0;
static const int OP_ERROR = -1;

/*
*   Functions declaratiosn area
*/

int _getParametersNumber(char* line, ssize_t len);
void _dispathLineCharacterCount
    (char character, int* chunkCounter, bool* isArgOpen, bool* isPrevoisuCharSpace, bool* isQuoteOpen);
int _dispathLineCharacterExtract(char* line, int i, int* startIdx, int* endIdx, bool* isArgOpen, 
    bool* isPrevoisuCharSpace, bool* isQuoteOpen, int* idxInArgs, char** args);
bool _setFlagIfArgStarted(char character, bool* isArgOpen, bool* isQuoteOpen, bool* isPrevoiusSpace);
bool _isArgEnded(char character, bool isArgOpen, bool isQuoteOpen, bool isPrevoiusSpace);
bool _isNewArgOpenInLastChar(char characted, bool* isArgOpen, bool* isQuoteOpen, bool* isPrevoisuCharSpace);
void _toggleStatus(bool* status);
int _fulfillArgsTable(char* line, ssize_t len, char** args);
char* _extractArgument(char* line, int startIdx, int endIdx);

/*
*   Functions declaratiosn area end
*/

char** preprocessLineOfFile(FILE *stream){
    char* line;
    size_t len = 0;
    ssize_t nread;
    int numOfParams;
    if((nread = getline(&line, &len, stream)) == -1 ||
       ((numOfParams = _getParametersNumber(line, nread)) == 0)) 
        return NULL;
    char** args = (char**)calloc(numOfParams + NULL_MARGIN, sizeof(char*));
    if(args == NULL) 
        return NULL;
    int opStatus = _fulfillArgsTable(line, nread, args);
    if(opStatus == OP_ERROR){
        for(int i = 0; i <= numOfParams; i++) free(args[i]);
        free(args);
        return NULL;
    } else {
        return args;
    }
}

int _getParametersNumber(char* line, ssize_t len){
    int chunkCounter = 0, posOfPossibleLF = len - 1;
    bool isPrevoisuCharSpace = false, isQuoteOpen = false, isArgOpen = false;
    for(int i = 0; i < posOfPossibleLF; i ++){
       _dispathLineCharacterCount
        (line[i], &chunkCounter, &isArgOpen, &isPrevoisuCharSpace, &isQuoteOpen);
    }
    // The last char can only start new arg if it is not \n or inappropriate in parser - unfinished "..." block
    if(_isNewArgOpenInLastChar(line[posOfPossibleLF], &isArgOpen, &isQuoteOpen, &isPrevoisuCharSpace))
        chunkCounter++;
    return chunkCounter;
}

void _dispathLineCharacterCount
    (char character, int* chunkCounter, bool* isArgOpen, bool* isPrevoisuCharSpace, bool* isQuoteOpen){
    if(_setFlagIfArgStarted(character, isArgOpen, isQuoteOpen, isPrevoisuCharSpace)){
        (*chunkCounter) += 1;
    } else if(_isArgEnded(character, *isArgOpen, *isQuoteOpen, *isPrevoisuCharSpace)) {
        _toggleStatus(isArgOpen);
    } else if(character == '"') {
        _toggleStatus(isQuoteOpen);
         *isPrevoisuCharSpace = false;
    } else if(character == ' '){
        *isPrevoisuCharSpace = true;
    }
}

bool _setFlagIfArgStarted(char character, bool* isArgOpen, bool* isQuoteOpen, bool* isPrevoiusSpace){
    if(character != ' ' && (*isArgOpen) == false && (*isQuoteOpen) == false){
        if(character == '"') _toggleStatus(isQuoteOpen);
        _toggleStatus(isArgOpen);
        *isPrevoiusSpace = false;
        return true;
    } else {
        return false;
    }
}

bool _isArgEnded(char character, bool isArgOpen, bool isQuoteOpen, bool isPrevoiusSpace){
    return (character == ' ' && isArgOpen == true && isPrevoiusSpace == false && isQuoteOpen == false);
}

bool _isNewArgOpenInLastChar(char characted, bool* isArgOpen, bool* isQuoteOpen, bool* isPrevoisuCharSpace){
    return (
        _setFlagIfArgStarted(characted, isArgOpen, isQuoteOpen, isPrevoisuCharSpace) &&
        characted != '\n' && characted != '"'
    );
}

void _toggleStatus(bool* status){
    if((*status) == true) *status = false;
    else *status = true; 
}

int _fulfillArgsTable(char* line, ssize_t len, char** args){
    int startIdx = 0, endIdx = 0, idxInArgs = 0, posOfPossibleLF = len - 1, opStatus = OP_OK;
    bool isPrevoisuCharSpace = false, isQuoteOpen = false, isArgOpen = false;
    for(int i = 0; i < posOfPossibleLF; i ++){
        opStatus = _dispathLineCharacterExtract(line, i, &startIdx, &endIdx, &isArgOpen, &isPrevoisuCharSpace,
        &isQuoteOpen, &idxInArgs, args);
    }
    if(opStatus == OP_ERROR) 
        return OP_ERROR;
    // last char handle
    if(isArgOpen == true){
        if(line[posOfPossibleLF] != '\n') args[idxInArgs] = _extractArgument(line, startIdx, len);
        else args[idxInArgs] = _extractArgument(line, startIdx, posOfPossibleLF);
    } 
    else if (_isNewArgOpenInLastChar(line[posOfPossibleLF], &isArgOpen, &isQuoteOpen, &isPrevoisuCharSpace)){
        args[idxInArgs] = _extractArgument(line, posOfPossibleLF, len);
    }
    return OP_OK;
}

int _dispathLineCharacterExtract(char* line, int i, int* startIdx, int* endIdx, bool* isArgOpen, 
bool* isPrevoisuCharSpace, bool* isQuoteOpen, int* idxInArgs, char** args){
    if(_setFlagIfArgStarted(line[i], isArgOpen, isQuoteOpen, isPrevoisuCharSpace)){
        *startIdx = i;
    } else if(_isArgEnded(line[i], *isArgOpen, *isQuoteOpen, *isPrevoisuCharSpace)) {
        _toggleStatus(isArgOpen);
        *endIdx = i;
        if((args[*idxInArgs] = _extractArgument(line, *startIdx, *endIdx)) == NULL){
            return OP_ERROR;
        }
        *idxInArgs += 1;
    } else if(line[i] == '"') {
        _toggleStatus(isQuoteOpen);
        *isPrevoisuCharSpace = false;
    } else if(line[i] == ' '){
        *isPrevoisuCharSpace = true;
    }
    return OP_OK;
}

char* _extractArgument(char* line, int startIdx, int endIdx){
    if(line[startIdx] == '"') startIdx++;
    if(line[endIdx - 1] == '"') endIdx--;
    char* extractedArg = (char*)calloc(endIdx - startIdx + 1, sizeof(char)); // + 1 - right exclusive! and \0 size
    if(extractedArg == NULL) return NULL;
    strncpy(extractedArg, line + startIdx, endIdx - startIdx);
    return extractedArg;
}
