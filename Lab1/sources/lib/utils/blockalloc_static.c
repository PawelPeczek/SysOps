#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>
#include "../../headers/macros.h"
#include "blockalloc_static.h"
#include "blockalloc_shared.h"

/*
* "Internal" functions declarations for static table
*/

int _initializeStaticLikeBlockTable(StaticLikeBlockTable*, size_t, size_t);
bool _isValidIndexStatic(StaticLikeBlockTable*, int);
bool _isFreeIndexStatic(StaticLikeBlockTable*, int);
int _checkStaticLikeBlockTableInitialization(StaticLikeBlockTable*, bool);
const char* _findStaticLikeBlockWithCloseToGivenASCIISum(StaticLikeBlockTable*, int);


/*
* "User" functions definitions for static like table
*/

StaticLikeBlockTable* createStaticLikeBlockTable(size_t numOfElements, size_t blockSize){
  StaticLikeBlockTable* blockTable  = NULL;
  do {
    int opStatus = _checkSizeArguments(numOfElements, blockSize);
    ERROR_OP_STATUS(opStatus);
    blockTable = (StaticLikeBlockTable*)calloc(1, sizeof(StaticLikeBlockTable));
    NOT_NULL(blockTable);
    opStatus = _initializeStaticLikeBlockTable(blockTable, numOfElements, blockSize);
    ERROR_OP_STATUS(opStatus);
    return blockTable;
  } while(true);
  freeStaticLikeBlockTable(blockTable);
  return NULL;
}

void freeStaticLikeBlockTable(StaticLikeBlockTable* blockTable){
  if(blockTable == NULL) return;
  free(blockTable->internalBlockTable);
  free(blockTable->sumOfASCIIChars);
  free(blockTable);
}

int deleteSingleBlockFromStaticLikeTable(StaticLikeBlockTable* blockTable, int index){
  do {
    int opStatus = _checkStaticLikeBlockTableInitialization(blockTable, false);
    ERROR_OP_STATUS(opStatus);
    CONDITION_FAILURE(_isValidIndexStatic(blockTable, index));
    char* iterator = blockTable->internalBlockTable + index * blockTable->blockSize;
    for(int i = 0; i < blockTable->blockSize; i++, iterator++){
      *iterator = '\0';
    }
    blockTable->sumOfASCIIChars[index] = 0;
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int insertToStaticLikeBlockTable
(StaticLikeBlockTable* blockTable, int index, const char* value, size_t len){
  char* result = NULL;
  do {
    int opStatus = _checkStaticLikeBlockTableInitialization(blockTable, false);
    ERROR_OP_STATUS(opStatus);
    CONDITION_FAILURE(len < blockTable->blockSize);
    CONDITION_FAILURE(_isFreeIndexStatic(blockTable, index));
    char* startPtr = blockTable->internalBlockTable + index * blockTable->blockSize;
    result = strncpy(startPtr, value, len);
    INVALID_REFERENCE(result);
    blockTable->sumOfASCIIChars[index] = _sumOfASCIICodes(value, len);
    return STATUS_OK;
  } while(true);
  // saftey memory clean
  deleteSingleBlockFromStaticLikeTable(blockTable, index);
  return STATUS_ERROR;
}

const char* getClosestASCIISumValueFromStaticLikeBlockTable
(StaticLikeBlockTable* blockTable, const char* value, size_t len){
  do {
    int opStatus = _checkStaticLikeBlockTableInitialization(blockTable, false);
    ERROR_OP_STATUS(opStatus);
    INVALID_SIZE(len);
    NOT_NULL(value);
    int originalValASCIISum = _sumOfASCIICodes(value, len);
    return _findStaticLikeBlockWithCloseToGivenASCIISum(blockTable, originalValASCIISum);
  } while(true);
  return NULL;
}

/*
* "Internal" functions area for static like allocation type
*/

int _initializeStaticLikeBlockTable
(StaticLikeBlockTable* blockTable, size_t numOfElements, size_t blockSize){
  do {
    INVALID_REFERENCE(blockTable);
    int opStatus = _checkSizeArguments(numOfElements, blockSize);
    ERROR_OP_STATUS(opStatus);
    blockTable->internalBlockTable = (char*)calloc(numOfElements * (blockSize + 1), sizeof(char));
    blockTable->sumOfASCIIChars = (int*)calloc(numOfElements, sizeof(int));
    blockTable->numOfElements = numOfElements;
    // Safety margin for "\n" in case of user truing to assign sequence of chars
    // without \0 at the end.
    blockTable->blockSize = blockSize + 1;
    opStatus = _checkStaticLikeBlockTableInitialization(blockTable, true);
    ERROR_OP_STATUS(opStatus);
    return STATUS_OK;
  } while(true);
  free(blockTable->internalBlockTable);
  free(blockTable->sumOfASCIIChars);
  return STATUS_ERROR;
}

bool _isValidIndexStatic(StaticLikeBlockTable* blockTable, int index){
  do {
    int opStatus = _checkStaticLikeBlockTableInitialization(blockTable, false);
    ERROR_OP_STATUS(opStatus);
    CONDITION_FAILURE(index >= 0 && index < blockTable->numOfElements);
    return true;
  } while(true);
  return false;
}

bool _isFreeIndexStatic(StaticLikeBlockTable* blockTable, int index){
  do {
    int opStatus = _checkStaticLikeBlockTableInitialization(blockTable, false);
    ERROR_OP_STATUS(opStatus);
    CONDITION_FAILURE(_isValidIndexStatic(blockTable, index));
    char* blockStartPointer = blockTable->internalBlockTable + index * blockTable->blockSize;
    return (*blockStartPointer == '\0');
  } while(true);
  return false;
}

int _checkStaticLikeBlockTableInitialization(StaticLikeBlockTable* blockTable, bool init){
  do {
    INVALID_REFERENCE(blockTable);
    if (init){
      NOT_NULL(blockTable->internalBlockTable);
      NOT_NULL(blockTable->sumOfASCIIChars);
    } else {
      INVALID_REFERENCE(blockTable->internalBlockTable);
      INVALID_REFERENCE(blockTable->sumOfASCIIChars);
    }
    INVALID_SIZE(blockTable->blockSize);
    INVALID_SIZE(blockTable->numOfElements);
    return STATUS_OK;
  } while (true);
  return STATUS_ERROR;
}

const char* _findStaticLikeBlockWithCloseToGivenASCIISum
(StaticLikeBlockTable* blockTable, int ASCIISum){
  do {
    int opStatus = _checkStaticLikeBlockTableInitialization(blockTable, false);
    ERROR_OP_STATUS(opStatus);
    const char* foundElement = NULL;
    int closestDistance = INT_MAX;
    int idxOfClosestValue = -1;
    for(int i = 0; i < blockTable->numOfElements; i++){
      if(_numbersDistSmallerThanPrev(ASCIISum, blockTable->sumOfASCIIChars[i], closestDistance)){
        idxOfClosestValue = i;
        closestDistance = _numDistance(ASCIISum, blockTable->sumOfASCIIChars[i]);
      }
    }
    if(idxOfClosestValue != -1){
      foundElement = blockTable->internalBlockTable + blockTable->blockSize * idxOfClosestValue;
    }
    return foundElement;
  } while(true);
  return NULL;
}
