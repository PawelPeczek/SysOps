#pragma once

#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include "../headers/program_input.h"

typedef struct {
    int buff_size;
    pthread_mutex_t * mutexes;
    char ** buffer;
    pthread_mutex_t prod_mutex;
    int prod_idx;
    pthread_mutex_t cons_mutex;
    int cons_idx;
    pthread_mutex_t elems_mutex;
    int elems_in_buff;
    pthread_mutex_t buffer_empty_mutex;
    pthread_cond_t buffer_empty;
    pthread_mutex_t buffer_full_mutex;
    pthread_cond_t buffer_full; 
    FILE * source_file;
    int all_source_read;
    bool verbose_mode; 
    bool (*compare_fun)(const int, const int);
    int compare_value;
    int consuments_number;
    int producers_number; 
} threading_control_struct;

int solve_prod_cons_problem(const program_input * const input);