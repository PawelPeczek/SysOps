#pragma once

#include <stdbool.h>

#define INPUT_ELEMS_REQUIRED 8

typedef struct {
    int prod_numb;
    int consuments_numb;
    int buff_size;
    char * file_name;
    int compare_value;
    bool (*compare_fun)(const int, const int);
    bool verbose;
    int nk;
} program_input;
