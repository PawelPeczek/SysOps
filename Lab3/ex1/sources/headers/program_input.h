#ifndef PROGRAM_INPUT_H
#define PROGRAM_INPUT_H

#include <stdbool.h>
#include <time.h>

typedef struct {
    const char* path;
    char cmp;
    size_t time;
    bool nftw;
} ProgramInput;

#endif