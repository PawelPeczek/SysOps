#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "system_generate.h"

static const int OP_ERROR = -1;
static const int OP_OK = 0;


/*
*   "Private" function declaration
*/

int _fulfillDestFile(int fileDscrp, int blockSize, int numOfRecords);
void _generateRandomBuffer(unsigned char* buffer, int blockSize);

/*
*   End of declarations
*/


int generateWithSystemCalls(ProgramInput* input){
    int fileDscrp = open(input->baseFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    if(fileDscrp < OP_OK)
        return OP_ERROR;
    int generateStatus = _fulfillDestFile(fileDscrp, input->blockSize, input->numOfRecords);
    if(close(fileDscrp) == OP_ERROR) return OP_ERROR;
    else return generateStatus;
}

int _fulfillDestFile(int fileDscrp, int blockSize, int numOfRecords){
    unsigned char buffer[blockSize];
    bool errorFlag = false;
    srand(time(NULL));
    for(int i = 0; i < numOfRecords; i++){
        _generateRandomBuffer(buffer, blockSize);
        if(write(fileDscrp, buffer, blockSize) != blockSize){
            errorFlag = true;
            break;
        }
    }
    if(errorFlag == true) return OP_ERROR;
    else return OP_OK;
}

void _generateRandomBuffer(unsigned char* buffer, int blockSize){    
    for(int i = 0; i < blockSize; i++){
        buffer[i] = 'A' + (rand() % ('z' - 'A'));
    }
}