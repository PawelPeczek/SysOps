#ifndef CONTRACT_H_
#define CONTRACT_H_

#include <stddef.h>

typedef struct {
  char* internalBlockTable;
  int* sumOfASCIIChars;
  size_t blockSize;
  size_t numOfElements;
} StaticLikeBlockTable;

typedef struct {
  char** internalBlockTable;
  int* sumOfASCIIChars;
  size_t blockSize;
  size_t numOfElements;
} DynamicBlockTable;

#endif
