#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include "system_sort.h"

static const int OP_ERROR = -1;
static const int OP_OK = 0;

/*
*   "Private" function declaration
*/

int _insertionSort(int fileDscrp, int blockSize);
int _runSort(int fileDscrp, int blockSize, long fileLen);
int _runInnerSortLoop(int fileDscrp, int blockSize, unsigned char* key, long uppLoopIdx);
long _getFiLeLengthAndRewind(int fileDscrp);
int _seekAndRead(int fileDscrp, int blockSize, int blockNumber, unsigned char* buffer);
int _putKeyAtBegin(int fileDscrp, int blockSize, unsigned char* key);

/*
*   End of declarations
*/

int sortWithSystemCalls(ProgramInput* input){
    int fileDscrp = open(input->baseFile, O_RDWR);
    if(fileDscrp < OP_OK)
        return OP_ERROR;
    int sortStatus = _insertionSort(fileDscrp, input->blockSize);
    if(close(fileDscrp) == OP_ERROR) return OP_ERROR;
    else return sortStatus;
}

int _insertionSort(int fileDscrp, int blockSize){
    long fileLen = _getFiLeLengthAndRewind(fileDscrp);
    if(fileLen == OP_ERROR || fileLen % blockSize != 0) 
        return OP_ERROR;
    else 
        return _runSort(fileDscrp, blockSize, fileLen);
}

long _getFiLeLengthAndRewind(int fileDscrp){
    int fileLen = lseek(fileDscrp, 0, SEEK_END);
    if(lseek(fileDscrp, 0, SEEK_SET) != 0) return OP_ERROR;
    else return fileLen;
}

int _runSort(int fileDscrp, int blockSize, long fileLen){
    unsigned char key[blockSize];
    bool errorFlag = false;
    for(long i = 1; (i < (fileLen / blockSize)) && (errorFlag == false); i++){
        if((_seekAndRead(fileDscrp, blockSize, i, key) == OP_ERROR) ||
           (_runInnerSortLoop(fileDscrp, blockSize, key, i) == OP_ERROR)){
            errorFlag = true;
            break;
        }
    }
    if(errorFlag == true) return OP_ERROR;
    else return OP_OK;
}

int _runInnerSortLoop(int fileDscrp, int blockSize, unsigned char* key, long uppLoopIdx){
    bool errorFlag = false;
    unsigned char* copyPtr;
    unsigned char compElem[blockSize];
    for(long j = uppLoopIdx - 1; (j >= 0); j--){
        if(_seekAndRead(fileDscrp, blockSize, j, compElem) == OP_ERROR){
            errorFlag = true;
            break;
        }
        if(key[0] < compElem[0]) copyPtr = compElem;
        else copyPtr = key;
        if(write(fileDscrp, copyPtr, blockSize) != blockSize){
            errorFlag = true;
            break;
        }
        if(copyPtr == key) break;
    }
    if(errorFlag == true) return OP_ERROR;
    else if(copyPtr != key){ // putting last element at begin -> no stop in inner loop
        return _putKeyAtBegin(fileDscrp, blockSize, key);
    }
    return OP_OK;
}

int _putKeyAtBegin(int fileDscrp, int blockSize, unsigned char* key){
    if(lseek(fileDscrp, 0, SEEK_SET) != 0 ||
       write(fileDscrp, key, blockSize) != blockSize){
        return OP_ERROR;
    } else return OP_OK;
}

int _seekAndRead(int fileDscrp, int blockSize, int blockNumber, unsigned char* buffer){
    if((lseek(fileDscrp, blockNumber * blockSize, SEEK_SET) < 0) ||
       (read(fileDscrp, buffer, blockSize) != blockSize)){
        return OP_ERROR;
    } else {
        return OP_OK;
    }
}
