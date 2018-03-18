#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <sys/times.h>
#include "../headers/contract.h"
#include "../headers/macros.h"
#include "../headers/program_input.h"
#include "./utils/static_utils.h"
#include "./utils/dynamic_utils.h"

#ifndef DYNAMIC
  #include "../lib/blockalloc.h"
#else
  #include <dlfcn.h>
#endif

extern int errno;

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[41m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define MIN_NUM_PARAMS 6

/*
* Functions declarations
*/

bool operationNameValid(char* argv[], int idx);
int preprocessOperations(ProgramInput* input, char* argv[]);
void freeInput(ProgramInput* input);
ProgramInput* preprocessProgramInput(int argc, char* argv[]);
void printHelp();
/*
* Functions definitions
*/
//
#ifdef DYNAMIC

static void *handle;

DynamicBlockTable* (*createDynamicBlockTable)(size_t, size_t);
void (*freeDynamicBlockTable)(DynamicBlockTable*);
int (*deleteSingleBlockFromDynamicTable)(DynamicBlockTable*, int);
int (*insertToDynamicBlockTable)(DynamicBlockTable*, int, const char*, size_t);
const char* (*getClosestASCIISumValueFromDynamicBlockTable)(DynamicBlockTable*, const char*, size_t);

StaticLikeBlockTable* (*createStaticLikeBlockTable)(size_t, size_t);
void (*freeStaticLikeBlockTable)(StaticLikeBlockTable*);
int (*deleteSingleBlockFromStaticLikeTable)(StaticLikeBlockTable*, int);
int (*insertToStaticLikeBlockTable)(StaticLikeBlockTable*, int, const char*, size_t);
const char* (*getClosestASCIISumValueFromStaticLikeBlockTable)(StaticLikeBlockTable*, const char*, size_t);

#endif

#ifdef DYNAMIC

int loadDynamicLibrary(){
  do {
    printf("DYNAMIC MODE\n");
    handle = dlopen("./bin/libblockalloc.so", RTLD_LAZY);
    if(handle == NULL) handle = dlopen("./libblockalloc.so", RTLD_LAZY);
    INVALID_REFERENCE(handle);
    createDynamicBlockTable =
      (DynamicBlockTable* (*)(size_t, size_t))dlsym(handle, "createDynamicBlockTable");
    CONDITION_FAILURE(dlerror() == NULL);
    freeDynamicBlockTable = (void (*)(DynamicBlockTable*))dlsym(handle, "freeDynamicBlockTable");
    CONDITION_FAILURE(dlerror() == NULL);
    deleteSingleBlockFromDynamicTable =
      (int (*)(DynamicBlockTable*, int))dlsym(handle, "deleteSingleBlockFromDynamicTable");
    CONDITION_FAILURE(dlerror() == NULL);
    insertToDynamicBlockTable =
      (int (*)(DynamicBlockTable*, int, const char*, size_t))dlsym(handle, "insertToDynamicBlockTable");
    CONDITION_FAILURE(dlerror() == NULL);
    getClosestASCIISumValueFromDynamicBlockTable =
      (const char* (*)(DynamicBlockTable*, const char*, size_t))dlsym(handle, "getClosestASCIISumValueFromDynamicBlockTable");
    CONDITION_FAILURE(dlerror() == NULL);

    createStaticLikeBlockTable =
      (StaticLikeBlockTable* (*)(size_t, size_t))dlsym(handle, "createStaticLikeBlockTable");
    CONDITION_FAILURE(dlerror() == NULL);
    freeStaticLikeBlockTable = (void (*)(StaticLikeBlockTable*))dlsym(handle, "freeStaticLikeBlockTable");
    CONDITION_FAILURE(dlerror() == NULL);
    deleteSingleBlockFromStaticLikeTable =
      (int (*)(StaticLikeBlockTable*, int))dlsym(handle, "deleteSingleBlockFromStaticLikeTable");
    CONDITION_FAILURE(dlerror() == NULL);
    insertToStaticLikeBlockTable =
      (int (*)(StaticLikeBlockTable*, int, const char*, size_t))dlsym(handle, "insertToStaticLikeBlockTable");
    CONDITION_FAILURE(dlerror() == NULL);
    getClosestASCIISumValueFromStaticLikeBlockTable =
      (const char* (*)(StaticLikeBlockTable*, const char*, size_t))dlsym(handle, "getClosestASCIISumValueFromStaticLikeBlockTable");
    CONDITION_FAILURE(dlerror() == NULL);

    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

#endif

int main(int argc, char* argv[]){
  int opStatus;

  #ifdef DYNAMIC
    opStatus = loadDynamicLibrary();
    if(opStatus == STATUS_ERROR){
      printf("Error loading library.\n");
      exit(3);
    }
  #endif

  ProgramInput* input = preprocessProgramInput(argc, argv);
  if(input == NULL){
    printHelp();
    exit(1);
  }
  if(input->allocMode == 's') opStatus = proceedStaticAllocTests(input);
  else opStatus = proceedDynamicAllocTests(input);
  if(opStatus == STATUS_ERROR){
    perror("Error while performing tests");
    free(input);
    exit(2);
  }

  #ifdef DYNAMIC
  dlclose(handle);
  #endif
  free(input);
  return 0;
}

ProgramInput* preprocessProgramInput(int argc, char* argv[]){
  ProgramInput* input = NULL;
  do {
    input = (ProgramInput*)calloc(1, sizeof(ProgramInput));
    NOT_NULL(input);
    CONDITION_FAILURE(argc >= MIN_NUM_PARAMS);
    input->numOfElements = atoi(argv[1]);
    CONDITION_FAILURE(input->numOfElements > 0);
    input->blockSize = atoi(argv[2]);
    CONDITION_FAILURE(input->blockSize > 0);
    CONDITION_FAILURE(strlen(argv[3]) == 1);
    input->allocMode = argv[3][0];
    CONDITION_FAILURE(input->allocMode == 's' || input->allocMode == 'd');
    int operationsParamNumber = argc + 2 - MIN_NUM_PARAMS;
    CONDITION_FAILURE(operationsParamNumber % 2 == 0);
    input->operationsNum = operationsParamNumber / 2;
    input->operations = (char**)calloc(input->operationsNum, sizeof(char*));
    NOT_NULL(input->operations);
    input->operationsParams = (int*)calloc(input->operationsNum, sizeof(int));
    NOT_NULL(input->operationsParams);
    ERROR_OP_STATUS(preprocessOperations(input, argv));
    return input;
  } while(true);
  freeInput(input);
  return NULL;
}

int preprocessOperations(ProgramInput* input, char* argv[]){
  do {
    bool errParseFlag = false;
    for(int i = 0; i < input->operationsNum; i++){
      int orgArgvIdx = 4 + (2*i);
      if(!operationNameValid(argv, orgArgvIdx)) errParseFlag = true;
      // safe to do that cause main scope are left at the end
      input->operations[i] = argv[orgArgvIdx];
      input->operationsParams[i] = atoi(argv[orgArgvIdx + 1]);
      if(input->operationsParams[i] < 1) errParseFlag = true;
      if(errParseFlag) break;
    }
    CONDITION_FAILURE(errParseFlag == false);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

void freeInput(ProgramInput* input){
  if(input == NULL) return;
  if(input->operationsParams != NULL) free(input->operationsParams);
  if(input->operations != NULL) free(input->operations);
  free(input);
}

bool operationNameValid(char* argv[], int idx){
  bool result = true;
  if(strlen(argv[idx]) == 2 && argv[idx][0] == 'o'){
    if(argv[idx][1] != '1' && argv[idx][1] != '2' && argv[idx][1] != '3'){
      result = false;
    }
  } else {
    result = false;
  }
  return result;
}

void printHelp(){
  printf("%sWelcome to TEST program.%s\n", KGRN, KNRM);
  printf("Valid program args: prog_name %snum_of_elems%s %ssize_of_block%s %salloc_mode%s ",
         KYEL, KNRM,  KBLU, KNRM, KCYN, KNRM);
  printf("%s[operations]%s\n", KMAG, KNRM);
  printf("%salloc_mode%s can take value s (static) or d (dynamic)\n\n",  KCYN, KNRM);
  printf("%sAllowed operations:%s\n", KMAG, KNRM);
  printf("%so1 n%s\tinserting n blocks, querying data structure for most common value ",
        KMAG, KNRM);
  printf("(taking into account sum of ASCII codes of randomly generated string) n times");
  printf(" and deleting blocks.\n");
  printf("%so2 n%s\tsequential insert n random values to data structure and then delete them.\n",
        KMAG, KNRM);
  printf("%so3 n%s\tinsert-delete operation n times.\n", KMAG, KNRM);
  printf("Allowed formating: %so1 30 o2 70 o3 45%s\n", KMAG, KNRM);
}
