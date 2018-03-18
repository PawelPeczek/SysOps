#include <stdbool.h>
#include <string.h>
#include "dynamic_utils.h"
#include "../../headers/macros.h"
#include "shared_utils.h"
#ifndef DYNAMIC
#include "../../lib/blockalloc.h"
#else
#include "../main.h"
#endif

#include <stdio.h>
/*
* "Internal" functions declatarions
*/

int proceedDynamicOperations(DynamicBlockTable* blockTable, ProgramInput* input);
int proceedDynamicInsertQueryDelete(DynamicBlockTable* blockTable, int repeatNum);
int proceedDynamicBatchInsert(DynamicBlockTable* blockTable, int repeatNum);
int proceedDynamicBatchQuery(DynamicBlockTable* blockTable, int repeatNum);
int proceedDynamicBatchDelete(DynamicBlockTable* blockTable, int repeatNum);
int dispathDynamicOperation(DynamicBlockTable* blockTable, ProgramInput* input, int selOp);
int proceedDynamicSeqInsertDelete(DynamicBlockTable* blockTable, int repeatNum);
int proceedDynamicInsertDelete(DynamicBlockTable* blockTable, int repeatNum);

/*
* Functions definitions
*/

int proceedDynamicAllocTests(ProgramInput* input){
  do {
    struct timespec startRealTime;
    struct tms stTMS;
    clock_t stCT;
    clock_gettime(CLOCK_REALTIME, &startRealTime);
    stCT = times(&stTMS);
    DynamicBlockTable* blockTable =
      createDynamicBlockTable(input->numOfElements, input->blockSize);
    INVALID_REFERENCE(blockTable);
    printf("%ld", blockTable->numOfElements);
    printTimeStatsFromGivenTimePoint("DYNAMIC init", stCT, &startRealTime, &stTMS);
    ERROR_OP_STATUS(proceedDynamicOperations(blockTable, input));
    freeDynamicBlockTable(blockTable);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int proceedDynamicOperations(DynamicBlockTable* blockTable, ProgramInput* input){
  do {
    int opStatus = STATUS_ERROR;
    for(int i = 0; i < input->operationsNum; i++){
      opStatus = dispathDynamicOperation(blockTable, input, i);
      ERROR_OP_STATUS(opStatus);
    }
    ERROR_OP_STATUS(opStatus);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int dispathDynamicOperation(DynamicBlockTable* blockTable, ProgramInput* input, int selOp){
  int opStatus = STATUS_ERROR;
  // assuming validity of operations after being prevoiusly checked.
  struct timespec startRealTime;
  struct tms stTMS;
  clock_t stCT;
  clock_gettime(CLOCK_REALTIME, &startRealTime);
  stCT = times(&stTMS);
  if(strcmp("o1", input->operations[selOp]) == 0){
    opStatus = proceedDynamicInsertQueryDelete(blockTable, input->operationsParams[selOp]);
  } else if(strcmp("o2", input->operations[selOp]) == 0){
    opStatus = proceedDynamicSeqInsertDelete(blockTable, input->operationsParams[selOp]);
  } else if(strcmp("o3", input->operations[selOp]) == 0) {
    opStatus = proceedDynamicInsertDelete(blockTable, input->operationsParams[selOp]);
  }
  printTimeStatsFromGivenTimePoint(input->operations[selOp], stCT, &startRealTime, &stTMS);
  return opStatus;
}

int proceedDynamicInsertQueryDelete(DynamicBlockTable* blockTable, int repeatNum){
  do {
    CONDITION_FAILURE(repeatNum <= blockTable->numOfElements);
    ERROR_OP_STATUS(proceedDynamicBatchInsert(blockTable, repeatNum));
    ERROR_OP_STATUS(proceedDynamicBatchQuery(blockTable, repeatNum));
    ERROR_OP_STATUS(proceedDynamicBatchDelete(blockTable, repeatNum));
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int proceedDynamicSeqInsertDelete(DynamicBlockTable* blockTable, int repeatNum){
  do {
    int opStatus = STATUS_ERROR;
    // max valid len of block is blockTable->blockSize - 1 (buffer size +1 => '\0')
    char buffer[blockTable->blockSize];
    cleanBuffer(buffer, blockTable->blockSize);
    for(int i = 0; i < repeatNum; i++) {
      generateRandomValue(buffer, blockTable->blockSize);
      int translatedIdx = i % blockTable->numOfElements;
      opStatus = insertToDynamicBlockTable(blockTable, translatedIdx, buffer, strlen(buffer));
      ERROR_OP_STATUS(opStatus);
      opStatus = deleteSingleBlockFromDynamicTable(blockTable, translatedIdx);
      ERROR_OP_STATUS(opStatus);
      cleanBuffer(buffer, blockTable->blockSize);
    }
    ERROR_OP_STATUS(opStatus);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int proceedDynamicInsertDelete(DynamicBlockTable* blockTable, int repeatNum){
  do {
    CONDITION_FAILURE(repeatNum <= blockTable->numOfElements);
    ERROR_OP_STATUS(proceedDynamicBatchInsert(blockTable, repeatNum));
    ERROR_OP_STATUS(proceedDynamicBatchDelete(blockTable, repeatNum));
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int proceedDynamicBatchInsert(DynamicBlockTable* blockTable, int repeatNum){
  do {
    int opStatus = STATUS_ERROR;
    // max valid len of block is blockTable->blockSize - 1 (buffer size +1 => '\0')
    char buffer[blockTable->blockSize];
    cleanBuffer(buffer, blockTable->blockSize);
    for(int i = 0; i < repeatNum; i++) {
      generateRandomValue(buffer, blockTable->blockSize);
      opStatus = insertToDynamicBlockTable(blockTable, i, buffer, strlen(buffer));
      ERROR_OP_STATUS(opStatus);
      cleanBuffer(buffer, blockTable->blockSize);
    }
    ERROR_OP_STATUS(opStatus);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int proceedDynamicBatchQuery(DynamicBlockTable* blockTable, int repeatNum){
  do {
    const char* qResult = NULL;
    // max valid len of block is blockTable->blockSize - 1 (buffer size +1 => '\0')
    char buffer[blockTable->blockSize];
    cleanBuffer(buffer, blockTable->blockSize);
    for(int i = 0; i < repeatNum; i++) {
      generateRandomValue(buffer, blockTable->blockSize);
      qResult = getClosestASCIISumValueFromDynamicBlockTable(blockTable, buffer, strlen(buffer));
      INVALID_REFERENCE(qResult);
      //printf("%s\n", qResult);
      cleanBuffer(buffer, blockTable->blockSize);
    }
    INVALID_REFERENCE(qResult);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int proceedDynamicBatchDelete(DynamicBlockTable* blockTable, int repeatNum){
  do {
    int opStatus = STATUS_ERROR;
    for(int i = 0; i < repeatNum; i++) {
      opStatus = deleteSingleBlockFromDynamicTable(blockTable, i);
      ERROR_OP_STATUS(opStatus);
    }
    ERROR_OP_STATUS(opStatus);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}
