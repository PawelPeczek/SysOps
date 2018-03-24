#ifndef PROGRAM_INPUT_H
#define PROGRAM_INPUT_H

enum FuncType {SYSTEM, LIBRARY};

enum Modes {SORT, GENERATE, COPY};

typedef struct {
    int funcTypes;
    int mode;
    const char* baseFile;
    const char* copyDestFile;
    int numOfRecords;
    int blockSize;
} ProgramInput;

#endif