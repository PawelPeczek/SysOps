#include <stdio.h>
#include <stdbool.h>
#include "lib_copy.h"

static const int OP_ERROR = -1;
static const int OP_OK = 0;

/*
*   "Private" function declaration
*/

int _copyDataLib(FILE* source, FILE* dest, int blockSize);

/*
*   End of declarations
*/


int copyWithLibCalls(ProgramInput* input){
    FILE* source = fopen(input->baseFile, "rb");
    if(source == NULL) return OP_ERROR;
    FILE *dest = fopen(input->copyDestFile, "wb");
    if(dest == NULL) return OP_ERROR;
    int copyStatus = _copyDataLib(source, dest, input->blockSize);
    if((fclose(source) == EOF) | (fclose(dest) == EOF)){
        return OP_ERROR;
    } else return copyStatus;
}

int _copyDataLib(FILE* source, FILE* dest, int blockSize){
    unsigned char buffer[blockSize];
    int readBytesNum;
    bool errorFlag = false;
    while((readBytesNum = fread(buffer, sizeof(unsigned char), blockSize, source)) > 0){
        if(fwrite(buffer, sizeof(unsigned char), readBytesNum, dest) != readBytesNum){
            errorFlag = true;
            break;
        }
    }
    if(errorFlag == true) return OP_ERROR;
    else return OP_OK;
}