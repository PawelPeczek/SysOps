#pragma once

#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdbool.h>
#include "../headers/program_input.h"

typedef struct {
    int buff_size;
    sem_t * semaphores;
    char ** buffer;
    sem_t prod_sem;
    int prod_idx;
    sem_t cons_sem;
    int cons_idx;
    sem_t fill_count_sem;
    sem_t empty_count_sem;
    FILE * source_file;
    sem_t all_source_read_sem;
    int all_source_read;
    bool verbose_mode; 
    bool (*compare_fun)(const int, const int);
    int compare_value;
    int consuments_number;
    int producers_number;
} threading_control_struct;

int solve_prod_cons_problem(const program_input * const input);