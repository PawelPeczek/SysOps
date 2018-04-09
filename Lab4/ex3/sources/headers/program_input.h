#pragma once

#include <stdbool.h>

typedef enum {
    KILL_ASYNC = 1,
    KILL_SEQ = 2,
    RT = 3,
} Mode;

typedef struct {
    int L;
    Mode info_close_child;
} ProgramInput;
