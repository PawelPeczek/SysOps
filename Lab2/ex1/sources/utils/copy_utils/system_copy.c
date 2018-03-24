#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include "system_copy.h"

static const int OP_ERROR = -1;
static const int OP_OK = 0;

/*
*   "Private" function declaration
*/

int _copyData(int fileDscrp, int destDscrp, int blockSize);

/*
*   End of declarations
*/

int copyWithSystemCalls(ProgramInput* input){
    int sourceDscrp = open(input->baseFile, O_RDONLY);
    if(sourceDscrp < OP_OK)
        return OP_ERROR;
    int destDscrp = open(input->copyDestFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    if(destDscrp < OP_OK)
        return OP_ERROR;
    int copyStatus = _copyData(sourceDscrp, destDscrp, input->blockSize);
    if((close(sourceDscrp) == OP_ERROR) | (close(destDscrp) == OP_ERROR)) 
        return OP_ERROR;
    else 
        return copyStatus;
}

int _copyData(int sourceDscrp, int destDscrp, int blockSize){
    int readBytesNum;
    bool errorFlag = false;
    unsigned char buffer[blockSize];
    while((readBytesNum = read(sourceDscrp, buffer, blockSize)) > 0){
        if(write(destDscrp, buffer, readBytesNum) != readBytesNum){
            errorFlag = true;
            break;
        }
    }
    if(errorFlag == true) return OP_ERROR;
    else return OP_OK;
}