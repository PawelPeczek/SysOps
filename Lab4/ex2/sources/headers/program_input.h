#pragma once

#include <stdbool.h>

typedef struct {
    int N;
    int K; 
    bool info_child_create;
    bool info_child_request;
    bool info_child_response;
    bool info_real_time_sig;
    bool info_close_child;
} ProgramInput;

void initialize_struct(ProgramInput* input);