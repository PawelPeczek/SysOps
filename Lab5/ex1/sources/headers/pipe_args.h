#pragma once
#include <stdbool.h>

typedef struct {
    char** args;
    bool is_redirected;
} PipeArgs;
