#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>
#include "../../headers/macros.h"
#include "blockalloc_dynamic.h"
#include "blockalloc_shared.h"

/*
* "Internal" functions declarations for dynamic table
*/
int _initializeDynamicBlockTable(DynamicBlockTable*, size_t, size_t);
bool _isValidIndexDynamic(DynamicBlockTable*, int);
bool _isFreeIndexDynamic(DynamicBlockTable*, int);
int _checkDynamicBlockTableInitialization(DynamicBlockTable*, bool);
const char* _findBlockWithCloseToGivenASCIISum(DynamicBlockTable*, int);

/*
* "User" functions definitions for dynamic table
*/
DynamicBlockTable* createDynamicBlockTable(size_t numOfElements, size_t blockSize){
  DynamicBlockTable* blockTable = NULL;
  do {
    //printf("IN createDinamicBlockTable()\n");
    int opStatus = _checkSizeArguments(numOfElements, blockSize);
    //printf("OperationStatus - %d\n", opStatus);
    ERROR_OP_STATUS(opStatus);
    blockTable = (DynamicBlockTable*)calloc(1, sizeof(DynamicBlockTable));
    NOT_NULL(blockTable);
    //printf("MEM ALLOCATED!\n");
    opStatus = _initializeDynamicBlockTable(blockTable, numOfElements, blockSize);
    //printf("OperationStatus - %d\n", opStatus);
    ERROR_OP_STATUS(opStatus);
    return blockTable;
  } while(true);
  freeDynamicBlockTable(blockTable);
  return NULL;
}

void freeDynamicBlockTable(DynamicBlockTable* blockTable){
  if(blockTable == NULL) return;
  if(blockTable->internalBlockTable != NULL){
    for(int i = 0; i < blockTable->numOfElements; i++){
      free(blockTable->internalBlockTable[i]);
    }
  }
  free(blockTable->internalBlockTable);
  free(blockTable->sumOfASCIIChars);
  free(blockTable);
}

int deleteSingleBlockFromDynamicTable(DynamicBlockTable* blockTable, int index){
  do {
    int opStatus = _checkDynamicBlockTableInitialization(blockTable, false);
    ERROR_OP_STATUS(opStatus);
    CONDITION_FAILURE(_isValidIndexDynamic(blockTable, index));
    INVALID_REFERENCE(blockTable->internalBlockTable[index]);
    free(blockTable->internalBlockTable[index]);
    blockTable->internalBlockTable[index] = NULL;
    blockTable->sumOfASCIIChars[index] = 0;
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int insertToDynamicBlockTable
(DynamicBlockTable* blockTable, int index, const char* value, size_t len){
  char* allocatedBlock = NULL;
  do {
    //printf("IN insertToDynamicBlockTable()\n");
    int opStatus = _checkDynamicBlockTableInitialization(blockTable, false);
    //printf("OperationStatus - %d\n", opStatus);
    ERROR_OP_STATUS(opStatus);
    CONDITION_FAILURE(len < blockTable->blockSize);
    //printf("Condition OK\n");
    CONDITION_FAILURE(_isFreeIndexDynamic(blockTable, index));
    //printf("Condition OK\n");
    char* allocatedBlock = (char*)calloc(blockTable->blockSize, sizeof(char));
    NOT_NULL(allocatedBlock);
    //printf("Block allocated!\n");
    blockTable->internalBlockTable[index] = allocatedBlock;
    char* result = strncpy(blockTable->internalBlockTable[index], value, len);
    INVALID_REFERENCE(result);
    blockTable->sumOfASCIIChars[index] = _sumOfASCIICodes(value, len);
    return STATUS_OK;
  } while(true);
  // safety memory deallocation
  if(allocatedBlock != NULL) free(allocatedBlock);
  return STATUS_ERROR;
}

const char* getClosestASCIISumValueFromDynamicBlockTable
(DynamicBlockTable* blockTable, const char* value, size_t len){
  do {
    int opStatus = _checkDynamicBlockTableInitialization(blockTable, false);
    ERROR_OP_STATUS(opStatus);
    INVALID_SIZE(len);
    NOT_NULL(value);
    int originalValASCIISum = _sumOfASCIICodes(value, len);
    return _findBlockWithCloseToGivenASCIISum(blockTable, originalValASCIISum);
  } while(true);
  return NULL;
}

/*
* "Internal" functions area for dynamic table
*/

int _initializeDynamicBlockTable
(DynamicBlockTable* blockTable, size_t numOfElements, size_t blockSize){
  do {
    //printf("_initializeDynamicBlockTable()\n");
    INVALID_REFERENCE(blockTable);
    int opStatus = _checkSizeArguments(numOfElements, blockSize);
    ERROR_OP_STATUS(opStatus);
    blockTable->internalBlockTable = (char**)calloc(numOfElements, sizeof(char*));
    blockTable->sumOfASCIIChars = (int*)calloc(numOfElements, sizeof(int));
    blockTable->numOfElements = numOfElements;
    // Safety margin for "\n" in case of user truing to assign sequence of chars
    // without \0 at the end.
    blockTable->blockSize = blockSize + 1;
    opStatus = _checkDynamicBlockTableInitialization(blockTable, true);
    ERROR_OP_STATUS(opStatus);
    return STATUS_OK;
  } while(true);
  // free of allocated memory in case of error
  free(blockTable->internalBlockTable);
  free(blockTable->sumOfASCIIChars);
  return STATUS_ERROR;
}

int _checkDynamicBlockTableInitialization(DynamicBlockTable* blockTable, bool init){
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

bool _isValidIndexDynamic(DynamicBlockTable* blockTable, int index){
  do {
    int opStatus = _checkDynamicBlockTableInitialization(blockTable, false);
    ERROR_OP_STATUS(opStatus);
    CONDITION_FAILURE(index >= 0 && index < blockTable->numOfElements);
    return true;
  } while(true);
  return false;
}

bool _isFreeIndexDynamic(DynamicBlockTable* blockTable, int index){
  do {
    //printf("_isFreeIndexDynamic()\n");
    int opStatus = _checkDynamicBlockTableInitialization(blockTable, false);
    //printf("OperationStatus - %d\n", opStatus);
    ERROR_OP_STATUS(opStatus);
    CONDITION_FAILURE(_isValidIndexDynamic(blockTable, index));
    return (blockTable->internalBlockTable[index] == NULL);
  } while(true);
  return false;
}

const char* _findBlockWithCloseToGivenASCIISum(DynamicBlockTable* blockTable, int ASCIISum){
  do {
    //printf("_findBlockWithCloseToGivenASCIISum()\n");
    int opStatus = _checkDynamicBlockTableInitialization(blockTable, false);
    //printf("OperationStatus - %d\n", opStatus);
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
      foundElement = blockTable->internalBlockTable[idxOfClosestValue];
    }
    return foundElement;
  } while(true);
  return NULL;
}
