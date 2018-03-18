#include <stdbool.h>
#include <string.h>
#include "static_utils.h"
#include "../../headers/macros.h"
#include "shared_utils.h"
#ifndef DYNAMIC
#include "../../lib/blockalloc.h"
#else
#include "../main.h"
#endif


/*
* "Internal" functions declatarions
*/

int proceedStaticOperations(StaticLikeBlockTable* blockTable, ProgramInput* input);
int proceedStaticInsertQueryDelete(StaticLikeBlockTable* blockTable, int repeatNum);
int proceedStaticBatchInsert(StaticLikeBlockTable* blockTable, int repeatNum);
int proceedStaticBatchQuery(StaticLikeBlockTable* blockTable, int repeatNum);
int proceedStaticBatchDelete(StaticLikeBlockTable* blockTable, int repeatNum);
int dispathStaticOperation(StaticLikeBlockTable* blockTable, ProgramInput* input, int selOp);
int proceedStaticSeqInsertDelete(StaticLikeBlockTable* blockTable, int repeatNum);
int proceedStaticInsertDelete(StaticLikeBlockTable* blockTable, int repeatNum);

/*
* Functions definitions
*/

int proceedStaticAllocTests(ProgramInput* input){
    do {
      struct timespec startRealTime;
      struct tms stTMS;
      clock_t stCT;
      clock_gettime(CLOCK_REALTIME, &startRealTime);
      stCT = times(&stTMS);
      StaticLikeBlockTable* blockTable =
        createStaticLikeBlockTable(input->numOfElements, input->blockSize);
      INVALID_REFERENCE(blockTable);
      printTimeStatsFromGivenTimePoint("STATIC init", stCT, &startRealTime, &stTMS);
      ERROR_OP_STATUS(proceedStaticOperations(blockTable, input));
      freeStaticLikeBlockTable(blockTable);
      return STATUS_OK;
    } while(true);
    return STATUS_ERROR;
}

int proceedStaticOperations(StaticLikeBlockTable* blockTable, ProgramInput* input){
  do {
    int opStatus = STATUS_ERROR;
    for(int i = 0; i < input->operationsNum; i++){
      opStatus = dispathStaticOperation(blockTable, input, i);
      ERROR_OP_STATUS(opStatus);
    }
    ERROR_OP_STATUS(opStatus);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int dispathStaticOperation(StaticLikeBlockTable* blockTable, ProgramInput* input, int selOp){
  int opStatus = STATUS_ERROR;
  // assuming validity of operations after being prevoiusly checked.
  struct timespec startRealTime;
  struct tms stTMS;
  clock_t stCT;
  clock_gettime(CLOCK_REALTIME, &startRealTime);
  stCT = times(&stTMS);
  if(strcmp("o1", input->operations[selOp]) == 0){
    opStatus = proceedStaticInsertQueryDelete(blockTable, input->operationsParams[selOp]);
    //printf("opStatus: %i\n", opStatus);
  } else if(strcmp("o2", input->operations[selOp]) == 0){
    opStatus = proceedStaticSeqInsertDelete(blockTable, input->operationsParams[selOp]);
  } else if(strcmp("o3", input->operations[selOp]) == 0) {
    opStatus = proceedStaticInsertDelete(blockTable, input->operationsParams[selOp]);
  }
  printTimeStatsFromGivenTimePoint(input->operations[selOp], stCT, &startRealTime, &stTMS);
  return opStatus;
}

int proceedStaticInsertQueryDelete(StaticLikeBlockTable* blockTable, int repeatNum){
  do {
    //printf("proceedStaticInsertQueryDelete()\n");
    CONDITION_FAILURE(repeatNum <= blockTable->numOfElements);
    //printf("Cond ok\n");
    ERROR_OP_STATUS(proceedStaticBatchInsert(blockTable, repeatNum));
    //printf("Batch insert ok\n");
    ERROR_OP_STATUS(proceedStaticBatchQuery(blockTable, repeatNum));
    //printf("Batch query ok\n");
    ERROR_OP_STATUS(proceedStaticBatchDelete(blockTable, repeatNum));
    //printf("Batch delete ok\n");
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int proceedStaticSeqInsertDelete(StaticLikeBlockTable* blockTable, int repeatNum){
  do {
    int opStatus = STATUS_ERROR;
    // max valid len of block is blockTable->blockSize - 1 (buffer size +1 => '\0')
    char buffer[blockTable->blockSize];
    cleanBuffer(buffer, blockTable->blockSize);
    for(int i = 0; i < repeatNum; i++) {
      generateRandomValue(buffer, blockTable->blockSize);
      int translatedIdx = i % blockTable->numOfElements;
      opStatus = insertToStaticLikeBlockTable(blockTable, translatedIdx, buffer, strlen(buffer));
      ERROR_OP_STATUS(opStatus);
      opStatus = deleteSingleBlockFromStaticLikeTable(blockTable, translatedIdx);
      ERROR_OP_STATUS(opStatus);
      cleanBuffer(buffer, blockTable->blockSize);
    }
    ERROR_OP_STATUS(opStatus);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int proceedStaticInsertDelete(StaticLikeBlockTable* blockTable, int repeatNum){
  do {
    CONDITION_FAILURE(repeatNum <= blockTable->numOfElements);
    ERROR_OP_STATUS(proceedStaticBatchInsert(blockTable, repeatNum));
    ERROR_OP_STATUS(proceedStaticBatchDelete(blockTable, repeatNum));
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int proceedStaticBatchInsert(StaticLikeBlockTable* blockTable, int repeatNum){
  do {
    //printf("Batch insert()\n");
    int opStatus = STATUS_ERROR;
    // max valid len of block is blockTable->blockSize - 1 (buffer size +1 => '\0')
    char buffer[blockTable->blockSize];
    cleanBuffer(buffer, blockTable->blockSize);
    for(int i = 0; i < repeatNum; i++) {
      generateRandomValue(buffer, blockTable->blockSize);
      opStatus = insertToStaticLikeBlockTable(blockTable, i, buffer, strlen(buffer));
      ERROR_OP_STATUS(opStatus);
      cleanBuffer(buffer, blockTable->blockSize);
    }
    //printf("After loop\n");
    ERROR_OP_STATUS(opStatus);
    //printf("OP status ok!\n");
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int proceedStaticBatchQuery(StaticLikeBlockTable* blockTable, int repeatNum){
  do {
    const char* qResult = NULL;
    // max valid len of block is blockTable->blockSize - 1 (buffer size +1 => '\0')
    char buffer[blockTable->blockSize];
    cleanBuffer(buffer, blockTable->blockSize);
    for(int i = 0; i < repeatNum; i++) {
      generateRandomValue(buffer, blockTable->blockSize);
      qResult = getClosestASCIISumValueFromStaticLikeBlockTable(blockTable, buffer, strlen(buffer));
      INVALID_REFERENCE(qResult);
      cleanBuffer(buffer, blockTable->blockSize);
    }
    INVALID_REFERENCE(qResult);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int proceedStaticBatchDelete(StaticLikeBlockTable* blockTable, int repeatNum){
  do {
    int opStatus = STATUS_ERROR;
    for(int i = 0; i < repeatNum; i++) {
      opStatus = deleteSingleBlockFromStaticLikeTable(blockTable, i);
      ERROR_OP_STATUS(opStatus);
    }
    ERROR_OP_STATUS(opStatus);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}
