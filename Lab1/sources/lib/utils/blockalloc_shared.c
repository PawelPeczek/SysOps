#include <stdbool.h>
#include <stdlib.h>
#include "blockalloc_shared.h"
#include "../../headers/macros.h"
/*
* Functions of both allocation type area
*/

int _checkSizeArguments(size_t numOfElements, size_t blockSize){
  do {
    INVALID_SIZE(numOfElements);
    INVALID_SIZE(blockSize);
    return STATUS_OK;
  } while(true);
  return STATUS_ERROR;
}

int _sumOfASCIICodes(const char* value, size_t len){
  int accumulator = 0;
  if(value == NULL) return accumulator;
  for(int i = 0; i < len; i++){
    accumulator += (int)value[i];
  }
  return accumulator;
}

bool _numbersDistSmallerThanPrev(int ASCIISumOfInput, int ASCIISumOfBlock, int prevDist){
  bool result;
  if(ASCIISumOfBlock == 0) result = false;
  else result =_numDistance(ASCIISumOfInput, ASCIISumOfBlock) < prevDist;
  return result;
}

int _numDistance(int firstNum, int secondNum){
  return abs(firstNum - secondNum);
}
