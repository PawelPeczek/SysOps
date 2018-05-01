#pragma once

#include <stdbool.h>
#include "../headers/contract.h"

typedef struct {
    int elems[MAX_QUEUE_SIZE];
    int head;
    int tail;
    int size;
    int current_load;
    int instant_client;
    int waiting_counter;
    bool barber_sleeping;
    bool barber_checking;
} Queue;

int initialize_queue(Queue* q, int size);
int enqueue(Queue* q, int pid, int* ticket);
int dequeue(Queue* q, int* out);
void set_instant_client(Queue* q, int pid);