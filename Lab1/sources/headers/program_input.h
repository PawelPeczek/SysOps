#ifndef PROGRAM_INPUT_H
#define PROGRAM_INPUT_H

typedef struct {
  int numOfElements;
  int blockSize;
  char allocMode;
  char** operations;
  int* operationsParams;
  int operationsNum;
} ProgramInput;

#endif
