#include <stdio.h>
#include <stdbool.h>
#include "lib_sort.h"

static const int OP_ERROR = -1;
static const int OP_OK = 0;

/*
*   "Private" function declaration
*/

int _insertionSortLib(FILE* source, int blockSize);
int _runSortLib(FILE* source, int blockSize, long long fileLen);
int _runInnerSortLoopLib(FILE* source, int blockSize, unsigned char* key, long long uppLoopIdx);
long long _getFiLeLengthAndRewindLib(FILE* source);
int _seekAndReadLib(FILE* source, int blockSize, int blockNumber, unsigned char* buffer);
int _putKeyAtBeginLib(FILE* source, int blockSize, unsigned char* key);

/*
*   End of declarations
*/

int sortWithLibCalls(ProgramInput* input){
    FILE* source = fopen(input->baseFile, "r+b");
    if(source == NULL) return OP_ERROR;
    int copyStatus = _insertionSortLib(source, input->blockSize);
    if((fclose(source) == EOF)){
        return OP_ERROR;
    } else return copyStatus;
}

int _insertionSortLib(FILE* source, int blockSize){
    long long fileLen = _getFiLeLengthAndRewindLib(source);
    if(fileLen == OP_ERROR) //|| fileLen % blockSize != 0) 
        return OP_ERROR;
    else return _runSortLib(source, blockSize, fileLen);
}

long long _getFiLeLengthAndRewindLib(FILE* source){
    if(fseek(source, 0, SEEK_END) != 0) return OP_ERROR;
    long long pos;
    if(fgetpos(source, (fpos_t *)&pos) == EOF) return OP_ERROR;
    if(fseek(source, 0, SEEK_END) != 0) return OP_ERROR;
    else return pos;
}

int _runSortLib(FILE* source, int blockSize, long long fileLen){
    unsigned char key[blockSize];
    bool errorFlag = false;
    for(long long i = 0; (i < fileLen / blockSize) && (errorFlag == false); i++){
        if((_seekAndReadLib(source, blockSize, i, key) == OP_ERROR) ||
           (_runInnerSortLoopLib(source, blockSize, key, i))){
            errorFlag = true;
            break;
        }
    }
    if(errorFlag == true) return OP_ERROR;
    else return OP_OK;
}

int _runInnerSortLoopLib(FILE* source, int blockSize, unsigned char* key, long long uppLoopIdx){
    bool errorFlag = false;
    unsigned char* copyPtr;
    unsigned char compElem[blockSize];
    for(long long j = uppLoopIdx - 1; (j >= 0); j--){
        if(_seekAndReadLib(source, blockSize, j, compElem) == OP_ERROR){
            errorFlag = true;
            break;
        }
        if(key[0] < compElem[0]) copyPtr = compElem;
        else copyPtr = key;
        if(fwrite(copyPtr, sizeof(unsigned char), blockSize, source) != blockSize){
            errorFlag = true;
            break;
        }
        if(copyPtr == key) break;
    }
    if(errorFlag == true) return OP_ERROR;
    else if(copyPtr != key){ // putting last element at begin -> no stop in inner loop
        return _putKeyAtBeginLib(source, blockSize, key);
    }
    return OP_OK;
}

int _putKeyAtBeginLib(FILE* source, int blockSize, unsigned char* key){
    if(fseek(source, 0, SEEK_SET) != 0 ||
       fwrite(key, sizeof(unsigned char), blockSize, source) != blockSize){
        return OP_ERROR;
    } else return OP_OK;
}

int _seekAndReadLib(FILE* source, int blockSize, int blockNumber, unsigned char* buffer){
    if((fseek(source, blockNumber * blockSize, SEEK_SET) != 0) ||
       (fread(buffer, sizeof(unsigned char), blockSize, source) != blockSize)){
        return OP_ERROR;
    } else {
        return OP_OK;
    }
}